#pragma once

#include <windows.h>
#include <ShlObj.h>
#include <stdio.h>
#include <tchar.h>
#include "recoding.h"
#include "zmq.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson;

#define CX LOWORD(GetDialogBaseUnits())
#define CY HIWORD(GetDialogBaseUnits())
#define IDC_BUTTON_SELECT_FILE 1
#define IDC_BUTTON_SELECT_DOCUMENT 2
#define IDC_BUTTON_RECODE 3
#define IDC_BUTTON_REVIEW  4
#define IDC_EDIT_PATH 10
#define IDC_EDIT_LOG   11

#define TEXT_NAME_SELECT_FILE TEXT("选择文件")
#define TEXT_NAME_SELECT_DOCUMENT TEXT("选择文件夹")
#define TEXT_NAME_RECODE TEXT("转换")
#define TEXT_NAME_REVIEW TEXT("查看文件")

struct
{
	int style;
	TCHAR text[6];
	int handle;
} button[] = {
	BS_PUSHBUTTON, TEXT_NAME_SELECT_FILE, IDC_BUTTON_SELECT_FILE,
	BS_PUSHBUTTON, TEXT_NAME_SELECT_DOCUMENT, IDC_BUTTON_SELECT_DOCUMENT,
	BS_PUSHBUTTON, TEXT_NAME_RECODE, IDC_BUTTON_RECODE
	//BS_PUSHBUTTON, TEXT_NAME_REVIEW, IDC_BUTTON_REVIEW
};

#define BTNNUM (sizeof(button) / sizeof(button[0]))

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void createEdit(HWND, LPARAM);   // 创建文本 (显示所选择的文件、文件夹路径)
void createButton(HWND, LPARAM); // 创建按钮
void buttonFunc(HWND, WPARAM);   // 按钮对应功能

// 界面
const int WIDTH = 800;
const int HEIGHT = 600;

// 选择按钮
const int BTNWIDTH = (WIDTH / BTNNUM) - 6 * CX;
const int BTNHEIGHT = 1.75 * CY;
const int BTNYINTERVAL = HEIGHT / 3; // 纵向间隔
const int BTNINITX = ((WIDTH / BTNNUM) - BTNWIDTH) / 2; // 初始x坐标 (横向间隔)

// 选择文件、文件夹
void selectFile(HWND);
void selectDocument(HWND);

// recoding
void recoding(HWND);
typedef void(*RecodeFunc)(GoString, GoString);
DWORD WINAPI Recode(LPVOID);
DWORD WINAPI Sock(LPVOID);

// 查看文件
void openDocument(HWND);

// 路径
TCHAR filePath[256] = { 0 }; // 源文件路径

void tchar2char(TCHAR*, char*);
void char2tchar(TCHAR*, char*);
