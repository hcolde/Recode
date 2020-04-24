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
	BS_PUSHBUTTON, TEXT("ѡ���ļ�"), 1,
	BS_PUSHBUTTON, TEXT("ѡ���ļ���"), 2,
	BS_PUSHBUTTON, TEXT("ת��"), 3,
	BS_PUSHBUTTON, TEXT("�鿴�ļ�"), 4
};

#define BTNNUM (sizeof(button) / sizeof(button[0]))

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void createEdit(HWND, LPARAM);   // �����ı� (��ʾ��ѡ����ļ����ļ���·��)
void createButton(HWND, LPARAM); // ������ť
void buttonFunc(HWND, WPARAM);   // ��ť��Ӧ����

// ����
const int WIDTH = 800;
const int HEIGHT = 600;

// ѡ��ť
const int BTNWIDTH = (WIDTH / BTNNUM) - 6 * CX;
const int BTNHEIGHT = 1.75 * CY;
const int BTNYINTERVAL = HEIGHT / 3; // ������
const int BTNINITX = ((WIDTH / BTNNUM) - BTNWIDTH) / 2; // ��ʼx���� (������)

// ѡ���ļ����ļ���
void selectFile(HWND);
void selectDocument(HWND);

// recoding
void run(HWND);
void tchar2char(TCHAR*, char*);

// ·��
TCHAR filePath[256];
