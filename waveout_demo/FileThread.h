
#include<assert.h>
#include"MemTracker.h"
#include<Windows.h>
#include<string>
//all defines for jetsons
#define WAV_AUDIO_SIZE 524288
#define NO_OF_THREADS 20
#define WAVE_BUFFER_SIZE 2048 //2k each
//................

static LPSTR block;
static DWORD blockSize;
//
static bool filest = true;
static bool playing = false;
  
static int i = 0;

struct FileThread
{

	LPSTR loadAudioBlock(const char* filename, DWORD* blockSize)
	{
		FILE *hFile = 0;
		DWORD size = 0;
		DWORD readBytes = 0;

		fopen_s(&hFile, filename, "r");
		assert(hFile != 0);

		// get the size
		fseek(hFile, 0, SEEK_END);
		size = ftell(hFile);
		rewind(hFile);

		// safety
		assert(size != 0);

		// read the data into the buffer
		fread(block, size, 1, hFile);

		fclose(hFile);

		*blockSize = size;
		return (LPSTR)block;
	}

	void operator()()
	{
		std::string str = "wave_";
		while (!playing)
		{
			if ( i < 23 &&  filest == true )
			{
				str = "wave_";
				str = str + std::to_string(i) + ".wav";
				block = loadAudioBlock(str.c_str(), &blockSize);
				i++;
				filest = false;
			}
			else
				
			{Sleep(20); }
		}

	}

	FileThread()
	{
		block = new char[WAV_AUDIO_SIZE];
	}
};
