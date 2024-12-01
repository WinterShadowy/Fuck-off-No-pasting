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

// ����ȫ�ֱ���
HWND hEdit;
HWND hButton;
HWND tishi;
HWND hSpin_Min;
HWND hSpin_Max;
HWND hEditSpin_Min;
HWND hEditSpin_Max;

int count = 0, strLeng = 0;

std::atomic<bool> run(true); // �����̵߳�����״̬
std::atomic<bool> paused(false);  // ������ͣ״̬
std::thread inputThread;

#pragma comment(lib, "comctl32.lib")

// ģ������ĺ���
void SimulateInput()
{
    while (run)
    {
        // ��ȡ SpinBox ��ֵ
        TCHAR minText[10], maxText[10];
        GetWindowText(hEditSpin_Min, minText, 10);
        GetWindowText(hEditSpin_Max, maxText, 10);
        int minTime = _ttoi(minText); // ת��Ϊ����
        int maxTime = _ttoi(maxText);
        if(minTime > maxTime)
            std::swap(minTime, maxTime);

        // �����������
        std::random_device rd; // ����豸
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(minTime, maxTime); // ���巶Χ

        // ģ���������
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
            // ����ǻس�����ģ������س���
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
            Sleep(dis(gen)); // ʹ�����ֵ��Ϊ����ʱ��
            if(count == strLeng)
			{
				run = false;
                return;
			}
        }
    }
}

// ���ڹ��̺���
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    int sel;
    switch (uMsg)
    {
        case WM_CREATE:
            // �����ı���
            hEdit = CreateWindowEx(
                WS_EX_CLIENTEDGE, TEXT("EDIT"), NULL,
                WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | WS_VSCROLL | WS_HSCROLL,
                0, 0, 783, 500, hwnd, NULL, NULL, NULL);

            // �����ı����밴ť
            hButton = CreateWindow(
                TEXT("BUTTON"), TEXT("�����ı�"),
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                3, 510, 70, 30, hwnd, (HMENU)Start_Button, NULL, NULL);

            // ������ʾ
            tishi = CreateWindow(
                TEXT("STATIC"), TEXT("���뿪ʼ����: ? s"),
                WS_VISIBLE | WS_CHILD,
                100, 510, 150, 30, hwnd, (HMENU)ID_STATIC, hIns, NULL);

            // ������հ�ť
            CreateWindow(
                TEXT("BUTTON"), TEXT("��ձ༭��"),
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                250, 510, 90, 30, hwnd, (HMENU)Clear_Button, hIns, NULL);

            CreateWindow(
                TEXT("STATIC"), TEXT("| ������ʱ��  |\n| ����С�����  |\n| ��λ �� ms      |"),
                WS_VISIBLE | WS_CHILD,
                350, 510, 150, 50, hwnd, NULL, hIns, NULL);

            hEditSpin_Min = CreateWindow(TEXT("EDIT"), TEXT("5"),
                WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
                470, 500 + 10, 60, 30, hwnd, NULL, hIns, NULL);
            hSpin_Min = CreateWindow(UPDOWN_CLASS, NULL,
                WS_CHILD | WS_VISIBLE | UDS_SETBUDDYINT | UDS_ALIGNRIGHT | UDS_ARROWKEYS,
                530, 500 + 10, 30, 30, hwnd, (HMENU)5, hIns, NULL);
            SendMessage(hSpin_Min, UDM_SETBUDDY, (WPARAM)hEditSpin_Min, 0);  // �󶨻��
            SendMessage(hSpin_Min, UDM_SETRANGE, 0, MAKELPARAM(10000, 1));  // ���÷�Χ
            SendMessage(hSpin_Min, UDM_SETPOS, 0, 5);  // ���ó�ʼֵΪ 5ms

            hEditSpin_Max = CreateWindow(TEXT("EDIT"), TEXT("25"),
                WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
                540, 500 + 10, 60, 30, hwnd, NULL, hIns, NULL);
            hSpin_Max = CreateWindow(UPDOWN_CLASS, NULL,
                WS_CHILD | WS_VISIBLE | UDS_SETBUDDYINT | UDS_ALIGNRIGHT | UDS_ARROWKEYS,
                600, 500 + 10, 30, 30, hwnd, (HMENU)6, hIns, NULL);
            SendMessage(hSpin_Max, UDM_SETBUDDY, (WPARAM)hEditSpin_Max, 0);  // �󶨻��
            SendMessage(hSpin_Max, UDM_SETRANGE, 0, MAKELPARAM(10000, 1));  // ���÷�Χ
            SendMessage(hSpin_Max, UDM_SETPOS, 0, 10);  // ���ó�ʼֵΪ 10ms

            break;

        case WM_COMMAND:
            sel = LOWORD(wParam);
            if (sel == Start_Button)
            {
                ZeroMemory(text, sizeof(text));
                // ��ȡ�ı�������
                if (GetWindowText(hEdit, text, 100000) == 0)
                {
                    MessageBox(NULL, TEXT("��ȡʧ�ܣ�"), TEXT("error: "), MB_OK);
                    break;
                }
                else
                {
                    strLeng = (int)_tcslen(text);
                    count = 0;
                }
                for (int i = 0; i <= 3; ++i)
                {
                    std::wstring _text = TEXT("��ʼ���뻹��: ") + std::to_wstring((3 - i)) + TEXT(" s");
                    SetWindowText(tishi, _text.c_str());
                    Sleep(1000);
                }
                // ���������߳�
                if (inputThread.joinable()) 
                {
                    run = false; // ֹͣ���߳�
                    inputThread.join(); // �ȴ��߳̽���
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

            // ��ֹû��ֹͣ�߳�
        case WM_DESTROY:
            run = false; // ֹͣ�߳�
            if (inputThread.joinable()) 
            {
                inputThread.join(); // �ȴ��߳̽���
            }
            PostQuitMessage(0);
            break;
        case WM_CLOSE:
		    run = false; // ֹͣ�߳�
		    if (inputThread.joinable()) 
		    {
			    inputThread.join(); // �ȴ��߳̽���
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
    InitCommonControls(); // ��ʼ�������ؼ�
    hIns = hInstance;

    // ע�ᴰ����
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.lpszClassName = TEXT("WindowClass");

    RegisterClass(&wc);

    // ��������
    HWND hwnd = CreateWindowEx(0, TEXT("WindowClass"), TEXT("Fuck off no pasting"),
        (WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME),
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, hInstance, NULL);

    // ��ʾ����
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // ��Ϣѭ��
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}