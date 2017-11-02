#define MEM_TRACKER_ENABLED

#include<thread>
#include"Cordinator.h"
#include"MemTracker.h"

int main()
{
	MEM_TRACKER_BEGIN

	FileThread thread1;
	Cordinator thread2;
	Playback   thread3;

	std::thread fileth(thread1);
	std::thread cordinatorth(thread2);
	std::thread playbackth(thread3);

	fileth.join();
	cordinatorth.join();
	playbackth.join();
}

 

 


