#include<mutex>
#include<stdlib.h>
#include<queue>
#include<mmeapi.h>
#include"MemTracker.h"



static std::mutex locker;
std::mutex player_quitter;
static std::condition_variable cv1;
static std::condition_variable cv2;
std::condition_variable cv3;
//
static DWORD currentsize = 0;
static std::queue<WAVEHDR *> wavehdrQueue;
//
static bool callbackState = true;
static bool done = false;
static bool waveThread[NO_OF_THREADS];
static int threadCounter = 0;


struct UserWavePlayer
{
	UserWavePlayer()
	{
		this->donePlaying = false;
		this->closed = false;
		this->numWaves = 0;
	}

	~UserWavePlayer()
	{

	}

	bool donePlaying;
	bool closed;
	volatile int  numWaves;
};

UserWavePlayer wavePlayer;


struct WaveBuffers
{
	HWAVEOUT hWaveOut; 
	MMRESULT result; 
	WAVEHDR *header;
	char	  *buff;

	WaveBuffers(HWAVEOUT hWave)
	{
		this->hWaveOut = hWave;
		buff = new char[WAVE_BUFFER_SIZE];
		memset(buff, '\0', WAVE_BUFFER_SIZE);

		this->header = new WAVEHDR();
		header->lpData = buff;
		header->dwBufferLength = WAVE_BUFFER_SIZE;
	}

	~WaveBuffers()
	{
	}

	void operator()()
	{
		while (!wavePlayer.donePlaying)
		{
			std::unique_lock<std::mutex> lock(locker);
			//lock mutex
			while (waveThread[threadCounter] && callbackState)
			{
				cv2.wait(lock);
			}

			waveThread[threadCounter] = true;
			waveOutPrepareHeader(hWaveOut, header, sizeof(WAVEHDR));
			waveOutWrite(hWaveOut, header, sizeof(WAVEHDR));
			threadCounter++;
			lock.unlock();

			if (threadCounter >= NO_OF_THREADS)
			{
				threadCounter = 0;

				for (int i = 0; i < NO_OF_THREADS; i++)
				{
					waveThread[i] = false;
				}
			}
		}

	}
};
