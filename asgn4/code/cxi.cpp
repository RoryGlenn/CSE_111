// $Id: cxi.cpp,v 1.1 2020-11-22 16:51:43-08 - - $

// how to submit
// submit cse111-wm.w21 asg4 *.cpp *.h README Makefile

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <fstream>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "protocol.h"
#include "logstream.h"
#include "sockets.h"
using namespace std;

// how to submit
// submit cse111-wm.w21 asg4 *.cpp *.h README Makefile

logstream outlog(cout);


struct cxi_exit : public exception
{
};

unordered_map<string, cxi_command> command_map
{
   {"exit", cxi_command::EXIT}, // X
   {"get",  cxi_command::GET }, 
   {"help", cxi_command::HELP}, // X
   {"ls",   cxi_command::LS  }, // X
   {"put",  cxi_command::PUT },
   {"rm",   cxi_command::RM  }
};



vector<string> split_string(const string& line)
{
   std::stringstream ss(line);
   string            temp_str;
   vector<string>    filenames;
   while ( getline(ss, temp_str, ' ') ) 
   {
      filenames.push_back(temp_str);
   } 

   return filenames; 
}



static const char help[] = R"||(
exit         - Exit the program.  Equivalent to EOF.
get filename - Copy remote file to local host.
help         - Print help summary.
ls           - List names of files on remote server.
put filename - Copy local file to remote host.
rm filename  - Remove file from remote server.
)||";


// HELP
void cxi_help()
{
   cout << help;
}

// LS
void cxi_ls(client_socket &server)
{
   cxi_header header;
   header.command = cxi_command::LS;
   outlog << "sending header " << header << endl;
   
   send_packet(server, &header, sizeof header);
   recv_packet(server, &header, sizeof header);
   
   outlog << "received header " << header << endl;
   
   if (header.command != cxi_command::LSOUT)
   {
      outlog << "sent LS, server did not return LSOUT" << endl;
      outlog << "server returned " << header << endl;
   }
   else
   {
      size_t host_nbytes = ntohl(header.nbytes);
      auto buffer = make_unique<char[]>(host_nbytes + 1);
      recv_packet(server, buffer.get(), host_nbytes);
      outlog << "received " << host_nbytes << " bytes" << endl;
      buffer[host_nbytes] = '\0';
      cout << buffer.get();
   }
}


// GET
void cxi_get(client_socket &server, string& filename)
{

   cxi_header header;
   header.command = cxi_command::GET;   
   sprintf(header.filename, "%s", filename.c_str());
   
   outlog << "sending header " << header << endl;
   send_packet(server, &header, sizeof(header));
   recv_packet(server, &header, sizeof(header));
   outlog << "received header " << header << endl;

   if (header.command != cxi_command::FILEOUT)
   {
      cerr << "Error: No such file: " << filename << endl;
   }
   else
   {
      size_t   host_nbytes = ntohl(header.nbytes); 
      uint32_t file_size   = host_nbytes;
      char*    buffer_ptr  = new char[file_size];
      
      if (file_size != 0)
      {
         recv_packet(server, buffer_ptr, file_size);
      }
      
      cout << "received " << file_size << " bytes" << endl;

      ofstream outfile(filename, ofstream::out | ofstream::trunc);
      
      if (outfile.fail())
      {
         cerr << "Error: " << filename 
         << ": error no: " << strerror(errno) << endl;         
      }
      else
      {
         outfile.write(buffer_ptr, file_size);
      }
      
      outfile.close();
      
      delete buffer_ptr;
   }

}


// PUT
void cxi_put(client_socket &server, string& filename)
{

   ifstream   inputfile(filename);
   cxi_header header;

   if (inputfile.fail())
   {
      cerr << "Error: " << filename << ": error no: "
         << strerror(errno) << endl;
   }
   else
   {
      struct stat statbuff;
      if (stat(filename.c_str(), &statbuff) != 0)
      {
         cerr << "Error: " << filename << ": error no: "
            << strerror(errno) << endl;
      }
      else
      {
         header.command = cxi_command::PUT;
         sprintf(header.filename, "%s", filename.c_str());

         // the file exists and is valid
         uint32_t file_size = statbuff.st_size;
         header.nbytes      = htonl(file_size);

         // create buffer pointer and null terminate it
         char* buffer_ptr      = new char[file_size+1];
         buffer_ptr[file_size] = '\0';

         // read from file and store in buffer_ptr
         inputfile.read(buffer_ptr, file_size);
         inputfile.close();

         outlog << "sending header " << header << endl;
         send_packet(server, &header, sizeof(header));

         send_packet(server, buffer_ptr, file_size);
         recv_packet(server, &header, sizeof(header));

         if (header.command == cxi_command::NAK)
         {
            cerr << "Error: " << header.filename << ": error no: "
               << strerror(ntohl(header.nbytes)) << endl;
         }
         else
         {
            cout << "received header " << header << endl;   
         }
                  
         delete buffer_ptr;
      }

   }

}


// RM
void cxi_rm(client_socket &server, string& filename)
{
   cxi_header header;
   header.command = cxi_command::RM;
   strcpy(header.filename, filename.c_str());

   outlog << "sending header " << header << endl;
   send_packet(server, &header, sizeof(header));
   recv_packet(server, &header, sizeof(header));
   outlog << "received header " << header << endl;

   if (header.command == cxi_command::ACK)
   {
      cout << "file " << filename << " removed successfully" << endl;
   }
   else
   {
      cerr << filename << ": No such file exists"  << endl;
   }

}


void usage()
{
   cerr << "Usage: " << outlog.execname() << " [host] [port]" << endl;
   throw cxi_exit();
}


int main(int argc, char **argv)
{

   outlog.execname(basename(argv[0]));
   outlog << "starting" << endl;

   vector<string> args(&argv[1], &argv[argc]);
   
   if (args.size() > 2)
   {
      usage();
   }
   
   string    host = get_cxi_server_host(args, 0);
   in_port_t port = get_cxi_server_port(args, 1);
   
   outlog << to_string(hostinfo()) << endl;
   
   try
   {
      
      outlog << "connecting to " << host << " port " << port << endl;
      client_socket server(host, port);
      outlog << "connected to " << to_string(server) << endl;
      
      // after we connect to the server
      while (true)
      {
         
         string line;
         getline(cin, line);
         vector<string> filenames = split_string(line);

         if ( cin.eof() ) { throw cxi_exit(); }
         
         // if nothing was entered on the command line
         if (line == "") { continue; }

         outlog << "command " << line << endl;

         const auto &itor = command_map.find(filenames[0]);
         cxi_command cmd  = itor == command_map.end() 
                     ? cxi_command::ERROR : itor->second;
         
         switch (cmd)
         {

            case cxi_command::EXIT:
               throw cxi_exit();
               break;
            case cxi_command::HELP:
               cxi_help();
               break;
            case cxi_command::LS:
               cxi_ls(server);
               break;
            case cxi_command::GET:
               if (!filenames.empty())
                  cxi_get(server, filenames.back());
               break;
            case cxi_command::PUT:
               if (!filenames.empty())
                  cxi_put(server, filenames.back());
               break;
            case cxi_command::RM:
               if (!filenames.empty())
                  cxi_rm(server, filenames.back());            
               break;
            default:
               outlog << line << ": invalid command" << endl;
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
   return 0;

}
