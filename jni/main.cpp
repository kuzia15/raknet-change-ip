#include "main.h"
#include "jniutil.h"

uintptr_t g_libGTASA = 0;
uintptr_t g_libSAMP = 0;
const char* g_pAPKPackage;

jobject appContext;
JavaVM *mVm;
JNIEnv *mEnv;

#define CUSTOM_RPC 251
#define PACKET_HELLOWORLD 1

void Packet_CustomRPC(Packet* p)
{
    RakNet::BitStream bs((unsigned char*)p->data, p->length, false);
	
    uint8_t packetID;
    uint32_t rpcID;
    bs.Read(packetID);
    bs.Read(rpcID);

    switch (rpcID)
    {
        case PACKET_HELLOWORLD:
        {
            toasty(OBFUSCATE("[WNRPC][ID: %s] Êàñòîìíûé ïàêåò äîñòàâëåí"), PACKET_HELLOWORLD);
            break;
        }
        default:
            break;
    }
    return;
}                           

int (*CNetGame__UpdateNetwork)(uintptr_t *data);
int CNetGame__UpdateNetwork_hook(uintptr_t *data)
{
    uint8_t *buffer; 
    int packetID;

    if(!data || !*data) {
        return 0;
    }

    Packet *pkt; 
    pkt = ((RakClientInterface*)(*data))->Receive();

    if (pkt) {
        while (1) {
            buffer = pkt->data;
            packetID = buffer[0];
            if (packetID == 40)
                packetID = buffer[5];
            
            switch (packetID) {
                case CUSTOM_RPC:
                    Packet_CustomRPC(pkt);

                    ((RakClientInterface*)(*data))->DeallocatePacket(pkt);
					break;
            }
        }
	}

    CNetGame__UpdateNetwork(data);
}

void initSamp() 
{
    CHook::InlineHook(g_libSAMP, 0x0, (uintptr_t)CNetGame__UpdateNetwork_hook, (uintptr_t*)&CNetGame__UpdateNetwork);
}

void Main() 
{
     // code for GTASA
}

void *InitialiseThread(void *p)
{
	Main();
	pthread_exit(0);
}          

JNIEnv *getEnv() 
{
	JNIEnv* env = nullptr;
    JavaVM* javaVM = mVm;
	int getEnvStat = javaVM->GetEnv((void**)& env, JNI_VERSION_1_6);

	if (getEnvStat == JNI_EDETACHED)
		if (javaVM->AttachCurrentThread(&env, NULL) != 0)
		  return nullptr;

	if (getEnvStat == JNI_EVERSION)
	    return nullptr;

	if (getEnvStat == JNI_ERR)
	   return nullptr;

	return env;
}

jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
    mVm = vm;
    mEnv = getEnv();

	appContext = GetGlobalActivity(mEnv);
    if(appContext != NULL) 
	{ 
        g_pAPKPackage = mEnv->GetStringUTFChars(GetPackageName(mEnv, appContext), NULL);
		toasty(OBFUSCATE("WNRPC"));

        char sea_of_feelings[100+1];
		sprintf(sea_of_feelings, OBFUSCATE("Package: %s"), g_pAPKPackage);

		__android_log_write(ANDROID_LOG_INFO, OBFUSCATE("WNPath"), OBFUSCATE("Powered by Weikton"));
		__android_log_write(ANDROID_LOG_INFO, OBFUSCATE("WNPath"), sea_of_feelings);
    }
    
	g_libGTASA = ARMHook::getLibraryAddress(OBFUSCATE("libGTASA.so"));
	if(g_libGTASA)
	{
		srand(time(0));

		uintptr_t memlib_start = (g_libGTASA + 0x174D4);
		uintptr_t size = 0x1234A;
		
		ARMHook::InitialiseTrampolines(memlib_start, size);

		pthread_t thread;
		pthread_create(&thread, 0, InitialiseThread, 0);
	}

    g_libSAMP = FindLibrary(OBFUSCATE("libsamp.so"));
    if(g_libSAMP) initSamp();

	return JNI_VERSION_1_6;
}

uint32_t GetTickCount()
{
	struct timeval tv;
	gettimeofday(&tv, nullptr);

	return (tv.tv_sec*1000 + tv.tv_usec/1000);
}
