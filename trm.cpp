/* Клиент монитора загрузчика УСО

	MonitorTerm com_port команда файл_образа

	команда:	reboot	- перезагрузиться
				run		- загрузить образ и запустить системы из образа, не сохраняя его на диск
				write	- загрузить образ, сохранить его на диск и перезапуститься
			 
*/

#include "stdafx.h"

#include <windows.h>
#include <stdio.h>
#include <iostream>

using namespace std;

HANDLE hCom;

bool serialConnect(TCHAR* portName)
{
	hCom = CreateFile(portName, 
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	if (hCom == INVALID_HANDLE_VALUE)  
	{
		cout << "failed - opening com port: " << GetLastError() << endl;
		return false;
	}

	DCB dcb;
	BOOL fSuccess = GetCommState(hCom, &dcb);

	if (!fSuccess) 
	{
		cout << "failed - GetCommState: " << GetLastError() << endl;
		CloseHandle(hCom);
		return false;
	}

	dcb.BaudRate = CBR_57600;
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;

	fSuccess = SetCommState(hCom, &dcb);

	COMMTIMEOUTS ct;

	ct.ReadIntervalTimeout = 1;
	ct.ReadTotalTimeoutConstant = 0;
	ct.ReadTotalTimeoutMultiplier = 0;
	ct.WriteTotalTimeoutConstant = 0;
	ct.WriteTotalTimeoutMultiplier = 0;
	
	SetCommTimeouts(hCom,&ct);
	if (!fSuccess) 
	{
		cout << "failed - SetCommState: " << GetLastError() << endl;
		CloseHandle(hCom);
		return false;
	}
}

void serialDisconnect()
{
	CloseHandle(hCom);
}

void printHelp(){
	wcout.imbue(locale("rus_rus.866"));
	std::wcout << L"trm имя_порта режим пакет\n   режим: crc - пакет является правильным пакетом без контрольной суммы. К пакету добавляется контрольная сумма.\n        Пакет отправляется и ожидается верный ответ.\n   raw - пакет отправляется как есть ответ не лжидается\n   Пример: trm com1 raw 1 3 5 78";
}

int main(int argc, char* argv[])
{
	if (argc == 1){
		printHelp();
		return 0;
	}

	static const unsigned int MODE_OFFSET = 2;
	static const unsigned int FRAME_OFFSET = 3;

	if (!serialConnect(argv[1]))
		return false;

	Sleep(100);	

	DWORD nWrite = 0;
	unsigned char buffer[20];

	if (strcmp(argv[MODE_OFFSET], "crc") == 0){
		unsigned char crc = 0;
		for (unsigned int i = 0; i < atoi(argv[FRAME_OFFSET + 3]) + 4; ++i)
		{
			buffer[i] = atoi(argv[i + FRAME_OFFSET]);
			crc += atoi(argv[i + FRAME_OFFSET]);
		}
	
		buffer[atoi(argv[FRAME_OFFSET + 3]) + 4] = crc;

		WriteFile(hCom, buffer, buffer[3] + 5, &nWrite, nullptr);
		printf("send: ");
		for (unsigned int i = 0; i < buffer[3] + 5; ++i)
			printf("%u ", buffer[i]);

		printf("\n");
	
		ReadFile(hCom, buffer, 20, &nWrite, nullptr);
		printf("recv: ");
		for (unsigned int i = 0; i < 20; ++i)
			printf("%u ", buffer[i]);

		printf("\n");
	}else if (strcmp(argv[MODE_OFFSET], "raw") == 0){
		for (unsigned int i = 0; i < argc - 3; ++i)
			buffer[i] = atoi(argv[FRAME_OFFSET + i]);
	
		WriteFile(hCom, buffer, argc - 3, &nWrite, nullptr);
		printf("send: ");
		for (unsigned int i = 0; i < argc - 3; ++i)
			printf("%u ", buffer[i]);

		printf("\n");
	}else
		printf("Neisvestnij parameter regima");

	serialDisconnect();
	return 0;
}
