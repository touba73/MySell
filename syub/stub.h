#pragma once
#include<Windows.h>
typedef struct _StubConf {
	DWORD oep;					//ԭʼOEP
	BYTE  encrypt_key;			//����key
	DWORD encrypt_rva;			//���ܶ�RVA
	DWORD encrypt_size;			//���ܴ�С
	DWORD compress_size;		//ѹ����Ĵ�С
	char str_key[5];			//�û����õ�����
	DWORD OldRelocAddress;		//ԭ�ض�λ��RVA
	DWORD OldRelocSize;			//ԭ�ض�λ���С

	DWORD Import_Rva;

}StubConf;
