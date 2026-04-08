// SacredGeometryGenerator.h
// Sistema para gerar estruturas baseadas em geometria sagrada
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "ProceduralMeshComponent.h"
#include "SacredGeometryGenerator.generated.h"

UENUM(BlueprintType)
enum class ESacredGeometryType : uint8
{
    FlowerOfLife     UMETA(DisplayName = "Flower of Life"),
    Merkaba         UMETA(DisplayName = "Merkaba"),
    SriYantra       UMETA(DisplayName = "Sri Yantra"),
    Mandala         UMETA(DisplayName = "Mandala"),
    Torus           UMETA(DisplayName = "Torus"),
    Dodecahedron    UMETA(DisplayName = "Dodecahedron"),
    Icosahedron     UMETA(DisplayName = "Icosahedron"),
    GoldenSpiral    UMETA(DisplayName = "Golden Spiral"),
    VesicaPiscis    UMETA(DisplayName = "Vesica Piscis"),
    TreeOfLife      UMETA(DisplayName = "Tree of Life")
};

USTRUCT(BlueprintType)
struct FGeometryParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Scale = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Subdivisions = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GoldenRatio = 1.618033988749f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor PrimaryColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor SecondaryColor = FLinearColor::Blue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EnergyIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAnimated = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AnimationSpeed = 1.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ASacredGeometryGenerator : public AActor
{
    GENERATED_BODY()

public:
    ASacredGeometryGenerator();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Componentes
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UProceduralMeshComponent* GeometryMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UInstancedStaticMeshComponent* EnergyParticles;

    // Configurações
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geometry")
    ESacredGeometryType GeometryType = ESacredGeometryType::FlowerOfLife;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geometry")
    FGeometryParameters Parameters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* GeometryMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* EnergyMaterial;

public:
    // Funções principais
    UFUNCTION(BlueprintCallable, Category = "Geometry")
    void GenerateGeometry();

    UFUNCTION(BlueprintCallable, Category = "Geometry")
    void SetGeometryType(ESacredGeometryType NewType);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StartEnergyFlow();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopEnergyFlow();

private:
    // Geradores específicos
    void GenerateFlowerOfLife();
    void GenerateMerkaba();
    void GenerateSriYantra();
    void GenerateMandala();
    void GenerateTorus();
    void GenerateDodecahedron();
    void GenerateIcosahedron();
    void GenerateGoldenSpiral();
    void GenerateVesicaPiscis();
    void GenerateTreeOfLife();

    // Funções auxiliares
    void CreateCircle(const FVector& Center, float Radius, int32 Segments, TArray<FVector>& Vertices, TArray<int32>& Triangles);
    void CreateSphere(const FVector& Center, float Radius, int32 Segments, TArray<FVector>& Vertices, TArray<int32>& Triangles);
    void CreateSpiral(const FVector& Start, float Radius, float Height, float Turns, TArray<FVector>& Vertices);
    
    FVector GetGoldenRatioPoint(const FVector& Base, float Angle);
    void AddEnergyParticle(const FVector& Position, float Size = 1.0f);
    void UpdateAnimation(float DeltaTime);
    
    // Variáveis de estado
    float AnimationTime;
    bool bEnergyFlowActive;
    TArray<FVector> CachedVertices;
    TArray<int32> CachedTriangles;
    TArray<FVector2D> CachedUVs;
};