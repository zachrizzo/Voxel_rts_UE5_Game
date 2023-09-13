#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Async/Async.h"
#include "ProceduralMeshComponent.h"
#include "VoxelEngine.generated.h"

USTRUCT(BlueprintType)
struct FVoxelMaterialMap
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Material")
    int Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Material")
    UMaterialInterface* Material;
};

UCLASS()
class RTS_API AVoxelEngine : public AActor
{
    GENERATED_BODY()

public:
    AVoxelEngine();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
    int32 VoxelArrayWidth = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
    int32 VoxelArrayHeight = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
    int32 VoxelArrayDepth = 10;

    UPROPERTY(EditAnywhere)
    float VoxelSize;
    
    UPROPERTY(VisibleAnywhere)
    UProceduralMeshComponent* ProceduralMesh;
    
    UPROPERTY(EditAnywhere, Category = "Voxel")
    TArray<FVoxelMaterialMap> VoxelMaterialMappings;
    
    struct Voxel {
        int type;
        int health;
        int voxelId;
        bool isVisible[6];
    };

    TArray<TArray<TArray<Voxel>>> world;
    float NoiseScale = 0.1f;
    
    virtual void OnConstruction(const FTransform& Transform) override;
    void CalculateVisibility();
    void GenerateGreedyMesh();
    float Perlin3D(float x, float y, float z);

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
};
