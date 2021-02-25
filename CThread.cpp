/*
 * CThread.cpp
 *
 *  Created on: 2020年3月5日
 *      Author: Administrator
 */

#include "CThread.h"
#include "vispeech_debug.h"

int CThread::start()
{
	if(pthread_create(&pid, NULL, start_thread, (void*)this) != 0)
	{
		return -1;
	}

	return 0;
}

void* CThread::start_thread(void *arg)
{
	CThread *ptr = (CThread*)arg;
	ptr->run();
}
