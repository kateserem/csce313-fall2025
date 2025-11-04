#include <iostream>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <vector>
#include <string>

#include "Tokenizer.h"

#include <fcntl.h> //for open()
#include <ctime>

// all the basic colours for a shell prompt
#define RED     "\033[1;31m"
#define GREEN	"\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE	"\033[1;34m"
#define WHITE	"\033[1;37m"
#define NC      "\033[0m"

using namespace std;

int main () {
    string prev_directory; //storing previous directories
    char current_directory[4096]; //
    for (;;) { //loop
        /////////////////task 4.2.9 user prompt//////////////////////////

        time_t now = time(NULL);  //getting current time                         
        struct tm *time_now = localtime(&now); //converting it into local time               
        char month[64];                                   
        strftime(month, sizeof(month), "%b %d %H:%M:%S", time_now); //date/time format

        getcwd(current_directory, sizeof(current_directory)); //getting current direcotry path

        const char* user = getenv("USER");//get user       
        if (!user) user = "user";                          
        cout << month << " " << user << ":" << current_directory << "$ ";
/////////////////////////////////////////////////////////////////////////

        // get user inputted command
        string input;
        getline(cin, input);

        if (input == "exit") {  // print exit message and break out of infinite loop
            cout << RED << "Now exiting shell..." << endl << "Goodbye" << NC << endl;
            break;
        }

        // get tokenized commands from user input
        Tokenizer tknr(input); 
        if (tknr.hasError()) {  // continue to next prompt if input had an error
            continue;
        }

        // // print out every command token-by-token on individual lines
        // // prints to cerr to avoid influencing autograder
        // for (auto cmd : tknr.commands) {
        //     for (auto str : cmd->args) {
        //         cerr << "|" << str << "| ";
        //     }
        //     if (cmd->hasInput()) {
        //         cerr << "in< " << cmd->in_file << " ";
        //     }
        //     if (cmd->hasOutput()) {
        //         cerr << "out> " << cmd->out_file << " ";
        //     }
        //     cerr << endl;
        // }

        //how many commands are there?
        int num_commands = tknr.commands.size();

        Command* first = tknr.commands.at(0); //get first command
        

        if (num_commands == 1 && !first->args.empty() &&first->args[0] == "cd") {                       
  
            getcwd(current_directory, sizeof(current_directory));//get the current direcotry          
            string current = current_directory;                          

            if (first->args[1] == "-") { //if cd -  
                if (!prev_directory.empty()) { //ensure we saved the prev direcotry
                    if (chdir(prev_directory.c_str()) < 0) {//attempt to change direcotries
                        perror("chdir"); //error occured
                    }
                    else {
                        prev_directory = current;       
                    }            
                }
            } else {  //if cd path             
                if (chdir(first->args[1].c_str()) < 0) { //attempt to go to that folder
                    perror("chdir"); //error occured
                }
                else {
                    prev_directory = current; 
                } 
               
            }
                                   
        }

        if (num_commands ==1) { //if only one command
            pid_t pid = fork(); //create new process by forking

            if (pid == 0) {  // if child, exec to run command
                // run single commands with no arguments JUST AN EXAMPLE
                // char* args[] = {(char*) tknr.commands.at(0)->args.at(0).c_str(), nullptr};

                // if (execvp(args[0], args) < 0) {  // error check
                //     perror("execvp");
                //     exit(2);
                // }
    ////////////////   I/O redirection   ///////////////////////////////////////
                Command* cmd0 = tknr.commands.at(0); //getting first command

                if (cmd0 -> hasInput()) { //if < for reading

                    int fd_in = open(cmd0 -> in_file.c_str(), O_RDONLY); //open the input file

                    if (dup2(fd_in, STDIN_FILENO) <0) { //replace the default input with the file
                        perror("dup2(stdin)");
                        close(fd_in);
                    }
                    close(fd_in); //close it
                }

                if (cmd0 -> hasOutput()) { //if > for writing
                    int fd_out = open(cmd0->out_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644); //replace default output with the file

                    if (dup2(fd_out, STDOUT_FILENO) < 0) {
                        perror("dup2(stdout)"); 
                        close(fd_out);
                    }
                    close(fd_out); //close it

                }
    ////////////////////////////////////////////////////////////////////////////

    ////////////////for buuild argv and execvp/////////////////////////////
                int num_args = cmd0->args.size(); //how many arguments
                char** argv = new char*[num_args + 1]; //create array of strings 

                //copying each arg from tokenizer into arg
                for (int i=0; i<num_args; i++) {
                    argv[i] = (char*)tknr.commands.at(0) -> args[i].c_str();
                }

                argv[num_args] = NULL;
                execvp(argv[0], argv); //run it

                //if it fails then error
                perror("execvp");
                delete[] argv;
    ///////////////////////////////////////////////////////////////////////
            }
            else {  // if parent, wait for child to finish
                int status = 0;
                waitpid(pid, &status, 0);
                if (status > 1) { // exit if child didnt exec properly
                    exit(status);
                }
            }
            continue;
        }

    ///////////////////task 4.1 "getting started"///////////////////
        int in_fd = dup(STDIN_FILENO); //copy of defualt input
        vector<pid_t> pids;   // store PIDs of all children

        for (int i = 0; i < num_commands; i++) {
            int pipefd[2];// array to hold 
            bool last = (i == num_commands - 1); //check if last command

            //create pipe if its not the last one
            if (!last) {
                if (pipe(pipefd) < 0) { 
                    perror("pipe");
                }
            }

            pid_t cpid = fork(); //create child process

            if (cpid == 0) { //if first command has the input redirection
                Command* cmd = tknr.commands.at(i);

                //for input
                if (i == 0 && cmd->hasInput()) {
                    int fd_in = open(cmd->in_file.c_str(), O_RDONLY);  //open input file
                    dup2(fd_in, STDIN_FILENO);  //replace input defualt with file input
                    close(fd_in); //close it
                } else {
                    dup2(in_fd, STDIN_FILENO); //use  prev pipe for input
                }

                //for output
                if (!last) { //if its not the last one
                    dup2(pipefd[1], STDOUT_FILENO); //and has ouptu redirection
                } else if (cmd->hasOutput()) {
                    int fd_out = open(cmd->out_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    dup2(fd_out, STDOUT_FILENO); //replace default out with file output
                    close(fd_out); //close it
                }
                
                if (!last) { //if its not th elast one close the pipes
                    close(pipefd[0]); 
                    close(pipefd[1]);
                }

                // build argv and execvp
                int argc_i = cmd->args.size(); //how many args
                if (argc_i == 0) _exit(0); //if no more args thene dxit child

                char** argv_i = new char*[argc_i + 1]; //creating arg array
                for (int k = 0; k < argc_i; k++) {
                    argv_i[k] = (char*)cmd->args[k].c_str(); //copying the arguments
                }
                argv_i[argc_i] = NULL; //return back to null

                execvp(argv_i[0], argv_i); //run it
                perror("execvp"); //if fails
                delete[] argv_i; //clean up alloc mem
               
            }

            // parent process
            pids.push_back(cpid); //save child pid for later
            close(in_fd);

            if (!last) { //if its not the last one
                in_fd = pipefd[0]; //read from this pipe
                close(pipefd[1]); //close write end
            
            }
        }

        close(in_fd); //close it

        int st = 0;
        for (int i = 0; i < (int)pids.size(); i++) {
            waitpid(pids[i], &st, 0); 
        }

    }

}
