#include "WindowsEngine.h"
#include "../../Debug/EngineDebug.h"
#include "../../Config/EngineRenderConfig.h"
#if defined(_WIN32)
#include "WindowsMessageProcessing.h"


FWindowsEngine::FWindowsEngine()
	:M4XNumQualityLevels(0)
	,bMSAA4XEnabled(false)
	,BufferFormat(DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM)
{

}

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

	if (InitDirect3D())
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
	FEngineRenderConfig::Destroy();
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

	RECT Rect = {0,0,FEngineRenderConfig::GetRenderConfig()->ScreenWidth,FEngineRenderConfig::GetRenderConfig()->ScreenHight};
	
	//@rect �ʿ�
	//WS_OVERLAPPEDWINDOW �ʿڷ��
	//NULL û�в˵�
	AdjustWindowRect(&Rect,WS_OVERLAPPEDWINDOW, NULL);

	int WindowWidth = Rect.right - Rect.left;
	int WindowHight = Rect.bottom - Rect.top;

	MainWindowsHandle = CreateWindowEx(
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
	if (!MainWindowsHandle)
	{
		Engine_Log_Error("CreateWindow Failed..");
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	//��ʾ����
	ShowWindow(MainWindowsHandle, SW_SHOW);

	//��������ģ�ˢ��һ��
	UpdateWindow(MainWindowsHandle);

	Engine_Log("InitWindows complete.");
}

bool FWindowsEngine::InitDirect3D()
{
	//HRESULT
	//S_OK                0x00000000
	//E_UNEXPECTED        0x8000FFFF  ����ʧ�� 
	//E_NOTIMPL           0x80004001  δʵ��
	//E_OUTOFMEMORY       0x8007000E  δ�ܷ��������ڴ�
	//E_INVALIDARS        0x80070057  һ������������Ч
	//E_NOINTERFACE       0x80004002  ��֧�ִ˽ӿ�
	//E_POINTER           0x80004003  ��Чָ��
	//E_HANDLE            0x80070006  ��Ч���
	//EABORT              0x80004004  ������ֹ
	//E_FAIL              0x80004005  ����
	//E_ACCESSDENIED      0x80070005  һ��ķ��ʱ��ܾ�����
	ANALYSIS_HRESULT(CreateDXGIFactory1(IID_PPV_ARGS(&DXGIFactory)));

	//��������
	HRESULT D3dDeviceResult = D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&D3dDevice));

	//���ʧ����warp
	if (FAILED(D3dDeviceResult))
	{
		//warp
		ComPtr<IDXGIAdapter> WARPAdapter;
		ANALYSIS_HRESULT(DXGIFactory->EnumWarpAdapter(IID_PPV_ARGS(&WARPAdapter)));

		ANALYSIS_HRESULT(D3D12CreateDevice(WARPAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&D3dDevice)));
	}

	//D3D12_FENCE_FLAG_NONE
	//D3D12_FENCE_FLAG_SHARED
	//D3D12_FENCE_FLAG_SHARED_CROSS_ADAPTER
	//����Fence���� ΪCPU��GPUͬ����׼��
	/*
	* Fence->SetEventOnCompletion
	* ִ������
	* �ύ����
	* �����ź�
	* �ȴ�
	*/
	ANALYSIS_HRESULT(D3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence)));

	//��ʼ���������
	//INT Priority Normal/High
	//UINT NodeMask ָʾ���������ĸ�GPU�ڵ���ִ��
	D3D12_COMMAND_QUEUE_DESC QueueDesc = {};
	QueueDesc.Type = D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT;//ֱ��
	QueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAGS::D3D12_COMMAND_QUEUE_FLAG_NONE;
	ANALYSIS_HRESULT(D3dDevice->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(&CommandQueue)));

	//����������
	ANALYSIS_HRESULT(D3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(CommandAllocator.GetAddressOf())));

	//���������б�
	/*
	* Ĭ�ϵ���CPU
	* ֱ��
	* ��CommandList������Allocator
	* ID3D12PipelineState
	*/
	ANALYSIS_HRESULT(D3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, CommandAllocator.Get(), NULL, IID_PPV_ARGS(GraphicsCommandList.GetAddressOf())));

	//���������б�
	GraphicsCommandList->Close();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//���ز���
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS QualityLevels;
	QualityLevels.SampleCount = 4;
	QualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVEL_FLAGS::D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	QualityLevels.NumQualityLevels = 0;
	
	D3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &QualityLevels, sizeof(QualityLevels));

	M4XNumQualityLevels = QualityLevels.NumQualityLevels;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//������
	SwapChain->Release();
	DXGI_SWAP_CHAIN_DESC SwapChainDesc;
	SwapChainDesc.BufferDesc.Width = FEngineRenderConfig::GetRenderConfig()->ScreenWidth;
	SwapChainDesc.BufferDesc.Height = FEngineRenderConfig::GetRenderConfig()->ScreenHight;
	SwapChainDesc.BufferDesc.RefreshRate.Numerator = FEngineRenderConfig::GetRenderConfig()->RefreshRate;
	SwapChainDesc.BufferDesc.RefreshRate .Denominator = 1;
	SwapChainDesc.BufferCount = FEngineRenderConfig::GetRenderConfig() ->SwapChainCount;
	SwapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER::DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;//ʹ�ñ������Դ��Ϊ�����ȾĿ��
	SwapChainDesc.OutputWindow = MainWindowsHandle;//ָ��Windows���
	SwapChainDesc.Windowed = true;//�Դ�������
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_FLIP_DISCARD;
	SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;//IDXGISwapChain::ResizeTarger()
	SwapChainDesc.BufferDesc.Format = BufferFormat;//��ʽ����

	//���ز�������
	SwapChainDesc.SampleDesc.Count = bMSAA4XEnabled ? 4 : 1;
	SwapChainDesc.SampleDesc.Quality = bMSAA4XEnabled ? (M4XNumQualityLevels - 1) : 0;

	DXGIFactory->CreateSwapChain(CommandQueue.Get(), &SwapChainDesc, SwapChain.GetAddressOf());


	return false;
}

#endif