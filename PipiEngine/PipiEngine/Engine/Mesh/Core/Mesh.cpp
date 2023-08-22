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

	//��ȡģ�����ݴ�С
	VertexSizeInBytes = InRenderingData->VertexData.size() * VertexStrideInBytes;
	IndexSizeInBytes = IndexSize * sizeof(uint16_t);

	//���������� �������ݿ�����������
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

	//����Ⱦ��ˮ���ϵ�����ۣ���������װ�����׶����붥������
	GetD3dGraphicsCommandList()->IASetVertexBuffers(
		0, //��ʼ����� 0-15
		1, //k+n-1 ���㻺����������
		&VertexBufferView);


	D3D12_INDEX_BUFFER_VIEW IndexBufferView = GetIndexBufferView();
	GetD3dGraphicsCommandList()->IASetIndexBuffer(&IndexBufferView);

	//������Ƶ�ͼԪ
	GetD3dGraphicsCommandList()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	GetD3dGraphicsCommandList()->DrawIndexedInstanced(
		IndexSize, //��������
		1, //����ʵ������
		0, //���㻺������һ�������Ƶ�����
		0, //GPU�������Ļ�������ȡ�ĵ�һ��������λ��
		0 //�ٴӶ��㻺������ȡÿ��ʵ������֮ǰ��ӵ�ÿ��������ֵ
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
