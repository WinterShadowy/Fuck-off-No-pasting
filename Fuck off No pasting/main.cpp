#include <windows.h>
#include <CommCtrl.h>
#include <string.h>
#include <strsafe.h>
#include <tchar.h>
#include <atomic>
#include <string>
#include <random>
#include <thread>
#include <utility>

#define Start_Button 0x1
#define Clear_Button 0x4
#define ID_STATIC 0x3

TCHAR text[100000];
HINSTANCE hIns;

// 定义全局变量
HWND hEdit;
HWND hButton;
HWND tishi;
HWND hSpin_Min;
HWND hSpin_Max;
HWND hEditSpin_Min;
HWND hEditSpin_Max;

int count = 0, strLeng = 0;

std::atomic<bool> run(true); // 控制线程的运行状态
std::atomic<bool> paused(false);  // 控制暂停状态
std::thread inputThread;

#pragma comment(lib, "comctl32.lib")

// 模拟输入的函数
void SimulateInput()
{
    while (run)
    {
        // 获取 SpinBox 的值
        TCHAR minText[10], maxText[10];
        GetWindowText(hEditSpin_Min, minText, 10);
        GetWindowText(hEditSpin_Max, maxText, 10);
        int minTime = _ttoi(minText); // 转换为整数
        int maxTime = _ttoi(maxText);
        if(minTime > maxTime)
            std::swap(minTime, maxTime);

        // 随机数生成器
        std::random_device rd; // 随机设备
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(minTime, maxTime); // 定义范围

        // 模拟键盘输入
        INPUT input;
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = 0;
        input.ki.wScan = 0;
        input.ki.dwFlags = KEYEVENTF_UNICODE;
        input.ki.time = 0;
        input.ki.dwExtraInfo = 0;

        for (int i = 0; i < (int)_tcslen(text); i++)
        {
            ++count;
            // 如果是回车符，模拟输入回车键
            if (text[i] == TEXT('\r'))
            {
                input.ki.wVk = VK_RETURN;
                input.ki.dwFlags = 0;
                SendInput(1, &input, sizeof(INPUT));
                input.ki.wVk = 0;
                input.ki.dwFlags = KEYEVENTF_UNICODE;
            }
            else
            {
                input.ki.wScan = text[i];
                SendInput(1, &input, sizeof(INPUT));
            }
            Sleep(dis(gen)); // 使用随机值作为休眠时间
            if(count == strLeng)
			{
				run = false;
                return;
			}
        }
    }
}

// 窗口过程函数
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    int sel;
    switch (uMsg)
    {
        case WM_CREATE:
            // 创建文本框
            hEdit = CreateWindowEx(
                WS_EX_CLIENTEDGE, TEXT("EDIT"), NULL,
                WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | WS_VSCROLL | WS_HSCROLL,
                0, 0, 783, 500, hwnd, NULL, NULL, NULL);

            // 创建文本输入按钮
            hButton = CreateWindow(
                TEXT("BUTTON"), TEXT("输入文本"),
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                3, 510, 70, 30, hwnd, (HMENU)Start_Button, NULL, NULL);

            // 创建提示
            tishi = CreateWindow(
                TEXT("STATIC"), TEXT("输入开始还有: ? s"),
                WS_VISIBLE | WS_CHILD,
                100, 510, 150, 30, hwnd, (HMENU)ID_STATIC, hIns, NULL);

            // 创建清空按钮
            CreateWindow(
                TEXT("BUTTON"), TEXT("清空编辑框"),
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                250, 510, 90, 30, hwnd, (HMENU)Clear_Button, hIns, NULL);

            CreateWindow(
                TEXT("STATIC"), TEXT("| 输入间隔时间  |\n| 左最小右最大  |\n| 单位 ： ms      |"),
                WS_VISIBLE | WS_CHILD,
                350, 510, 150, 50, hwnd, NULL, hIns, NULL);

            hEditSpin_Min = CreateWindow(TEXT("EDIT"), TEXT("5"),
                WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
                470, 500 + 10, 60, 30, hwnd, NULL, hIns, NULL);
            hSpin_Min = CreateWindow(UPDOWN_CLASS, NULL,
                WS_CHILD | WS_VISIBLE | UDS_SETBUDDYINT | UDS_ALIGNRIGHT | UDS_ARROWKEYS,
                530, 500 + 10, 30, 30, hwnd, (HMENU)5, hIns, NULL);
            SendMessage(hSpin_Min, UDM_SETBUDDY, (WPARAM)hEditSpin_Min, 0);  // 绑定伙伴
            SendMessage(hSpin_Min, UDM_SETRANGE, 0, MAKELPARAM(10000, 1));  // 设置范围
            SendMessage(hSpin_Min, UDM_SETPOS, 0, 5);  // 设置初始值为 5ms

            hEditSpin_Max = CreateWindow(TEXT("EDIT"), TEXT("25"),
                WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
                540, 500 + 10, 60, 30, hwnd, NULL, hIns, NULL);
            hSpin_Max = CreateWindow(UPDOWN_CLASS, NULL,
                WS_CHILD | WS_VISIBLE | UDS_SETBUDDYINT | UDS_ALIGNRIGHT | UDS_ARROWKEYS,
                600, 500 + 10, 30, 30, hwnd, (HMENU)6, hIns, NULL);
            SendMessage(hSpin_Max, UDM_SETBUDDY, (WPARAM)hEditSpin_Max, 0);  // 绑定伙伴
            SendMessage(hSpin_Max, UDM_SETRANGE, 0, MAKELPARAM(10000, 1));  // 设置范围
            SendMessage(hSpin_Max, UDM_SETPOS, 0, 10);  // 设置初始值为 10ms

            break;

        case WM_COMMAND:
            sel = LOWORD(wParam);
            if (sel == Start_Button)
            {
                ZeroMemory(text, sizeof(text));
                // 获取文本框内容
                if (GetWindowText(hEdit, text, 100000) == 0)
                {
                    MessageBox(NULL, TEXT("读取失败！"), TEXT("error: "), MB_OK);
                    break;
                }
                else
                {
                    strLeng = (int)_tcslen(text);
                    count = 0;
                }
                for (int i = 0; i <= 3; ++i)
                {
                    std::wstring _text = TEXT("开始输入还有: ") + std::to_wstring((3 - i)) + TEXT(" s");
                    SetWindowText(tishi, _text.c_str());
                    Sleep(1000);
                }
                // 启动输入线程
                if (inputThread.joinable()) 
                {
                    run = false; // 停止旧线程
                    inputThread.join(); // 等待线程结束
                }
                run = true;
                paused = true;
                inputThread = std::thread(SimulateInput);
            }
            else if (sel == Clear_Button)
            {
                SetWindowText(hEdit, TEXT(""));
            }
            break;

            // 防止没有停止线程
        case WM_DESTROY:
            run = false; // 停止线程
            if (inputThread.joinable()) 
            {
                inputThread.join(); // 等待线程结束
            }
            PostQuitMessage(0);
            break;
        case WM_CLOSE:
		    run = false; // 停止线程
		    if (inputThread.joinable()) 
		    {
			    inputThread.join(); // 等待线程结束
		    }
            DestroyWindow(hwnd);
		    break;
	    default:
		    return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    InitCommonControls(); // 初始化公共控件
    hIns = hInstance;

    // 注册窗口类
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.lpszClassName = TEXT("WindowClass");

    RegisterClass(&wc);

    // 创建窗口
    HWND hwnd = CreateWindowEx(0, TEXT("WindowClass"), TEXT("Fuck off no pasting"),
        (WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME),
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, hInstance, NULL);

    // 显示窗口
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // 消息循环
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}