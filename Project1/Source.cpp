#include <iostream>
#include <sstream>
#include <string>

#include <fstream> 
#include <stdlib.h>
#include <queue>
#include <time.h>


using namespace std;

int systime = 0;

int get_process_count()
{
	int requested_count;
	cout << "How many processes would you like to generate?\n";
	cin >> requested_count;
	return(requested_count);
}

class Process {
public:
	
	queue <char> instructions;
	string name;


	Process(string name = "process") {
		this->name = name;
	}

	int getInstuctionCount() {
		return(instructions.size());
	}

	char getNextInstruction() {
		if (!instructions.empty()) {
			return(instructions.front());
		}
		// !!!! process is out of instructions
		else {
			return ('X');
		}
		
	}

	//!!! is char for now maybe should be string
	void spawnInstructions(char type) {
		if (type == 'C' || type == 'I') {
			int number = random();
			for (int i = 0; i < number; i++) {
				instructions.push(type);
			}
		} 
		else
		{
			cout << "BAD INSTRUCTION TYPE";
		}
	}


	// This defaults as a random binomial number generator
	int random(int average = 5, int deviation = 2) {
		int loops = average * deviation;
		int total = 0;

		for (int i = 0; i < loops; i++) {
			total += ((rand() % deviation)/(deviation-1));
		}

		return(total+1);
	}

	void calculate(int i = 5, int j = 30) {
		if (this->getNextInstruction() == 'C') {
			instructions.pop();
			int k = i * j;
		}
		
	}
};

class pcb : public Process {
	// p state, progression, time left

	//Process *myprocess;
	int pid;
	string status;
	int executed_cycles = 0;
	int ttk = this->getInstuctionCount();  // Time to Kill

	using Process::Process;
	
	string getStatus() {
		return(status);
	}
public: int get_executed_cycles() {
	return(executed_cycles);
	}

public : void update_ttk() {
		ttk = this->getInstuctionCount();
	}

	void update_executed_cycles(int add) {
		executed_cycles = executed_cycles + add;
		systime = systime + add;
	}

public : void setStatus(string newstatus) {
		if (newstatus == "terminated") {
			status = "terminated";
		}
		else if (newstatus == "wait") {
			status = "wait";
		}
		else if (newstatus == "ready") {
			status = "ready";
		}
		else {
			cout << "[X]Unknown process status\n";
		}
	}
};

//waiting queue for IO
queue<pcb> waiting;

//ready queue ready for calc
queue<pcb> ready;

// Program counter
pcb current_process;
class dispatcher {
	int startingSize;

public : void load(pcb nextProces) {
		current_process = nextProces;
		startingSize = current_process.getInstuctionCount();
		current_process.setStatus("ready");
	}

	void unload() {
		int diff = startingSize - current_process.getInstuctionCount();
		current_process.setStatus("wait");
		current_process.update_ttk();
		current_process.update_executed_cycles(diff);
		this->respond();
	}

	void respond() {
		if (!waiting.empty()) {
			ready.push(waiting.front());
			waiting.pop();
		}
	}
};





// takes a proccess name, and loads it into the ready queue
void LoadProcess(string name) {
	pcb thisProcess(name);

	std::ifstream infile("templates.txt");
	std::string line;

	int pos = 0;
	while (std::getline(infile, line)) {
		std::istringstream iss(line);

		//find the instructionlist for the process
		if (line == name) {
			pos = 1;
		}

		//end of list, exit loop
		else if (pos > 2 and line == "") {
			break;
		}

		//line is a part of list
		else if (pos > 1) {
			int length = line.length();
			//cout << line[0] << line[length - 1] << endl;
			try {
				// give the first char for instruction 
				thisProcess.spawnInstructions(line[0]);
			}
			catch (const std::exception& e)
			{
				cout << "exception test";
				cout << e.what();
			}
		} 

		if (pos > 0) {
			pos++;
		}
	}
	cout << name << " has finished loading..." << endl;
	cout << endl;
	ready.push(thisProcess);
}


void round_robbin() {
	//time slice
	int p_time = 5;
	dispatcher disp;

	// go through the queue until all progams have finished
	while (!ready.empty())
	{
		// pop from queue, and load program
		current_process = ready.front();
		disp.load(ready.front());
		ready.pop();


		for (int i = 0; i < p_time; i++) {
			if (current_process.getNextInstruction() == 'C') {
				// perform calculations
				current_process.calculate();
			}
			else {
				// The program is not ready
				break;
			}
		}

		// if the program needs to wait, move it to the waiting queue
		if (current_process.getNextInstruction() == 'I') {
			current_process.instructions.pop();
			waiting.push(current_process);
			disp.unload();
		} 
		
		else if (current_process.instructions.size() > 0) {
			// If the proccess is not done, put it back onto the queue
			disp.unload();
			ready.push(current_process);
		}

		else if (current_process.getNextInstruction() == 'X') {
			cout << "[*]" << current_process.name << " has finished running." << endl;
			cout << "\t Cycles completed:" << current_process.get_executed_cycles() << endl;
		}
		
		
	} 
}

// As an honest confesion... This mode tends to lose track of cycles,
//	probably because of how the dispatcher works with the pcb,
//	and how the dispatcher is used in this mode. 
//  I have a feeling this problem will resolve itself when I face my fear of pointers,
//	and embrace oject oriented programming more religiously... Which you will see in assignment 3...  
void priorityQ() {
	// splits process queue into smaller, and larger processes.
	// Then will round robin untill they are done.
	// This will allow the smaller processes to be finished earlier
	// -without ignoring the larger ones

	queue<pcb> higher; // smaller processes
	queue<pcb> lower; // larger processes
	// its possible to distinguish even further and have a middle tier

	// first we will grab the average size, to differentiate 
	int total_load = 0;
	int pcount = ready.size();

	for (int i = 0; i < pcount; i++) {
		total_load = total_load + ready.front().getInstuctionCount();
		ready.push(ready.front());
		ready.pop();
	}

	int averge_size = total_load / pcount;

	for (int i = 0; i < pcount; i++) {
		pcb thisp = ready.front();

		if (thisp.getInstuctionCount() < averge_size) {
			higher.push(thisp);
			ready.pop();
		}
		else if (thisp.getInstuctionCount() >= averge_size) {
			lower.push(thisp);
			ready.pop();
		}
	}
	// Queue has been sorted into a higher, and lower priority
	// Now it will run both queues with a different time slices
	// I did it this way so the smaller processes will be finished quickly, but the larger processes will not be starved. 
	// Although, as of yet because all of the processes will be more or less the same size it doesnt really give mutch advantage. 
	// In the future I may want to add a way to update the priority queues, for when the high queue gets empty
	dispatcher disp;
	while (!higher.empty() || !lower.empty())
	{

		if (!higher.empty()){	
			disp.load(higher.front());
			higher.pop();

			for (int i = 0; i < 15; i++) {
				if (current_process.getNextInstruction() == 'C') {
					// perform calculations
					current_process.calculate();
				}
				else {
					// The program is not ready
					break;
				}
			}

			// if the program needs to wait, move it to the waiting queue
			if (current_process.getNextInstruction() == 'I') {
				current_process.instructions.pop();
				waiting.push(current_process);
				disp.unload();
				higher.push(ready.front());
				ready.pop();
			}

			else if (current_process.instructions.size() > 0) {
				// If the proccess is not done, put it back onto the queue
				disp.unload();
				higher.push(current_process);
			}

			else if (current_process.getNextInstruction() == 'X') {
				cout << "[*]" << current_process.name << " has finished running." << endl;
				cout << "\t Cycles completed:" << current_process.get_executed_cycles() << endl;
			}
			
		}
		

		if (!lower.empty()) {
			disp.load(lower.front());
			lower.pop();

			for (int i = 0; i < 3; i++) {
				if (current_process.getNextInstruction() == 'C') {
					// perform calculations
					current_process.calculate();
				}
				else {
					// The program is not ready
					break;
				}
			}

			// if the program needs to wait, move it to the waiting queue
			if (current_process.getNextInstruction() == 'I') {
				current_process.instructions.pop();
				waiting.push(current_process);
				disp.unload();
				lower.push(ready.front());
				ready.pop();
			}

			else if (current_process.instructions.size() > 0) {
				// If the proccess is not done, put it back onto the queue
				disp.unload();
				lower.push(current_process);

			}

			else if (current_process.getNextInstruction() == 'X') {
				cout << "[*]" << current_process.name << " has finished running." << endl;
				cout << "\t Cycles completed:" << current_process.get_executed_cycles() << endl;
			}

			
		}
	}
}


int main()
{
	int iSecret, iGuess;
	srand(time(NULL));

	cout << "\tWelcome to OwOS\n";
	cout << "Here are the programs detected, enter the character for the program you would like to generate.\n";

	string response;

	std::ifstream infile("templates.txt");
	
	string callingChars[50];
	int callingCounter = 0;

	string names[50];
	int namesCounter = 0;
	std::string line;

	int pos = 0;
	while (std::getline(infile, line)) {
		std::istringstream iss(line);
		if (line == "") {
			pos = 0;
		} else if (pos == 1) {
			callingChars[callingCounter] = line;
			callingCounter++;
		} else if (pos == 2) {
			names[namesCounter] = line;
			namesCounter++;
		}
		/*else {
			cout << line << endl;
			cout << pos << endl;
		}*/
		pos++;
	}

	//cout << "Options \n";
	for (int i = 0; i < callingCounter; i++) {
		cout << callingChars[i] << " for " << names[i] << endl;
	}

	cin >> response;
	int count;
	count = get_process_count();

	// Takes process name and count, and gets them loaded 
	for (int i = 0; i < callingCounter; i++) {
		if (response == callingChars[i]) {
			for (int j = 0; j < count; j++) {
				LoadProcess(names[i]);
			}
		}
	}
	cout << "\nOS has finished loading programs into memory..." << endl << endl;

	char resp;
	cout << "To run processes with (Round Robbin) scheduler, type 'r'" << endl;
	cout << "To run processes with (Priority Queue) scheduler, type 'q'" << endl;
	cin >> resp;

	if (resp == 'q' || resp == 'Q') {
		priorityQ();
	} else if (resp == 'r' || resp == 'R') {
		round_robbin();
	}
	
	//cout << "Time at finish: " << systime << endl;
	cout << "Press any key to finish";
	char fin;
	cin >> fin;
	return(0);
}