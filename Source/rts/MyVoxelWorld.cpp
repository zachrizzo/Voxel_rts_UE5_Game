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
    GridSize = FVector(20, 20,20);
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
//            UE_LOG(LogTemp, Warning, TEXT("Voxel at (%d, %d, %d) - Visibility Flags: %s"), x, y, z, *VisFlagsString);
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
    UE_LOG(LogTemp, Warning, TEXT("Vertices: %d"), GlobalVertices.Num());
    UE_LOG(LogTemp, Warning, TEXT("Triangles: %d"), GlobalTriangles.Num());

}





void AMyVoxelWorld::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

bool AMyVoxelWorld::ValidateMeshIntegrity() {
    // Check if GlobalVertices or GlobalTriangles are empty
    if (GlobalVertices.Num() == 0 || GlobalTriangles.Num() == 0) {
        UE_LOG(LogTemp, Error, TEXT("Validation Failed: GlobalVertices or GlobalTriangles are empty."));
        return false;
    }

    // Check if the number of triangles is a multiple of 3 (since they should form complete faces)
    if (GlobalTriangles.Num() % 3 != 0) {
        UE_LOG(LogTemp, Error, TEXT("Validation Failed: GlobalTriangles count is not a multiple of 3."));
        return false;
    }

    // Check for any null or zero vertices (optional, based on your use-case)
    for (const FVector& vertex : GlobalVertices) {
        if (vertex.IsZero()) {
            UE_LOG(LogTemp, Error, TEXT("Validation Failed: Found zero vertex."));
            return false;
        }
    }

    // Additional checks can be added here

    return true;  // If all checks pass, return true.
}




void AMyVoxelWorld::GenerateGreedyMeshes() {
    // Clear existing vertices and triangles
//    GlobalVertices.Empty();
//    GlobalTriangles.Empty();

    // Generate the greedy mesh for the entire grid
    for (int z = 0; z < GridSize.Z; ++z) {
        TArray<bool> mask;
        mask.SetNumZeroed(GridSize.X * GridSize.Y);

        // Populate the mask for this layer
        for (int x = 0; x < GridSize.X; ++x) {
            for (int y = 0; y < GridSize.Y; ++y) {
                mask[x + y * GridSize.X] = VoxelArray[x][y][z] == 1;
            }
        }

        // Generate the mesh for this layer
        GenerateMeshFromMask(mask, z, GlobalVertices, GlobalTriangles);
    }
    UE_LOG(LogTemp, Warning, TEXT("Vertices GenerateGreedyMeshes: %d"), GlobalVertices.Num());
    UE_LOG(LogTemp, Warning, TEXT("Triangles GenerateGreedyMeshes: %d"), GlobalTriangles.Num());

    // Update the mesh
    SetGreedyMesh(GlobalVertices, GlobalTriangles);
    
//    if (!ValidateMeshIntegrity()) {
//            UE_LOG(LogTemp, Error, TEXT("Mesh validation failed after generating."));
//        }
}


void AMyVoxelWorld::SetGreedyMesh(const TArray<FVector>& NewVertices, const TArray<int32>& NewTriangles)
{
    // Update the mesh using the new greedy vertices and triangles
    GreedyProceduralMesh->CreateMeshSection_LinearColor(0, NewVertices, NewTriangles, TArray<FVector>(), TArray<FVector2D>(), TArray<FLinearColor>(), TArray<FProcMeshTangent>(), true);

    // Scale the mesh by a factor of 100 in all dimensions
    FVector NewScale(100.0f, 100.0f, 100.0f);
    GreedyProceduralMesh->SetWorldScale3D(NewScale);
}



bool AMyVoxelWorld::CanMerge(const TArray<bool>& mask, int x1, int y1, int x2, int y2, int width) {
    return mask[x1 + y1 * width] && mask[x2 + y2 * width];
}



void AMyVoxelWorld::GenerateMeshFromMask(const TArray<bool>& mask, int z, TArray<FVector>& Vertices, TArray<int32>& Triangles)
{
    TArray<bool> maskCopy = mask;
    //log the number of vertices and triangles
    // Capture initial sizes of Vertices and Triangles
        int initialVertexCount = Vertices.Num();
        int initialTriangleCount = Triangles.Num();
    

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
//    if (LayerToVertexCount.Contains(z)) {
//        LayerToVertexCount[z] = Vertices.Num() - initialVertexCount;
//    }
//    if (LayerToTriangleCount.Contains(z)) {
//        LayerToTriangleCount[z] = Triangles.Num() - initialTriangleCount;
//    }

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




void AMyVoxelWorld::DestroyVoxel(FVector WorldPosition) {
    // Convert world position to voxel coordinates
    int voxelX = FMath::FloorToInt(WorldPosition.X / VoxelSize);
    int voxelY = FMath::FloorToInt(WorldPosition.Y / VoxelSize);
    int voxelZ = FMath::FloorToInt(WorldPosition.Z / VoxelSize);
    //log the destroyed voxel
    UE_LOG(LogTemp, Warning, TEXT("Destroyed Voxel at %s"), *WorldPosition.ToString());
    // Validate the coordinates
    if (voxelX < 0 || voxelX >= GridSize.X || voxelY < 0 || voxelY >= GridSize.Y || voxelZ < 0 || voxelZ >= GridSize.Z) {
        return;
    }
    //log the destroyed voxel
    UE_LOG(LogTemp, Warning, TEXT("Destroyed Voxel at %s"), *WorldPosition.ToString());

    // Update the VoxelArray to mark this voxel as destroyed
    VoxelArray[voxelX][voxelY][voxelZ] = 0;
    
    //log the number of vertices and triangles
    UE_LOG(LogTemp, Warning, TEXT("Vertices: %d"), GlobalVertices.Num());
    UE_LOG(LogTemp, Warning, TEXT("Triangles: %d"), GlobalTriangles.Num());
    
   
//    GlobalVertices.Empty();
//    GlobalTriangles.Empty();

    // Recalculate visibility flags for surrounding voxels
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dz = -1; dz <= 1; ++dz) {
                int nx = voxelX + dx, ny = voxelY + dy, nz = voxelZ + dz;
                if (nx >= 0 && nx < GridSize.X && ny >= 0 && ny < GridSize.Y && nz >= 0 && nz < GridSize.Z) {
                    visibilityFlags3D[nx][ny][nz] = CalculateVisibilityFlags(nx, ny, nz);
                }
            }
        }
    }

    // Generate the greedy meshes for the affected layers only
    // Assuming z is the height coordinate
//    for (int affectedZ = FMath::Max(0, voxelZ - 1); affectedZ <= FMath::Min(GridSize.Z - 1, voxelZ + 1); ++affectedZ) {
//        // Make sure the key exists before removing mesh for layer
//        if (LayerToVertexCount.Contains(affectedZ) && LayerToTriangleCount.Contains(affectedZ)) {
//            RemoveMeshForLayer(affectedZ);
//        }
//        // Remove old vertices and triangles for the affected layer before regenerating
//        RemoveMeshForLayer(affectedZ);
//        // Regenerate mesh for the affected layer
//        TArray<bool> mask;
//        mask.SetNumZeroed(GridSize.X * GridSize.Y);
//        for (int x = 0; x < GridSize.X; ++x) {
//            for (int y = 0; y < GridSize.Y; ++y) {
//                mask[x + y * GridSize.X] = VoxelArray[x][y][affectedZ] == 1;
//            }
//        }
//        GenerateMeshFromMask(mask, affectedZ, GlobalVertices, GlobalTriangles);
//    }
    // Update the mesh
    SetGreedyMesh(GlobalVertices, GlobalTriangles);
    
}


void AMyVoxelWorld::UpdateVoxelGrid() {
    // Logic to update your voxel grid and regenerate mesh
    // You can call methods like GenerateNewVoxels() and GenerateGreedyMeshes() here.
    GenerateGreedyMeshes();
    SetGreedyMesh(GlobalVertices, GlobalTriangles);
}

void AMyVoxelWorld::RemoveMeshForLayer(int layerZ) {
//    if (LayerToVertexCount.Contains(layerZ) && LayerToTriangleCount.Contains(layerZ)) {
//        int vertexCount = LayerToVertexCount[layerZ];
//        int triangleCount = LayerToTriangleCount[layerZ];
//        // Remove vertices for this layer
//        if (vertexCount > 0 && GlobalVertices.Num() >= vertexCount) {
//            GlobalVertices.RemoveAt(GlobalVertices.Num() - vertexCount, vertexCount, true);
//        }
//        // Remove triangles for this layer
//        if (triangleCount > 0 && GlobalTriangles.Num() >= triangleCount) {
//            GlobalTriangles.RemoveAt(GlobalTriangles.Num() - triangleCount, triangleCount, true);
//        }
//        // Optional: Remove the layer from the TMaps if it no longer exists
//        LayerToVertexCount.Remove(layerZ);
//        LayerToTriangleCount.Remove(layerZ);
//    }
}
