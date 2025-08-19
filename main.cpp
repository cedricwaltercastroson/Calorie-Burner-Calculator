#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <string>
#include <sstream>
#include <cstdio>

#define IDC_WEIGHT 101
#define IDC_CALC   102
#define IDC_OUTPUT 103

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Subclass procedure to handle Enter in the edit control
WNDPROC EdirProc;
LRESULT CALLBACK EditSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_GETDLGCODE:
        return DLGC_WANTALLKEYS | DLGC_WANTMESSAGE; // allow Enter
    case WM_KEYDOWN:
        if (wParam == VK_RETURN)
        {
            // Trigger calculation
            SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_CALC, BN_CLICKED), 0);
            return 0; // still needed
        }
        else if (wParam == VK_ESCAPE)
        {
            PostQuitMessage(0); // Quit the program
        }
        break;
    case WM_CHAR:
        if (wParam == VK_RETURN)
        {
            return 0; // suppress the beep here
        }
        else if (wParam == VK_ESCAPE)
        {
            return 0; // suppress the beep here
        }
        break;
    }
    return CallWindowProc(EdirProc, hwnd, msg, wParam, lParam);
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR pCmdLine, _In_ int nCmdShow)
{
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"CaloriesBurnerCalcApp";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClass(&wc);

    HWND hwnd = CreateWindow(
        L"CaloriesBurnerCalcApp",
        L"Calories Burner Calculator",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, // no maximize, no resize
        CW_USEDEFAULT, CW_USEDEFAULT, 1057, 320,
        NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static HWND hEditWeight, hButtonCalc, hEditOutput;

    switch (msg)
    {
    case WM_CREATE:
    {
        HFONT hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Consolas");

        HWND hStaticWeight = CreateWindow(L"static", L"Enter Weight(kg):",
            WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE,
            20, 20, 140, 25, hwnd, NULL, NULL, NULL);
        SendMessage(hStaticWeight, WM_SETFONT, (WPARAM)hFont, TRUE);

        hEditWeight = CreateWindow(L"edit", L"",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
            160, 20, 50, 25, hwnd, (HMENU)IDC_WEIGHT, NULL, NULL);
        SendMessage(hEditWeight, WM_SETFONT, (WPARAM)hFont, TRUE);

        SetFocus(hEditWeight);

        // Subclass the edit control so Enter works
        EdirProc = (WNDPROC)SetWindowLongPtr(hEditWeight, GWLP_WNDPROC, (LONG_PTR)EditSubclassProc);

        hButtonCalc = CreateWindow(L"button", L"Calculate",
            WS_CHILD | WS_VISIBLE,
            210, 18, 100, 27, hwnd, (HMENU)IDC_CALC, NULL, NULL);
        SendMessage(hButtonCalc, WM_SETFONT, (WPARAM)hFont, TRUE);

        hEditOutput = CreateWindow(L"edit", L"",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
            20, 60, 1000, 200,
            hwnd, (HMENU)IDC_OUTPUT, NULL, NULL);
        SendMessage(hEditOutput, WM_SETFONT, (WPARAM)hFont, TRUE);
        break;
    }

    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_CALC)
        {
            wchar_t buf[256];
            GetWindowText(hEditWeight, buf, sizeof(buf) / sizeof(wchar_t));
            double weight = _wtof(buf);
            if (weight <= 0)
            {
                MessageBox(hwnd, L"Please enter a valid weight!", L"Error", MB_OK | MB_ICONERROR);
                break;
            }

            const double stepLength = 0.74;
            const double calorieFactor = 1.05;
            std::wostringstream woss;

            // Displays the output depending on what weight was entered by the user.
            woss << L"Weight entered:\t" << weight << L" kilograms.\t\t\t\t\t\t\t\t\t\t\t\t";

            // Header row
            woss << L"Steps\tDistance(km)\tCalories\t2km/h Dur.\t4km/h Dur.\t6km/h Dur.\t8km/h Dur.\t10km/h Dur.\n";

            for (int steps = 5000; steps <= 30000; steps += 5000)
            {
                double distanceKm = steps * stepLength / 1000.0;
                double calories = distanceKm * weight * calorieFactor;
                double t2 = distanceKm / 2.0;
                double t4 = distanceKm / 4.0;
                double t6 = distanceKm / 6.0;
                double t8 = distanceKm / 8.0;
                double t10 = distanceKm / 10.0;

                int h2 = (int)t2, m2 = (int)((t2 - h2) * 60);
                int h4 = (int)t4, m4 = (int)((t4 - h4) * 60);
                int h6 = (int)t6, m6 = (int)((t6 - h6) * 60);
                int h8 = (int)t8, m8 = (int)((t8 - h8) * 60);
                int h10 = (int)t10, m10 = (int)((t10 - h10) * 60);

                woss << L"\t\t" << steps << L"\t"
                    << distanceKm << L"\t\t"
                    << (int)calories << L"\t\t"
                    << h2 << L"h" << m2 << L"m\t\t"
                    << h4 << L"h" << m4 << L"m\t\t"
                    << h6 << L"h" << m6 << L"m\t\t"
                    << h8 << L"h" << m8 << L"m\t\t"
                    << h10 << L"h" << m10 << L"m\t";
            }

            SetWindowText(hEditOutput, woss.str().c_str());
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}
