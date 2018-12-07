// Fill out your copyright notice in the Description page of Project Settings.
#include "GeneratedMeshComponent.h"
#include "DynamicMeshBuilder.h"
#include "Runtime/Launch/Resources/Version.h"
#include "LocalVertexFactory.h"

class FGeneratedMeshVertexBuffer 
	: public FVertexBuffer
{
public:
	TArray<FDynamicMeshVertex> Vertices;

	virtual void InitRHI()
	{
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 3
		FRHIResourceCreateInfo CreateInfo;
		VertexBufferRHI = RHICreateVertexBuffer(Vertices.Num() * sizeof(FDynamicMeshVertex), BUF_Static, CreateInfo);
#else
		VertexBufferRHI = RHICreateVertexBuffer(Vertices.Num() * sizeof(FDynamicMeshVertex), NULL, BUF_Static);
#endif
		void* VertexBufferData = RHILockVertexBuffer(VertexBufferRHI, 0, Vertices.Num() * sizeof(FDynamicMeshVertex), RLM_WriteOnly);
		FMemory::Memcpy(VertexBufferData, Vertices.GetData(), Vertices.Num() * sizeof(FDynamicMeshVertex));
		RHIUnlockVertexBuffer(VertexBufferRHI);
	}
};


class FGeneratedMeshIndexBuffer 
	: public FIndexBuffer
{
public:
	TArray<int> Indices;


	virtual void InitRHI()
	{
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 3
		FRHIResourceCreateInfo CreateInfo;
		IndexBufferRHI = RHICreateIndexBuffer(sizeof(int32), Indices.Num() * sizeof(int32), BUF_Static, CreateInfo);
#else
		IndexBufferRHI = RHICreateIndexBuffer(sizeof(int32), Indices.Num() * sizeof(int32), NULL, BUF_Static);
#endif
		void* Buffer = RHILockIndexBuffer(IndexBufferRHI, 0, Indices.Num() * sizeof(int32), RLM_WriteOnly);
		FMemory::Memcpy(Buffer, Indices.GetData(), Indices.Num() * sizeof(int32));
		RHIUnlockIndexBuffer(IndexBufferRHI);
	}
};


class FGeneratedMeshVertexFactory
	: public FLocalVertexFactory
{
public:
	FGeneratedMeshVertexFactory() 
		: FLocalVertexFactory(/*ERHIFeatureLevel::SM4, "FGeneratedMeshVertexFactory"*/)
	{
		
	}

	FGeneratedMeshVertexFactory(ERHIFeatureLevel::Type InFeatureLevel) :
		FLocalVertexFactory(/*InFeatureLevel, "FGeneratedMeshVertexFactory"*/)
	{

	}

	void Init(const FGeneratedMeshVertexBuffer* VertexBuffer)
	{
		check(!IsInRenderingThread());

		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(InitGeneratedMeshVertexFactory,
			FGeneratedMeshVertexFactory*, VertexFactory, this, const FGeneratedMeshVertexBuffer*, VertexBuffer, VertexBuffer,
			{
				FDataType NewData;
		NewData.PositionComponent = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FDynamicMeshVertex, Position, VET_Float3);
		NewData.TextureCoordinates.Add(
			FVertexStreamComponent(VertexBuffer, STRUCT_OFFSET(FDynamicMeshVertex, TextureCoordinate), sizeof(FDynamicMeshVertex), VET_Float2)
		);
		NewData.TangentBasisComponents[0] = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FDynamicMeshVertex, TangentX, VET_PackedNormal);
		NewData.TangentBasisComponents[1] = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FDynamicMeshVertex, TangentZ, VET_PackedNormal);
		NewData.ColorComponent = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FDynamicMeshVertex, Color, VET_Color);
		
		VertexFactory->SetData(NewData);
			}
		);
	}
};

UGeneratedMeshComponent::UGeneratedMeshComponent(const FObjectInitializer& PCIP) : Super(PCIP)
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UGeneratedMeshComponent::SetGeneratedMeshTriangles(const TArray<FGeneratedMeshTriangle>& Triangles)
{
	GeneratedMeshTris = Triangles;

	UpdateCollision();
	MarkRenderStateDirty();

	return true;
}

FPrimitiveSceneProxy* UGeneratedMeshComponent::CreateSceneProxy()
{
	class FGeneratedMeshSceneProxy : public FPrimitiveSceneProxy
	{
	public:
		FGeneratedMeshSceneProxy(UGeneratedMeshComponent* Component) : FPrimitiveSceneProxy(Component),
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 5
			MaterialRelevance(Component->GetMaterialRelevance(ERHIFeatureLevel::SM4))
#else
			MaterialRelevance(Component->GetMaterialRelevance())
#endif
		{
			const FColor VertexColor(255, 255, 255);

			for (int TriIdx = 0; TriIdx < Component->GeneratedMeshTris.Num(); TriIdx++)
			{
				FGeneratedMeshTriangle& Tri = Component->GeneratedMeshTris[TriIdx];

				const FVector Edge01 = (Tri.Vertex1 - Tri.Vertex0);
				const FVector Edge02 = (Tri.Vertex2 - Tri.Vertex0);

				const FVector TangentX = Edge01.GetSafeNormal();
				const FVector TangentZ = (Edge02^Edge01).GetSafeNormal();
				const FVector TangentY = (TangentX^TangentZ).GetSafeNormal();

				FDynamicMeshVertex Vert0;
				Vert0.Position = Tri.Vertex0;
				Vert0.Color = VertexColor;
				Vert0.SetTangents(TangentX, TangentY, TangentZ);
				int32 VIndex = VertexBuffer.Vertices.Add(Vert0);
				IndexBuffer.Indices.Add(VIndex);

				FDynamicMeshVertex Vert1;
				Vert1.Position = Tri.Vertex1;
				Vert1.Color = VertexColor;
				Vert1.SetTangents(TangentX, TangentY, TangentZ);
				VIndex = VertexBuffer.Vertices.Add(Vert1);
				IndexBuffer.Indices.Add(VIndex);

				FDynamicMeshVertex Vert2;
				Vert2.Position = Tri.Vertex2;
				Vert2.Color = VertexColor;
				Vert2.SetTangents(TangentX, TangentY, TangentZ);
				VIndex = VertexBuffer.Vertices.Add(Vert2);
				IndexBuffer.Indices.Add(VIndex);
			}

			VertexFactory.Init(&VertexBuffer);

			BeginInitResource(&VertexBuffer);
			BeginInitResource(&IndexBuffer);
			BeginInitResource(&VertexFactory);

			Material = Component->GetMaterial(0);
			if (Material == nullptr)
			{
				Material = UMaterial::GetDefaultMaterial(MD_Surface);
			}
		}

		virtual ~FGeneratedMeshSceneProxy()
		{
			VertexBuffer.ReleaseResource();
			IndexBuffer.ReleaseResource();
			VertexFactory.ReleaseResource();
		}
		
		virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, class FMeshElementCollector& Collector) const override
		{
			for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
			{
				FDynamicMeshBuilder MeshBuilder;
				if (VertexBuffer.Vertices.Num() == 0)
				{
					return;
				}

				MeshBuilder.AddVertices(VertexBuffer.Vertices);
				MeshBuilder.AddTriangles(IndexBuffer.Indices);
				MeshBuilder.GetMesh(/*FMatrix::Identity*/GetLocalToWorld(), new FColoredMaterialRenderProxy(Material->GetRenderProxy(false),
					FLinearColor::Gray), GetDepthPriorityGroup(Views[ViewIndex]), true, true, ViewIndex, Collector);
			}
		}

		virtual void DrawDynamicElements(FPrimitiveDrawInterface* PDI, const FSceneView* View)
		{
			QUICK_SCOPE_CYCLE_COUNTER(STAT_GeneratedMeshSceneProxy_DrawDynamicElements);
			const bool bWireframe = View->Family->EngineShowFlags.Wireframe;

			auto WireframeMaterialInstance = new FColoredMaterialRenderProxy(
				GEngine->WireframeMaterial ? GEngine->WireframeMaterial->GetRenderProxy(IsSelected()) : NULL,
				FLinearColor(0, 0.5f, 1.0f)
			);
			
			FMaterialRenderProxy* MaterialProxy = NULL;
			if (bWireframe)
			{
				MaterialProxy = WireframeMaterialInstance;
			}
			else
			{
				MaterialProxy = Material->GetRenderProxy(IsSelected());
			}
			
			FMeshBatch Mesh;
			FMeshBatchElement& BatchElement = Mesh.Elements[0];
			BatchElement.IndexBuffer = &IndexBuffer;
			Mesh.bWireframe = bWireframe;
			Mesh.VertexFactory = &VertexFactory;
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 5
			BatchElement.PrimitiveUniformBuffer = CreatePrimitiveUniformBufferImmediate(GetLocalToWorld(), GetBounds(), GetLocalBounds(), true, UseEditorDepthTest());
#else
			BatchElement.PrimitiveUniformBuffer = CreatePrimitiveUniformBufferImmediate(GetLocalToWorld(), GetBounds(), GetLocalBounds(), true);
#endif
			BatchElement.FirstIndex = 0;
			BatchElement.NumPrimitives = IndexBuffer.Indices.Num() / 3;
			BatchElement.MinVertexIndex = 0;
			BatchElement.MaxVertexIndex = VertexBuffer.Vertices.Num() - 1;
			Mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
			Mesh.Type = PT_TriangleList;
			Mesh.DepthPriorityGroup = SDPG_World;
			PDI->DrawMesh(Mesh);
		}

		virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
		{
			FPrimitiveViewRelevance Result;
			Result.bDynamicRelevance = true;
			Result.bDrawRelevance = true;
			Result.bNormalTranslucencyRelevance = true;
			return Result;
		}

		virtual bool CanBeOccluded() const override
		{
			return !MaterialRelevance.bDisableDepthTest;
		}
		
		virtual uint32 GetMemoryFootprint(void) const
		{
			return (sizeof(*this) + GetAllocatedSize());
		}
		
		uint32 GetAllocatedSize(void) const
		{
			return FPrimitiveSceneProxy::GetAllocatedSize();
		}

		virtual SIZE_T GetTypeHash() const 
		{
			static size_t UniquePointer;
			return reinterpret_cast<size_t>(&UniquePointer);
		}

	private:
		UMaterialInterface* Material;
		FGeneratedMeshVertexBuffer VertexBuffer;
		FGeneratedMeshIndexBuffer IndexBuffer;
		FGeneratedMeshVertexFactory VertexFactory;
		FMaterialRelevance MaterialRelevance;
	};

	if (GeneratedMeshTris.Num() > 0)
	{
		return new FGeneratedMeshSceneProxy(this);
	}
	else
	{
		return nullptr;
	}
}

int32 UGeneratedMeshComponent::GetNumMaterials() const
{
	return 1;
}

FBoxSphereBounds UGeneratedMeshComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	if (GeneratedMeshTris.Num() < 1)
		return FBoxSphereBounds();

	FVector vecMin = GeneratedMeshTris[0].Vertex0;
	FVector vecMax = GeneratedMeshTris[1].Vertex0;

	for (int32 TriIdx = 0; TriIdx < GeneratedMeshTris.Num(); TriIdx++)
	{
		vecMin.X = (vecMin.X > GeneratedMeshTris[TriIdx].Vertex0.X) ? GeneratedMeshTris[TriIdx].Vertex0.X : vecMin.X;
		vecMin.X = (vecMin.X > GeneratedMeshTris[TriIdx].Vertex1.X) ? GeneratedMeshTris[TriIdx].Vertex1.X : vecMin.X;
		vecMin.X = (vecMin.X > GeneratedMeshTris[TriIdx].Vertex2.X) ? GeneratedMeshTris[TriIdx].Vertex2.X : vecMin.X;

		vecMin.Y = (vecMin.Y > GeneratedMeshTris[TriIdx].Vertex0.Y) ? GeneratedMeshTris[TriIdx].Vertex0.Y : vecMin.Y;
		vecMin.Y = (vecMin.Y > GeneratedMeshTris[TriIdx].Vertex1.Y) ? GeneratedMeshTris[TriIdx].Vertex1.Y : vecMin.Y;
		vecMin.Y = (vecMin.Y > GeneratedMeshTris[TriIdx].Vertex2.Y) ? GeneratedMeshTris[TriIdx].Vertex2.Y : vecMin.Y;

		vecMin.Z = (vecMin.Z > GeneratedMeshTris[TriIdx].Vertex0.Z) ? GeneratedMeshTris[TriIdx].Vertex0.Z : vecMin.Z;
		vecMin.Z = (vecMin.Z > GeneratedMeshTris[TriIdx].Vertex1.Z) ? GeneratedMeshTris[TriIdx].Vertex1.Z : vecMin.Z;
		vecMin.Z = (vecMin.Z > GeneratedMeshTris[TriIdx].Vertex2.Z) ? GeneratedMeshTris[TriIdx].Vertex2.Z : vecMin.Z;

		vecMax.X = (vecMax.X < GeneratedMeshTris[TriIdx].Vertex0.X) ? GeneratedMeshTris[TriIdx].Vertex0.X : vecMax.X;
		vecMax.X = (vecMax.X < GeneratedMeshTris[TriIdx].Vertex1.X) ? GeneratedMeshTris[TriIdx].Vertex1.X : vecMax.X;
		vecMax.X = (vecMax.X < GeneratedMeshTris[TriIdx].Vertex2.X) ? GeneratedMeshTris[TriIdx].Vertex2.X : vecMax.X;

		vecMax.Y = (vecMax.Y < GeneratedMeshTris[TriIdx].Vertex0.Y) ? GeneratedMeshTris[TriIdx].Vertex0.Y : vecMax.Y;
		vecMax.Y = (vecMax.Y < GeneratedMeshTris[TriIdx].Vertex1.Y) ? GeneratedMeshTris[TriIdx].Vertex1.Y : vecMax.Y;
		vecMax.Y = (vecMax.Y < GeneratedMeshTris[TriIdx].Vertex2.Y) ? GeneratedMeshTris[TriIdx].Vertex2.Y : vecMax.Y;

		vecMax.Z = (vecMax.Z < GeneratedMeshTris[TriIdx].Vertex0.Z) ? GeneratedMeshTris[TriIdx].Vertex0.Z : vecMax.Z;
		vecMax.Z = (vecMax.Z < GeneratedMeshTris[TriIdx].Vertex1.Z) ? GeneratedMeshTris[TriIdx].Vertex1.Z : vecMax.Z;
		vecMax.Z = (vecMax.Z < GeneratedMeshTris[TriIdx].Vertex2.Z) ? GeneratedMeshTris[TriIdx].Vertex2.Z : vecMax.Z;
	}
	
	FVector vecOrigin = (vecMax - vecMin) / 2 + vecMin;
	FVector BoxPoint = vecMax - vecMin;
	
	return FBoxSphereBounds(vecOrigin, BoxPoint, BoxPoint.Size()).TransformBy(LocalToWorld);
}

bool UGeneratedMeshComponent::GetPhysicsTriMeshData(struct FTriMeshCollisionData* CollisionData, bool InUseAllTriData)
{
	for (int32 i = 0; i < GeneratedMeshTris.Num(); i++)
	{
		const FGeneratedMeshTriangle& tri = GeneratedMeshTris[i];

		CollisionData->Vertices.Add(tri.Vertex0);
		CollisionData->Vertices.Add(tri.Vertex1);
		CollisionData->Vertices.Add(tri.Vertex2);
	}

	CollisionData->bFlipNormals = true;

	return true;
}

bool UGeneratedMeshComponent::ContainsPhysicsTriMeshData(bool InUseAllTriData) const
{
	return (GeneratedMeshTris.Num() > 0);
}

void UGeneratedMeshComponent::UpdateBodySetup()
{
	if (ModelBodySetup == nullptr)
	{
		ModelBodySetup = NewObject<UBodySetup>(this, UBodySetup::StaticClass());
		ModelBodySetup->CollisionTraceFlag = CTF_UseComplexAsSimple;
		ModelBodySetup->bMeshCollideAll = true;
	}
}

void UGeneratedMeshComponent::UpdateCollision()
{
	if (bPhysicsStateCreated)
	{
		DestroyPhysicsState();
		UpdateBodySetup();
		CreatePhysicsState();
		
		ModelBodySetup->InvalidatePhysicsData();
		ModelBodySetup->CreatePhysicsMeshes();
	}
}

UBodySetup* UGeneratedMeshComponent::GetBodySetup()
{
	UpdateBodySetup();
	return ModelBodySetup;
}
