#include"WaveBuffers.h"
//......................
#include<mmeapi.h>
#include<Windows.h>

struct Playback
{
	HWAVEOUT hWaveOut; 
	WAVEFORMATEX wfx; 
	MMRESULT result;  

	void static CALLBACK waveOutProc(HWAVEOUT hWaveOut, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
	{
		UserWavePlayer *pWavePlayer = (UserWavePlayer *)dwInstance;
		WAVEHDR *waveHdr = 0;

		switch (uMsg)
		{
		case WOM_DONE:
			// with this case, dwParam1 is the wavehdr
			waveHdr = (WAVEHDR *)dwParam1;

			if (pWavePlayer->numWaves>0)
			{
				pWavePlayer->numWaves--;
				wavehdrQueue.push(waveHdr);
				//std::cout << "WOM_DONE: " << waveHdr->dwUser << " remain: " << pWavePlayer->numWaves /*<< " current size " << currentsize / 1024 << " " << blockSize */ << std::endl;

				if (wavehdrQueue.size() == NO_OF_THREADS)
				{
					cv1.notify_one();
					callbackState = false;
				}
			}

			break;

		case WOM_CLOSE:
			pWavePlayer->closed = true;
			printf("WOM_CLOSE:\n");
			break;

		case WOM_OPEN:
			printf("WOM_OPEN:\n");
			break;

		default:
			assert(false);
		}


	}


	Playback()
	{
		wfx.nSamplesPerSec = 22050; // sample rate 
		wfx.wBitsPerSample = 16;    // number of bits per sample of mono data 
		wfx.nChannels = 2;          // number of channels (i.e. mono, stereo...) 
		wfx.wFormatTag = WAVE_FORMAT_PCM;								// format type 
		wfx.nBlockAlign = (wfx.wBitsPerSample >> 3) * wfx.nChannels;	// block size of data 
		wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;		// for buffer estimation 
		wfx.cbSize = 0;
		wfx.wFormatTag = WAVE_FORMAT_PCM;								// format type 
		wfx.nBlockAlign = (wfx.wBitsPerSample >> 3) * wfx.nChannels;	// block size of data 
		wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;		// for buffer estimation 
		wfx.cbSize = 0;

		result = waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, (DWORD_PTR)this->waveOutProc, (DWORD_PTR)&wavePlayer, CALLBACK_FUNCTION);
		if (result != MMSYSERR_NOERROR)
		{
			fprintf(stderr, "unable to open WAVE_MAPPER device\n");
			ExitProcess(1);
		}
		printf("The Wave Mapper device was opened successfully!\n");


		for (int i = 1; i <= NO_OF_THREADS; i++)
		{
			WaveBuffers wb(hWaveOut);
			std::thread t2(wb);
			t2.detach();
			wavePlayer.numWaves++;
		}
	}


	void operator()()
	{
		std::unique_lock<std::mutex> playend(player_quitter);
		while (!wavePlayer.donePlaying)
		{
			cv3.wait(playend);
		}
		waveOutClose(hWaveOut);
	}
};
