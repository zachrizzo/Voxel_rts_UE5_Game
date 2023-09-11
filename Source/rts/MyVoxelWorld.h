#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Async/Async.h"
#include "MyVoxel.h"
#include "ProceduralMeshComponent.h"
#include "MyVoxelWorld.generated.h"

UCLASS()
class RTS_API AMyVoxelWorld : public AActor
{
    GENERATED_BODY()

public:
    // Constructor
    AMyVoxelWorld();

    // Variables
    UPROPERTY(EditAnywhere)
    FVector GridSize;

    UPROPERTY(EditAnywhere)
    float VoxelSize;

    UPROPERTY(EditAnywhere, Category = "Mesh")
    TArray<UStaticMesh*> VoxelMeshes;
    
    UPROPERTY()
        UProceduralMeshComponent* GreedyProceduralMesh;

    int VoxelArrayWidth;
    int VoxelArrayHeight;
    int VoxelArrayDepth;

    TArray<TArray<TArray<TArray<bool>>>> visibilityFlags3D;
    TArray<TArray<TArray<int>>> VoxelArray;
    TArray<AMyVoxel*> SpawnedVoxels;
  


    AActor* GreedyMeshActor;  // or the actual type of GreedyMeshActor
    UProceduralMeshComponent* ProceduralMesh;  // or the actual type of ProceduralMesh

    TArray<FVector> GlobalVertices;
    TArray<int32> GlobalTriangles;
    TArray<FVector> GreedyVertices;
    TArray<int32> GreedyTriangles;

    // Methods
    virtual void OnConstruction(const FTransform& Transform) override;
    void InitializeAsyncTasks();
    void DestroyPreviousVoxels();
    void GenerateNewVoxels();
    void GenerateColumn(int x, int y, const FName& VoxelFolderName);
    void SpawnVoxel(int x, int y, int z, const FName& VoxelFolderName);
    void PopulateVoxelArray(int x, int y);
    bool CanMerge(const TArray<bool>& mask, int x1, int y1, int x2, int y2, int width);
    void AddFace(const FVector& v0, const FVector& v1, const FVector& v2, const FVector& v3, TArray<FVector>& Vertices, TArray<int32>& Triangles);
    void SetGreedyMesh(const TArray<FVector>& NewVertices, const TArray<int32>& NewTriangles);
    void AddVisibleFaces(
        const FVector& v0, const FVector& v1, const FVector& v2, const FVector& v3,
        const FVector& v4, const FVector& v5, const FVector& v6, const FVector& v7,
        const TArray<bool>& visibilityFlags,
        TArray<FVector>& Vertices, TArray<int32>& Triangles
    );
    void DestroyVoxel(FVector WorldPosition);
    TArray<bool> CalculateVisibilityFlags(int x, int y, int z);
    void GenerateGreedyMeshes();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
    void DestroySpawnedVoxels();
    void GenerateMeshFromMask(const TArray<bool>& mask, int z, TArray<FVector>& Vertices, TArray<int32>& Triangles);

    UFUNCTION(BlueprintCallable, Category = "VoxelEngine")
    void DestroyVoxel(int x, int y, int z);
};
