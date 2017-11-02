#include"FileThread.h"
#include"Playback.h"

struct Cordinator
{
	char *bufferA;
	char *bufferB;
	//
	int   sizeBufferA;
	int   sizeBufferB;
	//
	int   bufferSwitch;
	char *buff;
	int   buffLength;
	int	  fileCounter;
	DWORD currentsize;

	Cordinator()
	{
		bufferSwitch = 0;
		buffLength = 0;
		bufferA = new char[WAV_AUDIO_SIZE];
		bufferB = new char[WAV_AUDIO_SIZE];
		memset(bufferA, '\0', WAV_AUDIO_SIZE);
		memset(bufferB, '\0', WAV_AUDIO_SIZE);
		sizeBufferA = 0;
		sizeBufferB = 0;
	    currentsize = 0;
		fileCounter = 1;
	}

	

	void reloadBuffers()
	{
		if (bufferSwitch == 0)
		{
			memcpy(bufferA, block, blockSize);
			sizeBufferA = blockSize;
			bufferSwitch = 1;
			buff = block;
			buffLength = sizeBufferA;
		}
		else
		{
			memcpy(bufferB, block, blockSize);
			sizeBufferB = blockSize;
			bufferSwitch = 0;
			buff = block;
			buffLength = sizeBufferB;
		}
		filest = true;
	}

	void operator()()
	{
		reloadBuffers();
		while (!done)
		{
			if (blockSize >0)
			{
				std::unique_lock<std::mutex> lock(locker);
				//unique locking to lock mutex

				while (wavehdrQueue.size() == NO_OF_THREADS)
				{
					cv1.wait(lock);
				}

				for (int i = 0; i < NO_OF_THREADS && i < wavehdrQueue.size(); i++)
				{
					WAVEHDR *hdr = wavehdrQueue.front();
					hdr->dwBufferLength = WAVE_BUFFER_SIZE;
					memcpy(hdr->lpData, buff, WAVE_BUFFER_SIZE);

					buff += WAVE_BUFFER_SIZE;
					currentsize += WAVE_BUFFER_SIZE;
					wavehdrQueue.pop();
				}

				//wakeup only one thread wich is waiting for the mutex
				cv2.notify_one();
				lock.unlock();
				wavePlayer.donePlaying = false;
				wavePlayer.numWaves = NO_OF_THREADS;
			
				if ( fileCounter <= 23 && (currentsize / 1024) >= (blockSize / 1024))
				{
					fileCounter++;
					if (fileCounter == 24)
					{
						playing = true;
						done = true;
						wavePlayer.donePlaying = true;					
						cv2.notify_one();
						break;
					}

					reloadBuffers();
					//refresh the buffers
					currentsize = 0;
				}
			}
		}
	}
};