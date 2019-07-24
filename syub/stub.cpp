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
		size_t Orig_Size = aPsafe_get_orig_size(Text);

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

	// �������

	//���ڻص�����
	LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		DEFAPI("Kernel32.dll", GetModuleHandleW);
		DEFAPI("user32.dll", CreateWindowExW);
		DEFAPI("user32.dll", PostQuitMessage);
		DEFAPI("user32.dll", DefWindowProcW);
		DEFAPI("user32.dll", GetWindowTextA);
		DEFAPI("user32.dll", MessageBoxA);
		DEFAPI("user32.dll", GetDlgItem);
		DEFAPI("user32.dll", ShowWindow);
		
		//��Ϣ����
		switch (msg)
		{
			//���ڱ�����
		case WM_CREATE:
		{   
			// �������������
			My_CreateWindowExW(WS_EX_CLIENTEDGE,L"EDIT", NULL,
				WS_CHILD | WS_VISIBLE | WS_BORDER,50, 20,120, 30,
				hwnd,(HMENU)0x1001,NULL,NULL);

			// ������ť
			My_CreateWindowExW(0,L"button", L"ȷ��", WS_CHILD | WS_VISIBLE,
				70, 60,60, 20,hwnd,(HMENU)0x1002,NULL,NULL);

			return 0;
		}
			// ��Ӧ��׼�ؼ���Ϣ
		case WM_COMMAND:
		{
			// �ؼ�ID
			DWORD ID = LOWORD(wParam);

			// ��Ϣ֪ͨ��
			DWORD code = HIWORD(wParam);
			
			if (ID == 0x1002 && code == BN_CLICKED)
			{
				// ��ȡ�������
				HWND hEdit = My_GetDlgItem(hwnd, 0x1001);
				
				// ���������
				char buff[5]={};
				char str[5] = { '1','5','p','b','\0' };
				DWORD flag = 0;

				// �ӿռ��ȡ����
				My_GetWindowTextA(hEdit, buff, 5);

				// ����������֤
				__asm
				{
					pushad
					mov ecx, 0x4
					lea edi, str
					lea esi, buff
					cld
					repe cmpsb
					jnz No
					mov flag, 1
					jmp End
				No :
					mov flag, 0
				End :
					popad
				}

				if (flag)
				{
					// ������ȷ�رյ���
					My_ShowWindow(hwnd, SW_HIDE);
					My_PostQuitMessage(0);
					return 0;
				}
				else
				{
					My_MessageBoxA(0, "�������!", 0, MB_OK);
				}
			}
			break;
		}
		//��������
		case WM_DESTROY:
		{
			// ֱ�ӽ�������
			DEFAPI("kernel32.dll", ExitProcess);
			My_ExitProcess(0);
		}
		}
		//��Windows��Ĭ�ϵķ�ʽ������û�д������Ϣ��	
		return My_DefWindowProcW(hwnd, msg, wParam, lParam);
	}

	//����������֤��
	void UserCheck()
	{
		DEFAPI("Kernel32.dll", GetModuleHandleW);
		DEFAPI("user32.dll", RegisterClassW);
		DEFAPI("gdi32.dll", GetStockObject);
		DEFAPI("user32.dll", CreateWindowExW);
		DEFAPI("user32.dll", ShowWindow);
		DEFAPI("user32.dll", UpdateWindow);
		DEFAPI("user32.dll", GetMessageW);
		DEFAPI("user32.dll", TranslateMessage);
		DEFAPI("user32.dll", DispatchMessageW);


		// 1.���һ�������ࣨΪ������ĸ����ֶθ�ֵ��
		WNDCLASS wc;//����һ��������
		static TCHAR szClassName[] = TEXT("����һ����");//��������

		wc.style = CS_HREDRAW | CS_VREDRAW;//������ķ��һ��Ϊ��������񣬱�ʾ��������ʱ�ػ洰�ڣ�
		wc.cbClsExtra = 0;//���ɸ����������չ���ֽ����������ڴ棩
		wc.cbWndExtra = 0;//���ɸ�����ʵ������չ���ֽ����������ڴ棩
		wc.hIcon = 0;//����ͼ��
		wc.hCursor = 0;//�����ʽ
		wc.hbrBackground = (HBRUSH)My_GetStockObject(WHITE_BRUSH);//���ڱ�����ˢ
		wc.lpszMenuName = NULL;//���ڲ˵�
		wc.hInstance = My_GetModuleHandleW(NULL);//��ǰ���ھ��
		wc.lpfnWndProc = WndProc;//ָ�򴰿ڹ��̵�ָ��(��Ҫ!!����)	
		wc.lpszClassName = szClassName;//������������Ҫ!!���

		// 2.ע�ᴰ��
		My_RegisterClassW(&wc);

		// 3.��������
		HWND hwnd;//����һ�����ھ��
		hwnd = My_CreateWindowExW(
			0,//������չ���
			szClassName,//�����������
			TEXT("����"),//���ڱ���
			WS_OVERLAPPEDWINDOW,//���ڷ��
			400,//��ʼ��ʱx���λ��
			400,//��ʼ��ʱy���λ��
			240,//���ڿ��
			150,//���ڸ߶�
			NULL,//�����ھ��
			NULL,//���ڲ˵����
			My_GetModuleHandleW(NULL),//��ǰ���ڵľ��
			NULL//Ϊ���ڸ��Ӳ�����Ϣ
		);

		// 4.��ʾ���ڡ����»��ƴ���	
		My_ShowWindow(hwnd, SW_SHOW);
		My_UpdateWindow(hwnd);

		// 5.��Ϣѭ��
		MSG msg;//����һ����Ϣ
		//GetMessage���������ȡ��WM_QUIT��Ϣ��ʱ�򣬻᷵��false
		while (My_GetMessageW(&msg, NULL, 0, 0))//NULL��ʾ�������д��ڵ���Ϣ����������0��ʾ�������е���Ϣ
		{
			My_TranslateMessage(&msg);
			My_DispatchMessageW(&msg);//������Ϣ�����ڹ���
		}
	}


	_declspec(dllexport)
		void _declspec(naked) start() {

		// ��ʼ��������Ҫ�����ĵ�ַ
		getApis();

		UserCheck();
		// ��ѹ��
		DeCompress();

		// ����
		Decode();
		
		g_conf.oep+= GetBaseAddress();
		_asm jmp g_conf.oep;
	}


}
