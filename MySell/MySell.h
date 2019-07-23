#pragma once
#include<Windows.h>
#include "../syub/stub.h"

typedef struct _StubInfo {
	PIMAGE_NT_HEADERS Dll_pNT;
	char* Dll_Buff;				// dll���ص��ڴ���׵�ַ(���ػ�ַ)
	DWORD Text_Size;			// ����εĴ�С
	char* Text_Buff;			// ����εĿ�ʼ��ַ
	DWORD  Start_Offset;	// start���������ڴ���εĶ���ƫ��
	StubConf* g_Conf;		// dll������ȫ�ֱ����ĵ�ַ
}StubInfo;

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

	StubInfo m_StubInfo;

private:

	// �����С ��������
	DWORD Alignment(DWORD Size,DWORD Grain_Size);
	
	// ����ԭ�������һ������
	PIMAGE_SECTION_HEADER Last_Section();

	//�����������Ʋ�������
	PIMAGE_SECTION_HEADER Scn_by_name(char* buff, const char* section_name);

public:
	// 1.�ж��Ƿ���PE�ļ�
	bool IsPE();

	//2. ��ȡ�����ε���Ϣ
	void GetDllInfo();

	//3.���������
	void Add_Section();

	// 4.�޸�������Ϣ
	void Alter_Other();

	// �����ļ�
	void SaveFile();

};