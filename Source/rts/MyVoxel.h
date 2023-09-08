// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "MyVoxel.generated.h"

UCLASS()
class RTS_API AMyVoxel : public AActor
{
	GENERATED_BODY()
	
public:
    AMyVoxel();
    void UpdateProceduralMesh(UStaticMesh* NewMesh);

    
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    void SetVisibleFaces( TArray<bool> visibilityFlags);
    void SetGreedyMesh(const TArray<FVector>& NewVertices, const TArray<int32>& NewTriangles);
    void CreateCube(float CubeSize);
    
    UPROPERTY()
    UProceduralMeshComponent* ProceduralMesh;
    
    TArray<FVector> Vertices;
    TArray<int32> Triangles;
    
    


    // Mesh component for the voxel
//    UPROPERTY()
//    UStaticMeshComponent* MeshComponent;
    
private:
    void AddFace(const FVector& v0, const FVector& v1, const FVector& v2, const FVector& v3);


//protected:
//	// Called when the game starts or when spawned
//	virtual void BeginPlay() override;
//
//public:
//	// Called every frame
//	virtual void Tick(float DeltaTime) override;

};
