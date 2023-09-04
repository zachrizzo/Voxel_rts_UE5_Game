#include "MyVoxelWorld.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"  // Add this line
#include <cmath>
#include "NoiseUtility.h"
#include "MyVoxel.h"


AMyVoxelWorld::AMyVoxelWorld()
{
    // Set this actor to call Tick() every frame
    PrimaryActorTick.bCanEverTick = true;
    

    // Default grid size and voxel size
    GridSize = FVector(100, 100, 100);
    VoxelSize = 100.0f;
    ChunkSize = 100;  // Initialize ChunkSize
    LoadRadius = 2;  // Initialize LoadRadius
    VoxelArryWidth = GridSize.X;
    VoxelArryHeight = GridSize.Y;
    VoxelArryDepth = GridSize.Z;
    VoxelArray.SetNum(VoxelArryWidth);
    for (int x = 0; x < VoxelArryWidth; ++x) {
        VoxelArray[x].SetNum(VoxelArryHeight);
        for (int y = 0; y < VoxelArryHeight; ++y) {
            VoxelArray[x][y].SetNum(VoxelArryDepth);
        }
        
    }

}

TArray<bool> AMyVoxelWorld::CalculateVisibilityFlags(int x, int y, int z) {
    TArray<bool> visibilityFlags;
    visibilityFlags.Init(false, 6);  // Initialize with 6 false values
    
    if (x >= 0 && x < VoxelArryWidth && y >= 0 && y < VoxelArryHeight && z >= 0 && z < VoxelArryDepth) {
        if (VoxelArray[x][y][z] == 1) {
          
            
            if (y + 1 < VoxelArryHeight && VoxelArray[x][y + 1][z] == 1) {  // There is a voxel immediately behind
                    visibilityFlags[0] = false;
            } else {
                visibilityFlags[0] = true;
            }

            // Front
            if (y - 1 >= 0 && VoxelArray[x][y - 1][z] == 1) {
                visibilityFlags[1] = false;
            } else {
                visibilityFlags[1] = true;
            }

        
            // Right (in negative X-direction)
            if (x - 1 >= 0 && VoxelArray[x - 1][y][z] == 1) {
                visibilityFlags[2] = false;
            } else {
                visibilityFlags[2] = true;
            }



            // Left
            if (x + 1 < VoxelArryWidth && VoxelArray[x + 1][y][z] == 1) {
                visibilityFlags[3] = false;
            } else {
                visibilityFlags[3] = true;
            }

            // Top (in positive Z-direction)
            if (z + 1 < VoxelArryDepth && VoxelArray[x][y][z + 1] == 1) {
                visibilityFlags[4] = false;
            } else {
                visibilityFlags[4] = true;
            }

            // Bottom (in negative Z-direction)
            if (z - 1 >= 0 && VoxelArray[x][y][z - 1] == 1) {
                visibilityFlags[5] = false;
            } else {
                visibilityFlags[5] = true;
            }
        }
    }

    return visibilityFlags;
}





void AMyVoxelWorld::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    Async(EAsyncExecution::Thread, [=]()
    {
        // Destroy previously spawned voxels
        AsyncTask(ENamedThreads::GameThread, [=]()
        {
            for (AMyVoxel* Voxel : SpawnedVoxels)
            {
                if (Voxel && Voxel->IsValidLowLevel() && Voxel->GetWorld())
                {
                    Voxel->Destroy();
                }
            }
            SpawnedVoxels.Empty();

            // ... (Chunking, LOD, Meshing, and other optimizations)

            // Folder name in World Outliner
            FName VoxelFolderName = FName(TEXT("Voxels"));

            if (VoxelMeshes.Num() == 0) return;

            for (int x = 0; x < GridSize.X; ++x)
            {
                for (int y = 0; y < GridSize.Y; ++y)
                {
                    float height = UNoiseUtility::PerlinNoise(x * 0.1f, y * 0.1f) * 10;  // Use Perlin noise
                    int maxHeight = FMath::RoundToInt(height);  // Convert to an integer for use in the loop

                    for (int z = 0; z < GridSize.Z; ++z)
                    {
                        if (z <= maxHeight)  // Only spawn voxels up to maxHeight
                        {
                            bool top = (z == maxHeight);
                            bool bottom = (z == 0);
                            bool left = (x == 0);
                            bool right = (x == GridSize.X - 1);
                            bool front = (y == 0);
                            bool back = (y == GridSize.Y - 1);

                            FVector Position(x * VoxelSize, y * VoxelSize, z * VoxelSize);
                            AMyVoxel* VoxelActor = GetWorld()->SpawnActor<AMyVoxel>(Position, FRotator::ZeroRotator);
                            SpawnedVoxels.Add(VoxelActor);

                            if (x < VoxelArryWidth && y < VoxelArryHeight && z < VoxelArryDepth)
                                {
                                    VoxelArray[x][y][z] = 1;
                                }

                            
                            
                            TArray<bool> visibilityFlags = CalculateVisibilityFlags(x, y, z);
                            VoxelActor->SetVisibleFaces(visibilityFlags);

                                
//                                // Log the visibility flags
//                                for (int i = 0; i < visibilityFlags.Num(); ++i) {
//                                    UE_LOG(LogTemp, Warning, TEXT("visibilityFlags[%d] = %d"), i, visibilityFlags[i]);
//                                }
                                

#if WITH_EDITOR
                                if (!VoxelFolderName.ToString().IsEmpty())
                                {
                                    VoxelActor->SetFolderPath(VoxelFolderName);
                                }
                                else
                                {
                                    UE_LOG(LogTemp, Warning, TEXT("VoxelFolderName is empty"));
                                }
#endif
//                                int RandomIndex = FMath::RandRange(0, VoxelMeshes.Num() - 1);
//                                UStaticMesh* SelectedMesh = VoxelMeshes[RandomIndex];
//
//                                if (VoxelActor && SelectedMesh)
//                                {
//                                    VoxelActor->UpdateProceduralMesh(SelectedMesh);
//                                }
                            
                        }
                    }
                }
            }
        });
    });
}


// New function to allow destruction of individual voxels
void AMyVoxelWorld::DestroyVoxel(FVector WorldPosition)
{
    for (AMyVoxel* Voxel : SpawnedVoxels)
    {
        if (Voxel)
        {
            if (Voxel->GetActorLocation() == WorldPosition)
            {
                Voxel->Destroy();
                SpawnedVoxels.Remove(Voxel);
                break;
            }
        }
    }
}

void AMyVoxelWorld::BeginPlay()
{
    Super::BeginPlay();
    LoadedChunks.Empty();  // Initialize the set of loaded chunks
    
}

void AMyVoxelWorld::LoadChunk(int chunkX, int chunkY)
{
//    UE_LOG(LogTemp, Warning, TEXT("Loading chunk (%d, %d)"), chunkX, chunkY);
//    for (int x = chunkX * ChunkSize; x < (chunkX + 1) * ChunkSize; ++x)
//    {
//        for (int y = chunkY * ChunkSize; y < (chunkY + 1) * ChunkSize; ++y)
//        {
//            float height = UNoiseUtility::PerlinNoise(x * 0.1f, y * 0.1f) * 10;  // Use Perlin noise
//            int maxHeight = FMath::RoundToInt(height);  // Convert to an integer for use in the loop
//
//            for (int z = 0; z < GridSize.Z; ++z)
//            {
//                if (z <= maxHeight)
//                {
//                    FVector Position(x * VoxelSize, y * VoxelSize, z * VoxelSize);
//                    AMyVoxel* VoxelActor = GetWorld()->SpawnActor<AMyVoxel>(Position, FRotator::ZeroRotator);
//                    if (VoxelActor)
//                    {
//                        SpawnedVoxels.Add(VoxelActor);
//                    }
//                }
//            }
//        }
//    }
}

void AMyVoxelWorld::UnloadChunk(int chunkX, int chunkY)
{
    
//    int startX = chunkX * ChunkSize;
//    int startY = chunkY * ChunkSize;
//    int endX = startX + ChunkSize;
//    int endY = startY + ChunkSize;
//    UE_LOG(LogTemp, Warning, TEXT("Unloading chunk (%d, %d)"), chunkX, chunkY);
//
//    TArray<AMyVoxel*> VoxelsToRemove;
//
//    for (AMyVoxel* Voxel : SpawnedVoxels)
//    {
//        if (Voxel)
//        {
//            FVector pos = Voxel->GetActorLocation();
//            int voxelX = pos.X / VoxelSize;
//            int voxelY = pos.Y / VoxelSize;
//
//            if (voxelX >= startX && voxelX < endX && voxelY >= startY && voxelY < endY)
//            {
//                Voxel->Destroy();
//                VoxelsToRemove.Add(Voxel);
//            }
//        }
//    }
//
//    for (AMyVoxel* Voxel : VoxelsToRemove)
//    {
//        SpawnedVoxels.Remove(Voxel);
//    }
}


FVector AMyVoxelWorld::GetPlayerPosition()
{
    // Get the player character (assuming the player is the first player, index 0)
    ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0);

    if (PlayerCharacter)
    {
        return PlayerCharacter->GetActorLocation();
    }
    else
    {
        // Return some default value if the player character is not available
        return FVector(0, 0, 0);
    }
}


void AMyVoxelWorld::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    FVector playerPosition = GetPlayerPosition();

    int currentPlayerChunkX = playerPosition.X / ChunkSize;
    int currentPlayerChunkY = playerPosition.Y / ChunkSize;

    TSet<FVector> ChunksToUnload = LoadedChunks;

    // Load chunks around the player
    for (int x = currentPlayerChunkX - LoadRadius; x <= currentPlayerChunkX + LoadRadius; ++x)
    {
        for (int y = currentPlayerChunkY - LoadRadius; y <= currentPlayerChunkY + LoadRadius; ++y)
        {
            FVector chunk(x, y, 0);
            if (!LoadedChunks.Contains(chunk))
            {
                LoadChunk(x, y);
                LoadedChunks.Add(chunk);
            }
            ChunksToUnload.Remove(chunk);
        }
    }

    // Unload chunks that are too far from the player
    for (FVector chunk : ChunksToUnload)
    {
        UnloadChunk(chunk.X, chunk.Y);
        LoadedChunks.Remove(chunk);
    }
}

