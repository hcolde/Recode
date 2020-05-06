#include "recode.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
    static TCHAR ceAppName[] = TEXT("Recode");
    HWND hwnd;
    MSG msg;
    WNDCLASS wndclass;

    // 获取屏幕工作区大小 (去除任务栏区域)
    RECT rect;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);

    // 设置居中
    int x = rect.right / 2 - WIDTH / 2;
    int y = rect.bottom / 2 - HEIGHT / 2;

    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = ceAppName;

    if (!RegisterClass(&wndclass))
    {
        MessageBox(NULL, TEXT("请在Windows NT中运行"), ceAppName, MB_ICONERROR);
        return 0;
    }

    hwnd = CreateWindow(
        ceAppName,
        TEXT("Recode"),
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        x,
        y,
        WIDTH,
        HEIGHT,
        NULL,
        NULL,
        hInstance,
        NULL);

    ShowWindow(hwnd, iCmdShow);
    UpdateWindow(hwnd);

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        createEdit(hwnd, lParam);
        createButton(hwnd, lParam);
        return 0;
    case WM_COMMAND:
        buttonFunc(hwnd, wParam);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}

void createEdit(HWND hwnd, LPARAM lParam)
{
    // 选择的文件路径
    CreateWindow(
        TEXT("edit"),
        NULL,
        WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY,
        WIDTH / 2 - WIDTH / 4,
        HEIGHT / 6,
        WIDTH / 2,
        CY * 2,
        hwnd, (HMENU)IDC_EDIT_PATH,
        ((LPCREATESTRUCT)lParam)->hInstance,
        NULL
    );

    // 日志栏
    CreateWindow(
        TEXT("edit"),
        NULL,
        WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN | WS_VSCROLL | ES_READONLY,
        0,
        BTNHEIGHT + CY + BTNYINTERVAL,
        WIDTH - 20,
        HEIGHT - BTNHEIGHT - CY - BTNYINTERVAL,
        hwnd,
        (HMENU)IDC_EDIT_LOG,
        ((LPCREATESTRUCT)lParam)->hInstance,
        NULL
    );
}

void createButton(HWND hwnd, LPARAM lParam)
{
    // 下一个版本:拖动文件区域(可将文件或文件夹拖动到该区域内)
    CreateWindow(
        TEXT("button"),
        TEXT("工作区"),
        BS_GROUPBOX | WS_CHILD | WS_VISIBLE | WS_EX_ACCEPTFILES,
        0,
        0,
        WIDTH,
        BTNHEIGHT + CY + BTNYINTERVAL,
        hwnd,
        (HMENU)0,
        ((LPCREATESTRUCT)lParam)->hInstance,
        NULL
    );

    // 四个按钮 选择文件 选择文件夹 运行 打开目录
    for (int i = 0; i < BTNNUM; i++)
    {
        CreateWindow(
            TEXT("button"),
            button[i].text,
            WS_CHILD | WS_VISIBLE | button[i].style,
            BTNINITX * 2 + i * (BTNINITX + BTNWIDTH),
            BTNYINTERVAL,
            BTNWIDTH,
            BTNHEIGHT,
            hwnd,
            (HMENU)button[i].handle,
            ((LPCREATESTRUCT)lParam)->hInstance,
            NULL
        );
    }
    EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_RECODE), FALSE);
    // EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_REVIEW), FALSE);
}

void buttonFunc(HWND hwnd, WPARAM wParam)
{
    switch (wParam)
    {
    case IDC_BUTTON_SELECT_FILE:
        selectFile(hwnd);
        break;
    case IDC_BUTTON_SELECT_DOCUMENT:
        selectDocument(hwnd);
        break;
    case IDC_BUTTON_RECODE:
        recoding(hwnd);
        break;
    }
}

void selectFile(HWND hwnd)
{
    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = TEXT("所有文件\0");
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = filePath;
    ofn.lpstrTitle = TEXT("选择一个文件");
    ofn.nMaxFile = sizeof(filePath);
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    ofn.lpstrInitialDir = NULL;

    if (GetOpenFileName(&ofn))
    {
        SetWindowText(GetDlgItem(hwnd, IDC_BUTTON_RECODE), TEXT_NAME_RECODE); // 还原运行按钮
        SetWindowText(GetDlgItem(hwnd, IDC_EDIT_LOG), NULL); // 清空日志栏
        //EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_REVIEW), FALSE); // 禁止点击查看目录
        EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_RECODE), TRUE); // 选择后允许点击运行
    }
    else
    {
        EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_RECODE), FALSE);
        memset(filePath, 0, sizeof(filePath));
    }
    SetWindowText(GetDlgItem(hwnd, IDC_EDIT_PATH), filePath);
}

void selectDocument(HWND hwnd)
{
    BROWSEINFO bi = { 0 };
    bi.hwndOwner = NULL;
    bi.lpszTitle = TEXT("选择一个文件夹");
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NONEWFOLDERBUTTON;
    bi.pszDisplayName = filePath;

    LPITEMIDLIST lpList = SHBrowseForFolder(&bi);

    if (SHGetPathFromIDList(lpList, filePath))
    {
        SetWindowText(GetDlgItem(hwnd, IDC_BUTTON_RECODE), TEXT_NAME_RECODE); // 还原运行按钮
        SetWindowText(GetDlgItem(hwnd, IDC_EDIT_LOG), NULL); // 清空日志栏
        //EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_REVIEW), FALSE); // 禁止点击查看目录
        EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_RECODE), TRUE); // 选择后允许点击运行
    }
    else
    {
        EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_RECODE), FALSE);
        memset(filePath, 0, sizeof(filePath));
    }
    SetWindowText(GetDlgItem(hwnd, IDC_EDIT_PATH), filePath);
}

void recoding(HWND hwnd)
{
    SetWindowText(GetDlgItem(hwnd, IDC_EDIT_LOG), NULL); // 清空日志栏
    HMODULE  dll = LoadLibrary(TEXT("recoding.dll"));
    if (NULL == dll)
    {
        SetWindowText(GetDlgItem(hwnd, IDC_EDIT_LOG), TEXT("无法加载recoding.dll\r\n"));
        return;
    }
    RecodeFunc recode = (RecodeFunc)GetProcAddress(dll, "Recode");
    if (recode)
    {
        // 按钮全部禁止点击
        EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_SELECT_FILE), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_SELECT_DOCUMENT), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_RECODE), FALSE);
        //EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_REVIEW), FALSE);

        CreateThread(NULL, NULL, Recode, (LPVOID)recode, NULL, NULL);
        CreateThread(NULL, NULL, Sock, (LPVOID)hwnd, NULL, NULL);
    }
    else
    {
        SetWindowText(GetDlgItem(hwnd, IDC_EDIT_LOG), TEXT("无法调用函数\r\n"));
    }
    //FreeLibrary(dll);
}

void tchar2char(TCHAR* tc, char* c)
{
    int len = WideCharToMultiByte(CP_ACP, 0, tc, -1, NULL, 0, NULL, NULL);
    WideCharToMultiByte(CP_ACP, 0, tc, -1, c, len, NULL, NULL);
}

void char2tchar(TCHAR* tc, char* c)
{
    int len = MultiByteToWideChar(CP_ACP, 0, c, strlen(c) + 1, NULL, 0);
    MultiByteToWideChar(CP_ACP, 0, c, strlen(c) + 1, tc, len);
}

DWORD WINAPI Recode(LPVOID param)
{
    RecodeFunc recode = (RecodeFunc)param;
    char s[256] = { 0 };
    char o[256] = { 0 };
    tchar2char(filePath, s);

    // 获取桌面路径
    TCHAR desktop[256] = { 0 };
    LPMALLOC mc = NULL;
    LPITEMIDLIST pid = NULL;
    SHGetMalloc(&mc); // 分配
    SHGetFolderLocation(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, &pid);
    SHGetPathFromIDList(pid, desktop);
    mc->Free(pid); // 释放 ...
    mc->Release();
    tchar2char(desktop, o);
    GoString output{ o, strlen(o) };
    GoString source{ s, strlen(s) };
    recode(source, output);
    return 0;
}

DWORD WINAPI Sock(LPVOID param)
{
    HWND hwnd = (HWND)param;

    void* context = zmq_ctx_new();
    void* requester = zmq_socket(context, ZMQ_REQ);
    zmq_connect(requester, "tcp://127.0.0.1:12138");

    int stop = 0;
    Document d;
    while (true)
    {
        zmq_send(requester, "recoding", 8, 0);
        if (stop == 1)
        {
            break;
        }
        char buff[1024] = { 0 };
        TCHAR rate[5] = { 0 };
        TCHAR log[1024] = { 0 };
        zmq_recv(requester, buff, 1024, 0);
        d.Parse(buff);
        char2tchar(rate, (char*)d["rate"].GetString());
        char2tchar(log, (char*)d["msg"].GetString());

        TCHAR* perRate = new TCHAR[lstrlen(rate) + 2];
        perRate[0] = _T('\0');
        lstrcat(perRate, rate);
        lstrcat(perRate, _T("%"));

        TCHAR* logMsg = new TCHAR[lstrlen(log) + 3];
        logMsg[0] = _T('\0');
        lstrcat(logMsg, log);
        lstrcat(logMsg, _T("\r\n"));

        SetWindowText(GetDlgItem(hwnd, IDC_BUTTON_RECODE), perRate);

        SendMessage(GetDlgItem(hwnd, IDC_EDIT_LOG), EM_SETSEL, -2, -1);
        SendMessage(GetDlgItem(hwnd, IDC_EDIT_LOG), EM_REPLACESEL, true, (LPARAM)logMsg);
        SendMessage(GetDlgItem(hwnd, IDC_EDIT_LOG), WM_VSCROLL, SB_BOTTOM, 0);
        delete[] perRate;
        delete[] logMsg;
        if (d["rate"] == "-1" || d["rate"] == "100")
        {
            stop = 1;
        }
    }

    zmq_close(requester);
    zmq_ctx_destroy(context);

    // 获取桌面路径
    TCHAR desktop[256] = { 0 };
    LPMALLOC mc = NULL;
    LPITEMIDLIST pid = NULL;
    SHGetMalloc(&mc); // 分配
    SHGetFolderLocation(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, &pid);
    SHGetPathFromIDList(pid, desktop);
    mc->Free(pid); // 释放 ...
    mc->Release();

    // 恢复按钮
    EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_SELECT_FILE), TRUE);
    EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_SELECT_DOCUMENT), TRUE);
    // EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_REVIEW), TRUE);
    return 0;
}
