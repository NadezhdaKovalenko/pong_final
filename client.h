#ifndef CLIENT_H
#define CLIENT_H

#include <sys/types.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <cstdlib>
#include <iostream>

#include "connector.h"


class Client
{
public:
	Client()
	{
		width = 600;
		height = 400;

		racketWidth = 10;
		racketHeight = 100;
	}

	int genPosY(int posBall)
        {
		return posBall;
	}

	void run(Connector& connector, sem_t* semHost, sem_t* semClient)
	{
		int posY;
		float posBall;
		int finish;
		
    		while (true)
		{
        		sem_wait(semClient);
    			connector.Read(&finish, sizeof(int));

    			if (finish == 1)
			{
        			sem_close(semClient);
    				sem_unlink("/semClient");
        			break;
        		}

			sem_post(semHost);
			sem_wait(semClient);
			connector.Read(&posBall, sizeof(posBall));
			//syslog(LOG_INFO, "Client - PosREAD = %f", posBall);

			posY = genPosY((int)posBall);
			//syslog(LOG_INFO, "Client - PosWRITE = %i", posY);
        		connector.Write(&posY, sizeof(posY));
        		sem_post(semHost);		
        	}

        	sem_post(semHost);
        	exit(0);
	}

private:
	int width;
	int height;

	int racketWidth;
	int racketHeight;

	int curPosY;
	int step;
};

#endif
