// VoxelEngine.cpp
#include "VoxelEngine.h"
#include "Math/UnrealMathUtility.h"  // for FMath
AVoxelEngine::AVoxelEngine()
{
    PrimaryActorTick.bCanEverTick = true;

    ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("GeneratedMesh"));
    RootComponent = ProceduralMesh;
}

void AVoxelEngine::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    world.SetNum(VoxelArrayWidth);
    for(auto& plane : world) {
        plane.SetNum(VoxelArrayHeight);
        for(auto& line : plane) {
            line.SetNum(VoxelArrayDepth);
        }
    }
    
    for(int x = 0; x < VoxelArrayWidth; ++x) {
        for(int y = 0; y < VoxelArrayHeight; ++y) {
            for(int z = 0; z < VoxelArrayDepth; ++z) {
                Voxel& voxel = world[x][y][z];
                voxel.type = 1;
                voxel.health = 100;
                voxel.voxelId = x * VoxelArrayHeight * VoxelArrayDepth + y * VoxelArrayDepth + z;
                for (int i = 0; i < 6; ++i)
                    voxel.isVisible[i] = false;
            }
        }
    }

    CalculateVisibility();
    GenerateGreedyMesh();
}

void AVoxelEngine::BeginPlay()
{
    Super::BeginPlay();
    // Add your play logic here
}



// In VoxelEngine.cpp
void AVoxelEngine::CalculateVisibility() {
    for (int x = 0; x < VoxelArrayWidth; ++x) {
        for (int y = 0; y < VoxelArrayHeight; ++y) {
            for (int z = 0; z < VoxelArrayDepth; ++z) {
                Voxel& currentVoxel = world[x][y][z];

//                // Check above
//                currentVoxel.isVisible[0] = (z == VoxelArrayDepth - 1) || world[x][y][z + 1].type == 0;
//
//                // Check below
//                currentVoxel.isVisible[1] = (z == 0) || world[x][y][z - 1].type == 0;
//
//                // Check left
//                currentVoxel.isVisible[2] = (x == 0) || world[x - 1][y][z].type == 0;
//
//                // Check right
//                currentVoxel.isVisible[3] = (x == VoxelArrayWidth - 1) || world[x + 1][y][z].type == 0;
//
//                // Check front
//                currentVoxel.isVisible[4] = (y == VoxelArrayHeight - 1) || world[x][y + 1][z].type == 0;
//
//                // Check back
//                currentVoxel.isVisible[5] = (y == 0) || world[x][y - 1][z].type == 0;
                
                 //Check above
                currentVoxel.isVisible[0] = true;

                // Check below
                currentVoxel.isVisible[1] = true;

                // Check left
                currentVoxel.isVisible[2] = true;

                // Check right
                currentVoxel.isVisible[3] = true;
                // Check front
                currentVoxel.isVisible[4] = true;
                // Check back
                currentVoxel.isVisible[5] = true;
                
                
                
                
                
                
            }
        }
    }
}

void AVoxelEngine::GenerateGreedyMesh() {
    TArray<FVector> vertices;
    TArray<int32> triangles;
    TSet<int32> visited; // Set to keep track of visited voxels

    
    for (int x = 0; x < VoxelArrayWidth; ++x) {
        for (int y = 0; y < VoxelArrayHeight; ++y) {
            for (int z = 0; z < VoxelArrayDepth; ++z) {
                
                int32 currentVoxelId = x * VoxelArrayHeight * VoxelArrayDepth + y * VoxelArrayDepth + z;
                
                // If this voxel is already part of another quad, skip it
                if (visited.Contains(currentVoxelId)) continue;
                
                
                Voxel& voxel = world[x][y][z];
                
                if (voxel.type == 0) continue; // Skip empty voxels
                
                for (int face = 0; face < 6; ++face) {
                    if (!voxel.isVisible[face]) continue; // Skip invisible faces
                    
                    // Start a new quad at this voxel
                    int width, height, depth;
                    width = height = depth = 1; // Initial dimensions of the block
                    
                    
                    
                    
                    // Try extending the width of the quad by check if the next voxel in the row is visible and has the same type
                    while (x + width < VoxelArrayWidth &&
                           world[x + width][y][z].isVisible[face] &&
                           world[x + width][y][z].type == voxel.type) {
                        ++width;
                    }
                    
                    // Try extending the height of the quad by check if the next voxel in the column is visible and has the same type
                    bool canExtendHeight = true;
                    while (canExtendHeight && y + height < VoxelArrayHeight) {
                        for (int dx = 0; dx < width; ++dx) {
                            if (!world[x + dx][y + height][z].isVisible[face] ||
                                world[x + dx][y + height][z].type != voxel.type) {
                                canExtendHeight = false;
                                break;
                            }
                        }
                        if (canExtendHeight) ++height;
                    }
                    
                    // Extend depth
                    bool canExtendDepth = true;
                    while (canExtendDepth && z + depth < VoxelArrayDepth) {
                        for (int dx = 0; dx < width; ++dx) {
                            for (int dy = 0; dy < height; ++dy) {
                                if (!world[x + dx][y + dy][z + depth].isVisible[face] ||
                                    world[x + dx][y + dy][z + depth].type != voxel.type) {
                                    canExtendDepth = false;
                                    break;
                                }
                            }
                            if (!canExtendDepth) break;
                        }
                        if (canExtendDepth) ++depth;
                    }
                    
                    FVector bottomLeft(x, y, z);
                    FVector bottomRight(x, y, z);
                    FVector topLeft(x, y, z);
                    FVector topRight(x, y, z);
                    
                    switch(face) {
                        case 0: // Top face (Z+)
                            // Extend corner vectors as required
                            bottomLeft.Z += height;
                            bottomRight = bottomLeft + FVector(width, 0, 0);
                            topLeft = bottomLeft + FVector(0, depth, 0);
                            topRight = bottomLeft + FVector(width, depth,0);
                            
                            break;
                            
                 
                        case 1: // Bottom face (Z-)
                            // Extend corner vectors as required
                            bottomRight = bottomLeft + FVector(width, 0, 0);
                            topLeft = bottomLeft + FVector(0, depth, 0);
                            topRight = bottomLeft + FVector(width, depth, 0);
                            break;
                            
                        case 2: // Right face (X+)
                            // Extend corner vectors as required
                            bottomRight = bottomLeft + FVector(0, depth, 0);
                            topLeft = bottomLeft + FVector(0, 0, height);
                            topRight = bottomLeft + FVector(0, depth, height);
                            break;
                            
                        case 3: // Left face (X-)
                            bottomLeft.X += width;
                            bottomRight = bottomLeft + FVector(0, depth, 0);
                            topLeft = bottomLeft + FVector(0, 0, height);
                            topRight = bottomLeft + FVector(0, depth, height);
                            break;
                            
                        case 4: // Front face (Y-)
                            // Extend corner vectors as required
                            bottomRight = bottomLeft + FVector(width, 0, 0);
                            topLeft = bottomLeft + FVector(0, 0, height);
                            topRight = bottomLeft + FVector(width, 0, height);
                            break;
                            
                        case 5: // Back face (Y+)
                            // Extend corner vectors as required
                            bottomLeft.Y += depth;
                            bottomRight = bottomLeft + FVector(width, 0, 0);
                            topLeft = bottomLeft + FVector(0, 0, height);
                            topRight = bottomLeft + FVector(width, 0, height);
                            break;
                    }
                    // Add vertices
                    int32 startIdx = vertices.Num(); // Get the current number of vertices
                    vertices.Add(bottomLeft);
                    vertices.Add(bottomRight);
                    vertices.Add(topLeft);
                    vertices.Add(topRight);
                    
                    // Add indices
                    if (face == 1 || face ==2 || face == 5 ) { // Reverse winding for bottom face
                        triangles.Add(startIdx + 1);
                        triangles.Add(startIdx + 2);
                        triangles.Add(startIdx + 0);
                        triangles.Add(startIdx + 1);
                        triangles.Add(startIdx + 3);
                        triangles.Add(startIdx + 2);
                    } else { // Default winding for all other faces
                        triangles.Add(startIdx + 0);
                        triangles.Add(startIdx + 2);
                        triangles.Add(startIdx + 1);
                        triangles.Add(startIdx + 2);
                        triangles.Add(startIdx + 3);
                        triangles.Add(startIdx + 1);
                    }
                    
                    
                    
                    
                    for (int dx = 0; dx < width; ++dx) {
                        for (int dy = 0; dy < height; ++dy) {
                            for (int dz = 0; dz < depth; ++dz) {
                                int32 visitedVoxelId = (x + dx) * VoxelArrayHeight * VoxelArrayDepth + (y + dy) * VoxelArrayDepth + (z + dz);
                                visited.Add(visitedVoxelId);
                            }
                        }
                    }
                }
            }
        }
    }

    // Update your procedural mesh component with the new vertices and triangles
    ProceduralMesh->CreateMeshSection_LinearColor(0, vertices, triangles, TArray<FVector>(), TArray<FVector2D>(), TArray<FLinearColor>(), TArray<FProcMeshTangent>(), true);
    
    // Log vertex and triangle count for debug
    UE_LOG(LogTemp, Warning, TEXT("Vertices Count: %d"), vertices.Num());
    UE_LOG(LogTemp, Warning, TEXT("Triangles Count: %d"), triangles.Num());
    
    // Assign materials
    if(VoxelMaterialMappings.Num() > 0) {
        ProceduralMesh->SetMaterial(0, VoxelMaterialMappings[0].Material);
    }
}



void AVoxelEngine::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // Your tick logic here
}
