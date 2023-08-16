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
	ComPtr<IDXGIFactory4> DXGIFactory;//���� DirectX ͼ�λ����ṹ��DXGI������
	ComPtr<ID3D12Device> D3dDevice;//��������������������б�������С�Fance����Դ���ܵ�״̬���󡢶ѡ���ǩ�����������������Դ��ͼ
	ComPtr<ID3D12Fence> Fence;//һ������ͬ�� CPU ��һ������GPU�Ķ���

	ComPtr<ID3D12CommandQueue> CommandQueue;//����
	ComPtr<ID3D12CommandAllocator> CommandAllocator;//�����������棩
	ComPtr<ID3D12GraphicsCommandList> GraphicsCommandList;//�����б�

	ComPtr<IDXGISwapChain> SwapChain;//������

protected:
	HWND MainWindowsHandle;//��Windows���
	UINT M4XNumQualityLevels;
	bool bMSAA4XEnabled;
	DXGI_FORMAT BufferFormat;
};
#endif