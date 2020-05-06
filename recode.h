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

#define TEXT_NAME_SELECT_FILE TEXT("ѡ���ļ�")
#define TEXT_NAME_SELECT_DOCUMENT TEXT("ѡ���ļ���")
#define TEXT_NAME_RECODE TEXT("ת��")
#define TEXT_NAME_REVIEW TEXT("�鿴�ļ�")

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
void recoding(HWND);
typedef void(*RecodeFunc)(GoString, GoString);
DWORD WINAPI Recode(LPVOID);
DWORD WINAPI Sock(LPVOID);

// �鿴�ļ�
void openDocument(HWND);

// ·��
TCHAR filePath[256] = { 0 }; // Դ�ļ�·��

void tchar2char(TCHAR*, char*);
void char2tchar(TCHAR*, char*);
