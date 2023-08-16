#pragma once

#if defined(_WIN32)
#include "../../Core/Engine.h"

class FWindowsEngine :public FEngine
{
public:
	FWindowsEngine();

	virtual int PreInit(FWinMainCommandParameters InParameters);

	virtual int Init();
	virtual int PostInit();

	virtual void Tick();

	virtual int PreExit();
	virtual int Exit();
	virtual int PostExit();

private:
	bool InitWindows(FWinMainCommandParameters InParameters);

	bool InitDirect3D();

protected:
	ComPtr<IDXGIFactory4> DXGIFactory;//创建 DirectX 图形基础结构（DXGI）对象
	ComPtr<ID3D12Device> D3dDevice;//创建命令分配器、命令列表、命令队列、Fance、资源、管道状态对象、堆、根签名、采样器和许多资源视图
	ComPtr<ID3D12Fence> Fence;//一个用于同步 CPU 和一个或多个GPU的对象

	ComPtr<ID3D12CommandQueue> CommandQueue;//队列
	ComPtr<ID3D12CommandAllocator> CommandAllocator;//分配器（储存）
	ComPtr<ID3D12GraphicsCommandList> GraphicsCommandList;//命令列表

	ComPtr<IDXGISwapChain> SwapChain;//交换链

protected:
	HWND MainWindowsHandle;//主Windows句柄
	UINT M4XNumQualityLevels;
	bool bMSAA4XEnabled;
	DXGI_FORMAT BufferFormat;
};
#endif