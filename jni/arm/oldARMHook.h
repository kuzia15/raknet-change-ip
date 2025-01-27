#pragma once

#define SET_TO(__a1, __a2) *(void**)&(__a1) = (void*)(__a2)

class ARMHook
{
public:
	static uintptr_t getLibraryAddress(const char* library);
	static void InitialiseTrampolines(uintptr_t dest, uintptr_t size);
	static void unprotect(uintptr_t ptr);
	static void writeMem(uintptr_t dest, uintptr_t src, size_t size);
	static void readMem(uintptr_t dest, uintptr_t src, size_t size);
	static void makeNOP(uintptr_t addr, unsigned int count);
	static void writeMemHookProc(uintptr_t addr, uintptr_t func);
	static void JMPCode(uintptr_t func, uintptr_t addr);
	static void installHook(uintptr_t addr, uintptr_t func, uintptr_t *orig);
	static void installMethodHook(uintptr_t addr, uintptr_t func);
	static void makeRet(uintptr_t dest);
	static void putCode(uintptr_t addr, uintptr_t point, uintptr_t func);
	static void injectCode(uintptr_t addr, uintptr_t func, int reg);
};