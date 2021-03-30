// $Id: cxid.cpp,v 1.3 2020-12-12 22:09:29-08 - - $

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <fstream>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <memory> 

#include "protocol.h"
#include "logstream.h"
#include "sockets.h"
using namespace std;

// how to submit
// submit cse111-wm.w21 asg4 *.cpp *.h README Makefile

logstream outlog(cout);

struct cxi_exit : public exception { };


void reply_ls(accepted_socket &client_sock, cxi_header &header)
{
   const char *ls_cmd = "ls -l 2>&1";
   FILE *ls_pipe = popen(ls_cmd, "r");

   if (ls_pipe == NULL)
   {
      outlog << ls_cmd << ": " << strerror(errno) << endl;
      header.command = cxi_command::NAK;
      header.nbytes = htonl(errno);
      send_packet(client_sock, &header, sizeof header);
      return;
   }

   string ls_output;
   char buffer[0x1000];

   while (true)
   {
      char *rc = fgets(buffer, sizeof buffer, ls_pipe);
      
      if (rc == nullptr)
      {
         break;
      }
         
      ls_output.append(buffer);
   }

   int status = pclose(ls_pipe);

   if (status < 0)
   {
      outlog << ls_cmd << ": " << strerror(errno) << endl;
   }
   else
   {
      outlog << ls_cmd << ": exit " << (status >> 8)
             << " signal " << (status & 0x7F)
             << " core " << (status >> 7 & 1) << endl;
   }


   header.command = cxi_command::LSOUT;
   header.nbytes  = htonl(ls_output.size());
   memset(header.filename, 0, FILENAME_SIZE);
   outlog << "sending header " << header << endl;
   send_packet(client_sock, &header, sizeof header);
   send_packet(client_sock, ls_output.c_str(), ls_output.size());
   outlog << "sent " << ls_output.size() << " bytes" << endl;
}


// GET
void reply_get(accepted_socket &client_sock, cxi_header &header)
{

   string   filename = header.filename;
   ifstream inputfile (filename);
   
   if (inputfile.fail())
   {
      header.command = cxi_command::NAK;
      header.nbytes  = htonl(errno);
      memset(header.filename, 0, FILENAME_SIZE);
      send_packet(client_sock, &header, sizeof(header));
   }
   else
   {
      struct stat statbuff;

      if (stat(filename.c_str(), &statbuff) != 0)
      {
         header.command = cxi_command::NAK;
         header.nbytes  = htonl(errno);
         memset(header.filename, 0, sizeof(header.filename));
         send_packet(client_sock, &header, sizeof(header));
      }
      else
      {
         uint32_t file_size = statbuff.st_size;

         // set header
         header.command = cxi_command::FILEOUT;
         header.nbytes  = htonl(statbuff.st_size);

         // create buffer pointer and null terminate it
         char* buffer_ptr      = new char[file_size + 1];
         buffer_ptr[file_size] = '\0';

         // read from file and store in buffer_ptr
         inputfile.read(buffer_ptr, file_size);
         inputfile.close();

         outlog << "sending header " << header << endl;
         send_packet(client_sock, &header,     sizeof(header));
         send_packet(client_sock,  buffer_ptr, statbuff.st_size);
         outlog << "sent " << statbuff.st_size << " bytes" << endl;
         
         memset (header.filename, 0, FILENAME_SIZE);
         delete buffer_ptr;
      }

   }

}


// PUT
void reply_put(accepted_socket& client_sock, cxi_header& header)
{
   ofstream outfile;
   uint32_t file_size  = ntohl(header.nbytes);
   char*    buffer_ptr = new char[file_size];
   
   if (file_size != 0)
   {
      recv_packet(client_sock, buffer_ptr, file_size);
   }

   outlog << "received " << file_size << " bytes" << endl;

   outfile.open(header.filename,
      std::ios_base::out | std::ios_base::trunc);

   
   if (outfile.fail())
   {
      cerr << strerror(errno) << endl;
      header.command = cxi_command::NAK;
      header.nbytes  = htonl(errno);
   }
   else
   {
      outfile.write(buffer_ptr, file_size);
      
      header.command = cxi_command::ACK;
      header.nbytes  = htonl(file_size);
      memset (header.filename, 0, FILENAME_SIZE);      
   }
   outfile.close();
   cout << "sending header " << header << endl;
   send_packet(client_sock, &header, sizeof(header));
   delete buffer_ptr;

}


void reply_rm(accepted_socket& client_sock, cxi_header& header)
{
   string filename = header.filename;

   if (unlink(filename.c_str()) == 0)
   {
      // success
      header.command = cxi_command::ACK;
      header.nbytes = htonl(0);
   }
   else
   {
      // failure
      header.command = cxi_command::NAK;
      header.nbytes = htonl(errno);
   }

   memset (header.filename, 0, FILENAME_SIZE);

   cout << "sending header " << header << endl;
   send_packet(client_sock, &header, sizeof(header));
}


void run_server(accepted_socket &client_sock)
{
   outlog.execname(outlog.execname() + "*");
   outlog << "connected to " << to_string(client_sock) << endl;
   try
   {
      while (true)
      {
         
         cxi_header header;
         recv_packet(client_sock, &header, sizeof header);
         outlog << "received header " << header << endl;
         
         switch (header.command)
         {
            case cxi_command::LS:
               reply_ls(client_sock, header);
               break;
            case cxi_command::GET:
               reply_get(client_sock, header);
               break;
            case cxi_command::PUT:
               reply_put(client_sock, header);
               break;
            case cxi_command::RM:
               reply_rm(client_sock, header);
               break;            
            default:
               outlog << "invalid client header:" << header << endl;
               break;
         }

      }
   }
   catch (socket_error &error)
   {
      outlog << error.what() << endl;
   }
   catch (cxi_exit &error)
   {
      outlog << "caught cxi_exit" << endl;
   }
   outlog << "finishing" << endl;
   throw cxi_exit();
}


void fork_cxiserver(server_socket &server, accepted_socket &accept)
{
   pid_t pid = fork();
   if (pid == 0)
   { 
      // child
      server.close();
      run_server(accept);
      throw cxi_exit();
   }
   else
   {
      accept.close();
      if (pid < 0)
      {
         outlog << "fork failed: " << strerror(errno) << endl;
      }
      else
      {
         outlog << "forked cxiserver pid " << pid << endl;
      }
   }
}


void reap_zombies()
{
   for (;;)
   {
      int status;
      pid_t child = waitpid(-1, &status, WNOHANG);
      if (child <= 0)
         break;
      outlog << "child " << child
             << " exit " << (status >> 8)
             << " signal " << (status & 0x7F)
             << " core " << (status >> 7 & 1) << endl;
   }
}


void signal_handler(int signal)
{
   outlog << "signal_handler: caught " << strsignal(signal) << endl;
   reap_zombies();
}


void signal_action(int signal, void (*handler)(int))
{
   struct sigaction action;
   action.sa_handler = handler;
   sigfillset(&action.sa_mask);
   action.sa_flags = 0;
   int rc = sigaction(signal, &action, nullptr);
   if (rc < 0)
      outlog << "sigaction " << strsignal(signal)
             << " failed: " << strerror(errno) << endl;
}


int main(int argc, char **argv)
{
   
   outlog.execname(basename(argv[0]));
   outlog << "starting" << endl;
   
   vector<string> args(&argv[1], &argv[argc]);
   signal_action(SIGCHLD, signal_handler);
   in_port_t port = get_cxi_server_port(args, 0);
   
   try
   {
      
      server_socket listener(port);
      
      while(true)
      {
         
         outlog << to_string(hostinfo()) << 
         " accepting port " << to_string(port) << endl;
         accepted_socket client_sock;
         
         while(true)
         {
            try
            {
               listener.accept(client_sock);
               break;
            }
            catch (socket_sys_error &error)
            {
               switch (error.sys_errno)
               {
               case EINTR:
                  outlog << "listener.accept caught " 
                  << strerror(EINTR) << endl;
                  break;
               default:
                  throw;
               }
            }

         }
         
         outlog << "accepted " << to_string(client_sock) << endl;
         
         try
         {
            fork_cxiserver(listener, client_sock);
            reap_zombies();
         }
         catch (socket_error &error)
         {
            outlog << error.what() << endl;
         }

      }
   }
   catch (socket_error &error)
   {
      outlog << error.what() << endl;
   }
   catch (cxi_exit &error)
   {
      outlog << "caught cxi_exit" << endl;
   }
   outlog << "finishing" << endl;
   return 0;
}
