#include"MySell.h"

int main()
{
	// �����ӿǶ��� 
	MySell Myself("E:\\MySell\\Release\\test.exe");

	// �޸�������Ϣ
	Myself.Add_Section();

	// ���ܴ����
	Myself.Encryption_Text();

	//����IAT
	Myself.Hide_IAT();

	// ѹ�������
	Myself.Compress_Text();

	// �޸�������Ϣ
	Myself.Alter_Other();

	// �޸��ض�λ
	Myself.Alter_Reloc();

	//�ѼӿǺ�ĳ��򱣴浽�ļ�
	Myself.SaveFile();

	system("pause");

	return 0;
}