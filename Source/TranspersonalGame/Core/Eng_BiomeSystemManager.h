#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "SharedTypes.h"
#include "Eng_BiomeSystemManager.generated.h"

/**
 * Engine Architect - Biome System Manager
 * Manages the 5 biome system architecture for the prehistoric world
 * Handles biome placement, asset distribution, and environmental parameters
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEng_BiomeSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AEng_BiomeSystemManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // === BIOME CONFIGURATION ===
    
    /** Biome definitions with coordinates and parameters */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    TArray<FBiomeDefinition> BiomeDefinitions;

    /** Current active biome based on player location */
    UPROPERTY(BlueprintReadOnly, Category = "Biome System")
    EBiomeType CurrentBiome;

    /** Biome transition radius in Unreal units */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    float BiomeTransitionRadius;

    /** Maximum spawn distance for biome assets */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    float MaxAssetSpawnDistance;

    // === ASSET MANAGEMENT ===

    /** Static mesh assets per biome type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    TMap<EBiomeType, TArray<TSoftObjectPtr<UStaticMesh>>> BiomeAssets;

    /** Spawned actors per biome for cleanup */
    UPROPERTY(BlueprintReadOnly, Category = "Assets")
    TMap<EBiomeType, TArray<AActor*>> SpawnedBiomeActors;

    /** Target actor count per biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    int32 TargetActorsPerBiome;

    // === ENVIRONMENTAL PARAMETERS ===

    /** Temperature ranges per biome (Celsius) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    TMap<EBiomeType, FVector2D> BiomeTemperatureRanges;

    /** Humidity levels per biome (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    TMap<EBiomeType, float> BiomeHumidityLevels;

    /** Danger levels per biome (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    TMap<EBiomeType, float> BiomeDangerLevels;

    // === CORE FUNCTIONS ===

    /** Initialize all biome definitions with default parameters */
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void InitializeBiomeSystem();

    /** Get biome type at world coordinates */
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EBiomeType GetBiomeAtLocation(FVector WorldLocation);

    /** Get distance to nearest biome center */
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetDistanceToBiomeCenter(FVector WorldLocation, EBiomeType BiomeType);

    /** Spawn assets for specific biome */
    UFUNCTION(BlueprintCallable, Category = "Assets")
    void SpawnBiomeAssets(EBiomeType BiomeType, int32 AssetCount = 100);

    /** Clear all spawned assets for biome */
    UFUNCTION(BlueprintCallable, Category = "Assets")
    void ClearBiomeAssets(EBiomeType BiomeType);

    /** Get environmental parameters for biome */
    UFUNCTION(BlueprintCallable, Category = "Environment")
    FBiomeEnvironmentalData GetBiomeEnvironmentalData(EBiomeType BiomeType);

    /** Update current biome based on player location */
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void UpdateCurrentBiome(FVector PlayerLocation);

    // === VALIDATION FUNCTIONS ===

    /** Validate biome system integrity */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Validation")
    bool ValidateBiomeSystem();

    /** Get biome system status report */
    UFUNCTION(BlueprintCallable, Category = "Validation")
    FString GetBiomeSystemStatus();

    /** Count spawned actors per biome */
    UFUNCTION(BlueprintCallable, Category = "Validation")
    TMap<EBiomeType, int32> GetSpawnedActorCounts();

protected:
    /** Setup default biome definitions */
    void SetupDefaultBiomes();

    /** Setup default environmental parameters */
    void SetupEnvironmentalParameters();

    /** Get random spawn location within biome radius */
    FVector GetRandomSpawnLocationInBiome(EBiomeType BiomeType);

    /** Validate spawn location (not overlapping, valid ground) */
    bool IsValidSpawnLocation(FVector Location);

private:
    /** Last player location for biome updates */
    FVector LastPlayerLocation;

    /** Biome update timer */
    float BiomeUpdateTimer;

    /** Biome update interval in seconds */
    float BiomeUpdateInterval;
};