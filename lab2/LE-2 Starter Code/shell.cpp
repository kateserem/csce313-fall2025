/****************
LE2: Introduction to Unnamed Pipes
****************/
#include <unistd.h> // pipe, fork, dup2, execvp, close
#include <iostream> 

using namespace std;

int main () {
    // lists all the files in the root directory in the long format
    char* cmd1[] = {(char*) "ls", (char*) "-al", (char*) "/", nullptr}; // command 1 (for execvp child 1)
    // translates all input from lowercase to uppercase
    char* cmd2[] = {(char*) "tr", (char*) "a-z", (char*) "A-Z", nullptr}; //command 2 (for execvp child 2)
    
    // TODO: add functionality
    // Create pipe
    int p[2];

    //  p[0] read end
    //  p[1] write end

    if(pipe(p) == -1) {
        perror("pipe"); //failed pipe creation
        return 1;
    }

    // Create child to run first command
    pid_t pid1 = fork();

    if (pid1 == -1) {
        cerr << "fork failed\n"; //failed to create first child
        return 1;
    }


    if (pid1 == 0) { // if child (created when forked)


        // In child, redirect output to write end of pipe
        dup2(p[1], STDOUT_FILENO); 

        // Close the read end of the pipe on the child side.
        close(p[0]); 

        // In child, execute the command 
        execvp(cmd1[0], cmd1); //cmd1 provided at beginning of main

        close(p[1]); //closing the write end 

        return 0;
    }


    // Create another child to run second command
    pid_t pid2 = fork();

        if (pid2 == -1) {
        cerr << "fork failed\n"; //failed to create sec child
        return 1;
    }

    if (pid2 == 0) { // if child (also created when forked)

        // In child, redirect input to the read end of the pipe
        dup2(p[0], STDIN_FILENO); //found in class lecture notes
        // Close the write end of the pipe on the child side.
        close(p[1]); 

        // Execute the second command.
        execvp(cmd2[0], cmd2); // cmd2 provided at beginning of main

        close(p[0]); //closing the read end

        return 0;
    } 

    // Close the pipe in the parent process
    close(p[0]); //read end
    close(p[1]); //write end

    // Reset the input and output file descriptors of the parent.
    //???? passed without it? why?
}
