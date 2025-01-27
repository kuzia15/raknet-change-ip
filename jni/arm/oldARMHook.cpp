#include "../main.h"
#include "oldARMHook.h"
#include <sys/mman.h>

#define HOOK_PROC_ARM OBFUSCATE("\x01\xB4\x01\xB4\x01\x48\x01\x90\x01\xBD\x00\xBF\x00\x00\x00\x00")

// --------------------------------------------------------------------------------------------
uintptr_t arm_mmap_start 	= 0;
uintptr_t arm_mmap_end		= 0;
uintptr_t local_trampoline	= 0;
uintptr_t remote_trampoline	= 0;
// --------------------------------------------------------------------------------------------

uintptr_t ARMHook::getLibraryAddress(const char* library)
{
    char filename[0xFF] = {0},
    buffer[2048] = {0};
    FILE *fp = 0;
    uintptr_t address = 0;

    sprintf(filename, OBFUSCATE("/proc/%d/maps"), getpid());

    fp = fopen(filename, "rt");

    if(fp == 0) goto done;

    while(fgets(buffer, sizeof(buffer), fp))
    {
        if(strstr(buffer, library))
        {
            address = (uintptr_t)strtoul(buffer, 0, 16);
            break;
        }
    }

    done:

    if(fp)
      fclose(fp);

    return address;
}

void ARMHook::InitialiseTrampolines(uintptr_t dest, uintptr_t size)
{   
	local_trampoline   = dest;
	remote_trampoline  = local_trampoline + size;

	arm_mmap_start = (uintptr_t)mmap(0, PAGE_SIZE, PROT_WRITE | PROT_READ | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	mprotect((void*)(arm_mmap_start & 0xFFFFF000), PAGE_SIZE, PROT_READ | PROT_EXEC | PROT_WRITE);
	arm_mmap_end = (arm_mmap_start + PAGE_SIZE);
}

void ARMHook::unprotect(uintptr_t ptr)
{
	mprotect((void*)(ptr & 0xFFFFF000), PAGE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC);
}

void ARMHook::writeMem(uintptr_t dest, uintptr_t src, size_t size)
{
	ARMHook::unprotect(dest);
	memcpy((void*)dest, (void*)src, size);
	cacheflush(dest, dest+size, 0);
}

void ARMHook::makeRet(uintptr_t dest)
{
    ARMHook::writeMem(dest, (uintptr_t)"\xF7\x46", 2);
}

void ARMHook::readMem(uintptr_t dest, uintptr_t src, size_t size)
{
	ARMHook::unprotect(src);
    memcpy((void*)dest, (void*)src, size);
}

void ARMHook::makeNOP(uintptr_t addr, unsigned int count)
{
	ARMHook::unprotect(addr);

    for(uintptr_t ptr = addr; ptr != (addr+(count*2)); ptr += 2)
    {
        *(char*)ptr = 0x00;
        *(char*)(ptr+1) = 0x46;
    }
}

void ARMHook::writeMemHookProc(uintptr_t addr, uintptr_t func)
{
    char code[16];
    memcpy(code, HOOK_PROC_ARM, 16);
    *(uint32_t*)&code[12] = (func | 1);
    ARMHook::writeMem(addr, (uintptr_t)code, 16);
}

void ARMHook::JMPCode(uintptr_t func, uintptr_t addr)
{
	uint32_t code = ((addr-func-4) >> 12) & 0x7FF | 0xF000 | ((((addr-func-4) >> 1) & 0x7FF | 0xB800) << 16);
    ARMHook::writeMem(func, (uintptr_t)&code, 4);
}

void ARMHook::installHook(uintptr_t addr, uintptr_t func, uintptr_t *orig)
{
    if(remote_trampoline < (local_trampoline + 0x10) || arm_mmap_end < (arm_mmap_start + 0x20))
        return std::terminate();

    ARMHook::readMem(arm_mmap_start, addr, 4);
    ARMHook::writeMemHookProc(arm_mmap_start + 4, addr+4);
    *orig = arm_mmap_start + 1;
    arm_mmap_start += 32;

    ARMHook::JMPCode(addr, local_trampoline);
    ARMHook::writeMemHookProc(local_trampoline, func);
    local_trampoline += 16;
}

void ARMHook::installMethodHook(uintptr_t addr, uintptr_t func)
{
    ARMHook::unprotect(addr);
    *(uintptr_t*)addr = func;
}

void ARMHook::putCode(uintptr_t addr, uintptr_t point, uintptr_t func)
{
    ARMHook::unprotect(addr+point);
    *(uintptr_t*)(addr+point) = func;
}

void ARMHook::injectCode(uintptr_t addr, uintptr_t func, int reg)
{
    char injectCode[12];

    injectCode[0] = 0x01;
    injectCode[1] = 0xA0 + reg;
    injectCode[2] = (0x08 * reg) + reg;
    injectCode[3] = 0x68;
    injectCode[4] = 0x87 + (0x08 * reg);
    injectCode[5] = 0x46;
    injectCode[6] = injectCode[4];
    injectCode[7] = injectCode[5];
    
    *(uintptr_t*)&injectCode[8] = func;

    ARMHook::writeMem(addr, (uintptr_t)injectCode, 12);
}