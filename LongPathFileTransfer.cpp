/**
 * Author: Cameron Oakley
 * Date: 2024-06-10
 * Description:
 */


 /* LIBRARIES */

#include <filesystem>
#include <iostream>
#include <string>
#include <tchar.h>
#include <windows.h>


/* PREPROCESSING STATEMENTS */

#define INVALID_INPUT -2
#define SRC_PATH_NOT_EXIST -1

#define COPY_PROCESS 1


/* GLOBAL VARIABLES */

static TCHAR szWindowClass[] = _T("LongPathFileTransfer");
static TCHAR szWindowTitle[] = _T("Long Path File Transfer");
HWND srcDirHandler;
HWND dstDirHandler;
static std::filesystem::path* srcDirPath, * dstDirPath;
static int numCharsInSrcPath;


/* FUNCTION DECLARATIONS */

LRESULT CALLBACK windowProcedure(_In_ HWND, _In_ UINT, _In_ WPARAM, _In_ LPARAM);
void addControls(HWND);
int IterateSrcDirAndCopyToTargetDir();
std::string ReplaceDoubleBackslash(std::string);


/* START OF MAIN */

int APIENTRY WinMain(
	_In_ HINSTANCE handleInstance,
	_In_opt_ HINSTANCE handlePrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nCmdShow
) {

	HWND windowHandler;
	MSG message;
	WNDCLASSEX windowClassData;

	//* Set Window class information.
	windowClassData.cbSize = sizeof(WNDCLASSEX);
	windowClassData.style = CS_HREDRAW | CS_VREDRAW;
	windowClassData.lpfnWndProc = windowProcedure;
	windowClassData.cbClsExtra = 0;
	windowClassData.cbWndExtra = 0;
	windowClassData.hInstance = handleInstance;
	windowClassData.hIcon = LoadIcon(
		windowClassData.hInstance,
		IDI_APPLICATION
	);
	windowClassData.hCursor = LoadCursor(
		NULL,
		IDC_ARROW
	);
	windowClassData.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	windowClassData.lpszMenuName = NULL;
	windowClassData.lpszClassName = szWindowClass;
	windowClassData.hIconSm = LoadIcon(
		windowClassData.hInstance,
		IDI_APPLICATION
	);

	//! Error handling.
	if (!RegisterClassEx(&windowClassData)) {
		MessageBox(NULL,
			_T("Call to RegisterClassEx API failed!"),
			szWindowTitle,
			NULL
		);

		return 1;
	}


	//* Create the main application window.
	windowHandler = CreateWindowEx(
		WS_EX_OVERLAPPEDWINDOW,
		szWindowClass,
		szWindowTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,	// location of window
		500, 500,						// size of window
		NULL,
		NULL,
		handleInstance,
		NULL
	);

	//! Error handling.
	if (!windowHandler) {
		MessageBox(NULL,
			_T("Call to CreateWindowEx API failed!"),
			szWindowTitle,
			NULL
		);

		return 1;
	}


	//* Show the main application window.
	ShowWindow(
		windowHandler,
		nCmdShow
	);
	UpdateWindow(windowHandler);


	//* Main message loop.
	while (GetMessage(&message, NULL, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	return (int)message.wParam;
}

/* END OF MAIN */


/* FUNCTION DEFINITIONS */

LRESULT CALLBACK windowProcedure(
	_In_ HWND   windowHandler,
	_In_ UINT   message,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
) {
	switch (message) {
	case WM_CREATE:
		addControls(windowHandler);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_COMMAND:
		switch (wParam) {
		case COPY_PROCESS:
			int copyProcRetVal = IterateSrcDirAndCopyToTargetDir();
			if (copyProcRetVal == 0) {
				CreateWindowW(
					L"Static", L"Done!",
					WS_VISIBLE | WS_CHILD,
					10, 100,			// x, y coordinates in relation to the parent window
					120, 20,			// w, h of the child window
					windowHandler,
					NULL,
					NULL,
					NULL
				);
			}
			else {
				CreateWindowW(
					L"Static", L"Failed!",
					WS_VISIBLE | WS_CHILD,
					10, 100,			// x, y coordinates in relation to the parent window
					120, 20,			// w, h of the child window
					windowHandler,
					NULL,
					NULL,
					NULL
				);
			}
			break;
		}
		break;

	default:
		return DefWindowProc(windowHandler, message, wParam, lParam);
		break;
	}

	return 0;
}

void addControls(HWND windowHandler) {
	CreateWindowW(
		L"Static", L"Source directory:",
		WS_VISIBLE | WS_CHILD,
		10, 15,			// x, y coordinates in relation to the parent window
		120, 20,		// w, h of the child window
		windowHandler,
		NULL,
		NULL,
		NULL
	);

	srcDirHandler = CreateWindowW(
		L"Edit", NULL,
		WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
		130, 15,		// x, y coordinates in relation to the parent window
		65, 20,			// w, h of the child window
		windowHandler,
		NULL,
		NULL,
		NULL
	);

	CreateWindowW(
		L"Static", L"Destination directory:",
		WS_VISIBLE | WS_CHILD,
		10, 40,			// x, y coordinates in relation to the parent window
		145, 20,		// w, h of the child window
		windowHandler,
		NULL,
		NULL,
		NULL
	);

	dstDirHandler = CreateWindowW(
		L"Edit", NULL,
		WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
		155, 40,		// x, y coordinates in relation to the parent window
		65, 20,			// w, h of the child window
		windowHandler,
		NULL,
		NULL,
		NULL
	);

	CreateWindowW(
		L"Button", L"Copy",
		WS_VISIBLE | WS_CHILD,
		10, 65,			// x, y coordinates in relation to the parent window
		65, 20,			// w, h of the child window
		windowHandler,
		(HMENU)COPY_PROCESS,
		NULL,
		NULL
	);
}

int IterateSrcDirAndCopyToTargetDir() {

	//* Get data from handlers.
	int srcDirPathLen = GetWindowTextLengthW(srcDirHandler) + 1;
	std::wstring srcDirPathWstr(srcDirPathLen, '0');
	GetWindowTextW(srcDirHandler, &srcDirPathWstr[0], srcDirPathLen);
	std::string srcDirPathStr(srcDirPathWstr.begin(), srcDirPathWstr.end());
	srcDirPathStr = srcDirPathStr.substr(0, srcDirPathStr.length() - 1);

	int dstDirPathLen = GetWindowTextLengthW(dstDirHandler) + 1;
	std::wstring dstDirPathWstr(dstDirPathLen, '\0');
	GetWindowTextW(dstDirHandler, &dstDirPathWstr[0], dstDirPathLen);
	std::string dstDirPathStr(dstDirPathWstr.begin(), dstDirPathWstr.end());
	dstDirPathStr = dstDirPathStr.substr(0, dstDirPathStr.length() - 1);


	//* Validate input data is in the expected format.

	// 1. Check that strings aren't NULL.
	if ((srcDirPathLen - 1) == 0
		|| (dstDirPathLen - 1) == 0) {
		return INVALID_INPUT;
	}

	// 2. Strings start with "*some drive letter*:\\".

	// a. The Win32 API will turn '\' into "\\". Replace this.
	// srcDirPathStr = ReplaceDoubleBackslash(srcDirPathStr);
	// dstDirPathStr = ReplaceDoubleBackslash(dstDirPathStr);

	// b. Note that ":\\" really just translates to :\, the second '\' is escaping.
	if ((srcDirPathStr.substr(1, 2) != ":\\")
		|| (dstDirPathStr.substr(1, 2) != ":\\")) {
		return INVALID_INPUT;
	}

	// 3. Strings don't point to the same directory.
	if (srcDirPathStr == dstDirPathStr) { return INVALID_INPUT; }


	//* Verify source directory exists
	srcDirPath = new std::filesystem::path(srcDirPathStr);
	if (!std::filesystem::exists(*srcDirPath)) { return SRC_PATH_NOT_EXIST; }

	//* Verify destination directory exists
	dstDirPath = new std::filesystem::path(dstDirPathStr);
	if (!std::filesystem::exists(*dstDirPath)) { std::filesystem::create_directory(*dstDirPath); }


	/** COPY FILES */
	for (const std::filesystem::directory_entry& srcDirItr : std::filesystem::recursive_directory_iterator(srcDirPathStr)) {
		std::string tempRelPath = srcDirItr.path().string().substr(srcDirPathLen);		// Isolates relative path
		std::filesystem::path tempTargetDirPath = (*dstDirPath) / tempRelPath;				// Combines relative path with target absolute path

		if (std::filesystem::is_directory(srcDirItr.path())) {
			if (!std::filesystem::exists(tempTargetDirPath)) { std::filesystem::create_directory(tempTargetDirPath); }
		}
		else if (is_regular_file(srcDirItr.path())) {
			std::filesystem::copy_file(srcDirItr, tempTargetDirPath);
		}
		else {
			std::cout << "Unknown file type...\t" << srcDirItr.path() << std::endl;
		}
	}

	return EXIT_SUCCESS;
}

std::string ReplaceDoubleBackslash(std::string str) {
	std::string retVal;
	for (int i = 0; i < str.size(); i++) {
		if (
			(str[i] == '\\')
			&& ((i + 1) < str.size())
			&& (str[i + 1] == '\\')
			) {
			retVal += str[i];
			i++;
		}
		else {
			retVal += str[i];
		}
	}
	return retVal;
}
