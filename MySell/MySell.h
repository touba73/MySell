#pragma once
#include<Windows.h>
#include "../syub/stub.h"
#include "../aplib.h"
#pragma comment(lib,"../aPlib.lib")

typedef struct _StubInfo {
	PIMAGE_NT_HEADERS Dll_pNT;
	char* Dll_Buff;				// dll���ص��ڴ���׵�ַ(���ػ�ַ)
	DWORD Text_Size;			// ����εĴ�С
	char* Text_Buff;			// ����εĿ�ʼ��ַ
	DWORD  Start_Offset;	// start���������ڴ���εĶ���ƫ��
	StubConf* g_Conf;		// dll������ȫ�ֱ����ĵ�ַ
}StubInfo;

typedef struct _Reloc {
	char* Reloc_Address;	// �ض�λ�������׵�ַ
	DWORD Reloc_Size;		//  �ض�λ���С

}Reloc,*PReloc;

class MySell{
public:
	MySell(const char* FilePath);
	 
	~MySell();

	//�ļ����ݻ���
	char* m_pFile;
	int m_Size;

	//��ȡDOSͷ
	PIMAGE_DOS_HEADER m_pDos=nullptr;

	//����NTͷ
	PIMAGE_NT_HEADERS m_pNT= nullptr;

	// �������������ε�����
	StubInfo m_StubInfo;

	// ���� ѹ���ε��ļ��׵�ַ
	unsigned char* m_Text = NULL;

private:

	// �����С ��������
	DWORD Alignment(DWORD Size,DWORD Grain_Size);
	
	// ����ԭ�������һ������
	PIMAGE_SECTION_HEADER Last_Section();

	//�����������Ʋ�������
	PIMAGE_SECTION_HEADER Scn_by_name(char* buff, const char* section_name);

	// ��ʼ���ض�λ���� ��ȡdll���ض�λ������
	Reloc Init_Reloc();

public:
	// 1.�ж��Ƿ���PE�ļ�
	bool IsPE();

	//2. ��ȡ�����ε���Ϣ
	void GetDllInfo();

	//3.���������
	void Add_Section();

	// 4.�Դ���ν��м���
	void Encryption_Text();

	// 5.�Դ���ν���ѹ��
	void Compress_Text();

	// 6.�޸�������Ϣ
	void Alter_Other();

	// 7.�ƶ��޸��ض�λ
	void Alter_Reloc();

	// 8.����IAT
	void Hide_IAT();

	// �����ļ�
	void SaveFile();

};