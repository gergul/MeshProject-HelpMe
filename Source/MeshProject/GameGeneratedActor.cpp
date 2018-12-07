// Fill out your copyright notice in the Description page of Project Settings.
#include "GameGeneratedActor.h"
#include "Engine.h"
#include "Tool/UE4Helper.h"

// Sets default values
AGameGeneratedActor::AGameGeneratedActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	InitMesh();
}

void AGameGeneratedActor::InitMesh()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>("Root");

	UGeneratedMeshComponent* mesh = CreateDefaultSubobject<UGeneratedMeshComponent>(TEXT("GeneratedMesh"));

	//生成自定义mesh的三角面
	TArray<FVector> points;
	points.Add(FVector(0, 10, 0));
	points.Add(FVector(10, 10, 0));
	TArray<FGeneratedMeshTriangle> triangles;
	Lathe(points, triangles, 4);
	mesh->SetGeneratedMeshTriangles(triangles);

	mesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	//UStaticMeshComponent* BoxOne = CreateDefaultSubobject<UStaticMeshComponent>("BoxOne");

	//auto MeshAsset = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'"));
	//if (MeshAsset.Object != nullptr)
	//{
	//	BoxOne->SetStaticMesh(MeshAsset.Object);
	//}

	//BoxOne->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
}

// Called when the game starts or when spawned
void AGameGeneratedActor::BeginPlay()
{
	Super::BeginPlay();


}

void AGameGeneratedActor::PostActorCreated()
{
	Super::PostActorCreated();

}

void AGameGeneratedActor::PostLoad()
{		
	//UObject* InOuter = this;
	//UObject* Obj = StaticFindObjectFast( /*Class=*/ NULL, InOuter, TEXT("GeneratedMesh"), true);

	//if (Obj/* && !Obj->GetClass()->IsChildOf(UGeneratedMeshComponent::StaticClass())*/)
	//{
	//	UGeneratedMeshComponent* mesh = Cast<UGeneratedMeshComponent>(Obj);
	//  //生成自定义mesh的三角面
	//	TArray<FVector> points;
	//	points.Add(FVector(20, 5, 0));
	//	points.Add(FVector(15, 6, 0));
	//	points.Add(FVector(12, 7, 0));
	//	points.Add(FVector(11, 8, 0));
	//	points.Add(FVector(8, 7, 0));
	//	points.Add(FVector(7, 6, 0));
	//	points.Add(FVector(4, 5, 0));
	//	points.Add(FVector(3, 4, 0));
	//	points.Add(FVector(2, 3, 0));
	//	points.Add(FVector(1, 4, 0));
	//	TArray<FGeneratedMeshTriangle> triangles;
	//	Lathe(points, triangles, 128);
	//	mesh->SetGeneratedMeshTriangles(triangles);

	//	//设置材质
	//	//mesh->SetMaterial(0, UE4Helper::LoadMaterial(TEXT("/Game/Textures/sample_Mat"), mesh));
	//}

	Super::PostLoad();
}

// Called every frame
void AGameGeneratedActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGameGeneratedActor::Lathe(const TArray<FVector>& points, TArray<FGeneratedMeshTriangle>& triangles, int segments)
{
	UE_LOG(LogClass, Log, TEXT("AGameGeneratedActor::Lathe POINTS %d"), points.Num());

	TArray<FVector> verts;

	float angle = FMath::DegreesToRadians(360.0f / segments);
	float sinA = FMath::Sin(angle);
	float cosA = FMath::Cos(angle);

	TArray<FVector> wp;
	for (int i = 0; i < points.Num(); i++)
	{
		wp.Add(points[i]);
	}

	FVector p0(wp[0].X, 0, 0);
	FVector pLast(wp[wp.Num() - 1].X, 0, 0);

	FGeneratedMeshTriangle tri;
	for (int segment = 0; segment < segments; segment++)
	{
		for (int i = 0; i < points.Num() - 1; i++)
		{
			FVector p1 = wp[i];
			FVector p2 = wp[i + 1];
			FVector p1r(p1.X, p1.Y*cosA - p1.Z*sinA, p1.Y*sinA + p1.Z*cosA);
			FVector p2r(p2.X, p2.Y*cosA - p2.Z*sinA, p2.Y*sinA + p2.Z*cosA);

			if (i == 0)
			{
				tri.Vertex0 = p1;
				tri.Vertex1 = p0;
				tri.Vertex2 = p1r;
				triangles.Add(tri);
			}

			tri.Vertex0 = p1;
			tri.Vertex1 = p1r;
			tri.Vertex2 = p2;
			triangles.Add(tri);

			tri.Vertex0 = p2;
			tri.Vertex1 = p1r;
			tri.Vertex2 = p2r;
			triangles.Add(tri);
			
			if (i == points.Num() - 2)
			{
				tri.Vertex0 = p2;
				tri.Vertex1 = p2r;
				tri.Vertex2 = pLast;
				triangles.Add(tri);
				wp[i + 1] = p2r;
			}

			wp[i] = p1r;
		}
	}
}
