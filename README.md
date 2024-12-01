# Logs Gui
A lightweight, header-only C++ Gui library for logs.
## What can you do with it?
- You can print by Print function or with << operator (like in std::cout).
- You can set font and font size.
- You can change text and background color.
- You can save logs to file ("File->Save logs" from menu bar).
- You can change colors and font in Options from menu bar.
## Usage
Example code:
```cpp
#include <LogsGui.h>

int main() {

    logs::window window("Logs 1.0 by Testmina77", 720, 480);

    window.SetColor(RGB(50, 200, 10), RGB(0, 0, 0));
    window.SetFont(12, L"Arial");
    window.Create();

    for (int i = 0; i < 27; i+= 2) {
        window.Print(L"Log: " + std::to_wstring(i) + L"\n");
        Sleep(500);
        window << L"Log: " << i + 1 << L"\n"; //operator << example
        Sleep(500);
    }

    window.Close();
    return 0;
}
```
Effect:

![How it looks](https://raw.githubusercontent.com/testmina77/LogsGui/refs/heads/main/log%20program.PNG)
### Other info:
- This library is only for Windows.
- It doesn't support other compilers than MSVC.
- This version is for C++ only.

Btw. This is my first project so if you find any bugs, make an issue :D.
