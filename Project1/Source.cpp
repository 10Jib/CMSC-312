#include <iostream>
#include <sstream>
#include <string>

#include <fstream> 
#include <stdlib.h>
#include <queue>
#include <time.h>
#include <mutex>

#include <thread>


using namespace std;

int systime = 0;

bool critical = false;


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
	Process* parent;
	Process* child;

	bool isCritical = false; 


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
		else if (type == 'E' || type == 'L' || type == 'F') {
			instructions.push(type);
		}
		else
		{
			cout << "BAD INSTRUCTION TYPE";
		}
	}

	void addInstruction(char type) {
		instructions.push(type);
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
		else if (this->getNextInstruction() == 'E') {
			if (this->isCritical == false) {
				cout << "\nEntering Critical Section...\n";
				this->isCritical = true;
			} 
			instructions.pop();		
		}
		else if (this->getNextInstruction() == 'L') {
			if (this->isCritical == true) {
				cout << "..Leaving Critical Section\n";
				this->isCritical = false;
			}
			instructions.pop();
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
	string childStatus;

	using Process::Process;

	// creates new process and hands off pointer
public:	void fork() {
		// at this time process can only have one child
		if (this->child == NULL) {
			Process newchild = Process(name);
			cout << "[*] New child!\n";

			newchild.parent = this;
			this->child = &newchild;
			this->childStatus = "Active";
		}
	}

	// goes down dynamic linked child list, and terminates
public:	void terminate() {
		cout << "[X] Child terminated!\n";
		this->childStatus = "TERMINATED";

		//Process* thisChild = this->child;
		//thisChild->child.Status = "TERMINATED";
		/*This is the part that would terminate all childproccesses, like a linked list
		while (thisChild->child != NULL) {
			thisChild->childStatus = "TERMINATED";
			thisChild = thisChild->child;
		}*/

	}

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


class storage_drive {
	// The first frame will be used to index the drive.
	// So with this setup a max of 1000 frames can be represented in one.
	// While limmiting, it makes things simpler for me.

	int frameSize = 1000;
	int totalFrams = frameSize + 1;
	int frameIndexLocation = 0;
	string driveName = "";
	//ofstream file_obj;

public: storage_drive(string name) {
		driveName = name;


	}

	void load_drive(string name) {
		// framesize, and count is assumed same.

	}

	void allocateSpace(int frameCount = 1) {
		//finds a spot in memory and allocates space
	}

public: void saveProcess(pcb *process, int location) {
		ofstream file_obj;
		file_obj.open(driveName, ios::out);
		file_obj.seekp(location*frameSize);

		string name = process->name;
		int size = strlen((char *)&name);

		int i;
		for (i = 0; i < size; i++) {
			file_obj.put((char)name[i]);
		}

		file_obj.put(' ');
		while (!process->instructions.empty()) {
			file_obj.put(process->instructions.front());
			process->instructions.pop();
		}
		file_obj.close();
	}

public: pcb loadProcess(int location) {
		fstream file_obj;
		file_obj.open(driveName, ios::in);
		file_obj.seekg(location*frameSize);


		string name = "";
		char curchar;

		while (file_obj >> noskipws >> curchar) {
			if (curchar == ' ') {
				break;
			}
			name + curchar;
		}
		pcb process(name);

		while (file_obj >> noskipws >> curchar) {
			if (curchar == ' ') {
				break;
			}
			process.addInstruction(curchar);
		}

		file_obj.close();
		return(process);
	}

	void deleteProcess() {

	}

	void write_frame() {

	}

	string read_frame() {

	}

	void wipe_frame() {

	}

};

//The single storage drive for this OS
storage_drive CDrive("CDrive.txt");

//waiting queue for IO
queue<pcb> waiting;


//ready queue ready for calc
queue<pcb> ready;
std::mutex readyMutex;

class dispatcher {
	pcb current_process;
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

void load_round_Robbin() {
	pcb current_process;
	int i;

	// store all of the processes into memory
	for (i = 0; i < ready.size(); i++) {
		current_process = ready.front();
		CDrive.saveProcess(&current_process, i + 1);
		ready.pop();
		ready.push(current_process);
	}
}

void round_robbin() {
	pcb current_process;

	//time slice
	int p_time = 5;
	dispatcher disp;

	// go through the queue until all progams have finished
	while (!ready.empty())
	{
		// wait until queue is ready
		while (!readyMutex.try_lock()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}

		if (ready.empty()) {
			readyMutex.unlock();
			break;  // If the queue is emptied while waiting this will break the loop
		}

		// pop from queue, and load program
		current_process = ready.front();
		disp.load(ready.front());
		ready.pop();
		readyMutex.unlock();

		for (int i = 0; i < p_time; i++) {
			if (current_process.getNextInstruction() == 'E') {
				// wants to enter critical
				if (critical) {
					// another process is critical
					break;
				}
				else {
					// process can be critical
					critical = true;
					current_process.calculate();
				}
			}
			else if (current_process.getNextInstruction() == 'L') {
				if (critical && current_process.isCritical) {
					// process is already in critical and wants to leave
					current_process.calculate();
					critical = false;
				}
			}
			else if (current_process.getNextInstruction() == 'C') {
				// perform calculations
				current_process.calculate();
			}
			else if (current_process.getNextInstruction() == 'F') {
				current_process.fork();
				current_process.instructions.pop();
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
			current_process.terminate();
			cout << "[*]" << current_process.name << " has finished running." << endl;
			//cout << "\t Cycles completed:" << current_process.get_executed_cycles() << endl;
		}
		
		
	} 
}

queue<pcb> higher; // smaller processes
std::mutex higherMutex;

queue<pcb> lower; // larger processes
std::mutex lowerMutex;

// This section had to be reworked to handle the concurrent processing
// it was not fun debugging this... butatleast I have the satisfaction that I know that it works...
// or atleast that I dont know that it doesnt work, but rest assured, in theory everything should work

void organizePQ() {
	// splits process queue into smaller, and larger processes.
	// Then will round robin untill they are done.
	// This will allow the smaller processes to be finished earlier
	// without ignoring the larger ones
	// when the smaller ones are finished first, it will give more cpu time to the larger-
	// processes that could have stalled the smaller ones

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
}

void priorityQ() {
	// Queue has been sorted into a higher, and lower priority
	// Now it will run both queues with a different time slices
	// I did it this way so the smaller processes will be finished quickly, but the larger processes will not be starved. 
	// Although, as of yet because all of the processes will be more or less the same size it doesnt really give mutch advantage. 
	// In the future I may want to add a way to update the priority queues, for when the high queue gets empty

	pcb current_process;
	dispatcher disp;
	while (!higher.empty() || !lower.empty())
	{

		if (!higher.empty()){
			// wait until queue is ready
			while (!higherMutex.try_lock()) {
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			}

			if (higher.empty()) {
				higherMutex.unlock();
				continue;  // If the queue is emptied while waiting this will cycle just incase lower queue is still full.
			}

			disp.load(higher.front());
			current_process = higher.front();
			higher.pop();
			higherMutex.unlock();

			for (int i = 0; i < 15; i++) {
				if (current_process.getNextInstruction() == 'E') {
					// wants to enter critical
					if (critical) {
						// another process is critical
						break;
					}
					else {
						// process can be critical
						critical = true;
						current_process.calculate();
					}
				}
				else if (current_process.getNextInstruction() == 'L') {
					if (critical && current_process.isCritical) {
						// process is already in critical and wants to leave
						current_process.calculate();
						critical = false;
					}
				}
				else if (current_process.getNextInstruction() == 'C') {
					// perform calculations
					current_process.calculate();
				}
				else if (current_process.getNextInstruction() == 'F') {
					current_process.fork();
					current_process.instructions.pop();
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
				current_process.terminate();
				cout << "[*]" << current_process.name << " has finished running." << endl;
				cout << "\t Cycles completed:" << current_process.get_executed_cycles() << endl;
			}
			
		}
		

		if (!lower.empty()) {
			// wait until queue is ready
			while (!lowerMutex.try_lock()) {
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			}

			if (lower.empty()) {
				lowerMutex.unlock();
				continue;  // If the queue is emptied while waiting this will cycle just incase higher queue is still full.
			}

			disp.load(lower.front());
			current_process = lower.front();
			lower.pop();
			lowerMutex.unlock();

			for (int i = 0; i < 3; i++) {
				if (current_process.getNextInstruction() == 'E') {
					// wants to enter critical
					if (critical) {
						// another process is critical
						break;
					}
					else {
						// process can be critical
						critical = true;
						current_process.calculate();
					}
				}
				else if (current_process.getNextInstruction() == 'L') {
					if (critical && current_process.isCritical) {
						// process is already in critical and wants to leave
						current_process.calculate();
						critical = false;
					}
				}
				else if (current_process.getNextInstruction() == 'C') {
					// perform calculations
					current_process.calculate();
				}
				else if (current_process.getNextInstruction() == 'F') {
					current_process.fork();
					current_process.instructions.pop();
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
				current_process.terminate();
				cout << "[*]" << current_process.name << " has finished running." << endl;
				cout << "\t Cycles completed:" << current_process.get_executed_cycles() << endl;
			}

			
		}
	}
}


int main()
{
	//int iSecret, iGuess;
	//srand(time(NULL));

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
		organizePQ();

		std::thread first(priorityQ);
		std::thread seccond(priorityQ);
		std::thread third(priorityQ);

		first.join();
		seccond.join();
		third.join();

	} else if (resp == 'r' || resp == 'R') {
		load_round_Robbin();

		std::thread first(round_robbin);
		std::thread seccond(round_robbin);
		std::thread third(round_robbin);

		first.join();
		seccond.join();
		third.join();
	}
	
	//cout << "Time at finish: " << systime << endl;
	cout << "Press any key to finish";
	char fin;
	cin >> fin;
	return(0);
}