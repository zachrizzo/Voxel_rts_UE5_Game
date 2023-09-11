#include "MyVoxelWorld.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Async/Async.h"
#include "Async/AsyncWork.h"
#include <cmath>
#include "NoiseUtility.h"
#include "MyVoxel.h"

AMyVoxelWorld::AMyVoxelWorld() {
    // Set this actor to call Tick() every frame
    PrimaryActorTick.bCanEverTick = true;
    GlobalVertices.Empty();
    GlobalTriangles.Empty();
    
    // Default grid size and voxel size
    GridSize = FVector(200, 200,200);
    VoxelSize = 100.0f;
    VoxelArrayWidth = GridSize.X;
    VoxelArrayHeight = GridSize.Y;
    VoxelArrayDepth = GridSize.Z;
    
    // Initialize Greedy Procedural Mesh
    GreedyProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("GreedyGeneratedMesh"));
    RootComponent = GreedyProceduralMesh;
    
    // Initialize VoxelArray and visibilityFlags3D 3D grid
    VoxelArray.SetNum(VoxelArrayWidth);
    visibilityFlags3D.SetNum(VoxelArrayWidth);
    for (int x = 0; x < VoxelArrayWidth; ++x) {
        VoxelArray[x].SetNum(VoxelArrayHeight);
        visibilityFlags3D[x].SetNum(VoxelArrayHeight);
        for (int y = 0; y < VoxelArrayHeight; ++y) {
            VoxelArray[x][y].SetNum(VoxelArrayDepth);
            visibilityFlags3D[x][y].SetNum(VoxelArrayDepth);
            for (int z = 0; z < VoxelArrayDepth; ++z) {
                VoxelArray[x][y][z] = 0;
                visibilityFlags3D[x][y][z] = TArray<bool>();  // Initialize with empty TArray
            }
        }
    }
}



TArray<bool> AMyVoxelWorld::CalculateVisibilityFlags(int x, int y, int z) {
    TArray<bool> visibilityFlags;
    visibilityFlags.Init(false, 6);  // Initialize with 6 false values
    
    if (x >= 0 && x < VoxelArrayWidth && y >= 0 && y < VoxelArrayHeight && z >= 0 && z < VoxelArrayDepth) {
        if (VoxelArray[x][y][z] == 1) {
            // Initialize all flags to true initially
            for (int i = 0; i < 6; ++i) {
                visibilityFlags[i] = true;
            }

            // Check all six directions and update the visibility flags
            if (x - 1 >= 0 && VoxelArray[x - 1][y][z] == 1) visibilityFlags[0] = false;
            if (x + 1 < VoxelArrayWidth && VoxelArray[x + 1][y][z] == 1) visibilityFlags[1] = false;
            if (y - 1 >= 0 && VoxelArray[x][y - 1][z] == 1) visibilityFlags[2] = false;
            if (y + 1 < VoxelArrayHeight && VoxelArray[x][y + 1][z] == 1) visibilityFlags[3] = false;
            if (z - 1 >= 0 && VoxelArray[x][y][z - 1] == 1) visibilityFlags[4] = false;
            if (z + 1 < VoxelArrayDepth && VoxelArray[x][y][z + 1] == 1) visibilityFlags[5] = false;
            
            // Log the visibility flags for debugging
            FString VisFlagsString;
            for (bool flag : visibilityFlags) {
                VisFlagsString.Append(flag ? TEXT("1, ") : TEXT("0, "));
            }
            UE_LOG(LogTemp, Warning, TEXT("Voxel at (%d, %d, %d) - Visibility Flags: %s"), x, y, z, *VisFlagsString);
        }
    }
    
    return visibilityFlags;
}







void AMyVoxelWorld::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
//    DestroyPreviousVoxels();
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

//    // Then, spawn the voxels and calculate visibility
//    for (int x = 0; x < GridSize.X; ++x)
//    {
//        for (int y = 0; y < GridSize.Y; ++y)
//        {
//
//            PopulateVoxelArray(x, y);
////            for (int z = 0; z < GridSize.Z; ++z)
////            {
////                if (VoxelArray[x][y][z] == 1)
////                {
////                    SpawnVoxel(x, y, z, VoxelFolderName);
////                }
////            }
//        }
//
//    }
    if (GreedyMeshActor && GreedyMeshActor->IsValidLowLevel() && GreedyMeshActor->GetWorld()) {
            GreedyMeshActor->Destroy();
        }
    GenerateGreedyMeshes();
}


void AMyVoxelWorld::PopulateVoxelArray(int x, int y) {
    float height = UNoiseUtility::PerlinNoise(x * 0.1f, y * 0.1f) * 10;
    int maxHeight = FMath::RoundToInt(height);

    for (int z = 0; z < GridSize.Z; ++z) {
        if (z <= maxHeight && x < VoxelArrayWidth && y < VoxelArrayHeight && z < VoxelArrayDepth) {
            VoxelArray[x][y][z] = 1;
            visibilityFlags3D[x][y][z] = CalculateVisibilityFlags(x, y, z);
        } else if (x < VoxelArrayWidth && y < VoxelArrayHeight && z < VoxelArrayDepth) {
            VoxelArray[x][y][z] = 0;
            visibilityFlags3D[x][y][z].Init(false, 6);  // Initialize with 6 false values
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
    // Clear the existing vertices and triangles
    GlobalVertices.Empty();
    GlobalTriangles.Empty();
    
    // Destroy the previous mesh if it exists
    if (GreedyMeshActor && GreedyMeshActor->IsValidLowLevel() && GreedyMeshActor->GetWorld()) {
        GreedyMeshActor->Destroy();
    }

    // Create a single AMyVoxel actor for the entire grid
    FVector Position(0, 0, 0);
    GreedyMeshActor = GetWorld()->SpawnActor<AMyVoxel>(Position, FRotator::ZeroRotator);

    // Generate the greedy mesh for the entire grid
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
        GenerateMeshFromMask(mask, z, GlobalVertices, GlobalTriangles);
    }

    // Finally, update the single VoxelActor with the complete mesh
    SetGreedyMesh(GlobalVertices, GlobalTriangles);}


void AMyVoxelWorld::SetGreedyMesh(const TArray<FVector>& NewVertices, const TArray<int32>& NewTriangles)
{
    // Update the mesh using the new greedy vertices and triangles
    GreedyProceduralMesh->CreateMeshSection_LinearColor(0, NewVertices, NewTriangles, TArray<FVector>(), TArray<FVector2D>(), TArray<FLinearColor>(), TArray<FProcMeshTangent>(), true);
}


bool AMyVoxelWorld::CanMerge(const TArray<bool>& mask, int x1, int y1, int x2, int y2, int width) {
    return mask[x1 + y1 * width] && mask[x2 + y2 * width];
}



void AMyVoxelWorld::GenerateMeshFromMask(const TArray<bool>& mask, int z, TArray<FVector>& Vertices, TArray<int32>& Triangles)
{
    TArray<bool> maskCopy = mask;

    for (int y = 0; y < GridSize.Y; ++y) {
        for (int x = 0; x < GridSize.X; ++x) {
            TArray<bool> visibilityFlags = visibilityFlags3D[x][y][z];
            
            // Check for a visible voxel at the current coordinates
            if (maskCopy[x + y * GridSize.X] && visibilityFlags.Contains(true)) {
                int width = 1, height = 1;
                
                // Check for expandability in x and y directions
                for (width = 1; x + width < GridSize.X; ++width) {
                    if (!CanMerge(maskCopy, x, y, x + width, y, GridSize.X) ||
                        !visibilityFlags3D[x + width][y][z].Contains(true)) {
                        break;
                    }
                }
                
                for (height = 1; y + height < GridSize.Y; ++height) {
                    bool canMerge = true;
                    for (int dx = 0; dx < width; ++dx) {
                        if (!CanMerge(maskCopy, x + dx, y, x + dx, y + height, GridSize.X) ||
                            !visibilityFlags3D[x + dx][y + height][z].Contains(true)) {
                            canMerge = false;
                            break;
                        }
                    }
                    if (!canMerge) break;
                }

                // Create the quad vertices
                FVector v0(x, y, z);
                FVector v1(x + width, y, z);
                FVector v2(x + width, y + height, z);
                FVector v3(x, y + height, z);
                FVector v4(x, y, z + 1);
                FVector v5(x + width, y, z + 1);
                FVector v6(x + width, y + height, z + 1);
                FVector v7(x, y + height, z + 1);

                // Loop over each voxel within the rectangle to add its visible faces
                for (int dx = 0; dx < width; ++dx) {
                    for (int dy = 0; dy < height; ++dy) {
                        TArray<bool> currentVisibilityFlags = visibilityFlags3D[x + dx][y + dy][z];
                        AddVisibleFaces(v0, v1, v2, v3, v4, v5, v6, v7, currentVisibilityFlags, Vertices, Triangles);
                    }
                }
                
                // Update maskCopy to skip processed voxels
                for (int dx = 0; dx < width; ++dx) {
                    for (int dy = 0; dy < height; ++dy) {
                        maskCopy[x + dx + (y + dy) * GridSize.X] = false;
                    }
                }

                x += width - 1; // Skip the voxels that were just processed
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
    // Note: Assuming visibilityFlags are in order: [Left, Right, Front, Back, Top, Bottom]
    if (visibilityFlags[1]) AddFace(v2, v1, v5, v6, Vertices, Triangles);   // right
    if (visibilityFlags[0]) AddFace(v0, v3, v7, v4, Vertices, Triangles);   // left
    if (visibilityFlags[2]) AddFace(v1, v0, v4, v5, Vertices, Triangles);   // Front
    if (visibilityFlags[3]) AddFace(v6, v7, v3, v2, Vertices, Triangles);   // Back
    if (visibilityFlags[5]) AddFace(v7, v6, v5, v4, Vertices, Triangles);   // bottom
    if (visibilityFlags[4]) AddFace(v0, v1, v2, v3, Vertices, Triangles);   // top
}
void AMyVoxelWorld::AddFace(const FVector& v0, const FVector& v1, const FVector& v2, const FVector& v3, TArray<FVector>& Vertices, TArray<int32>& Triangles)
{
    
    //this function creats a face using the input vertices and adds it to the mesh
        
    
    
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

void AMyVoxelWorld::DestroyVoxel(int x, int y, int z) {
    // Validate the input coordinates
    if (x < 0 || x >= GridSize.X || y < 0 || y >= GridSize.Y || z < 0 || z >= GridSize.Z) {
        return;
    }

    // Destroy the voxel in the internal representation
    VoxelArray[x][y][z] = 0;

    // Regenerate the greedy mesh
    GenerateGreedyMeshes();
}
