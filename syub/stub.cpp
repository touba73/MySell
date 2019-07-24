#include"stub.h"

#pragma comment(linker, "/merge:.data=.text") 
#pragma comment(linker, "/merge:.rdata=.text")
#pragma comment(linker, "/section:.text,RWE")
#include "../aplib.h"
#pragma comment(lib,"../aplib.lib")

HMODULE g_hkernel32;

typedef void* (WINAPI* FnGetProcAddress)(HMODULE hModule, LPCSTR lpProcName);
FnGetProcAddress pfnGetProcAddress;

typedef HMODULE(WINAPI* FnLoadLibraryA)(const char*);
FnLoadLibraryA pfnLoadLibraryA;

#define DEFAPI(modName,funType) \
	decltype(funType)* My_##funType = (decltype(funType)*)\
pfnGetProcAddress(pfnLoadLibraryA(modName), #funType);

// ��ȡ��ǰEIP
DWORD Getjizhi()
{
	_asm {
		CALL EIPP
		EIPP :
		POP EAX
	}
}
DWORD GetBaseAddress()
{
	DWORD aaa = Getjizhi();

	return aaa & 0xFFFF0000;
}

extern "C" {

	// �������� ���ӿ��� ��д ��Ҫ�ǽ��� ��OEP
	_declspec(dllexport)StubConf g_conf;

	// ��ȡ Kernel32��ַ
	HMODULE GetKernel32()
	{
		_asm
		{
			MOV EAX, DWORD PTR FS : [0x30]
			MOV EAX, DWORD PTR DS : [EAX + 0xC]
			MOV EAX, DWORD PTR DS : [EAX + 0xC]
			MOV EAX, DWORD PTR DS : [EAX]
			MOV EAX, DWORD PTR DS : [EAX]
			MOV EAX, DWORD PTR DS : [EAX + 0x18]
		}
	}

	//��ȡGetProcAddress ������ַ
	void* indexGetProcAddress()
	{
		// 1. ��ȡKernel32��ַ
		HMODULE hKernel32 = GetKernel32();

		IMAGE_DOS_HEADER* pDos = (IMAGE_DOS_HEADER*)hKernel32;
		IMAGE_NT_HEADERS* pNt = (IMAGE_NT_HEADERS*)
			(pDos->e_lfanew + (DWORD)pDos);

		IMAGE_EXPORT_DIRECTORY* pExp = (IMAGE_EXPORT_DIRECTORY*)
			(pNt->OptionalHeader.DataDirectory[0].VirtualAddress + (DWORD)pDos);

		DWORD* pEnt = (DWORD*)(pExp->AddressOfNames + (char*)pDos);
		DWORD* pEat = (DWORD*)(pExp->AddressOfFunctions + (char*)pDos);
		WORD* pEot = (WORD*)(pExp->AddressOfNameOrdinals + (char*)pDos);

		for (int i = 0; i < pExp->NumberOfFunctions; ++i)
		{
			// �������Ʊ��ҵ�GetP rocA ddre ss
			char* name = pEnt[i] + (char*)pDos;
			if (*(DWORD*)name == 'PteG'
				&& *(DWORD*)(name + 4) == 'Acor'
				&& *(DWORD*)(name + 8) == 'erdd'
				&& *(WORD*)(name + 12) == 'ss')
			{
				DWORD addrIndex = pEot[i];
				return pEat[addrIndex] + (char*)pDos;
			}

		}
		return NULL;
	}
	// ��ȡAPI�������ؼ�����
	void getApis()
	{
		g_hkernel32 = GetKernel32();
		pfnGetProcAddress = (FnGetProcAddress)
			(indexGetProcAddress());
		pfnLoadLibraryA = (FnLoadLibraryA)
			(pfnGetProcAddress(g_hkernel32, "LoadLibraryA"));
	}

	// ���ܺ���
	void Decode()
	{
		DEFAPI("kernel32.dll", VirtualProtect);

		unsigned char* text_buff = (unsigned char*)( g_conf.encrypt_rva+ GetBaseAddress());
		// �޸��ڴ��ҳ����Ϊ�ɶ���д
		DWORD old;
		My_VirtualProtect(text_buff, g_conf.encrypt_size, PAGE_READWRITE, &old);
		for (int i = 0; i < g_conf.encrypt_size; ++i) {
			text_buff[i] ^= g_conf.encrypt_key;
		}
		// �ָ��ڴ��ҳ����
		My_VirtualProtect(text_buff, g_conf.encrypt_size, old, &old);
	}

	// ��ѹ������
	void DeCompress()
	{
		DEFAPI("user32.dll", MessageBoxA);
		DEFAPI("kernel32.dll", ExitProcess);
		DEFAPI("kernel32.dll", VirtualProtect);
		DEFAPI("kernel32.dll", VirtualAlloc);
		DEFAPI("kernel32.dll", VirtualFree);

		//��ȡ�����׵�ַ
		 char* Text = ( char*)(g_conf.encrypt_rva + GetBaseAddress());
		
		// �޸��ڴ��ҳ����Ϊ�ɶ���д
		DWORD old;
		My_VirtualProtect(Text, g_conf.encrypt_size, PAGE_READWRITE, &old);

		//��ȡ��ѹ���Ĵ�С
		size_t Text_Size = g_conf.compress_size;

		// ����ԭʼ��С
		size_t Orig_Size = aPsafe_get_orig_size(Text);//���ԭ���ݵĴ�С

		// �����ڴ�ռ䱣����ܺ������
		//char* data=NULL;
		LPVOID Data=My_VirtualAlloc(0, Orig_Size, 0x1000|0x2000, 0x4);

		g_conf.compress_size = aPsafe_depack(
			Text,     //��ѹ��������
			Text_Size,//��ѹ����Ĵ�С
			Data,     //���ս�ѹ��������
			Orig_Size       //ԭ���Ĵ�С
		);

		// �ѽ��ܺ�����ݿ�����ԭ��ַ
		_asm{
			pushad
			mov esi, Data
			mov edi, Text
			mov ecx, g_conf.compress_size
			cld
			repe movsb
			popad

		}

		My_VirtualFree(Data, Orig_Size,2);
		// �ָ��ڴ��ҳ����
		My_VirtualProtect(Text, g_conf.encrypt_size, old, &old);

		if (g_conf.compress_size != Orig_Size) {
			My_MessageBoxA(0,"��ѹ��ʧ��!","����",0);
			My_ExitProcess(0);
		}else{
			My_MessageBoxA(0, "��ѹ���ɹ�!", "�ɹ�", 0);
		}
		
	}

	_declspec(dllexport)
		void _declspec(naked) start() {

		// ��ʼ��������Ҫ�����ĵ�ַ
		getApis();

		// ��ѹ��
		DeCompress();

		// ����
		Decode();
		
		g_conf.oep+= GetBaseAddress();
		_asm jmp g_conf.oep;
	}


}
