﻿#pragma once
#include <Windows.h>

template<typename T>
T readMemory(DWORD address)
{
	return *((T*)address);
}

template<typename T>
T* pointMemory(DWORD address)
{
	return ((T*)address);
}

template<typename T>
void writeMemory(DWORD address, T value)
{
	*((T*)address) = value;
}

template<typename T>
DWORD protectMemory(DWORD address, DWORD prot)
{
	DWORD oldProt;
	VirtualProtect((LPVOID)address, sizeof(T), prot, &oldProt);
	return oldProt;
}

DWORD getVF(DWORD classInst, DWORD funcIndex)
{
	DWORD VFTable = readMemory<DWORD>(classInst);
	DWORD hookAddress = VFTable + funcIndex * sizeof(DWORD);
	return readMemory<DWORD>(hookAddress);
}

DWORD hookVF(DWORD classInst, DWORD funcIndex, DWORD newFunc)
{
	DWORD VFTable = readMemory<DWORD>(classInst);
	DWORD hookAddress = VFTable + funcIndex * sizeof(DWORD);

	auto oldProtection = protectMemory<DWORD>(hookAddress, PAGE_READWRITE);
	DWORD originalFunc = readMemory<DWORD>(hookAddress);
	writeMemory<DWORD>(hookAddress, newFunc);
	protectMemory<DWORD>(hookAddress, oldProtection);

	return originalFunc;
}

unsigned char* hookWithJump(DWORD hookAt, DWORD newFunc)
{
	DWORD newOffset = newFunc - hookAt - 5;

	auto oldProtection = protectMemory<BYTE[5]>(hookAt, PAGE_EXECUTE_READWRITE);
	
	unsigned char* originals = new unsigned char[5];
	for (unsigned int i = 0; i < 5; i++)
		originals[i] = readMemory<unsigned char>(hookAt + i);

	writeMemory<BYTE>(hookAt, 0xE9);
	writeMemory<DWORD>(hookAt + 1, newOffset);

	protectMemory<BYTE[5]>(hookAt, oldProtection);
	return originals;
}

void unhookWithJump(DWORD hookAt, unsigned char* originals)
{
	auto oldProtection = protectMemory<BYTE[5]>(hookAt, PAGE_EXECUTE_READWRITE);
	for (unsigned int i = 0; i < 5; i++)
		writeMemory<BYTE>(hookAt + i, originals[i]);
	protectMemory<BYTE[5]>(hookAt + 1, oldProtection);

	delete [] originals;
}
