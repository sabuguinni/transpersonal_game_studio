#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "World_TerrainSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BiomeRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor BiomeColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float TemperatureRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float HumidityLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> VegetationTypes;

    FWorld_BiomeConfiguration()
    {
        BiomeName = TEXT("DefaultBiome");
        CenterLocation = FVector::ZeroVector;
        BiomeRadius = 2000.0f;
        BiomeColor = FLinearColor::White;
        TemperatureRange = 20.0f;
        HumidityLevel = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_WaterBodyData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    FString WaterBodyName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    EWaterType WaterType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    float FlowSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    bool bIsNavigable;

    FWorld_WaterBodyData()
    {
        WaterBodyName = TEXT("DefaultWater");
        Location = FVector::ZeroVector;
        Scale = FVector(10.0f, 10.0f, 1.0f);
        WaterType = EWaterType::Lake;
        FlowSpeed = 0.0f;
        bIsNavigable = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_TerrainSystem : public AActor
{
    GENERATED_BODY()

public:
    AWorld_TerrainSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Configuration")
    TArray<FWorld_BiomeConfiguration> BiomeConfigurations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Configuration")
    TArray<FWorld_WaterBodyData> WaterBodies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    int32 MaxRockFormations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    float TerrainSeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    float NoiseScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    float HeightMultiplier;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime Data")
    TArray<AActor*> SpawnedTerrainActors;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime Data")
    TArray<AActor*> SpawnedWaterActors;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime Data")
    TArray<AActor*> SpawnedRockFormations;

public:
    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void GenerateProceduralTerrain();

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void CreateBiomeZones();

    UFUNCTION(BlueprintCallable, Category = "Water Generation")
    void CreateWaterBodies();

    UFUNCTION(BlueprintCallable, Category = "Rock Generation")
    void GenerateRockFormations();

    UFUNCTION(BlueprintCallable, Category = "Terrain Utilities")
    FWorld_BiomeConfiguration GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Utilities")
    float GetTerrainHeightAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Utilities")
    bool IsLocationInWater(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Management")
    void ClearGeneratedTerrain();

    UFUNCTION(BlueprintCallable, Category = "Terrain Management", CallInEditor)
    void RegenerateAllTerrain();

private:
    float GeneratePerlinNoise(float X, float Y, float Scale) const;
    FVector CalculateBiomeInfluence(const FVector& Location) const;
    void SpawnTerrainMeshAtLocation(const FVector& Location, const FString& BiomeName);
    void SpawnWaterMeshAtLocation(const FWorld_WaterBodyData& WaterData);
    void SpawnRockFormationAtLocation(const FVector& Location, float Scale);
};