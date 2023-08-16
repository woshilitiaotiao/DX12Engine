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
	//日志系统初始化
	const char LogPath[] = "../log";
	init_log_system(LogPath);
	Engine_Log("Log Init.");

	//处理命令

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
	//注册窗口
	WNDCLASSEX WindowsClass;
	WindowsClass.cbSize = sizeof(WNDCLASSEX);//该对象实际占用多大内存
	WindowsClass.cbClsExtra = 0;//是否需要额外空间
	WindowsClass.cbWndExtra = 0;//是否需要额外内存
	WindowsClass.hbrBackground = nullptr;//如果有设置哪就是GDI擦除
	WindowsClass.hCursor = LoadCursor(NULL, IDC_ARROW);//设置一个箭头光标
	WindowsClass.hIcon = nullptr; //应用程序放在磁盘上显示的图标
	WindowsClass.hIconSm = NULL;//应用程序显示在左上角的图标
	WindowsClass.hInstance = InParameters.HInstance; //窗口实例
	WindowsClass.lpszClassName = L"PipiEngine";//窗口名字
	WindowsClass.lpszMenuName = nullptr;//
	WindowsClass.style = CS_VREDRAW | CS_HREDRAW;//怎么绘制窗口 垂直和水平重绘
	WindowsClass.lpfnWndProc = EngineWindowProc;//消息处理函数

	//注册窗口
	ATOM RegisterAtom = RegisterClassEx(&WindowsClass);
	if (!RegisterAtom)
	{
		Engine_Log_Error("Register windows class fail.");
		MessageBox(NULL, L"Register windows class fail.", L"Error", MB_OK);
	}

	RECT Rect = {0,0,FEngineRenderConfig::GetRenderConfig()->ScreenWidth,FEngineRenderConfig::GetRenderConfig()->ScreenHight};
	
	//@rect 适口
	//WS_OVERLAPPEDWINDOW 适口风格
	//NULL 没有菜单
	AdjustWindowRect(&Rect,WS_OVERLAPPEDWINDOW, NULL);

	int WindowWidth = Rect.right - Rect.left;
	int WindowHight = Rect.bottom - Rect.top;

	MainWindowsHandle = CreateWindowEx(
		NULL,//窗口额外的风格
		L"PipiEngine", // 窗口名称
		L"PIPI Engine",//会显示在窗口的标题栏上去
		WS_OVERLAPPEDWINDOW, //窗口风格
		0,0,//窗口的坐标
		WindowWidth, WindowHight,//
		NULL, //副窗口句柄
		nullptr, //菜单句柄
		InParameters.HInstance,//窗口实例
		NULL);//
	if (!MainWindowsHandle)
	{
		Engine_Log_Error("CreateWindow Failed..");
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	//显示窗口
	ShowWindow(MainWindowsHandle, SW_SHOW);

	//窗口是脏的，刷新一下
	UpdateWindow(MainWindowsHandle);

	Engine_Log("InitWindows complete.");
}

bool FWindowsEngine::InitDirect3D()
{
	//HRESULT
	//S_OK                0x00000000
	//E_UNEXPECTED        0x8000FFFF  意外失败 
	//E_NOTIMPL           0x80004001  未实现
	//E_OUTOFMEMORY       0x8007000E  未能分配所需内存
	//E_INVALIDARS        0x80070057  一个或多个参数无效
	//E_NOINTERFACE       0x80004002  不支持此接口
	//E_POINTER           0x80004003  无效指针
	//E_HANDLE            0x80070006  无效句柄
	//EABORT              0x80004004  操作终止
	//E_FAIL              0x80004005  错误
	//E_ACCESSDENIED      0x80070005  一般的访问被拒绝错误
	ANALYSIS_HRESULT(CreateDXGIFactory1(IID_PPV_ARGS(&DXGIFactory)));

	//创建驱动
	HRESULT D3dDeviceResult = D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&D3dDevice));

	//如果失败用warp
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
	//创建Fence对象 为CPU和GPU同步做准备
	/*
	* Fence->SetEventOnCompletion
	* 执行命令
	* 提交呈现
	* 队列信号
	* 等待
	*/
	ANALYSIS_HRESULT(D3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence)));

	//初始化命令对象
	//INT Priority Normal/High
	//UINT NodeMask 指示该命令在哪个GPU节点上执行
	D3D12_COMMAND_QUEUE_DESC QueueDesc = {};
	QueueDesc.Type = D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT;//直接
	QueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAGS::D3D12_COMMAND_QUEUE_FLAG_NONE;
	ANALYSIS_HRESULT(D3dDevice->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(&CommandQueue)));

	//创建分配器
	ANALYSIS_HRESULT(D3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(CommandAllocator.GetAddressOf())));

	//创建命令列表
	/*
	* 默认单个CPU
	* 直接
	* 将CommandList关联到Allocator
	* ID3D12PipelineState
	*/
	ANALYSIS_HRESULT(D3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, CommandAllocator.Get(), NULL, IID_PPV_ARGS(GraphicsCommandList.GetAddressOf())));

	//重置命令列表
	GraphicsCommandList->Close();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//多重采样
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS QualityLevels;
	QualityLevels.SampleCount = 4;
	QualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVEL_FLAGS::D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	QualityLevels.NumQualityLevels = 0;
	
	D3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &QualityLevels, sizeof(QualityLevels));

	M4XNumQualityLevels = QualityLevels.NumQualityLevels;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//交换链
	SwapChain->Release();
	DXGI_SWAP_CHAIN_DESC SwapChainDesc;
	SwapChainDesc.BufferDesc.Width = FEngineRenderConfig::GetRenderConfig()->ScreenWidth;
	SwapChainDesc.BufferDesc.Height = FEngineRenderConfig::GetRenderConfig()->ScreenHight;
	SwapChainDesc.BufferDesc.RefreshRate.Numerator = FEngineRenderConfig::GetRenderConfig()->RefreshRate;
	SwapChainDesc.BufferDesc.RefreshRate .Denominator = 1;
	SwapChainDesc.BufferCount = FEngineRenderConfig::GetRenderConfig() ->SwapChainCount;
	SwapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER::DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;//使用表面或资源作为输出渲染目标
	SwapChainDesc.OutputWindow = MainWindowsHandle;//指定Windows句柄
	SwapChainDesc.Windowed = true;//以窗口运行
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_FLIP_DISCARD;
	SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;//IDXGISwapChain::ResizeTarger()
	SwapChainDesc.BufferDesc.Format = BufferFormat;//格式纹理

	//多重采样设置
	SwapChainDesc.SampleDesc.Count = bMSAA4XEnabled ? 4 : 1;
	SwapChainDesc.SampleDesc.Quality = bMSAA4XEnabled ? (M4XNumQualityLevels - 1) : 0;

	DXGIFactory->CreateSwapChain(CommandQueue.Get(), &SwapChainDesc, SwapChain.GetAddressOf());


	return false;
}

#endif