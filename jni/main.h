#pragma once 

#include <jni.h>
#include <android/log.h>
#include <cstdlib>
#include <string>
#include <vector>
#include <list>
#include <unistd.h>
#include <algorithm>
#include <cmath>
#include <iostream>

#include <vendor/raknet/RakClient.h>
#include <vendor/raknet/RakNetworkFactory.h>
#include <vendor/raknet/PacketEnumerations.h>
#include <vendor/raknet/StringCompressor.h>
#include "vendor/raknet/types.h"

#define RAKSAMP_CLIENT
#define NETCODE_CONNCOOKIELULZ 0x6969

#include "vendor/raknet/SAMP/samp_netencr.h"
#include "vendor/raknet/SAMP/SAMPRPC.h"

#include "arm/oldARMHook.h"
#include "arm/armhook.h"
#include "arm/patch.h"
#include "arm/util.h"
#include "str_obfuscate.hpp"

extern uintptr_t g_libGTASA;
extern uintptr_t g_libSAMP;
#define SA_ADDR(addr) (g_libGTASA + (addr))

extern const char* g_pAPKPackage;
extern JavaVM *mVm;
extern JNIEnv *mEnv;

uint32_t GetTickCount();
void SetFunc(uintptr_t addr, uintptr_t *orig);