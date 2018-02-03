#pragma once

#ifndef IPCUTILS_DLL_EXPORTS
#define IPCUTILS_API __declspec(dllexport)
#else
#define IPCUTILS_API __declspec(dllimport)
#endif

#include <iostream>

#ifndef IPCMSG_CAP
#define IPCMSG_CAP 512
#endif

extern "C" {
	typedef int(*ReceiverCallback)(char* value, int length);
	typedef int(*SenderCallback)(char* value, int length);

	//Return 0 for success, otherwise something wrong
	IPCUTILS_API int _stdcall InitWithMode(int Mode, ReceiverCallback rCallback, SenderCallback sCallback);
	IPCUTILS_API void __stdcall SendIPCMsg(char* value, int length);
	IPCUTILS_API int __stdcall QueryRecvInfo();
	IPCUTILS_API int __stdcall RetriveRecvMsg(char* value,int length);
	IPCUTILS_API void __stdcall ActiveDebugWindow();
}

#include <Windows.h>
#include <functional>
#include <algorithm>
#include <tchar.h>
#include <mutex>
#include <thread>
#include <cstdint>
#include <locale>
#include <codecvt>
#include <conio.h>
#include <locale>

#ifndef WINAPI
#define WINAPI __stdcall
#endif

class IPCManager {
public:

private:
	enum  RunningMode{
		Uninitialized = -1,
		ReceiverMode = 0,
		SenderMode = 1,
		DualMode = 2
	};

	typedef struct {
		uint32_t senderUpdateFlag;
		uint32_t receiverUpdateFlag;
		time_t timestamp;

		char* rawData[IPCMSG_CAP];

	}IPCSharedMemoryPage,*pIPCSharedMemoryPage;

	static const size_t IPC_MEMORY_SIZE;

	typedef struct {

		HANDLE ipcDataSema;	//Core Named Sema, combined with MapView Access
		HANDLE ipcControlSema; //Unnamed
		int RunningInfo;
		HANDLE ipcMapFile;
		size_t ipcMemorySize;
		std::function<int(void)> senderCallback;
		std::function<void(int, int)> receiverCallback;

	}AsyncIPCData,*pAsyncIPCData;
private:
	HANDLE ipcSema;
	HANDLE ipcMapFile;
	HANDLE ipcReceiverTid;
	HANDLE ipcSenderTid;
	LPVOID ipcMapView;


	RunningMode mode;

	std::function<int(char*, int)> receiverCallback;
	std::function<int(char*, int)> senderCallback;

	pAsyncIPCData ipcData;
	char ipcMsgBuffer[IPCMSG_CAP];
private:
	IPCManager();
	~IPCManager();
private:
	//Auto dispose
	static IPCManager instance;
	static std::mutex innerLocker;
public:
	static IPCManager& getInstance();


public:
	inline bool hasInit()const;
	void initWithReceiverMode(ReceiverCallback rcb);
	void initWithSenderMode(SenderCallback scb);
	void initWithDualMode(ReceiverCallback rcb, SenderCallback scb);
public:
	void loadSendingMsg(char* value, int length);
	int checkRecvFlag();
	int retriveRecvMsg(char* buffer,int maxLen);
private:
	//Kernel Method
	void prepareSema();
	void initReceiverProc();
	void initSenderProc();

	void run();

	static void* WINAPI ReceiverProc(void* data);
	static void* WINAPI SenderProc(void* data);

};