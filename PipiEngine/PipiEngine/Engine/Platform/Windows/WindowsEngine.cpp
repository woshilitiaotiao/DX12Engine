#include "WindowsEngine.h"
#include "../../Debug/EngineDebug.h"
#include "../../Config/EngineRenderConfig.h"
#if defined(_WIN32)
#include "WindowsMessageProcessing.h"


FWindowsEngine::FWindowsEngine()
	:CurrentFenceIndex(0)
	, M4XNumQualityLevels(0)
	, bMSAA4XEnabled(false)
	, BackBufferFormat(DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM)
	, DepthStencilFormat(DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT)
	, CurrentSwapBuffIndex(0)
{
	for (int i = 0; i < FEngineRenderConfig::GetRenderConfig()->SwapChainCount; i++)
	{
		SwapChainBuffer.push_back(ComPtr<ID3D12Resource>());
	}
}

int FWindowsEngine::PreInit(FWinMainCommandParameters InParameters)
{
	//日志系统初始化
	const char LogPath[] = "../log";
	init_log_system(LogPath);
	Engine_Log("Log Init.");

	//处理命令


	Engine_Log("Engine pre initialization complete.");
	return 0;
}

int FWindowsEngine::Init(FWinMainCommandParameters InParameters)
{
	if (InitWindows(InParameters))
	{

	}

	if (InitDirect3D())
	{

	}

	Engine_Log("Engine initialization complete.");
	return 0;
}

int FWindowsEngine::PostInit()
{
	//同步
	WaitGPUCommandQueueComplete();

	for (int i = 0; i < FEngineRenderConfig::GetRenderConfig()->SwapChainCount; i++)
	{
		SwapChainBuffer[i].Reset();
	}
	DepthStencilBuffer.Reset();

	SwapChain->ResizeBuffers(FEngineRenderConfig::GetRenderConfig()->SwapChainCount, 
		FEngineRenderConfig::GetRenderConfig()->ScreenWidth, 
		FEngineRenderConfig::GetRenderConfig()->ScreenHight, 
		BackBufferFormat, 
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
	
	//拿到描述size
	RTVDescriptorsize = D3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE HeapHandle(RTVHeap->GetCPUDescriptorHandleForHeapStart());

	for (UINT i = 0; i < FEngineRenderConfig::GetRenderConfig()->SwapChainCount; i++)
	{
		SwapChain->GetBuffer(i, IID_PPV_ARGS(&SwapChainBuffer[i]));
		D3dDevice->CreateRenderTargetView(SwapChainBuffer[i].Get(), nullptr, HeapHandle);
		HeapHandle.Offset(1, RTVDescriptorsize);
	}

	D3D12_RESOURCE_DESC ResourceDesc;
	ResourceDesc.Width = FEngineRenderConfig::GetRenderConfig()->ScreenWidth;
	ResourceDesc.Height = FEngineRenderConfig::GetRenderConfig()->ScreenHight;
	ResourceDesc.Alignment = 0;
	ResourceDesc.MipLevels = 1;
	ResourceDesc.DepthOrArraySize = 1;
	ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	ResourceDesc.SampleDesc.Count = bMSAA4XEnabled ? 4 : 1;
	ResourceDesc.SampleDesc.Quality = bMSAA4XEnabled ? (M4XNumQualityLevels - 1) : 0;
	ResourceDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	ResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

	D3D12_CLEAR_VALUE ClearValue;
	ClearValue.DepthStencil.Depth = 1.f;
	ClearValue.DepthStencil.Stencil = 0;
	ClearValue.Format = DepthStencilFormat;

	CD3DX12_HEAP_PROPERTIES GeapProperties =  CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	D3dDevice->CreateCommittedResource(&GeapProperties, D3D12_HEAP_FLAG_NONE, &ResourceDesc, D3D12_RESOURCE_STATE_COMMON, &ClearValue, IID_PPV_ARGS(DepthStencilBuffer.GetAddressOf()));

	D3D12_DEPTH_STENCIL_VIEW_DESC DSVDesc;
	DSVDesc.Format = DepthStencilFormat;
	DSVDesc.Texture2D.MipSlice = 0;
	DSVDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	DSVDesc.Flags = D3D12_DSV_FLAG_NONE;

	//创建深度模板缓冲区
	D3dDevice->CreateDepthStencilView(DepthStencilBuffer.Get(), &DSVDesc, DSVHeap->GetCPUDescriptorHandleForHeapStart());

	CD3DX12_RESOURCE_BARRIER Barrier = CD3DX12_RESOURCE_BARRIER::Transition(DepthStencilBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);

	//通知我们的驱动程序需要同步对我们资源的多次访问 标明我们当前的资源状态 通用&&深度写
	GraphicsCommandList->ResourceBarrier(1, &Barrier);

	GraphicsCommandList->Close();

	ID3D12CommandList* CommandList[] = { GraphicsCommandList.Get() };
	
	//覆盖windows画布
	//描述视口的尺寸
	ViewportInfo.TopLeftX = 0;
	ViewportInfo.TopLeftY = 0;
	ViewportInfo.Width = FEngineRenderConfig::GetRenderConfig()->ScreenWidth;
	ViewportInfo.Height = FEngineRenderConfig::GetRenderConfig()->ScreenHight;
	ViewportInfo.MinDepth = 0.f;
	ViewportInfo.MinDepth = 1.f;

	//DX矩形
	ViewportRect.left = 0;
	ViewportRect.top = 0;
	ViewportRect.right = FEngineRenderConfig::GetRenderConfig()->ScreenWidth;
	ViewportRect.bottom = FEngineRenderConfig::GetRenderConfig()->ScreenHight;

	WaitGPUCommandQueueComplete();

	//把命令提交
	CommandQueue->ExecuteCommandLists(_countof(CommandList), CommandList);

	Engine_Log("Engine post initialization complete.");
	return 0;
}

void FWindowsEngine::Tick(float DeltaTime)
{
	//重置录制的相关内容，为下一帧做准备
	ANALYSIS_HRESULT(CommandAllocator->Reset());

	//重置命令列表
	GraphicsCommandList->Reset(CommandAllocator.Get(), NULL);

	//指向哪个资源，转换其形态
	CD3DX12_RESOURCE_BARRIER ResourceBarrierPresent = CD3DX12_RESOURCE_BARRIER::Transition(GetCurrentSwapBuff(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	GraphicsCommandList->ResourceBarrier(1, &ResourceBarrierPresent);

	//每帧绑定矩形框
	GraphicsCommandList->RSSetViewports(1, &ViewportInfo);
	GraphicsCommandList->RSSetScissorRects(1, &ViewportRect);

	//清除画布
	GraphicsCommandList->ClearRenderTargetView(GetCuurentSwapBufferView(), DirectX::Colors::Cornsilk, 0, nullptr);

	//清除深度模板缓冲区
	GraphicsCommandList->ClearDepthStencilView(GetCuurentDepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.f, 0, 0, NULL);

	//输出的合并阶段
	D3D12_CPU_DESCRIPTOR_HANDLE SwapBufferView = GetCuurentSwapBufferView();
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView = GetCuurentDepthStencilView();
	GraphicsCommandList->OMSetRenderTargets(1, &SwapBufferView, true, &DepthStencilView);

	//指向哪个资源，转换其形态//交换形态
	CD3DX12_RESOURCE_BARRIER ResourceBarrierRenderTarget = CD3DX12_RESOURCE_BARRIER::Transition(GetCurrentSwapBuff(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	GraphicsCommandList->ResourceBarrier(1, &ResourceBarrierRenderTarget);

	//录入完成
	ANALYSIS_HRESULT(GraphicsCommandList->Close());

	//提交命令
	ID3D12CommandList* CommandList[] = { GraphicsCommandList.Get() };
	CommandQueue->ExecuteCommandLists(_countof(CommandList), CommandList);

	//交换链交换两个buff缓冲区
	ANALYSIS_HRESULT(SwapChain->Present(0, 0));
	//交换index
	CurrentSwapBuffIndex = !(bool)CurrentSwapBuffIndex;

	//CPU等待GPU
	WaitGPUCommandQueueComplete();

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

ID3D12Resource* FWindowsEngine::GetCurrentSwapBuff() const
{
	return SwapChainBuffer[CurrentSwapBuffIndex].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE FWindowsEngine::GetCuurentSwapBufferView() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(RTVHeap->GetCPUDescriptorHandleForHeapStart(), CurrentSwapBuffIndex, RTVDescriptorsize);
}

D3D12_CPU_DESCRIPTOR_HANDLE FWindowsEngine::GetCuurentDepthStencilView() const
{
	return DSVHeap->GetCPUDescriptorHandleForHeapStart();
}

void FWindowsEngine::WaitGPUCommandQueueComplete()
{
	CurrentFenceIndex++;

	//向GPU设置新的隔离点 等待GPU处理完信号
	ANALYSIS_HRESULT(CommandQueue->Signal(Fence.Get(), CurrentFenceIndex));

	if (Fence->GetCompletedValue() < CurrentFenceIndex)
	{
		//创建或打开一个事件内核对象，返回内核对象句柄
		//SECURITY_ATTRIBUTES
		//CREATE_EVENT_INITIAL_SET  0x00000002
		//CREATE_EVENT_MANUAL_RESET 0x00000001
		//ResentEvents
		HANDLE EventEX = CreateEventEx(NULL, NULL, 0, EVENT_ALL_ACCESS);
		//GPU完成后通知Handle
		ANALYSIS_HRESULT(Fence->SetEventOnCompletion(CurrentFenceIndex, EventEX));

		//等待GPU，阻塞主线程
		WaitForSingleObject(EventEX, INFINITE);
		CloseHandle(EventEX);
	}
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
	//开启Debug
	ComPtr<ID3D12Debug> D2d12Debug;
	HRESULT DebugResult = D3D12GetDebugInterface(IID_PPV_ARGS(&D2d12Debug));
	if (SUCCEEDED(DebugResult))
	{
		D2d12Debug->EnableDebugLayer();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
	ANALYSIS_HRESULT(GraphicsCommandList->Close());

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//多重采样
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS QualityLevels;
	QualityLevels.Format = BackBufferFormat;
	QualityLevels.SampleCount = 4;
	QualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVEL_FLAGS::D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	QualityLevels.NumQualityLevels = 0;
	
	D3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &QualityLevels, sizeof(QualityLevels));

	M4XNumQualityLevels = QualityLevels.NumQualityLevels;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//交换链
	SwapChain.Reset();
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
	SwapChainDesc.BufferDesc.Format = BackBufferFormat;//格式纹理

	//多重采样设置
	SwapChainDesc.SampleDesc.Count = bMSAA4XEnabled ? 4 : 1;
	SwapChainDesc.SampleDesc.Quality = bMSAA4XEnabled ? (M4XNumQualityLevels - 1) : 0;

	ANALYSIS_HRESULT(DXGIFactory->CreateSwapChain(CommandQueue.Get(), &SwapChainDesc, SwapChain.GetAddressOf()));


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//资源描述符
	//  D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV       //CBV常量缓冲区视图 SRV着色器资源视图 UAV无序访问试图
	//  D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER	         //采样器视图
	//  D3D12_DESCRIPTOR_HEAP_TYPE_RTV	             //渲染目标视图资源
	//  D3D12_DESCRIPTOR_HEAP_TYPE_DSV	             //深度/模板视图资源
	//RTV
	D3D12_DESCRIPTOR_HEAP_DESC RTVDescriptorHeapDesc;
	RTVDescriptorHeapDesc.NumDescriptors = 2;
	RTVDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	RTVDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	RTVDescriptorHeapDesc.NodeMask = 0;
	ANALYSIS_HRESULT(D3dDevice->CreateDescriptorHeap(&RTVDescriptorHeapDesc, IID_PPV_ARGS(RTVHeap.GetAddressOf())));

	//DSV
	D3D12_DESCRIPTOR_HEAP_DESC DSVDescriptorHeapDesc;
	DSVDescriptorHeapDesc.NumDescriptors = 1;
	DSVDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	DSVDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DSVDescriptorHeapDesc.NodeMask = 0;
	ANALYSIS_HRESULT(D3dDevice->CreateDescriptorHeap(&DSVDescriptorHeapDesc, IID_PPV_ARGS(DSVHeap.GetAddressOf())));
	return false;
}

#endif