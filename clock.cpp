#include <windows.h>
#include <stdio.h>
#include <time.h>

// 全局变量，用于保存窗口句柄和字体句柄
HWND g_hwnd;
HFONT g_hFont = NULL;

// 窗口过程函数，处理窗口相关消息
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// 初始化 LOGFONT 结构体并设置字体大小
void InitializeLogFont(LOGFONT& lf, LONG height, LPCSTR faceName) {
	memset(&lf, 0, sizeof(LOGFONT)); // 清零
	lf.lfHeight = height; // 设置字体高度
	lf.lfWidth = 0; // 设置字体宽度
	lf.lfWeight = FW_NORMAL; // 设置字体粗细
	lf.lfItalic = 0; // 设置是否斜体
	lf.lfUnderline = 0; // 设置是否下划线
	lf.lfStrikeOut = 0; // 设置是否删除线
	lf.lfCharSet = DEFAULT_CHARSET; // 设置字符集
	lf.lfOutPrecision = 1; // 设置输出精度
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS; // 设置剪切精度
	lf.lfQuality = DEFAULT_QUALITY; // 设置输出质量
	lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE; // 设置间距和字体族
	
	// 复制字体名称
	strncpy(lf.lfFaceName, faceName, LF_FACESIZE - 1); // 确保不会超出数组界限
	lf.lfFaceName[LF_FACESIZE - 1] = '\0'; // 确保字符串以空字符结尾
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow){
	static TCHAR szAppName[] = TEXT("TopMostTimeLabel");
	HWND hwnd;
	MSG msg;
	WNDCLASS wndclass;
	
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);  // 设置背景为黑色
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szAppName;
	
	if (!RegisterClass(&wndclass)){
		MessageBox(NULL, TEXT("这个程序需要Windows NT!"),
			szAppName, MB_ICONERROR);
		return 0;
	}
	
	// 创建窗口，设置其初始位置和大小等
	hwnd = CreateWindow(szAppName, TEXT("clock"),
		WS_POPUP | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT, 160, 32,
		NULL, NULL, hInstance, NULL);
	g_hwnd = hwnd;  // 保存窗口句柄到全局变量
	
	// 设置窗口始终在最上层
	SetWindowPos(hwnd, HWND_TOPMOST, 860, 20, 260,64, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOACTIVATE);
	// 设置窗口扩展样式，使其不在任务栏显示，并启用分层窗口
	SetWindowLongPtr(hwnd, GWL_EXSTYLE, GetWindowLongPtr(hwnd, GWL_EXSTYLE) & ~WS_EX_APPWINDOW | WS_EX_TOOLWINDOW | WS_EX_LAYERED);
	
	// 设置分层窗口的透明度（这里先设置为半透明，透明度值为0 - 255，0为完全透明，255为完全不透明）
	// 并且设置键盘值，使背景透明而文字不透明
	COLORREF key = RGB(0, 0, 0); // 黑色背景
	BYTE alpha = 0; // 完全透明
	SetLayeredWindowAttributes(hwnd, key, alpha, LWA_COLORKEY);
	
	while(GetMessage(&msg, NULL, 0, 0)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

// 绘制文本的函数，类似Label显示效果
void DrawTimeText(HDC hdc){
	time_t currentTime; // 保存当前时间
	struct tm *timeInfo; // 保存时间信息
	char timeStr[9]; // 保存时间字符串
	time(&currentTime); // 获取当前时间
	timeInfo = localtime(&currentTime); // 转换为本地时间
	// 格式化时间字符串为"HH:MM:SS"
	strftime(timeStr, sizeof(timeStr), "%H:%M:%S", timeInfo);
	
	// 设置文本颜色为白色
	SetTextColor(hdc, RGB(255, 255, 255));
	// 设置文本背景模式为黑色不透明
	SetBkMode(hdc, TRANSPARENT);
	// 选择字体到设备上下文
	SelectObject(hdc, g_hFont);
	// 获取窗口客户区大小，用于确定文本绘制位置
	RECT rect;
	GetClientRect(g_hwnd, &rect);
	// 计算文本绘制的水平和垂直居中位置
	int x = (rect.right - rect.left) / 2;
	int y = (rect.bottom - rect.top) / 2;
	// 绘制文本
	// 如果时间为以下时间，改变文本颜色为红色
	// 1. 7:50~7:55
	// 2. 8:35~8:40
	// 3. 9:30~9:35
	// 4. 10:40~10:45
	// 5. 11:35~11:40
	// 6. 12:45~12:50
	// 7. 13:35~13:40
	// 8. 14:30~14:35
	// 9. 15:40~15:45
	// 10.16:35~16:40
	// 11.17:15~17:20
	// 12.17:55~18:00
	
	int hour = timeInfo->tm_hour;
	int minute = timeInfo->tm_min;
	int second = timeInfo->tm_sec;
	if (hour == 7 && minute >= 50 && minute < 55) {
		SetTextColor(hdc, RGB(255, 60, 60));
	}
	else if (hour == 8 && minute >= 35 && minute < 40) {
		SetTextColor(hdc, RGB(111,86,238));
	}
	else if (hour == 9 && minute >= 30 && minute < 35) {
		SetTextColor(hdc, RGB(128,128,156));
	}
	else if (hour == 10 && minute >= 40 && minute < 45) {
		SetTextColor(hdc, RGB(156,242,190));
	}
	else if (hour == 11 && minute >= 35 && minute < 40) {
		SetTextColor(hdc, RGB(108,252,98));
	}
	else if (hour == 12 && minute >= 45 && minute < 50) {
		SetTextColor(hdc, RGB(231,252,98));
	}
	else if (hour == 13 && minute >= 35 && minute < 40) {
		SetTextColor(hdc, RGB(252,216,98));
	}
	else if (hour == 14 && minute >= 30 && minute < 35) {
		SetTextColor(hdc, RGB(255,128,0));
	}
	else if (hour == 15 && minute >= 40 && minute < 45) {
		SetTextColor(hdc, RGB(224,224,224));
	}
	else if (hour == 16 && minute >= 35 && minute < 40) {
		SetTextColor(hdc, RGB(255, 0, 0));
	}
	else if (hour == 17 && minute >= 15 && minute < 20) {
		SetTextColor(hdc, RGB(255, 0, 0));
	}
	else if (hour == 17 && minute >= 55 && minute < 60) {
		SetTextColor(hdc, RGB(255, 0, 0));
	}
	else {
		SetTextColor(hdc, RGB(85,107,47));
	}
	DrawText(hdc, timeStr, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

// 处理消息
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_CREATE:
		SetTimer(hwnd, 1, 1000, NULL);  // 每隔1秒触发一次定时器消息
		
		LOGFONT lf;
		InitializeLogFont(lf, 40, "Consolas"); // 设置字体大小为40，字体名称为"Consolas"
		
		// 创建字体
		g_hFont = CreateFontIndirect(&lf);
		break;
		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			DrawTimeText(hdc);  // 绘制时间文本
			EndPaint(hwnd, &ps);
			break;
		}
	case WM_TIMER:
		InvalidateRect(hwnd, NULL, TRUE);  // 使窗口客户区无效，触发重绘以更新时间显示
		break;
	case WM_DESTROY:
		KillTimer(hwnd, 1);
		PostQuitMessage(0);
		// 删除字体
		if (g_hFont) {
			DeleteObject(g_hFont);
			g_hFont = NULL;
		}
		break;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}
