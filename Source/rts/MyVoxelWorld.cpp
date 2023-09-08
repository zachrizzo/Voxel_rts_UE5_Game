#include "MyVoxelWorld.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Async/Async.h"
#include "Async/AsyncWork.h"
#include <cmath>
#include "NoiseUtility.h"
#include "MyVoxel.h"

AMyVoxelWorld::AMyVoxelWorld()
{
    // Set this actor to call Tick() every frame
    PrimaryActorTick.bCanEverTick = true;
    

    // Default grid size and voxel size
    GridSize = FVector(20, 20, 20);
    VoxelSize = 100.0f;
    VoxelArrayWidth = GridSize.X;
    VoxelArrayHeight = GridSize.Y;
    VoxelArrayDepth = GridSize.Z;
    VoxelArray.SetNum(VoxelArrayWidth);
    for (int x = 0; x < VoxelArrayWidth; ++x) {
        VoxelArray[x].SetNum(VoxelArrayHeight);
        for (int y = 0; y < VoxelArrayHeight; ++y) {
            VoxelArray[x][y].SetNum(VoxelArrayDepth);
        }
        
    }
    visibilityFlags3D.SetNum(VoxelArrayWidth);
        for (int x = 0; x < VoxelArrayWidth; ++x) {
            visibilityFlags3D[x].SetNum(VoxelArrayHeight);
            for (int y = 0; y < VoxelArrayHeight; ++y) {
                visibilityFlags3D[x][y].SetNum(VoxelArrayDepth);
            }
        }

}


TArray<bool> AMyVoxelWorld::CalculateVisibilityFlags(int x, int y, int z) {
    TArray<bool> visibilityFlags;
    visibilityFlags.Init(false, 6);  // Initialize with 6 false values
    
    if (x >= 0 && x < VoxelArrayWidth && y >= 0 && y < VoxelArrayHeight && z >= 0 && z < VoxelArrayDepth) {
        if (VoxelArray[x][y][z] == 1) {
          
            
            if (y + 1 < VoxelArrayHeight) {  // Check that we're not out of bounds
                if (VoxelArray[x][y + 1][z] == 1) {  // There is a voxel immediately behind
                    visibilityFlags[3] = false;
                    UE_LOG(LogTemp, Warning, TEXT("0"));
                
                } else {
                    visibilityFlags[3] = true;
                }
            } else {
                // Handle the case where y + 1 is out of bounds. Here we assume no voxel is behind.
                visibilityFlags[3] = true;
            }

            // Front
            if (y - 1 >= 0 && VoxelArray[x][y - 1][z] == 1) {
                visibilityFlags[2] = false;
            } else {
                visibilityFlags[2] = true;
            }

        
            // Right (in negative X-direction)
            if (x - 1 >= 0 && VoxelArray[x - 1][y][z] == 1) {
                visibilityFlags[1] = false;
            } else {
                visibilityFlags[1] = true;
            }



            // Left
            if (x + 1 < VoxelArrayWidth && VoxelArray[x + 1][y][z] == 1) {
                visibilityFlags[0] = false;
            } else {
                visibilityFlags[0] = true;
            }

            // Top (in positive Z-direction)
            if (z + 1 < VoxelArrayDepth && VoxelArray[x][y][z + 1] == 1) {
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
    DestroyPreviousVoxels();
    InitializeAsyncTasks();
    
//    // Generate the greedy meshes for the entire grid
//    GenerateGreedyMeshes();
}

void AMyVoxelWorld::InitializeAsyncTasks()
{
//    Async(EAsyncExecution::Thread, [=]()
//    {
        GenerateNewVoxels();
//    });
}

void AMyVoxelWorld::DestroyPreviousVoxels()
{
//    AsyncTask(ENamedThreads::GameThread, [=]()
//    {
        for (AMyVoxel* Voxel : SpawnedVoxels)
        {
            if (Voxel && Voxel->IsValidLowLevel() && Voxel->GetWorld())
            {
                Voxel->Destroy();
            }
        }
        SpawnedVoxels.Empty();
//    });
}

void AMyVoxelWorld::GenerateNewVoxels()
{
    // Folder name in World Outliner
    FName VoxelFolderName = FName(TEXT("Voxels"));

    if (VoxelMeshes.Num() == 0) return;

    // First, populate the VoxelArray
    for (int x = 0; x < GridSize.X; ++x)
    {
        for (int y = 0; y < GridSize.Y; ++y)
        {
            PopulateVoxelArray(x, y);
        }
    }

    // Then, spawn the voxels and calculate visibility
    for (int x = 0; x < GridSize.X; ++x)
    {
        for (int y = 0; y < GridSize.Y; ++y)
        {
            
            PopulateVoxelArray(x, y);
//            for (int z = 0; z < GridSize.Z; ++z)
//            {
//                if (VoxelArray[x][y][z] == 1)
//                {
//                    SpawnVoxel(x, y, z, VoxelFolderName);
//                }
//            }
        }
     
    }
    GenerateGreedyMeshes();
}


void AMyVoxelWorld::PopulateVoxelArray(int x, int y)
{
    float height = UNoiseUtility::PerlinNoise(x * 0.1f, y * 0.1f) * 10;
    int maxHeight = FMath::RoundToInt(height);

    for (int z = 0; z < GridSize.Z; ++z)
    {
        if (z <= maxHeight && x < VoxelArrayWidth && y < VoxelArrayHeight && z < VoxelArrayDepth)
        {
            VoxelArray[x][y][z] = 1;
        }
        else if (x < VoxelArrayWidth && y < VoxelArrayHeight && z < VoxelArrayDepth)
        {
            VoxelArray[x][y][z] = 0;
        }
    }
}


void AMyVoxelWorld::GenerateColumn(int x, int y, const FName& VoxelFolderName)
{
    float height = UNoiseUtility::PerlinNoise(x * 0.1f, y * 0.1f) * 10;  // Use Perlin noise
    int maxHeight = FMath::RoundToInt(height);  // Convert to an integer for use in the loop

    for (int z = 0; z < GridSize.Z; ++z)
    {
        if (z <= maxHeight)
        {
            SpawnVoxel(x, y, z, VoxelFolderName);
        }
    }
}

void AMyVoxelWorld::SpawnVoxel(int x, int y, int z, const FName& VoxelFolderName)
{
    UE_LOG(LogTemp, Warning, TEXT("SpawnVoxel called with x: %d, y: %d, z: %d"), x, y, z);
        UE_LOG(LogTemp, Warning, TEXT("VoxelFolderName: %s"), *VoxelFolderName.ToString());

        FVector Position(x * VoxelSize, y * VoxelSize, z * VoxelSize);
        AMyVoxel* VoxelActor = GetWorld()->SpawnActor<AMyVoxel>(Position, FRotator::ZeroRotator);

    if(VoxelActor == nullptr) {
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn VoxelActor"));
        return;
    } else {
        UE_LOG(LogTemp, Warning, TEXT("Successfully spawned VoxelActor"));
    }

    if(VoxelActor != nullptr)
        {
            
            // You can add logging or other functionality here.
            SpawnedVoxels.Add(VoxelActor);
        }

    if (x < VoxelArrayWidth && y < VoxelArrayHeight && z < VoxelArrayDepth)
    {
        VoxelArray[x][y][z] = 1;
    }

    TArray<bool> visibilityFlags = CalculateVisibilityFlags(x, y, z);
    VoxelActor->SetVisibleFaces(visibilityFlags);

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
}



void AMyVoxelWorld::DestroySpawnedVoxels()
{
    for (AMyVoxel* Voxel : SpawnedVoxels)
    {
        if (Voxel && Voxel->IsValidLowLevel() && Voxel->GetWorld())
        {
            Voxel->Destroy();
        }
    }
    SpawnedVoxels.Empty();
}


void AMyVoxelWorld::BeginPlay()
{
    Super::BeginPlay();

}





void AMyVoxelWorld::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}



void AMyVoxelWorld::GenerateGreedyMeshes()
{
    // Create a single AMyVoxel actor for the entire grid
    FVector Position(0, 0, 0);
    AMyVoxel* VoxelActor = GetWorld()->SpawnActor<AMyVoxel>(Position, FRotator::ZeroRotator);

    // Generate the greedy mesh for the entire grid and set it on the AMyVoxel actor
    TArray<FVector> Vertices;
    TArray<int32> Triangles;

    for (int z = 0; z < GridSize.Z; ++z)
    {
        TArray<bool> mask;
        mask.SetNumZeroed(GridSize.X * GridSize.Y);

        // Populate the mask for this layer
        for (int x = 0; x < GridSize.X; ++x)
        {
            for (int y = 0; y < GridSize.Y; ++y)
            {
                mask[x + y * GridSize.X] = VoxelArray[x][y][z] == 1;
            }
        }

        // Generate the mesh for this layer
        GenerateMeshFromMask(mask, z, Vertices, Triangles);
    }

    // Finally, update the single VoxelActor with the complete mesh
    VoxelActor->SetGreedyMesh(Vertices, Triangles);
}




bool AMyVoxelWorld::CanMerge(const TArray<bool>& mask, int x1, int y1, int x2, int y2, int width) {
    return mask[x1 + y1 * width] && mask[x2 + y2 * width];
}



void AMyVoxelWorld::GenerateMeshFromMask(const TArray<bool>& mask, int z, TArray<FVector>& Vertices, TArray<int32>& Triangles)
{
    TArray<bool> maskCopy = mask;

    for (int y = 0; y < GridSize.Y; ++y) {
        int x = 0;
        while (x < GridSize.X) {
            if (mask[x + y * GridSize.X]) {
                // We found a visible voxel. Try to extend it as far as possible in the X direction.
                int width = 1;
                while (x + width < GridSize.X && CanMerge(mask, x, y, x + width, y, GridSize.X)) {
                    ++width;
                }

                // We've found a line of width "width" starting from (x, y). Now try to extend it in the Y direction.
                int height = 1;
                bool continueMerging = true;
                while (y + height < GridSize.Y && continueMerging) {
                    for (int dx = 0; dx < width; ++dx) {
                        if (!CanMerge(mask, x + dx, y, x + dx, y + height, GridSize.X)) {
                            continueMerging = false;
                            break;
                        }
                    }
                    if (continueMerging) {
                        ++height;
                    }
                }

                // Create the quad
                FVector v0(x, y, z);
                FVector v1(x + width, y, z);
                FVector v2(x + width, y + height, z);
                FVector v3(x, y + height, z);

                // Calculate the additional vertices based on your specific requirements
                FVector v4(x, y, z + 1);  // These are placeholders
                FVector v5(x + width, y, z + 1);  // Update as needed
                FVector v6(x + width, y + height, z + 1);
                FVector v7(x, y + height, z + 1);
                
                int BaseVertexIndex = Vertices.Num();
                
                Vertices.Add(v0);
                Vertices.Add(v1);
                Vertices.Add(v2);
                Vertices.Add(v3);
                
                AddVisibleFaces(v0, v1, v2, v3, v4, v5, v6, v7, CalculateVisibilityFlags(x, y, z), Vertices, Triangles);




                // Update the mask to indicate that these voxels have been processed
                for (int dx = 0; dx < width; ++dx) {
                    for (int dy = 0; dy < height; ++dy) {
                        maskCopy[x + dx + (y + dy) * GridSize.X] = false;
                    }
                }

                // Move the cursor
                x += width;
            } else {
                ++x;
            }
        }
    }
}


void AMyVoxelWorld::AddVisibleFaces(
    const FVector& v0, const FVector& v1, const FVector& v2, const FVector& v3,
    const FVector& v4, const FVector& v5, const FVector& v6, const FVector& v7,
    const TArray<bool>& visibilityFlags,
    TArray<FVector>& Vertices, TArray<int32>& Triangles)
{
    if (visibilityFlags[0]) {
   
        AddFace(v2, v1, v5, v6, Vertices,Triangles);   // Left
    }
    if (visibilityFlags[1]) {
        AddFace(v0, v3, v7,v4 ,Vertices,Triangles);    // Right
    }
    if (visibilityFlags[2]) {
        AddFace(v1, v0, v4, v5,Vertices,Triangles);      // Front
    }
    if (visibilityFlags[3]) {
        AddFace(v6, v7, v3, v2,Vertices,Triangles);    //Back
    }
    if (visibilityFlags[4]) {
        AddFace(v7, v6, v5, v4,Vertices,Triangles);   // Top face
    }
    if (visibilityFlags[5]) {
        AddFace(v0, v1, v2, v3,Vertices,Triangles);    // Bottom
    }
    
}

void AMyVoxelWorld::AddFace(const FVector& v0, const FVector& v1, const FVector& v2, const FVector& v3,
                            TArray<FVector>& Vertices, TArray<int32>& Triangles)
{
    int32 BaseVertexIndex = Vertices.Num();
        
    // Define vertices based on input vertices
    Vertices.Add(v0);
    Vertices.Add(v1);
    Vertices.Add(v2);
    Vertices.Add(v3);
    
    // First triangle
    Triangles.Add(BaseVertexIndex);
    Triangles.Add(BaseVertexIndex + 1);
    Triangles.Add(BaseVertexIndex + 2);
    
    // Second triangle
    Triangles.Add(BaseVertexIndex + 2);
    Triangles.Add(BaseVertexIndex + 3);
    Triangles.Add(BaseVertexIndex);
}
