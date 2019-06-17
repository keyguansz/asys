#define ATRACE_TAG ATRACE_TAG_GRAPHICS

//#define LOG_TAG "VsyncManager"
#include <math.h>
#include <gui/DisplayEventReceiver.h>
#include <utils/Looper.h>
#include <cutils/properties.h>
#include "VsyncManager.h"
#include <sys/resource.h>
#include "utils/CallStack.h"
#include "android/looper.h"

//TODO #Include "QosProbe/TraceThread.h"
using namespace android;
VsyncManager::VsyncManager()
{
    mThreadId = 0;
    mThreadStatus = pthread_create(&mThreadId, NULL, VsyncManager::start, this);
    setpriority(PRIO_PROCESS, mThreadId, ANDROID_PRIORITY_DISPLAY);
}

VsyncManager::~VsyncManager()
{
    stop();
}

void* VsyncManager::start(void *arg)
{
	ALOGD(" *************start****************");
	VsyncManager* mgr = (VsyncManager*)arg;
	mgr->mIsStart = true;
    //prctl(PR_SET_NAME, (unsigned long)"VsyncManager", 0, 0, 0);
    //TraceThread::getInstance()->writeTrace(mvsync->thread_id, LOG_TAG, "VsyncManager::startthread");

    if (!mgr->mDisplayReceiver.getFd()) {
        ALOGD("start,getFd error");
    }
    sp<Looper> loop = new Looper(false);
    loop->addFd(mgr->mDisplayReceiver.getFd(), 0,
                ALOOPER_EVENT_INPUT, VsyncManager::onReceive, arg);
    
    mgr->mDisplayReceiver.setVsyncRate(1);
    do {
        //printf("about to poll...\n");
        int32_t ret = loop->pollOnce(100);
        switch (ret) {
        case  ALOOPER_POLL_WAKE:
            //("ALOOPER_POLL_WAKE\n");
            break;
        case  ALOOPER_POLL_CALLBACK:
            //("ALOOPER_POLL_CALLBACK\n");
            break;
        case  ALOOPER_POLL_TIMEOUT:
            ALOGE("ALOOPER_POLL_TIMEOUT\n");
            break;
        case  ALOOPER_POLL_ERROR:
            ALOGE("ALOOPER_POLL_TIMEOUT\n");
            break;
        default:
            ALOGE("ugh? poll returned %d\n", ret);
            break;
        }

        //ALOGD("startthread(0*************");
    } while (mgr->mIsStart);

    if (loop.get()) {
        loop->removeFd(mgr->mDisplayReceiver.getFd());
        loop.clear();
        loop = NULL;
    }
	return ((void*)0);
}
void VsyncManager::stop()
{
	ALOGD(" *************stop****************");
    if (mThreadStatus >= 0) {
        mIsStart = false;
        if (pthread_join(mThreadId, NULL) != 0) {
            ALOGE("Couldn't cancel vsync thread ");
        }
        mThreadId = 0;
        mThreadStatus = -1;
    }
    ALOGD("stoped");
}
int VsyncManager::onReceive(int fd, int events, void* arg)
{
	UNUSED(fd);
	UNUSED(events);
	VsyncManager* mgr = (VsyncManager*)arg;
    DisplayEventReceiver* q = &(mgr->mDisplayReceiver);
    ssize_t n;
    DisplayEventReceiver::Event buffer[1];

    ALOGD("onReceive**************");
    while ((n = q->getEvents(buffer, 1)) > 0) {
        for (int i=0 ; i<n ; i++) {
			
			if( !mgr->mIsStart ){
				return 1;
			}
			
            if (buffer[i].header.type == DisplayEventReceiver::DISPLAY_EVENT_VSYNC) {
                ALOGE("==============event vsync: count=%d\t", buffer[i].vsync.count);
            }

			if(buffer[i].header.type == DisplayEventReceiver::DISPLAY_EVENT_HOTPLUG){
				//mvsync->mHdmiConnected = buffer[i].hotplug.connected;
				//mvsync->mHdmiPassFrame = mvsync->mHdmiConnected;
				//ALOGD("******hdmi connected(%d)******",mvsync->mHdmiConnected);
			}
			long timestamp = buffer[i].header.timestamp;
			ALOGD("******timestamp(%ld)******", timestamp);
            
        }
    }
    if ( n < 0 ) {
        ALOGE("reading events error(%s)\n", strerror(-n));
    }

    return 1;
}
