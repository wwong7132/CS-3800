
#ifndef HW2_H
#define HW2_H

unsigned int MAX_WORDMEM = 512;

struct Main_Memory
{
	unsigned int pid;
	unsigned int pages;
	unsigned int* table;
	
	~Main_Memory(){if(table != NULL) delete [] table;}
};

enum PageMethod{Demand, Pre};
bool checkPower2(int num);
void initiate(list<Main_Memory> &process, unsigned int pages, int* memory);
int simulation(list<Main_Memory> &process, ifstream &finProgramTrace, PageMethod pageMethod, unsigned int size, string replace);
void clock(list<Main_Memory>::iterator process, bool* recentlyUsed, int* memory, unsigned int& pagePointer, unsigned int pages, unsigned int pageNeed);
unsigned int lru(list<Main_Memory>::iterator process, unsigned int* lastUsed, int* memory, unsigned int pages, unsigned int pageNeed, unsigned int count);
void fifo(list<Main_Memory>::iterator process, int* memory, unsigned int& pagePointer, unsigned int pages, unsigned int pageNeed);

#endif