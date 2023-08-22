#pragma once

#if defined(_WIN32)
#include "../../Core/Engine.h"

class FWindowsEngine :public FEngine
{
	friend class IRenderingInterface;
public:
	FWindowsEngine();

	virtual int PreInit(FWinMainCommandParameters InParameters);

	virtual int Init(FWinMainCommandParameters InParameters);
	virtual int PostInit();

	virtual void Tick(float DeltaTime);

	virtual int PreExit();
	virtual int Exit();
	virtual int PostExit();

public:
	ID3D12Resource* GetCurrentSwapBuff()const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetCuurentSwapBufferView()const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetCuurentDepthStencilView()const;

protected:
	void WaitGPUCommandQueueComplete();


private:
	bool InitWindows(FWinMainCommandParameters InParameters);
	bool InitDirect3D();
	void PostInitDirect3D();

protected:
	UINT64 CurrentFenceIndex;
	int CurrentSwapBuffIndex;

	ComPtr<IDXGIFactory4> DXGIFactory;//创建 DirectX 图形基础结构（DXGI）对象
	ComPtr<ID3D12Device> D3dDevice;//创建命令分配器、命令列表、命令队列、Fance、资源、管道状态对象、堆、根签名、采样器和许多资源视图
	ComPtr<ID3D12Fence> Fence;//一个用于同步 CPU 和一个或多个GPU的对象

	ComPtr<ID3D12CommandQueue> CommandQueue;//队列
	ComPtr<ID3D12CommandAllocator> CommandAllocator;//分配器（储存）
	ComPtr<ID3D12GraphicsCommandList> GraphicsCommandList;//命令列表

	ComPtr<IDXGISwapChain> SwapChain;//交换链

	//描述符对象和堆
	ComPtr<ID3D12DescriptorHeap> RTVHeap;
	ComPtr<ID3D12DescriptorHeap> DSVHeap;

	vector<ComPtr<ID3D12Resource>> SwapChainBuffer;
	ComPtr<ID3D12Resource> DepthStencilBuffer;

	//屏幕视口信息
	D3D12_VIEWPORT ViewportInfo;
	D3D12_RECT ViewportRect;

protected:
	HWND MainWindowsHandle;//主Windows句柄
	UINT M4XNumQualityLevels;
	bool bMSAA4XEnabled;
	DXGI_FORMAT BackBufferFormat;
	DXGI_FORMAT DepthStencilFormat;
	UINT RTVDescriptorsize;
};
#endif