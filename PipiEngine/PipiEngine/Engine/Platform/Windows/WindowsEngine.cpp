#include "WindowsEngine.h"

#if defined(_WIN32)
#include "WindowsMessageProcessing.h"

int FWindowsEngine::PreInit(FWinMainCommandParameters InParameters)
{
	//��־ϵͳ��ʼ��
	const char LogPath[] = "../log";
	init_log_system(LogPath);
	Engine_Log("Log Init.");

	//��������

	//

	if (InitWindows(InParameters))
	{

	}

	Engine_Log("Engine pre initialization complete.");
	return 0;
}

int FWindowsEngine::Init()
{


	Engine_Log("Engine initialization complete.");
	return 0;
}

int FWindowsEngine::PostInit()
{

	Engine_Log("Engine post initialization complete.");
	return 0;
}

void FWindowsEngine::Tick()
{

}

int FWindowsEngine::PreExit()
{


	Engine_Log("Engine post exit complete.");
	return 0;
}

int FWindowsEngine::Exit()
{

	Engine_Log("Engine exit complete.");
	return 0;
}

int FWindowsEngine::PostExit()
{

	Engine_Log("Engine post exit complete.");
	return 0;
}

bool FWindowsEngine::InitWindows(FWinMainCommandParameters InParameters)
{
	//ע�ᴰ��
	WNDCLASSEX WindowsClass;
	WindowsClass.cbSize = sizeof(WNDCLASSEX);//�ö���ʵ��ռ�ö���ڴ�
	WindowsClass.cbClsExtra = 0;//�Ƿ���Ҫ����ռ�
	WindowsClass.cbWndExtra = 0;//�Ƿ���Ҫ�����ڴ�
	WindowsClass.hbrBackground = nullptr;//����������ľ���GDI����
	WindowsClass.hCursor = LoadCursor(NULL, IDC_ARROW);//����һ����ͷ���
	WindowsClass.hIcon = nullptr; //Ӧ�ó�����ڴ�������ʾ��ͼ��
	WindowsClass.hIconSm = NULL;//Ӧ�ó�����ʾ�����Ͻǵ�ͼ��
	WindowsClass.hInstance = InParameters.HInstance; //����ʵ��
	WindowsClass.lpszClassName = L"PipiEngine";//��������
	WindowsClass.lpszMenuName = nullptr;//
	WindowsClass.style = CS_VREDRAW | CS_HREDRAW;//��ô���ƴ��� ��ֱ��ˮƽ�ػ�
	WindowsClass.lpfnWndProc = EngineWindowProc;//��Ϣ������

	//ע�ᴰ��
	ATOM RegisterAtom = RegisterClassEx(&WindowsClass);
	if (!RegisterAtom)
	{
		Engine_Log_Error("Register windows class fail.");
		MessageBox(NULL, L"Register windows class fail.", L"Error", MB_OK);
	}

	RECT Rect = {0,0,1280,720};
	
	//@rect �ʿ�
	//WS_OVERLAPPEDWINDOW �ʿڷ��
	//NULL û�в˵�
	AdjustWindowRect(&Rect,WS_OVERLAPPEDWINDOW, NULL);

	int WindowWidth = Rect.right - Rect.left;
	int WindowHight = Rect.bottom - Rect.top;

	HWND Hwnd = CreateWindowEx(
		NULL,//���ڶ���ķ��
		L"PipiEngine", // ��������
		L"PIPI Engine",//����ʾ�ڴ��ڵı�������ȥ
		WS_OVERLAPPEDWINDOW, //���ڷ��
		0,0,//���ڵ�����
		WindowWidth, WindowHight,//
		NULL, //�����ھ��
		nullptr, //�˵����
		InParameters.HInstance,//����ʵ��
		NULL);//
	if (!Hwnd)
	{
		Engine_Log_Error("CreateWindow Failed..");
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	//��ʾ����
	ShowWindow(Hwnd, SW_SHOW);

	//��������ģ�ˢ��һ��
	UpdateWindow(Hwnd);

	Engine_Log("InitWindows complete.");
}

#endif