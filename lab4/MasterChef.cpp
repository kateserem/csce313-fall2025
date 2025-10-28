#include <getopt.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <string>
#include "StepList.h"

using namespace std;

StepList* recipeSteps; //pointer for all the recipe steps
vector<int>* completedSteps; // list of the completed step's id
int completeCount = 0; //how many steps have been completed

void PrintHelp() // Given
{

	cout << "Usage: ./MasterChef -i <file>\n\n";
	cout<<"--------------------------------------------------------------------------\n";
	cout<<"<file>:    "<<"csv file with Step, Dependencies, Time (m), Description\n";
	cout<<"--------------------------------------------------------------------------\n";
	exit(1);
}

string ProcessArgs(int argc, char** argv) // Given: reads the command line input
{
	string result = "";  //storing filename
	// print help if odd number of args are provided
	if (argc < 3) { //if not enough arguments
		PrintHelp(); 
	}

	while (true)
	{
		const auto opt = getopt(argc, argv, "i:h"); //read flags

		if (-1 == opt) //nore more to read
			break; 

		switch (opt)
		{
		case 'i': 
			result = std::string(optarg); //save filename
			break;
		case 'h': // -h or --help  
		default:
			PrintHelp();
			break;
		}
	}

	return result;
}

// Creates and starts a timer given a pointer to the step to start and when it will expire in seconds.
void makeTimer( Step *timerID, int expire) // Given: sets and starts timer
{
    struct sigevent te;
    struct itimerspec its;

    /* Set and enable alarm */
    te.sigev_notify = SIGEV_SIGNAL;  //sending a signal when done
    te.sigev_signo = SIGRTMIN;	//signal type
    te.sigev_value.sival_ptr = timerID; //attacking pointer to step
    timer_create(CLOCK_REALTIME, &te, &(timerID->t_id));

    its.it_interval.tv_sec = 0;  //dont repreat
    its.it_interval.tv_nsec = 0;
    its.it_value.tv_sec = expire; 
    its.it_value.tv_nsec = 0;
    timer_settime(timerID->t_id, 0, &its, NULL); //starting the timer
}

// Triggers when the time for the step has elapsed.
// the siginfo_t* pointer holds the step information in the si_value.sival_ptr
// You will need to print out that the step has completed
// Mark the step to be removed as a dependency, and trigger the remove dep handler.
// TO COMPLETE Section 2
static void timerHandler( int sig, siginfo_t *si, void *uc ) //when timer ends then run this
{
	// Retrieve timer pointer from the si->si_value
    Step* comp_item = (Step*)si->si_value.sival_ptr;

	/* TODO This Section - 2 */
	// Officially complete the step using completedSteps and completeCount
	comp_item -> PrintComplete(); //showing when tis done
	completedSteps -> push_back(comp_item -> id); //add to the list
	completeCount++; //increment for tracking

	// Ready to remove that dependency, call the trigger for the appropriate handler
	raise(SIGUSR1); 
	/* End Section - 2 */
}

// Removes the copmleted steps from the dependency list of the step list.
// Utilize the completedSteps vector and the RemoveDependency method.
// To Complete - Section 3
void RemoveDepHandler(int sig) { //removes the completed steps from the dependency lists
	/* TODO This Section - 3 */
	// Foreach step that has been completed since last run, remove it as a dependency
	for (size_t i=0; i < completedSteps -> size(); i++) { // interate through the done steps
		int id = (*completedSteps)[i];  //get its ID
		recipeSteps -> RemoveDependency(id); //remove dependency
	}

	completedSteps -> clear(); //clear list
	/* End Section - 3 */
}

// Associate the signals to the signal handlers as appropriate
// Continuously check what steps are ready to be run, and start timers for them with makeTimer()
// run until all steps are done.
// To Complete - Section 1
int main(int argc, char **argv)
{
	string input_file = ProcessArgs(argc, argv); 
	if(input_file.empty()) {
		exit(1);
	}
	
	// Initialize global variables
	completedSteps = new vector<int>(); //making the list
	recipeSteps = new StepList(input_file); //load steps

    /* Set up signal handler. */
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = timerHandler;
    sigemptyset(&sa.sa_mask);

	/* TODO This Section - 1 */
	// Associate the signal SIGRTMIN with the sa using the sigaction function
	if (sigaction(SIGRTMIN, &sa, NULL) == -1) {
		perror("sigaction(SIGRTMIN)"); 
	}
	// Associate the appropriate handler with the SIGUSR1 signal, for removing dependencies
	if (signal(SIGUSR1, RemoveDepHandler) == SIG_ERR) {
		perror("signal(SIGUSR1)");
		exit(1);
	}

	// Until all steps have been completed, check if steps are ready to be run and create a timer for them if so
	while (completeCount < recipeSteps -> Count()) {
		vector<Step*> ready;
		ready = recipeSteps -> GetReadySteps();

		//starting timers for any ready steps that are not running
		for (int i=0; i < (int)ready.size(); i++) { 
			Step* s = ready[i];  //current step
			if (!s -> running) { //if it is not running
				s -> running = true; //make it run
				makeTimer(s, s -> duration); //start the timer
			}
		}
	}

	/* End Section - 1 */

	cout << "Enjoy!" << endl;
}