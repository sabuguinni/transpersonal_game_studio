#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "../SharedTypes.h"
#include "BiomeManager.generated.h"

/**
 * Central biome system management for the 5 prehistoric biomes
 * Coordinates spawning, environmental conditions, and biome transitions
 * Based on coordinates from brain memories: 157,000 x 153,000 UU map
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // === BIOME COORDINATE SYSTEM ===
    
    /** Get biome type at specific world coordinates */
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EEng_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;
    
    /** Get safe spawn location within specified biome */
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FVector GetRandomSpawnLocationInBiome(EEng_BiomeType BiomeType) const;
    
    /** Check if location is within biome boundaries */
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    bool IsLocationInBiome(const FVector& WorldLocation, EEng_BiomeType BiomeType) const;
    
    /** Get biome center coordinates */
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FVector GetBiomeCenterLocation(EEng_BiomeType BiomeType) const;
    
    /** Get biome environmental conditions */
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FEng_BiomeConditions GetBiomeConditions(EEng_BiomeType BiomeType) const;

    // === SPAWN DISTRIBUTION ===
    
    /** Distribute actors across all biomes using proper coordinates */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome System")
    void DistributeActorsAcrossBiomes();
    
    /** Clean up actors spawned at origin (0,0,0) and redistribute */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome System") 
    void FixOriginSpawnsAndRedistribute();
    
    /** Spawn biome-specific environmental actors */
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void SpawnBiomeEnvironment(EEng_BiomeType BiomeType, int32 ActorCount);

    // === VALIDATION ===
    
    /** Validate all spawned actors are in correct biomes */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome System")
    void ValidateActorBiomePlacement();
    
    /** Generate biome distribution report */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome System")
    void GenerateBiomeReport();

protected:
    // === BIOME DEFINITIONS ===
    
    /** Biome boundary definitions based on brain memory coordinates */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome Configuration")
    TMap<EEng_BiomeType, FEng_BiomeBounds> BiomeBoundaries;
    
    /** Environmental conditions per biome */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome Configuration")
    TMap<EEng_BiomeType, FEng_BiomeConditions> BiomeConditions;
    
    /** Actor types allowed per biome */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome Configuration")
    TMap<EEng_BiomeType, TArray<TSubclassOf<AActor>>> BiomeActorTypes;

    // === RUNTIME STATE ===
    
    /** Current biome actor distribution */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime State")
    TMap<EEng_BiomeType, int32> CurrentActorCounts;
    
    /** Actors that need redistribution */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime State")
    TArray<AActor*> MisplacedActors;
    
    /** Last validation timestamp */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime State")
    float LastValidationTime;

private:
    // === INTERNAL METHODS ===
    
    /** Initialize biome boundaries from brain memory coordinates */
    void InitializeBiomeBoundaries();
    
    /** Initialize biome environmental conditions */
    void InitializeBiomeConditions();
    
    /** Get random location within biome bounds with height sampling */
    FVector GetRandomLocationInBounds(const FEng_BiomeBounds& Bounds) const;
    
    /** Sample terrain height at location */
    float SampleTerrainHeight(const FVector& Location) const;
    
    /** Check if actor type is appropriate for biome */
    bool IsActorTypeValidForBiome(AActor* Actor, EEng_BiomeType BiomeType) const;
};