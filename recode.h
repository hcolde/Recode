#pragma once

#include <windows.h>
#include <ShlObj.h>
#include <stdio.h>
#include <io.h>
#include <tchar.h>

#define CX LOWORD(GetDialogBaseUnits())
#define CY HIWORD(GetDialogBaseUnits())
#define IDC_EDIT_PATH 10

struct
{
	int style;
	TCHAR text[6];
	int handle;
} button[] = {
	BS_PUSHBUTTON, TEXT("选择文件"), 1,
	BS_PUSHBUTTON, TEXT("选择文件夹"), 2,
	BS_PUSHBUTTON, TEXT("转换"), 3,
	BS_PUSHBUTTON, TEXT("查看文件"), 4
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
void run(HWND);
void tchar2char(TCHAR*, char*);

// 路径
TCHAR filePath[256];
