#include "Mesh.h"

FMesh::FMesh()
	:VertexSizeInBytes(0)
	, VertexStrideInBytes(0)
	, IndexSizeInBytes(0)
	, IndexFormat(DXGI_FORMAT_R16_UINT)
	, IndexSize(0)
{

}

void FMesh::Init()
{

}

void FMesh::BuildMesh(const FMeshRenderingData* InRenderingData)
{
	VertexStrideInBytes = sizeof(FVertex);
	IndexSize = InRenderingData->IndexData.size();

	//获取模型数据大小
	VertexSizeInBytes = InRenderingData->VertexData.size() * VertexStrideInBytes;
	IndexSizeInBytes = IndexSize * sizeof(uint16_t);

	//创建缓冲区 并将数据拷贝刀缓冲区
	ANALYSIS_HRESULT(D3DCreateBlob(VertexSizeInBytes, &CPUVertexBufferPtr));
	memcpy(CPUVertexBufferPtr->GetBufferPointer(), InRenderingData->VertexData.data(), VertexSizeInBytes);

	ANALYSIS_HRESULT(D3DCreateBlob(IndexSizeInBytes, &CPUIndexBufferPtr));
	memcpy(CPUIndexBufferPtr->GetBufferPointer(), InRenderingData->IndexData.data(), IndexSizeInBytes);

	GPUVertexBufferPtr = ConstructDefaultBuffer(VertexBufferTmpPtr, InRenderingData->VertexData.data(), VertexSizeInBytes);
	GPUIndexBufferPtr = ConstructDefaultBuffer(IndexBufferTmpPtr, InRenderingData->IndexData.data(), IndexSizeInBytes);
}

void FMesh::Draw(float DeltaTime)
{

	D3D12_VERTEX_BUFFER_VIEW VertexBufferView = GetVertexBufferView();

	//绑定渲染流水线上的输入槽，可在输入装配器阶段输入顶点数据
	GetD3dGraphicsCommandList()->IASetVertexBuffers(
		0, //起始输入槽 0-15
		1, //k+n-1 顶点缓冲区的数量
		&VertexBufferView);


	D3D12_INDEX_BUFFER_VIEW IndexBufferView = GetIndexBufferView();
	GetD3dGraphicsCommandList()->IASetIndexBuffer(&IndexBufferView);

	//定义绘制的图元
	GetD3dGraphicsCommandList()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	GetD3dGraphicsCommandList()->DrawIndexedInstanced(
		IndexSize, //顶点数量
		1, //绘制实例数量
		0, //顶点缓冲区第一个被绘制的索引
		0, //GPU从索引的缓冲区读取的第一个索引的位置
		0 //再从顶点缓冲区读取每个实例数据之前添加到每个索引的值
		);
}

FMesh* FMesh::CreateMesh(const FMeshRenderingData* InRenderingData)
{
	FMesh* InMesh = new FMesh();
	InMesh->BuildMesh(InRenderingData);

	return InMesh;
}

D3D12_VERTEX_BUFFER_VIEW FMesh::GetVertexBufferView()
{
	D3D12_VERTEX_BUFFER_VIEW VBV;
	VBV.BufferLocation = GPUVertexBufferPtr->GetGPUVirtualAddress();
	VBV.SizeInBytes = VertexSizeInBytes;
	VBV.StrideInBytes = VertexStrideInBytes;

	return VBV;
}

D3D12_INDEX_BUFFER_VIEW FMesh::GetIndexBufferView()
{
	D3D12_INDEX_BUFFER_VIEW IBV;
	IBV.BufferLocation = GPUIndexBufferPtr->GetGPUVirtualAddress();
	IBV.SizeInBytes = IndexSizeInBytes;
	IBV.Format = IndexFormat;

	return IBV;
}
