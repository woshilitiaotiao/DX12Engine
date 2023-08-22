#pragma once
#include "../../Core/Engine.h"

class IRenderingInterface
{
	friend class FWindowsEngine;
public:
	IRenderingInterface();
	virtual ~IRenderingInterface();

	virtual void Init();

	virtual void Draw(float DeltaTime);

	bool operator==(const IRenderingInterface& InOther)
	{
		return guid_equal(&Guid, &InOther.Guid);
	}

	simple_c_guid GetGuid() { return Guid; }

protected:
	ComPtr<ID3D12Resource> ConstructDefaultBuffer(ComPtr<ID3D12Resource>& OutTmpBuffer, const void* InData, UINT64 InDataSize);

protected:
	ComPtr<ID3D12Device> GetD3dDevice();
	ComPtr<ID3D12GraphicsCommandList> GetD3dGraphicsCommandList();

private:
	static vector<IRenderingInterface*> RenderingInterface;
	simple_c_guid Guid;

};