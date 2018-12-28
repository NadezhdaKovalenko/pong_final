#include "host.h"

Host::Host()
{
	width = 600;
	height = 400;

	scoreClient = 0;
	scoreHost = 0;

	racketWidth = 10;
	racketHeight = 100;

	racketHost_x = racketWidth;
	racketHost_y = height/2 - racketHeight/2;
	
	racketClient_x = width - racketWidth;
	racketClient_y = height/2;

	newPosYClient = 0;

	ballPos_x = width / 2;
	ballPos_y = height / 2;
	ballDir_x = -1.0f;
	ballDir_y = 0.1f;
	ballSize = 20;
	ballSpeed = 2;

	final = 0;
}

float Host::myRand(float a, float b)
{
	return (a + rand() * (b - a)/(RAND_MAX - 1));
}

void Host::norm(float& x, float &y)
{
	float length = sqrt((x * x) + (y * y));

	if (length != 0.0f)
	{
		x = x / length;
		y = y / length;
	}
}

void Host::drawRect(int p_x, int p_y, float width, float height)
{
	glBegin (GL_QUADS);
	glVertex2f (p_x - width / 2, p_y - height / 2);
	glVertex2f (p_x - width / 2, p_y + height / 2);
	glVertex2f (p_x + width / 2, p_y + height / 2);
	glVertex2f (p_x + width / 2, p_y - height / 2);
	glEnd ();
}

void Host::drawScore(float x, float y)
{
	glRasterPos2f(x, y);
	std::stringstream ss;
	ss << scoreHost << " : " << scoreClient;
	std::string result = ss.str();
	glutBitmapString(GLUT_BITMAP_8_BY_13, (const unsigned char*)result.c_str());
}

void Host::drawHost()
{
	drawRect(racketHost_x, racketHost_y, racketWidth, racketHeight);
	drawRect(racketClient_x, racketClient_y, racketWidth, racketHeight);
	drawRect(ballPos_x, ballPos_y, ballSize, ballSize);
	drawScore(width / 2 - 20, height - 20);
}

bool Host::isInRacket(int p_x, int p_y)
{
	return ballPos_x<(p_x+racketWidth/2) &&
		ballPos_x>(p_x-racketWidth/2) &&
		ballPos_y<(p_y+racketHeight/2) &&
		ballPos_y>p_y-racketHeight/2;
}

void Host::getNewBallPos()
{
	ballPos_x += ballDir_x * ballSpeed;
	ballPos_y += ballDir_y * ballSpeed;
}

void Host::checkLeftWall()
{
	if (ballPos_x - ballSize / 2 <= 0)
	{
		ballPos_x = width / 2;
		ballPos_y = height / 2;
		
		ballDir_x = fabs(myRand(0.0f, 1.0f));
		ballDir_y = 0;
		scoreClient++;
	}
}

void Host::checkRightWall()
{
	if (ballPos_x + ballSize / 2 >= width)
	{
		ballPos_x = width / 2;
		ballPos_y = height / 2;
		
		ballDir_x = -fabs(myRand(0.0f,1.0f)); 
		ballDir_y = 0;
		scoreHost++;	
	}
}

void Host::checkTopWall()
{
	if (ballPos_y + ballSize / 2 > height)
	{
		ballDir_y = -fabs(myRand(0.0f,1.0f)); 
	}
}

void Host::checkBottomWall()
{
	if (ballPos_y - ballSize / 2 < 0)
	{
		ballDir_y = fabs(myRand(0.0f,1.0f)); 
	}
}

void Host::updateField() 
{
	// ball racket
	getNewBallPos();

	if(isInRacket(racketHost_x, racketHost_y))
	{
		float t = ((ballPos_y - racketHost_y) / racketHeight) - 0.5f;
		ballDir_x = fabs(myRand(0.0f,1.0f));
		ballDir_y = t;
	}

	if (isInRacket(racketClient_x, racketClient_y))
	{
		float t = ((ballPos_y - racketClient_y) / racketHeight) - 0.5f;
		ballDir_x = -fabs(myRand(0.0f,1.0f));
		ballDir_y = t;
	}

	checkLeftWall();
	checkRightWall();
	checkTopWall();
	checkBottomWall();

	//norm ball direction
	norm(ballDir_x, ballDir_y);

	
	if(racketHost_y - racketHeight/2+ballSize >= ballPos_y && racketHost_y - racketHeight/2 >= 0)
	{
		racketHost_y -= 1;
	}
	else if(racketHost_y + racketHeight/2-ballSize <= ballPos_y && racketHost_y + racketHeight/2 <= height)
	{
		racketHost_y += 1;	
	}
	

	if(racketClient_y < newPosYClient && racketClient_y + racketHeight/2 <=  height)
	{
		racketClient_y += 1;	
	}
	else if(racketClient_y > newPosYClient && racketClient_y - racketHeight/2 > 0)
	{
		racketClient_y -= 1;
	}
}

void Host::updateClientRacket(int posY)
{
		newPosYClient = posY;
}

void Host::run(Connector& connector, sem_t* semHost, sem_t* semClient)
{
	int newPosRacketClient_y;

	while(true)
	{      
		connector.Write(&final, sizeof(int));
		sem_post(semClient);
        	
	        if(final == 1)
		{
	       		sem_close(semHost);
	        	sem_unlink("/semHost");
	     		break;	
	     	}

		sem_wait(semHost);
		//syslog(LOG_INFO, "Host - PosWRITE = %f", ballPos_y);
		
		connector.Write(&ballPos_y, sizeof(ballPos_y));
		
		sem_post(semClient);
		sem_wait(semHost);		
	        connector.Read(&newPosRacketClient_y, sizeof(newPosRacketClient_y));
		//syslog(LOG_INFO, "Host - PosREAD = %i", newPosRacketClient_y);
    
	        updateClientRacket(newPosRacketClient_y);
	     	glutMainLoopEvent();
	}

    	sem_post(semClient);
	exit(0);
}

