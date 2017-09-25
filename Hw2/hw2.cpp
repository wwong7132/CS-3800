
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <list>
using namespace std;
#include "hw2.h"

int main(int argc, char* argv[])
{
	string programList, programTrace, replaceAlg;
	unsigned int pageSize, pageFaults = 0;
	PageMethod pageMethod;
	ifstream finProgramList, finProgramTrace;
	
	list<Main_Memory> process;
	
	//Error Handling
	try
	{
		if(argc != 6)
		{
			throw string("Error");
		}
		programList = argv[1];
		programTrace = argv[2];
		pageSize = atoi(argv[3]);
		replaceAlg = argv[4];
		
		//Check if files are open
		finProgramList.open(programList.c_str());
		if(!finProgramList.good())
			throw string("Error: ProgramList loading failed.");
		finProgramTrace.open(programTrace.c_str());
		if(!finProgramTrace.good())
			throw string("Error: ProgramTrace loading failed.");
		if(!checkPower2(pageSize))
			throw string("Error: Page Size is not a power of 2.");
		if(pageSize < 0 || pageSize > MAX_WORDMEM)
			throw string("Error: Page size is not between 0 and 512");
		if(replaceAlg != "clock" && replaceAlg != "lru" && replaceAlg != "fifo")
			throw string("Error: Invalid Algorithm (clock, lru, fifo).");
		if(atoi(argv[5]) == 1)
			pageMethod = Pre;
		else if(atoi(argv[5]) == 0)
			pageMethod = Demand;
		else
			throw string("Error: Invalid Paging Method.");
	}catch(string err)
	{
		cout << err << endl <<  endl;
		cout << "Exiting" << endl;
		exit (EXIT_FAILURE);
	}
	
	//Reading
	int pid, memory, pageNum = 0;
	Main_Memory memoryStuff;
	while(finProgramList >> pid)
	{
		finProgramList >> memory;
		memoryStuff.pid = pid;
		memoryStuff.pages = (memory - 1)/pageSize + 1;
		memoryStuff.table = new unsigned int[memoryStuff.pages];
		for(int i = 0; i < memoryStuff.pages; i++, pageNum++)
			memoryStuff.table[i] = pageNum;
		process.push_back(memoryStuff);
	}
	memoryStuff.table = NULL;
	
	//Run algorithms
	pageFaults = simulation(process, finProgramTrace, pageMethod, pageSize, replaceAlg);
	cout << "Replacement Algorithm: " << replaceAlg << endl;
	cout << "Paging Algorithm: " << (pageMethod == Pre ? "Pre-Paging" : "Demand Paging") << endl;
	cout << "Page Size: " << pageSize << endl;
	cout << "Page Faults: " << pageFaults << endl;
	return 0;
}

bool checkPower2(int num)
{
	unsigned int count = 0;
	while(num && count <= 1)
	{
		if((num & 1) == 1)
			count++;
		num >>= 1;
	}
	return  (count == 1);
}

void initiate(list<Main_Memory> &process, unsigned int pages, int* memory)
{
	unsigned int currentPage = 0;
	unsigned int numProcesses = process.size();
	unsigned int pagesPerProcess = pages/numProcesses;
	unsigned int extras = pages % numProcesses;
	list<Main_Memory>::iterator loading = process.begin();
	
	while(loading != process.end())
	{
		for(unsigned int i = 0; i < pagesPerProcess + (extras > 0 ?1:0); i++)
		{
			if(i < loading -> pages)
				memory[currentPage] = loading->table[i];
			else
				memory[currentPage] = -1;
			currentPage++;
		}
		loading++;
		if(extras > 0)
			extras--;
	}
	return;
}

int simulation(list<Main_Memory> &process, ifstream &finProgramTrace, PageMethod pageMethod, unsigned int size, string replace)
{
	unsigned int pages = MAX_WORDMEM/size;
	int *memory = new int[pages];
	unsigned int *previous = new unsigned int[pages];
	bool *recentlyUsed = new bool[pages];
	unsigned int pid;
	unsigned int wordNeed;
	unsigned int pageNeed;
	unsigned int pageFaults = 0;
	unsigned int pagePointer = 0;
	unsigned long count = 1;
	unsigned int oldestIndex;
	bool memoryI;
	unsigned int location;
	
	for(int i = 0; i < pages; i++)
	{
	previous[i] = 0;
	recentlyUsed[i] = true;
	}
	initiate(process, pages, memory);
	while(finProgramTrace >> pid)
	{
		finProgramTrace >> wordNeed;
		pageNeed = wordNeed/size;
		memoryI = false;
		location = 0;
		list<Main_Memory>::iterator processes = process.begin();
		while(pid != processes -> pid && processes != process.end())
			processes++;
		do
		{
		  memoryI = (processes -> table[pageNeed] == memory[location]);
		  if(!memoryI)
			location++;
		}while(!memoryI && location < pages);
		
		if(!memoryI && !(location < pages))
		{
			pageFaults++;
			
			//Clock
			if(replace == "clock")
			{
				clock(processes, recentlyUsed, memory, pagePointer, pages, pageNeed);
				if(pageMethod == Pre && processes -> pages > 1 && pages > 1)
				{
					memoryI = false;
					unsigned int predLocation = 0;
					do
					{
						memoryI = (processes -> table[(pageNeed + 1) % processes -> pages] == memory[predLocation]);
						if(!memoryI)
							predLocation++;
					}while(!memoryI && predLocation < pages);
					if(!memoryI)
					{
						memory[pagePointer] = processes -> table[(pageNeed + 1) % processes -> pages];
						recentlyUsed[pagePointer] = true;
						pagePointer++;
						pagePointer %= pages;
					}
				}
			}
			
			//LRU
			else if(replace == "lru")
			{
				oldestIndex = lru(processes, previous, memory, pages, pageNeed, count);
				  
				if(pageMethod == Pre && processes -> pages > 1 && pages > 1)
				{
				  memoryI = false;
				  unsigned int predLocation = 0;
				  do
				  {
					memoryI = (processes -> table[(pageNeed + 1) % processes -> pages] == memory[predLocation]);
					if(!memoryI)
					  predLocation++;
				  }while(!memoryI && predLocation < pages);
				
				  if(!memoryI)
				  {
					memory[(oldestIndex+1)%pages] = processes -> table[(pageNeed + 1) % processes -> pages];
					previous[(oldestIndex+1)%pages] = count;
				  }
				}
			}
			
			//FIFO
			else if(replace == "fifo")
			{
				fifo(processes, memory, pagePointer, pages, pageNeed);
				if(pageMethod == Pre && processes -> pages > 1 && pages > 1)
				{
					memoryI = false;
					unsigned int predLocation = 0;
					do
					{
						memoryI = (processes -> table[(pageNeed + 1) % processes -> pages] == memory[predLocation]);
						if(!memoryI)
						predLocation++;
					}while(!memoryI && predLocation < pages);
					if(!memoryI)
					{
						memory[pagePointer] = processes -> table[(pageNeed + 1) % processes -> pages];
						pagePointer++;
						pagePointer %= pages;
					}
				}
			}
		}
    else
    {
      recentlyUsed[location] = true;
      previous[location] = count;
    }
    count++;
  }
  delete [] recentlyUsed;
  delete [] previous;
  delete [] memory;
  return pageFaults;  
}

void clock(list<Main_Memory>::iterator process, bool* recentlyUsed, int* memory, unsigned int& pagePointer, unsigned int pages, unsigned int pageNeed)
{
	while(recentlyUsed[pagePointer] && memory[pagePointer] != -1)
	{
		recentlyUsed[pagePointer] = false;
		pagePointer++;
		pagePointer %= pages;
	}
	memory[pagePointer] = process -> table[pageNeed];
	recentlyUsed[pagePointer] = true;
	pagePointer++;
	pagePointer %= pages;
	return;
}

unsigned int lru(list<Main_Memory>::iterator process, unsigned int* lastUsed, int* memory, unsigned int pages, unsigned int pageNeed, unsigned int count)
{
  unsigned int oldest = lastUsed[0];
  unsigned int oldestIndex = 0;

  for(unsigned int i = 1; i < pages; i++)
    if(lastUsed[i] < oldest)
    {
      oldest = lastUsed[i];
      oldestIndex = i;
    }
  memory[oldestIndex] = process -> table[pageNeed];
  lastUsed[oldestIndex] = count;
  return oldestIndex;
}

void fifo(list<Main_Memory>::iterator process, int* memory, unsigned int& pagePointer, unsigned int pages, unsigned int pageNeed)
{
  memory[pagePointer] = process -> table[pageNeed];
  pagePointer ++;
  pagePointer %= pages;
  return;
}