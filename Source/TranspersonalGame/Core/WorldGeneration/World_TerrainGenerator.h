#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Landscape/Landscape.h"
#include "SharedTypes.h"
#include "World_TerrainGenerator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_TerrainFeature
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float Density;

    FWorld_TerrainFeature()
        : Location(FVector::ZeroVector)
        , Scale(FVector::OneVector)
        , Rotation(FRotator::ZeroRotator)
        , BiomeType(EBiomeType::Savanna)
        , Density(1.0f)
    {
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_WaterBody
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    FVector2D Size;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    float Depth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    bool bIsRiver;

    FWorld_WaterBody()
        : Location(FVector::ZeroVector)
        , Size(FVector2D(1000.0f, 1000.0f))
        , Depth(100.0f)
        , bIsRiver(false)
    {
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_TerrainGenerator : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_TerrainGenerator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Terrain generation methods
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateBaseTerrain();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void CreateBiomeRegions();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateWaterBodies();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void AddTerrainFeatures();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void OptimizeTerrainLOD();

    // Utility methods
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    FVector GetBiomeCenter(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    float GetTerrainHeightAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    EBiomeType GetBiomeAtLocation(const FVector& Location) const;

protected:
    // Terrain configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
    float TerrainScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
    int32 TerrainResolution;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
    float MaxTerrainHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
    TArray<FWorld_TerrainFeature> TerrainFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Settings")
    TArray<FWorld_WaterBody> WaterBodies;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxLODLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnablePerformanceOptimization;

private:
    // Internal terrain generation
    void CreateTerrainMesh(const FVector& Location, const FVector& Size, EBiomeType BiomeType);
    void CreateWaterPlane(const FWorld_WaterBody& WaterConfig);
    void ApplyBiomeMaterial(AStaticMeshActor* TerrainActor, EBiomeType BiomeType);
    
    // Performance optimization
    void UpdateTerrainLOD(float DeltaTime);
    float CalculateDistanceToPlayer(const FVector& Location) const;

    // Cached references
    UPROPERTY()
    TArray<AStaticMeshActor*> GeneratedTerrainActors;

    UPROPERTY()
    TArray<AStaticMeshActor*> GeneratedWaterActors;

    float LODUpdateTimer;
    static constexpr float LOD_UPDATE_INTERVAL = 2.0f;
};