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
    CreateWindow(
        TEXT("edit"),
        NULL,
        WS_CHILD | WS_VISIBLE | ES_CENTER | WS_DISABLED,
        WIDTH / 2 - WIDTH / 4 ,
        HEIGHT / 6,
        WIDTH / 2,
        CY * 2,
        hwnd, (HMENU)IDC_EDIT_PATH,
        ((LPCREATESTRUCT)lParam)->hInstance,
        NULL
    );

    
}

void createButton(HWND hwnd, LPARAM lParam)
{
    CreateWindow(
        TEXT("button"),
        TEXT("可将文件或文件夹拖动到该区域内"),
        BS_GROUPBOX | WS_CHILD | WS_VISIBLE,
        0,
        0,
        WIDTH,
        BTNHEIGHT + CY + BTNYINTERVAL,
        hwnd,
        (HMENU)0,
        ((LPCREATESTRUCT)lParam)->hInstance,
        NULL
    );

    for ( int i = 0; i < BTNNUM; i++)
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
    EnableWindow(GetDlgItem(hwnd, 3), FALSE);
    EnableWindow(GetDlgItem(hwnd, 4), FALSE);
}

void buttonFunc(HWND hwnd, WPARAM wParam)
{
    switch (wParam)
    {
    case 1:
        selectFile(hwnd);
        break;
    case 2:
        selectDocument(hwnd);
        break;
    case 3:
        run(hwnd);
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
        EnableWindow(GetDlgItem(hwnd, 3), TRUE);
    }
    else
    {
        EnableWindow(GetDlgItem(hwnd, 3), FALSE);
        memset(filePath, 0, sizeof(filePath));
    }
    SetWindowText(GetDlgItem(hwnd, IDC_EDIT_PATH), filePath);
}

void selectDocument(HWND hwnd)
{
    BROWSEINFO bi = {0};
    bi.hwndOwner = NULL;
    bi.lpszTitle = TEXT("选择一个文件夹");
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NONEWFOLDERBUTTON;
    bi.pszDisplayName = filePath;

    LPITEMIDLIST lpList = SHBrowseForFolder(&bi);

    if (SHGetPathFromIDList(lpList, filePath))
    {
        EnableWindow(GetDlgItem(hwnd, 3), TRUE);
    }
    else
    {
        EnableWindow(GetDlgItem(hwnd, 3), FALSE);
        memset(filePath, 0, sizeof(filePath));
    }
    SetWindowText(GetDlgItem(hwnd, IDC_EDIT_PATH), filePath);
}

void run(HWND hwnd)
{
    /*EnableWindow(GetDlgItem(hwnd, 3), FALSE);
    if (_access("recoding.exe", 0) == -1) // recoding.exe不存在
    {
        return;
    }*/
    
    // 获取桌面路径
    TCHAR desktop[256];
    LPMALLOC mc = NULL;
    LPITEMIDLIST pid = NULL;
    SHGetMalloc(&mc); // 分配
    SHGetFolderLocation(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, &pid);
    SHGetPathFromIDList(pid, desktop);
    mc->Free(pid); // 释放 ...
    mc->Release();

    TCHAR cur[256];
    GetModuleFileName(NULL, cur, 256);
    (_tcsrchr(cur, TEXT('\\')))[1] = 0;

    TCHAR cmd[1024];
    swprintf_s(cmd, 1024, TEXT("%srecoding.exe -source=%s -output=%s\\output"), cur, filePath, desktop);
    //MessageBox(NULL, cmd, NULL, MB_OK);
    char c[1024];
    tchar2char(cmd, c);
    //system("E:\\devloper\\vs\\recode\\Debug\\recoding.exe -source E:\\xxx\\a\\add_report_list_info.php -output C:\\Users\\xxx\\Desktop\\output");
    WinExec("E:\\devloper\\vs\\recode\\recoding.exe", SW_HIDE);
}

void tchar2char(TCHAR* tc, char* c)
{
    int len = WideCharToMultiByte(CP_ACP, 0, tc, -1, NULL, 0, NULL, NULL);
    WideCharToMultiByte(CP_ACP, 0, tc, -1, c, len, NULL, NULL);
}
