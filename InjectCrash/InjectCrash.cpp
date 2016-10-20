#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <string>
#include <sstream>
#include <iostream>
#include <memory>

using namespace std;


// Attempts to convert a string to a DWORD, returning false if not possible.
bool stringToDword(wstring in, DWORD& out) {
	wstringstream ss(in);
	ss >> out;
	return !ss.fail();
}


// Returns the absolute path to the executable. 
// Note: If the path is greater than MAX_PATH * 4 the result is undefined. This is because the
// GetModuleFilename() function has no way to return the actual size of the string.
string getModuleDirectory() {
	const DWORD maxSize = MAX_PATH * 4;
	auto buffer = std::unique_ptr<char[]>{new char[maxSize]};
	GetModuleFileNameA(NULL, buffer.get(), maxSize);
	string path = buffer.get();
	return path.substr(0, path.find_last_of("\\") + 1);
}


// Prints information about last Win32 error.
void printLastError(wstring hint) {
	wcout << L"Error: " << hint << endl
		<< L" Error code: " << GetLastError() << endl;
}


// Help message.
void printUsage() {
	wcout << L"InjectCrash usage guide" << endl
		<< L"https://github.com/MrEricSir/InjectCrash" << endl 
		<< endl
		<< L"Usage: injectcrash pid [type]"
		<< endl
		<< L"Run this program with a PID as an argument to cause that program to crash." << endl
		<< L"You can find the PID for the target process using Task Manager." << endl
		<< endl
		<< L"Example:" << endl
		<< L"   injectcrash 100" << endl
		<< endl
		<< L"The optional \"type\" parameter is a number specifying the the type of crash." << endl
		<< L"Crash type options:" << endl
		<< L"   0  Null pointer dereference (default)" << endl
		<< L"   1  Divide by zero" << endl
		<< L"   2  Stack overflow" << endl
		<< L"   3  Debug breakpoint" << endl
		<< endl
		<< L"For example, to crash process with PID 100 with a stack overflow:" << endl
		<< L"   injectcrash 100 2"
		<< endl;
}


// Returns the absolute path to the DLL for the crash type, or the empty string if invalid.
string getDllPathForCrashType(DWORD crashType) {
	string path = getModuleDirectory() + "InjectCrash";
	switch (crashType) {
	case 0:
		return path + "NullPointerException.dll";
	case 1:
		return path + "DivideByZero.dll";
	case 2:
		return path + "StackOverflow.dll";
	case 3:
		return path + "DebugBreak.dll";
	}

	return "";
}


// Attempts to crash a process with the provided PID.
int crashProcess(DWORD pid, string dllPath) {
	HANDLE hProc = NULL;
	HANDLE hThread = NULL;
	LPVOID loadLibraryAnsiAddress = NULL;
	LPVOID remoteDllNameString = NULL;
	
	// Get handle to the target process.
	hProc = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION
		| PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, pid);
	if (!hProc) {
		printLastError(L"Could not open process.");
		return -1;
	}

	// Lookup the address of LoadLibraryA.
	loadLibraryAnsiAddress = (LPVOID)GetProcAddress(GetModuleHandleW(L"kernel32.dll"),
		"LoadLibraryA");
	if (!loadLibraryAnsiAddress) {
		printLastError(L"Unable to locate address of LoadLibraryA.");
		CloseHandle(hProc);
		return -2;
	}

	// Allocate space in the target process for the name of our DLL.
	remoteDllNameString = VirtualAllocEx(hProc, NULL, dllPath.size() + 1,
		MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (!remoteDllNameString) {
		printLastError(L"Couldn't reserve memory within remote process.");
		CloseHandle(hProc);
		return -2;
	}

	// Copy the name of our DLL into the allocated memory.
	if (!WriteProcessMemory(hProc, remoteDllNameString, dllPath.c_str(), dllPath.size() + 1, NULL)) {
		printLastError(L"Failed to copy DLL name into remote process.");
		CloseHandle(hProc);
		return -3;
	}

	// Create a thread in the target process that loads our DLL.
	hThread = CreateRemoteThread(hProc, NULL, 0, (LPTHREAD_START_ROUTINE)loadLibraryAnsiAddress,
		(LPVOID)remoteDllNameString, 0, NULL);
	if (!hThread) {
		printLastError(L"Unable to create thread in target process.");
		CloseHandle(hProc);
		return -4;
	}
	
	CloseHandle(hThread);
	CloseHandle(hProc);
	return 0;
}


int wmain(int argc, wchar_t* argv[]) {
	// Print help text if the number of arguments is invalid.
	if (argc == 1 || argc > 3) {
		printUsage();
		return 0;
	}

	// Fetch the first parameter (PID) and print help text if invalid.
	DWORD pid = 0;
	if (!stringToDword(argv[1], pid)) {
		printUsage();
		return 0;
	}

	// Fetch second parameter if available (crash type) or print help text if invalid.
	DWORD crashType = 0;
	if (argc == 3 && !stringToDword(argv[2], crashType)) {
		printUsage();
		return 0;
	}

	// Build the absolute path to the DLL.
	string dllPath = getDllPathForCrashType(crashType);
	if (dllPath == "") {
		printUsage();
		return 0;
	}

	// Attempt to crash the process with our DLL.
	return crashProcess(pid, dllPath);
}
