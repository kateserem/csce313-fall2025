/*
	Original author of the starter code
    Tanzir Ahmed
    Department of Computer Science & Engineering
    Texas A&M University
    Date: 2/8/20
	
	Please include your Name, UIN, and the date below
	Name:
	UIN: 
	Date:
*/

//TASK 4: 
	//4.1: 15 points DONE
	//4.2 15 points
	//4.3 35 points
	//4.4 15 points
	//4.5 5 points DONE

#include "common.h"
#include "FIFORequestChannel.h"

using namespace std;


int main (int argc, char *argv[]) {
	int opt;
	int p = 1;
	double t = 0.0;
	int e = 1;
	
	string filename = "";
	while ((opt = getopt(argc, argv, "p:t:e:f:")) != -1) {
		switch (opt) {
			case 'p':
				p = atoi (optarg);
				break;
			case 't':
				t = atof (optarg);
				break;
			case 'e':
				e = atoi (optarg);
				break;
			case 'f':
				filename = optarg;
				break;
		}
	}

/////////TASK 4: Run server as a child process////////////
	pid_t child = fork();
	if (child == 0) {
		execl("./server", "server", (char *)nullptr);
		perror("execl ./server");
	}

//////////////////////////////////////////////////////////
    FIFORequestChannel chan("control", FIFORequestChannel::CLIENT_SIDE);
	
	// example data point request
    char buf[MAX_MESSAGE]; // 256
    datamsg x(1, 0.0, 1);
	
	memcpy(buf, &x, sizeof(datamsg));
	chan.cwrite(buf, sizeof(datamsg)); // question
	double reply;
	chan.cread(&reply, sizeof(double)); //answer
	cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << reply << endl;
	
    // sending a non-sense message, you need to change this
	filemsg fm(0, 0);
	string fname = "teslkansdlkjflasjdf.dat";
	
	int len = sizeof(filemsg) + (fname.size() + 1);
	char* buf2 = new char[len];
	memcpy(buf2, &fm, sizeof(filemsg));
	strcpy(buf2 + sizeof(filemsg), fname.c_str());
	chan.cwrite(buf2, len);  // I want the file length;

	delete[] buf2;
	
	// closing the channel    
    MESSAGE_TYPE m = QUIT_MSG;
    chan.cwrite(&m, sizeof(MESSAGE_TYPE));
}
