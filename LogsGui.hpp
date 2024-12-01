// Copyright (C) Testmina77. Licensed under the GNU General Public License v3.

#ifdef _WIN64
#ifdef _MSC_VER
#ifdef __cplusplus
#ifndef LOGWINDOW_H
#include <string>
#include <sstream>
#include <windows.h>
#include <commctrl.h>
#include <richedit.h>
#include <atomic>
#include <thread>

#pragma comment(lib, "comctl32.lib")

#define ID_EXIT_PROGRAM 254
#define ID_FILE_SAVE 250
#define ID_OPTIONS_FONT 251
#define ID_OPTIONS_COLOR 252
#define ID_OPTIONS_BGCOLOR 253
class WINExcept : public std::exception {
private:
std::string message; public:
    explicit WINExcept(const std::string& msg) : message(msg) {}

    const char* what() const noexcept override {
        return message.c_str();
    }
};
namespace logs {
    class window {
    public:
        window(std::wstring title) : name(title) { LoadLibrary(TEXT("Msftedit.dll")); };
        window(std::string title) : name(std::wstring(title.begin(), title.end())) { LoadLibrary(TEXT("Msftedit.dll")); };
        window(std::wstring title, int sizex, int sizey) : name(title) { setstaticvars(sizex, sizey); LoadLibrary(TEXT("Msftedit.dll")); };
        window(std::string title, int sizex, int sizey) : name(std::wstring(title.begin(), title.end())) { setstaticvars(sizex, sizey); LoadLibrary(TEXT("Msftedit.dll")); };
        window(int sizex, int sizey) { setstaticvars(sizex, sizey); LoadLibrary(TEXT("Msftedit.dll")); };
        template <typename T>
        window& operator<<(const T& value);
        static void SetColor(COLORREF textcolor, COLORREF backgroundcolor);
        static void SetFont(int Points, std::wstring Font);
        template<typename T>
        void Print(T str);
        void Create();
        void Close();
    private:
        /* static */
        static int points;
        static std::wstring font;
        static COLORREF txtcolor;
        static COLORREF bgcolor;
        static HMENU hMenu;
        static HMENU hOptionsMenu;
        static HMENU hFileMenu;
        static void setstaticvars(int sizex, int sizey);
        static void uninit();
        static HWND win;
        static HWND Edit;
        static int sx;
        static int sy;
        static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        static void setmenu();



        /* nonstatic */
        std::atomic<bool> running = false;
        bool created = false;
        std::wstring name = L"Logs 1.0 by Testmina77";
        void thr_createwin();
        void rawPrint(std::wstring str);
        std::wstringstream ss; // for operator <<
    };
}
#define LOGWINDOW_H
#endif/*logwindow_h*/
#ifndef LOGWINDOW_F

/*======== variables ========*/

/* font */
int logs::window::points = 12;
std::wstring logs::window::font = L"Consolas";

/* hwnds */
HWND logs::window::win = nullptr;
HWND logs::window::Edit = nullptr;

/* colors */
COLORREF logs::window::txtcolor = RGB(0, 0, 0);
COLORREF logs::window::bgcolor = RGB(240, 240, 240);

/* menu bar */
HMENU logs::window::hMenu = nullptr;
HMENU logs::window::hOptionsMenu = nullptr;
HMENU logs::window::hFileMenu = nullptr;

/* window size */
int logs::window::sx = 300;
int logs::window::sy = 300;

/*======== functions ========*/

/* operator << */
template <typename T>
logs::window& logs::window::operator<<(const T& value) {
    ss << value; // Dodajemy wartość do bufora
    rawPrint(ss.str());
    ss.str(L""); // Czyścimy bufor
    ss.clear(); // Resetujemy flagi strumienia
    return *this;
}

/* Init helper */
void logs::window::setstaticvars(int sizex, int sizey) {
    sx = sizex;
    sy = sizey;
}

/* Colors */
void logs::window::SetColor(COLORREF textcolor, COLORREF backgroundcolor) {
    txtcolor = textcolor;
    bgcolor = backgroundcolor;
}
void logs::window::SetFont(int Points, std::wstring Font) {
    font = Font;
    points = Points;
}

/* Window Creation */
void logs::window::Create() {
    if (!created && !running) {
        running = true;
        created = true;
        thr_createwin();
    }
}
void logs::window::Close() {
    if (created && running) running = false;
    else if ((created && !running) || (!created && running)) {
        throw WINExcept("Error: Cannot close (created && !running) || (!created && running)");
    }
}
//create menu
void logs::window::setmenu() {
    hMenu = CreateMenu();

    hFileMenu = CreatePopupMenu();

    AppendMenu(hFileMenu, MF_STRING, ID_FILE_SAVE, L"&Save Logs");
    hOptionsMenu = CreatePopupMenu();
    AppendMenu(hOptionsMenu, MF_STRING, ID_OPTIONS_FONT, L"&Font");
    AppendMenu(hOptionsMenu, MF_STRING, ID_OPTIONS_COLOR, L"&Text Color");
    AppendMenu(hOptionsMenu, MF_STRING, ID_OPTIONS_BGCOLOR, L"&Background Color");

    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, L"&File");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hOptionsMenu, L"&Options");
    AppendMenu(hMenu, MF_STRING, ID_EXIT_PROGRAM, L"&Exit program");

    SetMenu(win, hMenu);
}
void logs::window::thr_createwin() {
    std::thread([this] {
        WNDCLASS wc = {};
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = name.c_str();

        RegisterClass(&wc);
        win = CreateWindowEx(
            WS_EX_TOPMOST,
            name.c_str(),
            name.c_str(),
            WS_OVERLAPPEDWINDOW & ~(WS_SIZEBOX | WS_MAXIMIZEBOX) | WS_POPUP,
            CW_USEDEFAULT, CW_USEDEFAULT, sx, sy,
            NULL,
            NULL,
            wc.hInstance,
            NULL
        );
        HMENU hSysMenu = GetSystemMenu(win, FALSE);

        if (hSysMenu != NULL) {
            EnableMenuItem(hSysMenu, SC_CLOSE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        }
        setmenu();
        ShowWindow(win, SW_SHOW);
        UpdateWindow(win);

        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0) && running) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        }).detach();
}

/* WindowProc */
LRESULT CALLBACK logs::window::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    COLORREF customColors[16] = {
    RGB(255, 0, 0),
    RGB(0, 255, 0),
    RGB(0, 0, 255),
    RGB(255, 255, 0),
    RGB(0, 255, 255),
    RGB(255, 0, 255),
    RGB(192, 192, 192),
    RGB(128, 128, 128),
    RGB(255, 165, 0),
    RGB(0, 128, 128),
    RGB(128, 0, 128),
    RGB(255, 255, 255),
    RGB(0, 0, 0),
    RGB(0, 128, 0),
    RGB(128, 0, 0),
    RGB(0, 0, 128) };
    CHOOSECOLOR cc;
    switch (uMsg) {
    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case ID_FILE_SAVE:
            OPENFILENAME ofn;
            TCHAR szFile[260];
            ZeroMemory(&ofn, sizeof(ofn));
            ZeroMemory(szFile, sizeof(szFile));

            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFile = szFile;
            ofn.lpstrFile[0] = '\0';
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrFilter = L"All Files (*.*)\0*.*\0";
            ofn.nFilterIndex = 1;
            ofn.lpstrFileTitle = NULL;
            ofn.nMaxFileTitle = 0;
            ofn.lpstrInitialDir = NULL;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;

            if (GetSaveFileName(&ofn))
            {
                int length = GetWindowTextLength(Edit);
                if (length > 0)
                {
                    TCHAR* buffer = new TCHAR[length + 1];
                    GetWindowText(Edit, buffer, length + 1);
                    HANDLE hFile = CreateFileW(ofn.lpstrFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

                    if (hFile != INVALID_HANDLE_VALUE)
                    {
                        DWORD written;
                        WriteFile(hFile, buffer, lstrlen(buffer) * sizeof(TCHAR), &written, NULL);
                        CloseHandle(hFile);
                        MessageBox(win, L"Saved succesfully!", L"Info", MB_OK);
                    }
                    else
                    {
                        MessageBox(win, L"Cannot open file to save!", L"ERROR", MB_OK | MB_ICONERROR);
                    }
                    delete[] buffer;

                }
            }
            break;
        case ID_OPTIONS_COLOR:
            ZeroMemory(&cc, sizeof(cc));
            cc.lStructSize = sizeof(cc);
            cc.hwndOwner = hwnd;
            cc.lpCustColors = customColors;
            cc.Flags = CC_FULLOPEN | CC_RGBINIT;
            cc.rgbResult = RGB(0, 0, 0);
            if (ChooseColor(&cc)) {
                CHARFORMAT cf;
                cf.cbSize = sizeof(CHARFORMAT);
                cf.dwMask = CFM_COLOR;
                cf.crTextColor = RGB(GetRValue(cc.rgbResult), GetGValue(cc.rgbResult), GetBValue(cc.rgbResult));                 cf.dwEffects = 0;
                txtcolor = RGB(GetRValue(cc.rgbResult), GetGValue(cc.rgbResult), GetBValue(cc.rgbResult));
                SendMessage(Edit, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
            }
            break;
        case ID_OPTIONS_BGCOLOR:
            ZeroMemory(&cc, sizeof(cc));
            cc.lStructSize = sizeof(cc);
            cc.hwndOwner = win;
            cc.lpCustColors = customColors;
            cc.Flags = CC_FULLOPEN | CC_RGBINIT;
            cc.rgbResult = RGB(255, 255, 255);

            if (ChooseColor(&cc)) {
                bgcolor = RGB(GetRValue(cc.rgbResult), GetGValue(cc.rgbResult), GetBValue(cc.rgbResult));
                SendMessage(Edit, EM_SETBKGNDCOLOR, 0, RGB(GetRValue(cc.rgbResult), GetGValue(cc.rgbResult), GetBValue(cc.rgbResult)));
            }
            break;
        case ID_OPTIONS_FONT:
            CHOOSEFONT cf;
            LOGFONT lf;

            ZeroMemory(&cf, sizeof(cf));
            ZeroMemory(&lf, sizeof(lf));
            cf.lStructSize = sizeof(cf);
            cf.hwndOwner = win;
            cf.lpLogFont = &lf;
            cf.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;
            cf.rgbColors = txtcolor;
            if (ChooseFont(&cf)) {
                txtcolor = cf.rgbColors;

                HFONT hFont = CreateFontIndirect(&lf);
                SendMessage(Edit, WM_SETFONT, (WPARAM)hFont, TRUE);

                CHARFORMAT2 cf2 = {};
                cf2.cbSize = sizeof(CHARFORMAT2);
                cf2.dwMask = CFM_COLOR;
                cf2.crTextColor = txtcolor;
                SendMessage(Edit, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf2);
            }
            break;
        case ID_EXIT_PROGRAM:
            if (MessageBox(win, L"Do you want to exit?", L"Question", MB_YESNO | MB_ICONQUESTION) == IDYES) ExitProcess(0);
            break;
        }
        return 0;
    }
    case WM_CREATE:
        Edit = CreateWindowEx(
            0,
            MSFTEDIT_CLASS,
            NULL,
            WS_CHILD | WS_EX_CLIENTEDGE | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY,
            0, 0, sx, sy,
            hwnd,
            NULL,
            ((LPCREATESTRUCT)lParam)->hInstance,
            NULL
        );
        if (Edit) {
            SendMessage(Edit, EM_SETBKGNDCOLOR, 0, bgcolor);

            CHARFORMAT2 cf;
            cf.cbSize = sizeof(CHARFORMAT2);
            cf.dwMask = CFM_BOLD | CFM_COLOR | CFM_FACE | CFM_SIZE;
            cf.dwEffects = 0;
            cf.crTextColor = txtcolor;
            wcscpy_s(cf.szFaceName, font.c_str());
            cf.yHeight = points * 20;
            SendMessage(Edit, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
            SetWindowLongPtr(Edit, GWL_STYLE, GetWindowLongPtr(Edit, GWL_STYLE) | ES_READONLY);
            CreateCaret(Edit, NULL, 20, 1);
#ifdef SEND_INITIALIZED_MSG
            SendMessage(Edit, EM_REPLACESEL, 0, (LPARAM)L"Logging...\r\n");
            SendMessage(Edit, EM_SETSEL, -1, -1);
#endif
        }
        break;
    case WM_SIZE:
        MoveWindow(Edit, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

/* Print functions */
void logs::window::rawPrint(std::wstring str) {
    CreateCaret(Edit, NULL, 1, 1);
    ShowCaret(Edit);
    SendMessage(Edit, EM_SETSEL, -1, -1);
    SendMessage(Edit, EM_REPLACESEL, 0, (LPARAM)str.c_str());
    SendMessage(Edit, EM_SETSEL, -1, -1);
    SendMessage(Edit, WM_VSCROLL, SB_BOTTOM, 0);
}
template<typename T>
void logs::window::Print(T str) {
    ss << str;
    rawPrint(ss.str());
    ss.str(L""); // Czyścimy bufor
    ss.clear(); // Resetujemy flagi strumienia
}

#define LOGWINDOW_F /* Functions code */
#endif/*logwindow_f*/
#else/*__cplusplus*/
#error THIS LIB IS ONLY FOR C++ AT THE MOMENT!
#endif/*__cplusplus*/
#else
#error ONLY MSVC IS SUPPORTED!
#endif/*compilers*/
#elif _WIN32
#ifdef _MSC_VER
#ifdef __cplusplus
#ifndef LOGWINDOW_H
#include <string>
#include <sstream>
#include <windows.h>
#include <commctrl.h>
#include <richedit.h>
#include <atomic>
#include <thread>

#pragma comment(lib, "comctl32.lib")

#define ID_EXIT_PROGRAM 254
#define ID_FILE_SAVE 250
#define ID_OPTIONS_FONT 251
#define ID_OPTIONS_COLOR 252
#define ID_OPTIONS_BGCOLOR 253
class WINExcept : public std::exception {
private:
std::string message; public:
    explicit WINExcept(const std::string& msg) : message(msg) {}

    const char* what() const noexcept override {
        return message.c_str();
    }
};
namespace logs {
    class window {
    public:
        window(std::wstring title) : name(title) { LoadLibrary(TEXT("Msftedit.dll")); };
        window(std::string title) : name(std::wstring(title.begin(), title.end())) { LoadLibrary(TEXT("Msftedit.dll")); };
        window(std::wstring title, int sizex, int sizey) : name(title) { setstaticvars(sizex, sizey); LoadLibrary(TEXT("Msftedit.dll")); };
        window(std::string title, int sizex, int sizey) : name(std::wstring(title.begin(), title.end())) { setstaticvars(sizex, sizey); LoadLibrary(TEXT("Msftedit.dll")); };
        window(int sizex, int sizey) { setstaticvars(sizex, sizey); LoadLibrary(TEXT("Msftedit.dll")); };
        template <typename T>
        window& operator<<(const T& value);
        static void SetColor(COLORREF textcolor, COLORREF backgroundcolor);
        static void SetFont(int Points, std::wstring Font);
        template<typename T>
        void Print(T str);
        void Create();
        void Close();
    private:
        /* static */
        static int points;
        static std::wstring font;
        static COLORREF txtcolor;
        static COLORREF bgcolor;
        static HMENU hMenu;
        static HMENU hOptionsMenu;
        static HMENU hFileMenu;
        static void setstaticvars(int sizex, int sizey);
        static void uninit();
        static HWND win;
        static HWND Edit;
        static int sx;
        static int sy;
        static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        static void setmenu();



        /* nonstatic */
        std::atomic<bool> running = false;
        bool created = false;
        std::wstring name = L"Logs 1.0 by Testmina77";
        void thr_createwin();
        void rawPrint(std::wstring str);
        std::wstringstream ss; // for operator <<
    };
}
#define LOGWINDOW_H
#endif/*logwindow_h*/
#ifndef LOGWINDOW_F

/*======== variables ========*/

/* font */
int logs::window::points = 12;
std::wstring logs::window::font = L"Consolas";

/* hwnds */
HWND logs::window::win = nullptr;
HWND logs::window::Edit = nullptr;

/* colors */
COLORREF logs::window::txtcolor = RGB(0, 0, 0);
COLORREF logs::window::bgcolor = RGB(240, 240, 240);

/* menu bar */
HMENU logs::window::hMenu = nullptr;
HMENU logs::window::hOptionsMenu = nullptr;
HMENU logs::window::hFileMenu = nullptr;

/* window size */
int logs::window::sx = 300;
int logs::window::sy = 300;

/*======== functions ========*/

/* operator << */
template <typename T>
logs::window& logs::window::operator<<(const T& value) {
    ss << value; // Dodajemy wartość do bufora
    rawPrint(ss.str());
    ss.str(L""); // Czyścimy bufor
    ss.clear(); // Resetujemy flagi strumienia
    return *this;
}

/* Init helper */
void logs::window::setstaticvars(int sizex, int sizey) {
    sx = sizex;
    sy = sizey;
}

/* Colors */
void logs::window::SetColor(COLORREF textcolor, COLORREF backgroundcolor) {
    txtcolor = textcolor;
    bgcolor = backgroundcolor;
}
void logs::window::SetFont(int Points, std::wstring Font) {
    font = Font;
    points = Points;
}

/* Window Creation */
void logs::window::Create() {
    if (!created && !running) {
        running = true;
        created = true;
        thr_createwin();
    }
}
void logs::window::Close() {
    if (created && running) running = false;
    else if ((created && !running) || (!created && running)) {
        throw WINExcept("Error: Cannot close (created && !running) || (!created && running)");
    }
}
//create menu
void logs::window::setmenu() {
    hMenu = CreateMenu();

    hFileMenu = CreatePopupMenu();

    AppendMenu(hFileMenu, MF_STRING, ID_FILE_SAVE, L"&Save Logs");
    hOptionsMenu = CreatePopupMenu();
    AppendMenu(hOptionsMenu, MF_STRING, ID_OPTIONS_FONT, L"&Font");
    AppendMenu(hOptionsMenu, MF_STRING, ID_OPTIONS_COLOR, L"&Text Color");
    AppendMenu(hOptionsMenu, MF_STRING, ID_OPTIONS_BGCOLOR, L"&Background Color");

    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, L"&File");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hOptionsMenu, L"&Options");
    AppendMenu(hMenu, MF_STRING, ID_EXIT_PROGRAM, L"&Exit program");

    SetMenu(win, hMenu);
}
void logs::window::thr_createwin() {
    std::thread([this] {
        WNDCLASS wc = {};
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = name.c_str();

        RegisterClass(&wc);
        win = CreateWindowEx(
            WS_EX_TOPMOST,
            name.c_str(),
            name.c_str(),
            WS_OVERLAPPEDWINDOW & ~(WS_SIZEBOX | WS_MAXIMIZEBOX) | WS_POPUP,
            CW_USEDEFAULT, CW_USEDEFAULT, sx, sy,
            NULL,
            NULL,
            wc.hInstance,
            NULL
        );
        HMENU hSysMenu = GetSystemMenu(win, FALSE);

        if (hSysMenu != NULL) {
            EnableMenuItem(hSysMenu, SC_CLOSE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        }
        setmenu();
        ShowWindow(win, SW_SHOW);
        UpdateWindow(win);

        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0) && running) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        }).detach();
}

/* WindowProc */
LRESULT CALLBACK logs::window::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    COLORREF customColors[16] = {
    RGB(255, 0, 0),
    RGB(0, 255, 0),
    RGB(0, 0, 255),
    RGB(255, 255, 0),
    RGB(0, 255, 255),
    RGB(255, 0, 255),
    RGB(192, 192, 192),
    RGB(128, 128, 128),
    RGB(255, 165, 0),
    RGB(0, 128, 128),
    RGB(128, 0, 128),
    RGB(255, 255, 255),
    RGB(0, 0, 0),
    RGB(0, 128, 0),
    RGB(128, 0, 0),
    RGB(0, 0, 128) };
    CHOOSECOLOR cc;
    switch (uMsg) {
    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case ID_FILE_SAVE:
            OPENFILENAME ofn;
            TCHAR szFile[260];
            ZeroMemory(&ofn, sizeof(ofn));
            ZeroMemory(szFile, sizeof(szFile));

            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFile = szFile;
            ofn.lpstrFile[0] = '\0';
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrFilter = L"All Files (*.*)\0*.*\0";
            ofn.nFilterIndex = 1;
            ofn.lpstrFileTitle = NULL;
            ofn.nMaxFileTitle = 0;
            ofn.lpstrInitialDir = NULL;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;

            if (GetSaveFileName(&ofn))
            {
                int length = GetWindowTextLength(Edit);
                if (length > 0)
                {
                    TCHAR* buffer = new TCHAR[length + 1];
                    GetWindowText(Edit, buffer, length + 1);
                    HANDLE hFile = CreateFileW(ofn.lpstrFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

                    if (hFile != INVALID_HANDLE_VALUE)
                    {
                        DWORD written;
                        WriteFile(hFile, buffer, lstrlen(buffer) * sizeof(TCHAR), &written, NULL);
                        CloseHandle(hFile);
                        MessageBox(win, L"Saved succesfully!", L"Info", MB_OK);
                    }
                    else
                    {
                        MessageBox(win, L"Cannot open file to save!", L"ERROR", MB_OK | MB_ICONERROR);
                    }
                    delete[] buffer;

                }
            }
            break;
        case ID_OPTIONS_COLOR:
            ZeroMemory(&cc, sizeof(cc));
            cc.lStructSize = sizeof(cc);
            cc.hwndOwner = hwnd;
            cc.lpCustColors = customColors;
            cc.Flags = CC_FULLOPEN | CC_RGBINIT;
            cc.rgbResult = RGB(0, 0, 0);
            if (ChooseColor(&cc)) {
                CHARFORMAT cf;
                cf.cbSize = sizeof(CHARFORMAT);
                cf.dwMask = CFM_COLOR;
                cf.crTextColor = RGB(GetRValue(cc.rgbResult), GetGValue(cc.rgbResult), GetBValue(cc.rgbResult));                 cf.dwEffects = 0;
                txtcolor = RGB(GetRValue(cc.rgbResult), GetGValue(cc.rgbResult), GetBValue(cc.rgbResult));
                SendMessage(Edit, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
            }
            break;
        case ID_OPTIONS_BGCOLOR:
            ZeroMemory(&cc, sizeof(cc));
            cc.lStructSize = sizeof(cc);
            cc.hwndOwner = win;
            cc.lpCustColors = customColors;
            cc.Flags = CC_FULLOPEN | CC_RGBINIT;
            cc.rgbResult = RGB(255, 255, 255);

            if (ChooseColor(&cc)) {
                bgcolor = RGB(GetRValue(cc.rgbResult), GetGValue(cc.rgbResult), GetBValue(cc.rgbResult));
                SendMessage(Edit, EM_SETBKGNDCOLOR, 0, RGB(GetRValue(cc.rgbResult), GetGValue(cc.rgbResult), GetBValue(cc.rgbResult)));
            }
            break;
        case ID_OPTIONS_FONT:
            CHOOSEFONT cf;
            LOGFONT lf;

            ZeroMemory(&cf, sizeof(cf));
            ZeroMemory(&lf, sizeof(lf));
            cf.lStructSize = sizeof(cf);
            cf.hwndOwner = win;
            cf.lpLogFont = &lf;
            cf.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;
            cf.rgbColors = txtcolor;
            if (ChooseFont(&cf)) {
                txtcolor = cf.rgbColors;

                HFONT hFont = CreateFontIndirect(&lf);
                SendMessage(Edit, WM_SETFONT, (WPARAM)hFont, TRUE);

                CHARFORMAT2 cf2 = {};
                cf2.cbSize = sizeof(CHARFORMAT2);
                cf2.dwMask = CFM_COLOR;
                cf2.crTextColor = txtcolor;
                SendMessage(Edit, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf2);
            }
            break;
        case ID_EXIT_PROGRAM:
            if (MessageBox(win, L"Do you want to exit?", L"Question", MB_YESNO | MB_ICONQUESTION) == IDYES) ExitProcess(0);
            break;
        }
        return 0;
    }
    case WM_CREATE:
        Edit = CreateWindowEx(
            0,
            MSFTEDIT_CLASS,
            NULL,
            WS_CHILD | WS_EX_CLIENTEDGE | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY,
            0, 0, sx, sy,
            hwnd,
            NULL,
            ((LPCREATESTRUCT)lParam)->hInstance,
            NULL
        );
        if (Edit) {
            SendMessage(Edit, EM_SETBKGNDCOLOR, 0, bgcolor);

            CHARFORMAT2 cf;
            cf.cbSize = sizeof(CHARFORMAT2);
            cf.dwMask = CFM_BOLD | CFM_COLOR | CFM_FACE | CFM_SIZE;
            cf.dwEffects = 0;
            cf.crTextColor = txtcolor;
            wcscpy_s(cf.szFaceName, font.c_str());
            cf.yHeight = points * 20;
            SendMessage(Edit, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
            SetWindowLongPtr(Edit, GWL_STYLE, GetWindowLongPtr(Edit, GWL_STYLE) | ES_READONLY);
            CreateCaret(Edit, NULL, 20, 1);
#ifdef SEND_INITIALIZED_MSG
            SendMessage(Edit, EM_REPLACESEL, 0, (LPARAM)L"Logging...\r\n");
            SendMessage(Edit, EM_SETSEL, -1, -1);
#endif
        }
        break;
    case WM_SIZE:
        MoveWindow(Edit, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

/* Print functions */
void logs::window::rawPrint(std::wstring str) {
    CreateCaret(Edit, NULL, 1, 1);
    ShowCaret(Edit);
    SendMessage(Edit, EM_SETSEL, -1, -1);
    SendMessage(Edit, EM_REPLACESEL, 0, (LPARAM)str.c_str());
    SendMessage(Edit, EM_SETSEL, -1, -1);
    SendMessage(Edit, WM_VSCROLL, SB_BOTTOM, 0);
}
template<typename T>
void logs::window::Print(T str) {
    ss << str;
    rawPrint(ss.str());
    ss.str(L""); // Czyścimy bufor
    ss.clear(); // Resetujemy flagi strumienia
}

#define LOGWINDOW_F
#endif/*logwindow_f*/
#else/*__cplusplus*/
#error THIS LIB IS ONLY FOR C++!
#endif/*__cplusplus*/
#else
#error ONLY MSVC IS SUPPORTED!
#endif/*compilers*/
#else
#error ONLY SYSTEM WINDOWS IS SUPPORTED!
#endif/*OS*/
