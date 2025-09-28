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
	int p = 1;  //patient num	
	double t = 0.0;  //time (every 4ms)
	int e = 1;  //ecg num	(each patient have 2)

	bool has_t = false;

	string filename = "";
	while ((opt = getopt(argc, argv, "p:t:e:f:")) != -1) {
		switch (opt) {
			case 'p':
				p = atoi (optarg);
				break;
			case 't':
				t = atof (optarg);
				has_t = true;
				break;
			case 'e':
				e = atoi (optarg);
				break;
			case 'f':
				filename = optarg;
				break;

		}
	}

/////////TASK 4.1 : Run server as a child process////////////
	pid_t child = fork();

	if (child == 0) {
		//	./server is the path to the server exec file
		//	"server" is the name of the exec file
		execl("./server","server", (char *)nullptr); 
	}
//////////////////////////////////////////////////////////

////////////TASK 4.2: Requesting data points/////////////////////////////
	// you can write your code here

	FIFORequestChannel chan("control", FIFORequestChannel::CLIENT_SIDE);
	char buf[MAX_MESSAGE]; // 256

	if (has_t && e > 0) {
		datamsg x(p, t, e);
		memcpy(buf, &x, sizeof(datamsg));
		chan.cwrite(buf, sizeof(datamsg));
		double reply;
		chan.cread(&reply, sizeof(double));
		cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << reply << endl;
	} else if (filename.empty()) {	

		string path = "received/x" + to_string(p) + ".csv";
		FILE* fp = fopen(path.c_str(), "w");

		for (int i=0; i <1000; i++) {//1000 data points

			double data_time = (i*4) / 1000.0; //every 4ms

			// e1
			datamsg e1(p, data_time, 1);
			memcpy(buf, &e1, sizeof(datamsg));
			chan.cwrite(buf, sizeof(datamsg));
			double reply1;
			chan.cread(&reply1, sizeof(double));
			// cout << "For person " << p << ", at time " << data_time << ", the value of ecg 1 is " << reply1 << endl;

			//e2
			datamsg e2(p, data_time, 2);
			memcpy(buf, &e2, sizeof(datamsg));
			chan.cwrite(buf, sizeof(datamsg));
			double reply2;
			chan.cread(&reply2, sizeof(double));
			// cout << "For person " << p << ", at time " << data_time << ", the value of ecg 2 is " << reply2 << endl;

			fprintf(fp, "%g,%g,%g\n", data_time, reply1, reply2);
		}
		fclose(fp);
	}

/////////////////////////////////////////////////////////////////////////////


    // // sending a non-sense message, you need to change this
	// filemsg fm(0, 0);
	// string fname = "teslkansdlkjflasjdf.dat";
	
	// int len = sizeof(filemsg) + (fname.size() + 1);
	// char* buf2 = new char[len];
	// memcpy(buf2, &fm, sizeof(filemsg));
	// strcpy(buf2 + sizeof(filemsg), fname.c_str());
	// chan.cwrite(buf2, len);  // I want the file length;

	// delete[] buf2;
	

/////////////4.3: Requesting files////////////////////////////////////////
	if (!filename.empty()) {

		//send a file message to get the file lenght
		filemsg fm(0, 0);  //send me the total file size
		string fname = filename;  //copy filename to a string object
		int len = sizeof(filemsg) + (fname.size() + 1); // total size of request buffer
		char* buf2 = new char[len]; // allocate a buffer of that size
		memcpy(buf2, &fm, sizeof(filemsg)); // copy the filemsg struct
		strcpy(buf2 + sizeof(filemsg), fname.c_str()); // append the filename string
		chan.cwrite(buf2, len);  // I want the file length;

		
		__int64_t filesize; //hold the file size of the servers reply
    	chan.cread(&filesize, sizeof(__int64_t));  //read the file size from the server into filesize variable
		

		//put the received file under "received" with the same name
		string outpath = string("received/") + fname;
		FILE* fp = fopen(outpath.c_str(), "wb"); //open for writing 

		// send a series of messages and write junks of data to the output file
		char databuf[MAX_MESSAGE];  //buffer to hold the data received from the server
		__int64_t offset = 0; //offset to keep track of where we are inthe file
		while (offset < filesize) { //keep requesting until we get the whole file

			int chunk; //how many bytes to request
			if ((filesize - offset) < MAX_MESSAGE) { //if the remaining bytes are less than 256
				chunk = (int)(filesize - offset); //request only the remaining bytes
			} else { //request MAX_MESSAGE bytes
				chunk = MAX_MESSAGE; //256
			}

			fm.offset = offset; //update the offset
			fm.length  = chunk; //update the length
			memcpy(buf2, &fm, sizeof(filemsg)); // copy the filemsg struct     

			chan.cwrite(buf2, len); // request that chunk                   
			int n = chan.cread(databuf, chunk); // read the chunk from the server    
			fwrite(databuf, 1, (size_t)n, fp);  // write to the output file      

			offset += n; //increment the offset by the number of bytes read from the server
		}


		fclose(fp); //close the output file
		delete[] buf2;
	}

////////////////////////////////////////////////////////////////////////////
	// closing the channel    
    MESSAGE_TYPE m = QUIT_MSG;
    chan.cwrite(&m, sizeof(MESSAGE_TYPE));

}