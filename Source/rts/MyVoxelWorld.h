// MyVoxelWorld.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Async/Async.h"
#include "MyVoxel.h"
#include "MyVoxelWorld.generated.h"

UCLASS()
class RTS_API AMyVoxelWorld : public AActor
{
    GENERATED_BODY()
    
public:
    // Sets default values for this actor's properties
    AMyVoxelWorld();

    UPROPERTY(EditAnywhere)
    FVector GridSize;
    UPROPERTY(EditAnywhere)
    int ChunkSize;  // Size of a chunk in voxels
    UPROPERTY(EditAnywhere)
    int LoadRadius;  // Number of chunks to load around the player
    UPROPERTY(EditAnywhere)
    float VoxelSize;
    
    int VoxelArryWidth;
    int VoxelArryHeight;
    int VoxelArryDepth;
    TArray<TArray<TArray<int>>> VoxelArray;

    UPROPERTY(EditAnywhere, Category = "Mesh")
    TArray<UStaticMesh*> VoxelMeshes;
    TSet<FVector> LoadedChunks;  // Set of currently loaded chunks
    TArray<AMyVoxel*> SpawnedVoxels;

    virtual void OnConstruction(const FTransform& Transform) override;
    void DestroyVoxel(FVector WorldPosition);  // New function to destroy voxels
    void LoadChunk(int chunkX, int chunkY);
    void UnloadChunk(int chunkX, int chunkY);
    
    TArray<bool> CalculateVisibilityFlags(int x, int y, int z);

    FVector GetPlayerPosition();  // Placeholder for getting the player's position
    


protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Add functions for LOD, Streaming, Noise generation etc.
};
