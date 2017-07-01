/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <cutils/native_handle.h>
#include <hardware/sensors.h>

#include <utils/Log.h>

#if 0
#define  LOG_D(...)  printf(__VA_ARGS__)
#else
#define  LOG_D(...)  ((void)0)
#endif

#include "SensorsServer.h"


#define MAX_NUM_SENSORS 8
#define SUPPORTED_SENSORS  ((1<<MAX_NUM_SENSORS)-1)

#define  ID_BASE           SENSORS_HANDLE_BASE
#define  ID_ACCELERATION   (ID_BASE+0)
#define  ID_MAGNETIC_FIELD (ID_BASE+1)
#define  ID_ORIENTATION    (ID_BASE+2)
#define  ID_TEMPERATURE    (ID_BASE+3)
#define  ID_GYROSCOPE    (ID_BASE+4)
#define  ID_GRAVITY    (ID_BASE+5)
#define  ID_LINEAR_ACCELERATION    (ID_BASE+6)
#define  ID_ROTATION_VECTOR    (ID_BASE+7)

#define SENSOR_TYPE_ACCELERATION    1
#define SENSOR_TYPE_ORIENTATION    3
#define SENSOR_TYPE_GYROSCOPE     4
#define SENSOR_TYPE_GRAVITY    9
#define SENSOR_TYPE_LINEAR_ACCELERATION    10
#define SENSOR_TYPE_ROTATION_VECTOR    11


typedef struct sensors_poll_context{
    struct sensors_poll_device_t  device;
    sensors_event_t               sensors[MAX_NUM_SENSORS];
    int                           fd;
    uint32_t                      pendingSensors;
}sensors_poll_context;


static const struct sensor_t sSensorList[MAX_NUM_SENSORS] = {
    { 
        .name       = "Goldfish 3-axis Accelerometer",
        .vendor     = "The Android Open Source Project",
        .version    = 1,
        .handle     = ID_ACCELERATION,
        .type       = SENSOR_TYPE_ACCELEROMETER,
        .maxRange   = 2.8f,
        .resolution = 1.0f/4032.0f,
        .power      = 3.0f,
        .minDelay   = 0,
        .reserved   = {}
    },
    
    { 
        .name       = "Goldfish 3-axis Magnetic field sensor",
        .vendor     = "The Android Open Source Project",
        .version    = 1,
        .handle     = ID_MAGNETIC_FIELD,
        .type       = SENSOR_TYPE_MAGNETIC_FIELD,
        .maxRange   = 2000.0f,
        .resolution = 1.0f,
        .power      = 6.7f,
        .minDelay   = 0,
        .reserved   = {}
    },
    
    { 
        .name       = "Goldfish Orientation sensor",
        .vendor     = "The Android Open Source Project",
        .version    = 1,
        .handle     = ID_ORIENTATION,
        .type       = SENSOR_TYPE_ORIENTATION,
        .maxRange   = 360.0f,
        .resolution = 1.0f,
        .power      = 9.7f,
        .minDelay   = 0,
        .reserved   = {}
    },
    
    { 
        .name       = "Goldfish Temperature sensor",
        .vendor     = "The Android Open Source Project",
        .version    = 1,
        .handle     = ID_TEMPERATURE,
        .type       = SENSOR_TYPE_TEMPERATURE,
        .maxRange   = 80.0f,
        .resolution = 1.0f,
        .power      = 0.0f,
        .minDelay   = 0,
        .reserved   = {}
    },

    { 
        .name       = "Goldfish Gyroscope sensor",
        .vendor     = "The Android Open Source Project",
        .version    = 1,
        .handle     = ID_GYROSCOPE,
        .type       = SENSOR_TYPE_GYROSCOPE,
        .maxRange   = 600.0f,
        .resolution = 1.0f,
        .power      = 6.0f,
        .minDelay   = 0,
        .reserved   = {}
    },

    { 
        .name       = "Goldfish Gravity sensor",
        .vendor     = "The Android Open Source Project",
        .version    = 1,
        .handle     = ID_GRAVITY,
        .type       = SENSOR_TYPE_GRAVITY,
        .maxRange   = GRAVITY_EARTH * 2,
        .resolution = 1.0f,
        .power      = 6.0f,
        .minDelay   = 0,
        .reserved   = {}
    },

    { 
        .name       = "Goldfish Linear acceleration sensor",
        .vendor     = "The Android Open Source Project",
        .version    = 1,
        .handle     = ID_LINEAR_ACCELERATION,
        .type       = SENSOR_TYPE_LINEAR_ACCELERATION,
        .maxRange   = 100.0f,
        .resolution = 1.0f,
        .power      = 6.0f,
        .minDelay   = 0,
        .reserved   = {}
    },

    { 
        .name       = "Goldfish Rotation vector sensor",
        .vendor     = "The Android Open Source Project",
        .version    = 1,
        .handle     = ID_ROTATION_VECTOR,
        .type       = SENSOR_TYPE_ROTATION_VECTOR,
        .maxRange   = 1,
        .resolution = 1.0f / (1<<24),
        .power      = 0,
        .minDelay   = 0,
        .reserved   = {}
    },
};

static int sensor__close(struct hw_device_t *dev)
{
    sensors_poll_context* poll_ctx = (void*)dev;
    
    LOG_D("sensor__close poll_ctx->fd: %d", poll_ctx->fd);
    
    close(poll_ctx->fd);
    free(poll_ctx);
    return 0;
}

static int sensor__activate(struct sensors_poll_device_t *dev,
        int handle, int enabled)
{
    sensors_poll_context*    poll_ctx = (void*)dev;

    LOG_D("sensor__activate handle: %d, enabled: %d", handle, enabled);

    LOG_D("sensor__activate poll_ctx->fd: %d", poll_ctx->fd);
    if (poll_ctx->fd < 0) {
        poll_ctx->fd = MakeSocketIn();
    }
            
    return 0;
}


static int sensor__setDelay(struct sensors_poll_device_t *dev, int handle, int64_t ns)
{
    return 0;
}

static int pick_sensor(sensors_poll_context* poll_ctx, sensors_event_t* data)
{
    uint32_t mask = SUPPORTED_SENSORS;
    
    LOG_D("pick_sensor");
    
    if (poll_ctx->pendingSensors != 0xffffffff)
    {
        *data = poll_ctx->sensors[poll_ctx->pendingSensors];
        LOG_D("data->sensor: %d, data->type: %d, data->timestamp: %lld(0x%x)", data->sensor, data->type, data->timestamp, data->timestamp);
        poll_ctx->pendingSensors = 0xffffffff;
        return 1;
    }

    // we may end-up in a busy loop, slow things down, just in case.
    usleep(100000);
    return -1;
}

static void sGetProcessName(int pid, char *buffer)
{
    int fd;
    
    sprintf(buffer, "/proc/%d/cmdline", pid);
    fd = open(buffer, O_RDONLY);
    if (fd < 0) {
        strcpy(buffer, "???");
    } else {
        int length = read(fd, buffer, PATH_MAX - 1);
        buffer[length] = 0;
        close(fd);
    }
}

int getintL(  char* pstr, int len )
{
    int temp =0;
    if( 2 == len )
    {
        temp =(int)(pstr[0]&0x000000FF);
        temp |=(int)((pstr[1]&0x000000FF)<<8);
        return temp;
    }
    
    temp =(int)(pstr[0]&0x000000FF);
    temp |=(int)((pstr[1]&0x000000FF)<<8);
    temp |=(int)((pstr[2]&0x000000FF)<<16);
    temp |=(int)((pstr[3]&&0x000000FF)<<24);

    return temp;    
}

float floatmySwitch( char* pstr)
{
    return *(float *)pstr;
}

/* return the current time in nanoseconds */
static int64_t data__now_ns(void)
{
    struct timespec  ts;
    
    clock_gettime(CLOCK_MONOTONIC, &ts);
    
    return (int64_t)ts.tv_sec * 1000000000 + ts.tv_nsec;
}

static int sensor__poll(struct sensors_poll_device_t *dev, sensors_event_t* data, int count)
{
    sensors_poll_context* poll_ctx = (void*)dev;
    
    LOG_D("entry sensor__poll count: %d", count);
    
    // there are pending sensors, returns them now...
    LOG_D("sensor__poll poll_ctx->pendingSensors: %d", poll_ctx->pendingSensors); 
    if (poll_ctx->pendingSensors != 0xffffffff) {
        return pick_sensor(poll_ctx, data);
    }
    
    // wait until we get a complete event for an enabled sensor
    while (1) {
        /* read the next event */
        char     buff[512],namebuf[256];
        int      len = RecvSensorsData(poll_ctx->fd, buff, sizeof(buff) - 1);
        float     params[4];
        int64_t  event_time;
        int data_type=0,sensors_type=0,accuracy=0;
        
        LOG_D("sensor__poll len: %d", len);
        
        if (len < 0) continue;
        
        buff[len] = 0;
        
        /* "wake" is sent from the emulator to exit this loop. This shall
        * really be because another thread called "control__wake" in this
        * process.
        */
        if (!strcmp((const char*)poll_ctx, "wake")) {
            LOG_D("sensor__poll wake");
            return 0x7FFFFFFF;
        }
        
        sGetProcessName( getpid(), namebuf);
        
        if( NULL != strstr(namebuf,"system_server") )
        {
            LOG_D("sensor__poll system_server");
            //return 0x7FFFFFFF;
            LOG_D("sensor__poll andy");
        }



        {
            char *pcDataTmp = buff;
            int nDataLen = len;
            int dataType, sensorType, accuracyTmp;
            char dataTypeStr[12], sensorTypeStr[12], accuracyStr[12];
            char *str1 = NULL;
            char *str2 = NULL;

            LOG_D("sensor__poll pcDataTmp = %s, nDataLen = %d\n", pcDataTmp, nDataLen);

            memset(params, 0, sizeof(params));
            
            str1 = pcDataTmp + strlen("datatype=");
            if (str1 - pcDataTmp >= nDataLen) continue;
            str2 = strstr(str1, "&sensortype=");
            if (str2 == NULL) continue;
            memset(dataTypeStr, 0, sizeof(dataTypeStr));
            memcpy((void*)dataTypeStr, str1, str2 - str1);
            dataType = atoi(dataTypeStr);
            LOG_D("dataType = %d\n", dataType);

            data_type = dataType;
            
            str1 = str2 + strlen("&sensortype=");
            if (str1 - pcDataTmp >= nDataLen) continue;
            str2 = strstr(str1, "&accuracy=");
            if (str2 == NULL) continue;
            memset(sensorTypeStr, 0, sizeof(sensorTypeStr));
            memcpy((void*)sensorTypeStr, str1, str2 - str1);
            sensorType = atoi(sensorTypeStr);
            LOG_D("sensorType = %d\n", sensorType);

            sensors_type = sensorType;
            
            if (sensorType == SENSOR_TYPE_ACCELERATION || sensorType == SENSOR_TYPE_GYROSCOPE
                || sensorType == SENSOR_TYPE_GRAVITY || sensorType == SENSOR_TYPE_LINEAR_ACCELERATION
                || sensorType == SENSOR_TYPE_ROTATION_VECTOR) {
                float x, y, z, value;
                char xStr[12], yStr[12], zStr[12], valueStr[12];
                
                str1 = str2 + strlen("&accuracy=");
                if (str1 - pcDataTmp >= nDataLen) continue;
                str2 = strstr(str1, "&x=");
                if (str2 == NULL) continue;
                memset(accuracyStr, 0, sizeof(accuracyStr));
                memcpy((void*)accuracyStr, str1, str2 - str1);
                accuracyTmp = atoi(accuracyStr);
                LOG_D("accuracy = %d\n", accuracyTmp);
            
                str1 = str2 + strlen("&x=");
                if (str1 - pcDataTmp >= nDataLen) continue;
                str2 = strstr(str1, "&y=");
                if (str2 == NULL) continue;
                memset(xStr, 0, sizeof(xStr));
                memcpy((void*)xStr, str1, str2 - str1);
                x = atof(xStr);
                LOG_D("x = %f\n", x);
            
                str1 = str2 + strlen("&y=");
                if (str1 - pcDataTmp >= nDataLen) continue;
                str2 = strstr(str1, "&z=");
                if (str2 == NULL) continue;
                memset(yStr, 0, sizeof(yStr));
                memcpy((void*)yStr, str1, str2 - str1);
                y = atof(yStr);
                LOG_D("y = %f\n", y);

                str1 = str2 + strlen("&z=");
                if (str1 - pcDataTmp >= nDataLen) continue;
                memset(zStr, 0, sizeof(zStr));
                if ((str2 = strstr(str1, "&value=")) == NULL) {
                    strcpy((void*)zStr, str1);
                    z = atof(zStr);
                    LOG_D("z = %f\n", z);
                } else {
                    memcpy((void*)zStr, str1, str2 - str1);
                    z = atof(zStr);
                    LOG_D("z = %f\n", z);

                     str1 = str2 + strlen("&value=");
                     if (str1 - pcDataTmp >= nDataLen) continue;
                     memset(valueStr, 0, sizeof(valueStr));
                     strcpy((void*)valueStr, str1);
                     value = atof(valueStr);
                     LOG_D("value = %f\n", value);

                }
                
                params[0] = x;
                params[1] = y;
                params[2] = z;
                params[3] = value;
            } else if (sensorType == SENSOR_TYPE_ORIENTATION) {
                float azimuth, pitch, roll;
                char azimuthStr[12], pitchStr[12], rollStr[12];
                
                str1 = str2 + strlen("&accuracy=");
                if (str1 - pcDataTmp >= nDataLen) continue;
                str2 = strstr(str1, "&azimuth=");
                if (str2 == NULL) continue;
                memset(accuracyStr, 0, sizeof(accuracyStr));
                memcpy((void*)accuracyStr, str1, str2 - str1);
                accuracyTmp = atoi(accuracyStr);
                LOG_D("accuracy = %d\n", accuracyTmp);
            
                str1 = str2 + strlen("&azimuth=");
                if (str1 - pcDataTmp >= nDataLen) continue;
                str2 = strstr(str1, "&pitch=");
                if (str2 == NULL) continue;
                memset(azimuthStr, 0, sizeof(azimuthStr));
                memcpy((void*)azimuthStr, str1, str2 - str1);
                azimuth = atof(azimuthStr);
                LOG_D("azimuth = %f\n", azimuth);
            
                str1 = str2 + strlen("&pitch=");
                if (str1 - pcDataTmp >= nDataLen) continue;
                str2 = strstr(str1, "&roll=");
                if (str2 == NULL) continue;
                memset(pitchStr, 0, sizeof(pitchStr));
                memcpy((void*)pitchStr, str1, str2 - str1);
                pitch = atof(pitchStr);
                LOG_D("pitch = %f\n", pitch);
            
                str1 = str2 + strlen("&roll=");
                if (str1 - pcDataTmp >= nDataLen) continue;
                memset(rollStr, 0, sizeof(rollStr));
                strcpy((void*)rollStr, str1);
                roll = atof(rollStr);
                LOG_D("roll = %f\n", roll);    
                
                params[0] = azimuth;
                params[1] = pitch;
                params[2] = roll;

            }
            
            accuracy = accuracyTmp;
        }
                
        LOG_D("sensor__poll data_type: %d, sensors_type: %d, accuracy: %d", data_type, sensors_type, accuracy);
        LOG_D("sensor__poll params[0]: %f, params[1]: %f, params[2]: %f", params[0], params[1], params[2]);

        switch (sensors_type)
        {
            case SENSOR_TYPE_ACCELERATION:
                poll_ctx->pendingSensors = ID_ACCELERATION;
                break;
            case SENSOR_TYPE_ORIENTATION:
                poll_ctx->pendingSensors = ID_ORIENTATION;
                break;
            case SENSOR_TYPE_GYROSCOPE:
                poll_ctx->pendingSensors = ID_GYROSCOPE;
                break;
            case SENSOR_TYPE_GRAVITY:
                poll_ctx->pendingSensors = ID_GRAVITY;
                break;
            case SENSOR_TYPE_LINEAR_ACCELERATION:
                poll_ctx->pendingSensors = ID_LINEAR_ACCELERATION;
                break;
            case SENSOR_TYPE_ROTATION_VECTOR:
                poll_ctx->pendingSensors = ID_ROTATION_VECTOR;
                break;
            default:
                break;
        }

        poll_ctx->sensors[poll_ctx->pendingSensors].data[0] = params[0];
        poll_ctx->sensors[poll_ctx->pendingSensors].data[1] = params[1];
        poll_ctx->sensors[poll_ctx->pendingSensors].data[2] = params[2];
        poll_ctx->sensors[poll_ctx->pendingSensors].data[3] = params[3];

        LOG_D("sensor__poll poll_ctx->pendingSensors: %d",poll_ctx->pendingSensors);
        if (poll_ctx->pendingSensors != 0xffffffff) {
            int64_t t = 0;//event_time * 1000LL;  /* convert to nano-seconds */
            
            t =data__now_ns();
            poll_ctx->sensors[poll_ctx->pendingSensors].timestamp = t;
            poll_ctx->sensors[poll_ctx->pendingSensors].sensor = poll_ctx->pendingSensors;
            
            return pick_sensor(poll_ctx, data);
        } else {
            LOG_D("huh ? sync without any sensor data ?");
        }
    }
}

static int open_sensors(const struct hw_module_t* module, const char* name, struct hw_device_t** device)
{
    int status = -EINVAL;
    sensors_poll_context *poll = malloc(sizeof(sensors_poll_context));
    
    LOG_D("open_sensors name: %s", name);
    memset(poll, 0, sizeof(sensors_poll_context));
    
    poll->device.common.tag       = HARDWARE_DEVICE_TAG;
    poll->device.common.version = 0;
    poll->device.common.module  = (struct hw_module_t*) module;
    poll->device.common.close   = sensor__close;
    poll->device.activate        = sensor__activate;
    poll->device.setDelay        = sensor__setDelay;
    poll->device.poll            = sensor__poll;
    
    poll->fd = MakeSocketIn();
    LOG_D("open_sensors poll->fd: %d", poll->fd);
    poll->pendingSensors = 0xffffffff;
    
    *device = &poll->device.common;
    status    = 0;
    
    return status;
}

static int sensors__get_sensors_list(struct sensors_module_t* module,
        struct sensor_t const** list)
{
    LOG_D("sensors__get_sensors_list");
    *list = sSensorList;
    return sizeof(sSensorList)/sizeof(sSensorList[0]);
}

static struct hw_module_methods_t sensors_module_methods = {
    .open = open_sensors
};

const struct sensors_module_t HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .version_major = 1,
        .version_minor = 0,
        .id = SENSORS_HARDWARE_MODULE_ID,
        .name = "Amber3 SENSORS Module",
        .author = "The Android Open Source Project",
        .methods = &sensors_module_methods,
    },
    .get_sensors_list = sensors__get_sensors_list
};


