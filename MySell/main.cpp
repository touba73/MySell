#include"MySell.h"

int main()
{
	// �����ӿǶ��� 
	MySell Myself("E:\\MySell\\Release\\test.exe");

	Myself.Add_Section();

	Myself.Alter_Other();

	Myself.SaveFile();

	return 0;
}