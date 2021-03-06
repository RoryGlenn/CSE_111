// $Id: commands.h,v 1.11 2016-01-14 14:45:21-08 - - $

// to run the program:
// ./yshell

// commands to run would include these:
// pwd
// cd ls
// ls

// to turn on debug flags and print flags associated with i
// yshell -@i

// how the graders use files
//    cd into the dot.score folder and run these commands
//       mk.build
//       mk.tests
//

#ifndef __COMMANDS_H__
#define __COMMANDS_H__

#include <unordered_map>
#include "file_sys.h"
#include "util.h"

using namespace std;

// A couple of convenient usings to avoid verbosity.

using command_fn = void (*)(inode_state &state, const wordvec &words);
using command_hash = unordered_map<string, command_fn>;

// command_error -
//    Extend runtime_error for throwing exceptions related to this
//    program.

class command_error : public runtime_error
{
public:
   explicit command_error(const string &what);
};

// execution functions -

void fn_cat(inode_state &state, const wordvec &words);
void fn_cd(inode_state &state, const wordvec &words);
void fn_echo(inode_state &state, const wordvec &words);
void fn_exit(inode_state &state, const wordvec &words);
void fn_ls(inode_state &state, const wordvec &words);
void fn_lsr(inode_state &state, const wordvec &words);
void fn_make(inode_state &state, const wordvec &words);
void fn_mkdir(inode_state &state, const wordvec &words);
void fn_prompt(inode_state &state, const wordvec &words);
void fn_pwd(inode_state &state, const wordvec &words);
void fn_rm(inode_state &state, const wordvec &words);
void fn_rmr(inode_state &state, const wordvec &words);

// in case the user enters '#' for a comment
void fn_hash(inode_state &, const wordvec &);

command_fn find_command_fn(const string &command);

// exit_status_message -
//    Prints an exit message and returns the exit status, as recorded
//    by any of the functions.

int exit_status_message();
class ysh_exit : public exception
{
};

// recursion functions
void rmr_recursion(inode_state &state, wordvec words);
vector<inode_ptr> lsr_recursion(inode_state &state, inode_ptr &ptr, vector<inode_ptr> &directories);

/////////////////////////////////////////////////////////
// utility functions
wordvec string_to_wordvec(string chop_me);
void print_directory(inode_state &state, inode_ptr &ptr);
void print_file(string &pathname);
bool is_number(string word);
int word_to_number(string word);
// inode_ptr parse_path(inode_ptr cwd, string path);

////////////////////////////////////////////////////////

#endif // __COMMANDS_H__
