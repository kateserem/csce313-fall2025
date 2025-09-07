/****************
LE2: Introduction to Unnamed Pipes
****************/
#include <unistd.h> // pipe, fork, dup2, execvp, close
#include <iostream> 


#include <sys/wait.h>

using namespace std;

int main () {
    // lists all the files in the root directory in the long format
    char* cmd1[] = {(char*) "ls", (char*) "-al", (char*) "/", nullptr};
    // translates all input from lowercase to uppercase
    char* cmd2[] = {(char*) "tr", (char*) "a-z", (char*) "A-Z", nullptr};

    cout << "=== DEMO: fork() ===" << endl;
    
    // TODO: add functionality
    // Create pipe
    int p[2];

    if(pipe(p) == -1) {
        perror("pipe");
        return 1;
    }

    // Create child to run first command
    pid_t pid = fork();

    if (pid == -1) {
        cerr << "fork failed\n";
        return 1;
    }


    if (pid == 0) {
        // In child, redirect output to write end of pipe
        cout << "Hello from the CHILD process! \n";

        // close(p[0]); //close the read end 
        
        return 0;
    } else {
        cout << "Hello from the PARENT process! \n";
        wait(nullptr); //making parent pause until the child finishes 
    }


    // Close the read end of the pipe on the child side.
    // In child, execute the command

    // Create another child to run second command
    // In child, redirect input to the read end of the pipe
    // Close the write end of the pipe on the child side.
    // Execute the second command.

    // Reset the input and output file descriptors of the parent.
}
