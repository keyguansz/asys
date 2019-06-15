/*
 #include "VsyncManager.h"
 VsyncManager vsyncManager = new VsyncManager();

 */
#pragma once

#include <utils/threads.h>
#include <utils/Vector.h>
#include <semaphore.h>

#include <gui/DisplayEventReceiver.h>

#define UNUSED(expr) do { (void)(expr); } while (0)

namespace android
{
class VsyncManager: public RefBase
{
public:
    VsyncManager();
    ~VsyncManager();
	//TODO static??
	static void* start(void *arg);
	void stop();
	static int onReceive(int fd, int events, void* data);
private:
	DisplayEventReceiver mDisplayReceiver;
	bool	mIsStart;
	pthread_t	mThreadId;
	int	mThreadStatus;
};
	
}
