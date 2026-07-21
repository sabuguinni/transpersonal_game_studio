#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "../SharedTypes.h"
#include "World_PCGBiomeSystem.generated.h"

/**
 * PCG-based biome system that procedurally generates vegetation, terrain features,
 * and environmental assets specific to each biome type.
 * Integrates with UE5's Procedural Content Generation framework.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_PCGBiomeSystem : public AActor
{
    GENERATED_BODY()

public:
    AWorld_PCGBiomeSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // === BIOME CONFIGURATION ===
    
    /** Current biome type for this PCG system */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Configuration")
    EBiomeType BiomeType = EBiomeType::Savanna;
    
    /** Generation radius around this actor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Configuration", meta = (ClampMin = "1000", ClampMax = "50000"))
    float GenerationRadius = 15000.0f;
    
    /** Vegetation density (objects per 1000 square units) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Configuration", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float VegetationDensity = 2.0f;
    
    /** Rock/terrain feature density */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Configuration", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float TerrainFeatureDensity = 1.0f;
    
    // === VEGETATION SETTINGS ===
    
    /** Primary tree types for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TArray<FString> PrimaryTreeTypes;
    
    /** Secondary vegetation (bushes, grass, etc.) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TArray<FString> SecondaryVegetation;
    
    /** Tree scale variation range */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    FVector2D TreeScaleRange = FVector2D(0.8f, 2.5f);
    
    // === TERRAIN FEATURES ===
    
    /** Rock formation types for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Features")
    TArray<FString> RockFormationTypes;
    
    /** Special landmark features */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Features")
    TArray<FString> LandmarkFeatures;
    
    // === WATER FEATURES ===
    
    /** Water body type for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Features")
    EWaterType WaterBodyType = EWaterType::Lake;
    
    /** Water body scale */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Features")
    FVector WaterBodyScale = FVector(50.0f, 50.0f, 1.0f);
    
    // === GENERATION METHODS ===
    
    /** Generate all biome content */
    UFUNCTION(BlueprintCallable, Category = "PCG Generation")
    void GenerateBiomeContent();
    
    /** Generate vegetation only */
    UFUNCTION(BlueprintCallable, Category = "PCG Generation")
    void GenerateVegetation();
    
    /** Generate terrain features only */
    UFUNCTION(BlueprintCallable, Category = "PCG Generation")
    void GenerateTerrainFeatures();
    
    /** Generate water bodies */
    UFUNCTION(BlueprintCallable, Category = "PCG Generation")
    void GenerateWaterBodies();
    
    /** Clear all generated content */
    UFUNCTION(BlueprintCallable, Category = "PCG Generation")
    void ClearGeneratedContent();
    
    /** Get biome-specific vegetation types */
    UFUNCTION(BlueprintCallable, Category = "PCG Generation")
    TArray<FString> GetBiomeVegetationTypes(EBiomeType InBiomeType);
    
    /** Get biome-specific terrain features */
    UFUNCTION(BlueprintCallable, Category = "PCG Generation")
    TArray<FString> GetBiomeTerrainFeatures(EBiomeType InBiomeType);
    
    // === RUNTIME MANAGEMENT ===
    
    /** Check if generation is complete */
    UFUNCTION(BlueprintCallable, Category = "PCG Status")
    bool IsGenerationComplete() const { return bGenerationComplete; }
    
    /** Get number of generated objects */
    UFUNCTION(BlueprintCallable, Category = "PCG Status")
    int32 GetGeneratedObjectCount() const { return GeneratedActors.Num(); }
    
    /** Regenerate biome with new settings */
    UFUNCTION(BlueprintCallable, Category = "PCG Management")
    void RegenerateBiome();

protected:
    // === INTERNAL GENERATION METHODS ===
    
    /** Spawn vegetation cluster at location */
    void SpawnVegetationCluster(const FVector& Location, const FString& VegetationType);
    
    /** Spawn terrain feature at location */
    void SpawnTerrainFeature(const FVector& Location, const FString& FeatureType);
    
    /** Spawn water body at location */
    void SpawnWaterBody(const FVector& Location);
    
    /** Get random location within generation radius */
    FVector GetRandomLocationInRadius();
    
    /** Check if location is valid for spawning */
    bool IsValidSpawnLocation(const FVector& Location);
    
    /** Initialize biome-specific settings */
    void InitializeBiomeSettings();

private:
    // === INTERNAL STATE ===
    
    /** Generated actors for cleanup */
    UPROPERTY()
    TArray<AActor*> GeneratedActors;
    
    /** Generation completion flag */
    UPROPERTY()
    bool bGenerationComplete = false;
    
    /** Random seed for consistent generation */
    UPROPERTY()
    int32 RandomSeed = 12345;
    
    /** Generation timer */
    UPROPERTY()
    float GenerationTimer = 0.0f;
    
    /** Maximum objects per generation pass */
    UPROPERTY()
    int32 MaxObjectsPerPass = 50;
    
    /** Current generation pass */
    UPROPERTY()
    int32 CurrentGenerationPass = 0;
};