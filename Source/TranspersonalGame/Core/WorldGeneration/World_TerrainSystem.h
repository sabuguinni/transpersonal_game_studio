#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "SharedTypes.h"
#include "World_TerrainSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float TreeDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float RockDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float ElevationVariation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType;

    FWorld_BiomeDefinition()
    {
        BiomeName = TEXT("Default");
        CenterLocation = FVector::ZeroVector;
        Radius = 1000.0f;
        TreeDensity = 0.5f;
        RockDensity = 0.3f;
        ElevationVariation = 100.0f;
        BiomeType = EWorld_BiomeType::Forest;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_WaterBodyData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    FString WaterBodyName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    EWorld_WaterType WaterType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    float FlowSpeed;

    FWorld_WaterBodyData()
    {
        WaterBodyName = TEXT("Unnamed Water");
        WaterType = EWorld_WaterType::Lake;
        Location = FVector::ZeroVector;
        Scale = FVector(10.0f, 10.0f, 1.0f);
        FlowSpeed = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_CaveSystemData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave")
    FString CaveName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave")
    FVector EntranceLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave")
    EWorld_CaveSize CaveSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave")
    TArray<FVector> ChamberLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave")
    TArray<EWorld_MineralType> MineralDeposits;

    FWorld_CaveSystemData()
    {
        CaveName = TEXT("Unnamed Cave");
        EntranceLocation = FVector::ZeroVector;
        CaveSize = EWorld_CaveSize::Medium;
    }
};

/**
 * Procedural terrain generation and management system
 * Handles biome creation, water bodies, cave systems, and terrain features
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UWorld_TerrainSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_TerrainSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Biome Management
    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void GenerateBiomeSystem();

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void CreateBiomeZone(const FWorld_BiomeDefinition& BiomeData);

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& Location) const;

    // Water System
    UFUNCTION(BlueprintCallable, Category = "Water Generation")
    void GenerateWaterSystem();

    UFUNCTION(BlueprintCallable, Category = "Water Generation")
    void CreateRiverSystem(const TArray<FVector>& RiverPoints);

    UFUNCTION(BlueprintCallable, Category = "Water Generation")
    void CreateWaterBody(const FWorld_WaterBodyData& WaterData);

    // Cave System
    UFUNCTION(BlueprintCallable, Category = "Cave Generation")
    void GenerateCaveSystem();

    UFUNCTION(BlueprintCallable, Category = "Cave Generation")
    void CreateCaveEntrance(const FWorld_CaveSystemData& CaveData);

    UFUNCTION(BlueprintCallable, Category = "Cave Generation")
    void PopulateCaveWithMinerals(const FWorld_CaveSystemData& CaveData);

    // Terrain Features
    UFUNCTION(BlueprintCallable, Category = "Terrain Features")
    void PopulateBiomeWithVegetation(const FWorld_BiomeDefinition& BiomeData);

    UFUNCTION(BlueprintCallable, Category = "Terrain Features")
    void CreateRockFormations(const FVector& Location, int32 Count, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Terrain Features")
    void CreateTerrainLandmarks();

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Terrain Utility")
    float GetTerrainHeightAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Utility")
    bool IsLocationInWater(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Utility")
    FVector GetNearestWaterSource(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Utility")
    TArray<AActor*> GetActorsInBiome(EWorld_BiomeType BiomeType) const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Configuration")
    TArray<FWorld_BiomeDefinition> BiomeDefinitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Configuration")
    TArray<FWorld_WaterBodyData> WaterBodies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Configuration")
    TArray<FWorld_CaveSystemData> CaveSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Settings")
    int32 MaxVegetationPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Settings")
    int32 MaxRocksPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Settings")
    float TerrainGenerationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Settings")
    bool bAutoGenerateOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float BiomeUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActorsPerBiome;

private:
    void InitializeDefaultBiomes();
    void InitializeDefaultWaterBodies();
    void InitializeDefaultCaveSystems();
    
    AActor* SpawnTerrainActor(UClass* ActorClass, const FVector& Location, const FRotator& Rotation, const FVector& Scale);
    void CleanupExcessiveActors();
    
    float LastBiomeUpdateTime;
};

#include "World_TerrainSystem.generated.h"