#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "SharedTypes.h"
#include "World_RiverSystemGenerator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_RiverSegment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    FVector StartPoint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    FVector EndPoint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    float Width;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    float Depth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    float FlowSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    EBiomeType BiomeType;

    FWorld_RiverSegment()
    {
        StartPoint = FVector::ZeroVector;
        EndPoint = FVector::ZeroVector;
        Width = 500.0f;
        Depth = 100.0f;
        FlowSpeed = 1.0f;
        BiomeType = EBiomeType::Temperate;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_RiverNetwork
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Network")
    TArray<FWorld_RiverSegment> MainRiver;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Network")
    TArray<FWorld_RiverSegment> Tributaries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Network")
    TArray<FVector> LakePositions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Network")
    float TotalLength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Network")
    int32 NetworkComplexity;

    FWorld_RiverNetwork()
    {
        TotalLength = 0.0f;
        NetworkComplexity = 1;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_RiverSystemGenerator : public AActor
{
    GENERATED_BODY()

public:
    AWorld_RiverSystemGenerator();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USplineComponent* MainRiverSpline;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* RiverMesh;

    // River Generation Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Generation")
    int32 RiverSeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Generation")
    float RiverLength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Generation")
    float RiverWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Generation")
    float MeanderIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Generation")
    int32 TributaryCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Generation")
    float ElevationInfluence;

    // Visual Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Visuals")
    class UMaterialInterface* WaterMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Visuals")
    class UStaticMesh* RiverMeshAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Visuals")
    float WaterTransparency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Visuals")
    FLinearColor WaterColor;

    // Ecosystem Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Ecosystem")
    TArray<EBiomeType> RiverBiomes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Ecosystem")
    float RiparianZoneWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Ecosystem")
    float WildlifeSpawnRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Ecosystem")
    bool bEnableFishSpawning;

    // Generated Data
    UPROPERTY(BlueprintReadOnly, Category = "Generated Data")
    FWorld_RiverNetwork GeneratedNetwork;

    UPROPERTY(BlueprintReadOnly, Category = "Generated Data")
    TArray<FVector> RiverPoints;

    UPROPERTY(BlueprintReadOnly, Category = "Generated Data")
    TArray<AActor*> SpawnedRiverActors;

public:
    // Blueprint Callable Functions
    UFUNCTION(BlueprintCallable, Category = "River Generation")
    void GenerateRiverSystem();

    UFUNCTION(BlueprintCallable, Category = "River Generation")
    void GenerateMainRiver(const FVector& StartPoint, const FVector& EndPoint);

    UFUNCTION(BlueprintCallable, Category = "River Generation")
    void GenerateTributaries();

    UFUNCTION(BlueprintCallable, Category = "River Generation")
    void CreateRiverMesh();

    UFUNCTION(BlueprintCallable, Category = "River Generation")
    void SpawnRiparianVegetation();

    UFUNCTION(BlueprintCallable, Category = "River Generation")
    void SpawnAquaticWildlife();

    UFUNCTION(BlueprintCallable, Category = "River Generation")
    void ApplyWaterPhysics();

    UFUNCTION(BlueprintCallable, Category = "River Generation")
    void ClearGeneratedRiver();

    // Utility Functions
    UFUNCTION(BlueprintPure, Category = "River Utils")
    float CalculateRiverFlow(const FVector& Point) const;

    UFUNCTION(BlueprintPure, Category = "River Utils")
    FVector GetNearestRiverPoint(const FVector& WorldPosition) const;

    UFUNCTION(BlueprintPure, Category = "River Utils")
    bool IsPointInRiver(const FVector& WorldPosition, float Tolerance = 100.0f) const;

    UFUNCTION(BlueprintPure, Category = "River Utils")
    EBiomeType GetRiverBiomeAtPoint(const FVector& WorldPosition) const;

private:
    void InitializeRiverComponents();
    FVector CalculateMeanderPoint(const FVector& Start, const FVector& End, float MeanderFactor, int32 SegmentIndex);
    void UpdateSplineFromPoints();
    void GenerateRiverMeshFromSpline();
    FWorld_RiverSegment CreateRiverSegment(const FVector& Start, const FVector& End, float Width, EBiomeType Biome);
};

#include "World_RiverSystemGenerator.generated.h"