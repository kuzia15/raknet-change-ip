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

void (*CNetGame__CNetGame)(char *a1, const char *a2, int a3, const char *a4, int a5);
void CNetGame__CNetGame_hook(char *a1, const char *a2, int a3, const char *a4, int a5)
{
   Log("a1 -> %s", a1);

   Log("a2 -> %s", a2);
   a2 = WEIKTON("127.0.0.1");
   Log("a2 ->-> %s", a2);

   Log("a3 -> %d", a3);
   a3 = 7777;
   Log("a3 ->-> %d", a3);

   Log("a4 -> %s", a4);
   Log("a5 -> %d", a5);

   CNetGame__CNetGame(a1, a2, a3, a4, a5);
}

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
    ARMHook::installHook(g_libSAMP + 0x0, (uintptr_t)CNetGame__CNetGame_hook, (uintptr_t*)&CNetGame__CNetGame);
    CHook::InlineHook(g_libSAMP, 0x0, (uintptr_t)CNetGame__UpdateNetwork_hook, (uintptr_t*)&CNetGame__UpdateNetwork);
}

void Main() 
{
    const char* thumb = OBFUSCATE("texdb/%s/%s.etc.tmb");
    ARMHook::writeMem(g_libGTASA + 0x573648,(int) thumb, 20);
    ARMHook::writeMem(g_libGTASA + 0x57365C,(int) thumb, 20);
    ARMHook::writeMem(g_libGTASA + 0x573670,(int) thumb, 20);
    ARMHook::writeMem(g_libGTASA + 0x573684,(int) thumb, 20);
    const char* dataoff = OBFUSCATE("texdb/%s/%s.etc");
    ARMHook::writeMem(g_libGTASA + 0x5736AC,(int) dataoff, 16);
    ARMHook::writeMem(g_libGTASA + 0x5736BC,(int) dataoff, 16);
    ARMHook::writeMem(g_libGTASA + 0x5736CC,(int) dataoff, 16);
    ARMHook::writeMem(g_libGTASA + 0x5736DC,(int) dataoff, 16);
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

        char sea_of_feelings[100+1];
		sprintf(sea_of_feelings, OBFUSCATE("Package: %s"), g_pAPKPackage);

		__android_log_write(ANDROID_LOG_INFO, OBFUSCATE("WNPath"), OBFUSCATE("Hello "));
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
