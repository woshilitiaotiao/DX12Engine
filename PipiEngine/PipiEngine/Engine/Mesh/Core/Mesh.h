#pragma once
#include "../../Core/Engine.h"
#include "../../Rendering/Core/Rendering.h"
#include "MeshType.h"

class FMesh : public IRenderingInterface
{
public:
	FMesh();

	virtual void Init();

	virtual void BuildMesh(const FMeshRenderingData* InRenderingData);

	virtual void Draw(float DeltaTime);

	static FMesh* CreateMesh(const FMeshRenderingData* InRenderingData);

	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView();
	D3D12_INDEX_BUFFER_VIEW GetIndexBufferView();

protected:
	ComPtr<ID3DBlob> CPUVertexBufferPtr;
	ComPtr<ID3DBlob> CPUIndexBufferPtr;

	ComPtr<ID3D12Resource> GPUVertexBufferPtr;
	ComPtr<ID3D12Resource> GPUIndexBufferPtr;

	ComPtr<ID3D12Resource> VertexBufferTmpPtr;
	ComPtr<ID3D12Resource> IndexBufferTmpPtr;

protected:
	UINT VertexSizeInBytes ;
	UINT VertexStrideInBytes;

	UINT IndexSizeInBytes;
	DXGI_FORMAT IndexFormat;

	UINT IndexSize;
};