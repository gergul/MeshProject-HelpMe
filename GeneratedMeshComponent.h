// Fill out your copyright notice in the Description page of Project Settings.


#pragma once


#include "CoreMinimal.h"
#include "Components/MeshComponent.h"
#include "PhysicsEngine/BodySetup.h"
#include "GeneratedMeshComponent.generated.h"

USTRUCT(BlueprintType)
struct FGeneratedMeshTriangle
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY(EditAnywhere, Category = Triangle)
		FVector Vertex0;

	UPROPERTY(EditAnywhere, Category = Triangle)
		FVector Vertex1;

	UPROPERTY(EditAnywhere, Category = Triangle)
		FVector Vertex2;
};


/**
 * 
 */
UCLASS(editinlinenew, meta = (BlueprintSpawnableComponent), ClassGroup = Rendering)
class MESHPROJECT_API UGeneratedMeshComponent : public UMeshComponent, public IInterface_CollisionDataProvider
{
	GENERATED_BODY()

public:
	UGeneratedMeshComponent(const FObjectInitializer& PCIP);

public:
	UFUNCTION(BlueprintCallable, Category = "Component|GeneratedMesh")
		bool SetGeneratedMeshTriangles(const TArray<FGeneratedMeshTriangle>& Triangles);

	UPROPERTY(BlueprintReadOnly, Category = "Collision")
		class UBodySetup* ModelBodySetup;


	virtual int32 GetNumMaterials() const override;

	virtual bool GetPhysicsTriMeshData(struct FTriMeshCollisionData* CollisionData, bool InUseAllTriData) override;

	virtual bool ContainsPhysicsTriMeshData(bool InUseAllTriData) const override;

	virtual bool WantsNegXTriMesh() override { return false; }

	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

	virtual class UBodySetup* GetBodySetup() override;

	void UpdateBodySetup();


	void UpdateCollision();

private:
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;

	TArray<FGeneratedMeshTriangle> GeneratedMeshTris;

	friend class FGeneratedMeshSceneProxy;
};
