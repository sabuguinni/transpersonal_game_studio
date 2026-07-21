#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "Eng_BiomeArchitecture.generated.h"

/**
 * Engine Architect - Biome Architecture System
 * Defines the technical foundation for the 5-biome world structure
 * Coordinates with World Generation Agent (#5) for biome placement
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector WorldPosition = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BiomeRadius = 100000.0f; // 1km radius

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 MinActorCount = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 MaxActorCount = 2000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> RequiredAssetTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bIsPopulated = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FDateTime LastPopulationUpdate;

    FEng_BiomeDefinition()
    {
        RequiredAssetTypes.Add(TEXT("Vegetation"));
        RequiredAssetTypes.Add(TEXT("Rocks"));
        RequiredAssetTypes.Add(TEXT("Terrain"));
        LastPopulationUpdate = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeValidationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TMap<EBiomeType, int32> ActorCounts;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TMap<EBiomeType, bool> BiomeCompletionStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 TotalActorsSpawned = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bAllBiomesPopulated = false;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString ValidationSummary;

    FEng_BiomeValidationReport()
    {
        ActorCounts.Empty();
        BiomeCompletionStatus.Empty();
        TotalActorsSpawned = 0;
        bAllBiomesPopulated = false;
        ValidationSummary = TEXT("Not validated");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_BiomeArchitecture : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UEng_BiomeArchitecture();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /**
     * Initializes the 5-biome world structure with correct coordinates
     * Savanna (0,0), Swamp (-50000,-45000), Forest (-45000,40000), Desert (55000,0), Mountains (40000,50000)
     */
    UFUNCTION(BlueprintCallable, Category = "Biome Architecture", CallInEditor = true)
    void InitializeBiomeStructure();

    /**
     * Validates that all biomes meet the minimum actor count requirement (500+ actors each)
     */
    UFUNCTION(BlueprintCallable, Category = "Biome Architecture", CallInEditor = true)
    FEng_BiomeValidationReport ValidateBiomePopulation();

    /**
     * Gets the biome definition for a specific biome type
     */
    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    FEng_BiomeDefinition GetBiomeDefinition(EBiomeType BiomeType);

    /**
     * Registers that a biome has been populated by another agent
     */
    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    void RegisterBiomePopulation(EBiomeType BiomeType, int32 ActorCount);

    /**
     * Gets the total number of actors in a specific biome area
     */
    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    int32 GetBiomeActorCount(EBiomeType BiomeType);

    /**
     * Checks if a world position falls within a specific biome
     */
    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    EBiomeType GetBiomeAtPosition(const FVector& WorldPosition);

    /**
     * Gets all actors within a biome's area
     */
    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    TArray<AActor*> GetActorsInBiome(EBiomeType BiomeType);

    /**
     * Spawns a validation marker at each biome center for visualization
     */
    UFUNCTION(BlueprintCallable, Category = "Biome Architecture", CallInEditor = true)
    void SpawnBiomeMarkers();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Biome Architecture")
    TMap<EBiomeType, FEng_BiomeDefinition> BiomeDefinitions;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Architecture")
    FEng_BiomeValidationReport LastValidationReport;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Architecture")
    TArray<AActor*> BiomeMarkers;

    /**
     * Creates the standard biome definitions with correct world coordinates
     */
    void CreateStandardBiomeDefinitions();

    /**
     * Counts actors within a circular area around a biome center
     */
    int32 CountActorsInRadius(const FVector& Center, float Radius);

private:
    bool bIsInitialized = false;
    FDateTime LastValidationTime;
};