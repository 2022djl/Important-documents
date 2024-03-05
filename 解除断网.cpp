#include<cstdio> 
#include<windows.h> 
#include <TlHelp32.h> 
#define KILL_FORCE 1
#define KILL_DEFAULT 2
//用杀掉每个线程的方法解决某些进程hook住了TerminateProcess()的问题
bool KillProcess(DWORD dwProcessID, int way) {
	if (way == KILL_FORCE) {
		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, dwProcessID);

		if (hSnapshot != INVALID_HANDLE_VALUE) {
			bool rtn = false;
			THREADENTRY32 te = {sizeof(te)};
			BOOL fOk = Thread32First(hSnapshot, &te);
			for (; fOk; fOk = Thread32Next(hSnapshot, &te)) {
				if (te.th32OwnerProcessID == dwProcessID) {
					HANDLE hThread = OpenThread(THREAD_TERMINATE, FALSE, te.th32ThreadID);
					if (TerminateThread(hThread, 0)) rtn = true;
					CloseHandle(hThread);
				}
			}
			CloseHandle(hSnapshot);
			return rtn;
		}
		return false;
	} else if (way == KILL_DEFAULT) {
		//默认方法，稳定安全
		HANDLE handle = OpenProcess(PROCESS_TERMINATE, FALSE, dwProcessID);
		WINBOOL sta = TerminateProcess(handle, 0);
		CloseHandle(handle);
		return sta;
	}
	return false;
}

DWORD GetProcessIdFromName(char *name)
{
    HANDLE  hsnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if(hsnapshot == INVALID_HANDLE_VALUE) 
    {   
        printf("CreateToolhelp32Snapshot Error!\n");
        return 0;  
    }
    
    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    int flag = Process32First(hsnapshot, &pe);

    while (flag != 0)
    {
        if (strcmp(pe.szExeFile, name) == 0)
        {
            return pe.th32ProcessID;
        }
        flag = Process32Next(hsnapshot, &pe);
    }

    CloseHandle(hsnapshot);

    return 0;
}

int main(){
	printf("%d\n",GetProcessIdFromName("MasterHelper.exe"));
	printf("%d\n",GetProcessIdFromName("GATESRV.exe"));
	KillProcess(GetProcessIdFromName("MasterHelper.exe"),KILL_FORCE);
	KillProcess(GetProcessIdFromName("GATESRV.exe"),KILL_FORCE);
	KillProcess(GetProcessIdFromName("MasterHelper.exe"),KILL_DEFAULT);
	KillProcess(GetProcessIdFromName("GATESRV.exe"),KILL_DEFAULT);
	
	SC_HANDLE TD = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
	SC_HANDLE Net = OpenService(TD, "TDNetFilter", SERVICE_STOP);
	SERVICE_STATUS ss = {};
	ControlService(Net, SERVICE_CONTROL_STOP, &ss);
	CloseServiceHandle(TD);
	CloseServiceHandle(Net);
	
	
	printf("%d\n",GetProcessIdFromName("MasterHelper.exe"));
	printf("%d\n",GetProcessIdFromName("GATESRV.exe"));
} 
