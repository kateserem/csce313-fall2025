/****************
LE2: Introduction to Unnamed Pipes
****************/
#include <unistd.h> // pipe, fork, dup2, execvp, close
#include <iostream> 


#include <sys/wait.h> //for wait

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
    pid_t pid1 = fork();

    if (pid1 == -1) {
        cerr << "fork failed\n";
        return 1;
    }


    if (pid1 == 0) {
        cout << "Hello from the CHILD process! \n";

        // In child, redirect output to write end of pipe
        dup2(p[1], STDOUT_FILENO); 
        
        // Close the read end of the pipe on the child side.
        close(p[0]); 

        // In child, execute the command
        execvp(cmd1[0], cmd1); 
        perror("execvp"); //if execvp fails

        close(p[1]); //closing the write end after duplicating it to stdout

        return 0;
    } else {
        cout << "Hello from the PARENT process! \n";
        wait(nullptr); //making parent pause until the child finishes 
    }


    // Create another child to run second command
    pid_t pid2 = fork();

        if (pid2 == -1) {
        cerr << "fork failed\n";
        return 1;
    }

    if (pid2 == 0) {
        // In child, redirect input to the read end of the pipe
        dup2(p[0], STDIN_FILENO); 
        // Close the write end of the pipe on the child side.
        close(p[1]); 

        // In child, execute the command
        execvp(cmd2[0], cmd2); 
        perror("execvp"); //if execvp fails

        close(p[0]); //closing the read end after duplicating it to stdin

        return 0;
    } else {
        wait(nullptr); //making parent pause until the child finishes 
    }

    close(p[0]);
    close(p[1]);

    // Reset the input and output file descriptors of the parent.
}
