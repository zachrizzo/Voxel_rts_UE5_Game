// MyVoxel.cpp

#include "MyVoxel.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMesh.h"
#include "StaticMeshResources.h"

// Sets default values
AMyVoxel::AMyVoxel()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    // Create mesh component and attach to root
    ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("GeneratedMesh"));
    RootComponent = ProceduralMesh;
}

// Called when the game starts or when spawned
void AMyVoxel::BeginPlay()
{
    Super::BeginPlay();

    UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(ProceduralMesh->GetMaterial(0), this);  // Changed from MeshComponent to ProceduralMesh


    // Set the material color to blue
    if (DynamicMaterial)
        {
            DynamicMaterial->SetVectorParameterValue(FName("BaseColor"), FLinearColor::Blue);
            ProceduralMesh->SetMaterial(0, DynamicMaterial);  // Changed from MeshComponent to ProceduralMesh
        }
}

// Called every frame
void AMyVoxel::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}
//void AMyVoxel::CreateCube(float CubeSize)
//{
//    // Reset vertices and triangles
//    Vertices.Empty();
//    Triangles.Empty();
//
//    // Define vertices for each face and add them
//    // ... (Add faces based on visibility flags)
//
//    // Generate the mesh
//    ProceduralMesh->CreateMeshSection_LinearColor(0, Vertices, Triangles, TArray<FVector>(), TArray<FVector2D>(), TArray<FLinearColor>(), TArray<FProcMeshTangent>(), true);
//}
void AMyVoxel::SetVisibleFaces( TArray<bool> visibilityFlags)
{
    Vertices.Empty();
        Triangles.Empty();
        TArray<FLinearColor> VertexColors;

        float CubeSize = 100.0f;

        // Define base vertices for the cube
        FVector v0(0, 0, 0);  // 0
        FVector v1(0, CubeSize, 0);  // 1
        FVector v2(CubeSize, CubeSize, 0);  // 2
        FVector v3(CubeSize, 0, 0);  // 3
        FVector v4(0, 0, CubeSize);  // 4
        FVector v5(0, CubeSize, CubeSize);  // 5
        FVector v6(CubeSize, CubeSize, CubeSize);  // 6
        FVector v7(CubeSize, 0, CubeSize);  // 7
    
//looking at the cube from the ftony of the XYZ mover when its in the bottom right 

        if (visibilityFlags[0]) {
            //print to console back true
            UE_LOG(LogTemp, Warning, TEXT("back true"));
            
            AddFace(v6, v5, v1, v2);   // Back
        }
        if (visibilityFlags[1]) {
            AddFace(v4, v7, v3, v0);   //   front
        }
        if (visibilityFlags[2]) {
            AddFace(v5, v4, v0, v1);  // Right
        
        }
        if (visibilityFlags[3]) {
            AddFace(v2, v3, v7, v6);  // left
        }
        if (visibilityFlags[4]) {
            AddFace(v4, v5, v6, v7);  // Top face
        }
        if (visibilityFlags[5]) {
            AddFace(v3, v2, v1, v0);  //botom
        }



        // Update the mesh
        ProceduralMesh->CreateMeshSection_LinearColor(0, Vertices, Triangles, TArray<FVector>(), TArray<FVector2D>(), VertexColors, TArray<FProcMeshTangent>(), true);
}




void AMyVoxel::AddFace(const FVector& v0, const FVector& v1, const FVector& v2, const FVector& v3)
{
    float CubeSize = 100.0f;
//    Vertices.Empty();
//    Triangles.Empty();
    
    int32 BaseVertexIndex = Vertices.Num();
        
        // Define vertices based on TopLeft, TopRight, BottomLeft, and BottomRight
    
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
////
//    // Define vertices
//    Vertices.Add(FVector(0, 0, 0));  // 0
//    Vertices.Add(FVector(0, CubeSize, 0));  // 1
//    Vertices.Add(FVector(CubeSize, CubeSize, 0));  // 2
//    Vertices.Add(FVector(CubeSize, 0, 0));  // 3
////    Vertices.Add(FVector(0, 0, CubeSize));  // 4
////    Vertices.Add(FVector(0, CubeSize, CubeSize));  // 5
////    Vertices.Add(FVector(CubeSize, CubeSize, CubeSize));  // 6
////    Vertices.Add(FVector(CubeSize, 0, CubeSize));  // 7
//
//
////
////    // Define triangles (counter-clockwise vertex order to make the face normal point outwards)
////    // Bottom
//    Triangles.Add(2); Triangles.Add(1); Triangles.Add(0);
//    Triangles.Add(0); Triangles.Add(3); Triangles.Add(2);
//    // Top
//    Triangles.Add(6); Triangles.Add(7); Triangles.Add(4);
//    Triangles.Add(4); Triangles.Add(5); Triangles.Add(6);
//    // Front
//    Triangles.Add(7); Triangles.Add(3); Triangles.Add(0);
//    Triangles.Add(0); Triangles.Add(4); Triangles.Add(7);
//     //Back
//    Triangles.Add(6); Triangles.Add(5); Triangles.Add(1);
//    Triangles.Add(1); Triangles.Add(2); Triangles.Add(6);
//    // Left
//    Triangles.Add(5); Triangles.Add(4); Triangles.Add(0);
//    Triangles.Add(0); Triangles.Add(1); Triangles.Add(5);
//    // Right
//    Triangles.Add(6); Triangles.Add(2); Triangles.Add(3);
//    Triangles.Add(3); Triangles.Add(7); Triangles.Add(6);
//
//    // Define vertices
//
//
//    // Define triangles (counter-clockwise vertex order to make the face normal point outwards)
//    // Bottom
//
//    // Update the mesh
//    ProceduralMesh->CreateMeshSection_LinearColor(0, Vertices, Triangles, TArray<FVector>(), TArray<FVector2D>(), TArray<FLinearColor>(), TArray<FProcMeshTangent>(), true);
}
void AMyVoxel::UpdateProceduralMesh(UStaticMesh* NewMesh)
{
    if (NewMesh)
    {
        // Get the LOD (Level of Detail) you want to use from the new mesh
        FStaticMeshLODResources& LODModel = NewMesh->GetRenderData()->LODResources[0];  // Use accessor method

        // Get vertices and triangles from the new mesh
        TArray<FVector> NewVertices;
        FIndexArrayView NewTriangles = LODModel.IndexBuffer.GetArrayView();

        // Convert FIndexArrayView to TArray<int32> for ProceduralMesh
        TArray<int32> NewTrianglesArray;
        for (int32 i = 0; i < NewTriangles.Num(); ++i)  // Traditional for loop
        {
            NewTrianglesArray.Add(NewTriangles[i]);
        }

        // Create an array of colors (one for each vertex)
        TArray<FLinearColor> VertexColors;

        // Correctly iterate over PositionVertexBuffer
        for (auto i = 0; i < LODModel.VertexBuffers.PositionVertexBuffer.GetNumVertices(); ++i)
                {
                    FVector VertexPosition = FVector(LODModel.VertexBuffers.PositionVertexBuffer.VertexPosition(i));  // Explicit cast
                    NewVertices.Add(VertexPosition);
                    VertexColors.Add(FLinearColor::Red);
                }

        // Update the procedural mesh
        ProceduralMesh->CreateMeshSection_LinearColor(0, NewVertices, NewTrianglesArray, TArray<FVector>(), TArray<FVector2D>(), VertexColors, TArray<FProcMeshTangent>(), true);
    }
}


void AMyVoxel::SetGreedyMesh(const TArray<FVector>& NewVertices, const TArray<int32>& NewTriangles)
{
    // Update the mesh using the new greedy vertices and triangles
    ProceduralMesh->CreateMeshSection_LinearColor(0, NewVertices, NewTriangles, TArray<FVector>(), TArray<FVector2D>(), TArray<FLinearColor>(), TArray<FProcMeshTangent>(), true);
}
