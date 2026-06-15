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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Segment")
    FVector StartLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Segment")
    FVector EndLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Segment")
    float Width;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Segment")
    float Depth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Segment")
    float FlowSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Segment")
    EBiomeType BiomeType;

    FWorld_RiverSegment()
    {
        StartLocation = FVector::ZeroVector;
        EndLocation = FVector::ZeroVector;
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
    FVector SourceLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Network")
    FVector DeltaLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Network")
    float TotalLength;

    FWorld_RiverNetwork()
    {
        SourceLocation = FVector::ZeroVector;
        DeltaLocation = FVector::ZeroVector;
        TotalLength = 0.0f;
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
    USplineComponent* RiverSpline;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* RiverMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Generation")
    FWorld_RiverNetwork RiverNetwork;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Generation")
    int32 MaxTributaries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Generation")
    float MinSegmentLength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Generation")
    float MaxSegmentLength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Generation")
    float RiverMeandering;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Generation")
    float ErosionStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* WaterMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* RiverbedMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
    UStaticMesh* RiverSegmentMesh;

public:
    UFUNCTION(BlueprintCallable, Category = "River Generation")
    void GenerateRiverSystem(FVector StartPoint, FVector EndPoint, int32 Segments);

    UFUNCTION(BlueprintCallable, Category = "River Generation")
    void AddTributary(FVector BranchPoint, FVector EndPoint, float BranchWidth);

    UFUNCTION(BlueprintCallable, Category = "River Generation")
    void ApplyErosionToTerrain();

    UFUNCTION(BlueprintCallable, Category = "River Generation")
    void SpawnRiverVegetation();

    UFUNCTION(BlueprintCallable, Category = "River Generation")
    void CreateRiverSounds();

    UFUNCTION(BlueprintCallable, Category = "River Generation", CallInEditor = true)
    void RegenerateRiver();

    UFUNCTION(BlueprintCallable, Category = "River Generation")
    FWorld_RiverSegment GetNearestRiverSegment(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "River Generation")
    bool IsLocationNearWater(FVector Location, float MaxDistance) const;

    UFUNCTION(BlueprintCallable, Category = "River Generation")
    float GetWaterDepthAtLocation(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "River Generation")
    FVector GetFlowDirectionAtLocation(FVector Location) const;

private:
    void GenerateSplinePoints();
    void CreateRiverMesh();
    void ApplyMaterials();
    FVector CalculateMeanderingOffset(FVector BasePoint, float MeanderAmount);
    void SpawnRiverRocks();
    void SpawnAquaticLife();
};