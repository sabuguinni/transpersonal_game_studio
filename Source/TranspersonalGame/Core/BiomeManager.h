#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/DataTable.h"
#include "../SharedTypes.h"
#include "BiomeManager.generated.h"

/**
 * Biome data structure for configuration
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEng_BiomeType BiomeType = EEng_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName = TEXT("Unknown");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float TemperatureRange = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float HumidityLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> DinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector2D BiomeBounds = FVector2D(5000.0f, 5000.0f);

    FEng_BiomeData()
    {
        DinosaurSpecies.Add(TEXT("Compsognathus"));
        DinosaurSpecies.Add(TEXT("Parasaurolophus"));
    }
};

/**
 * Biome transition zone data
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeTransition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    EEng_BiomeType FromBiome = EEng_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    EEng_BiomeType ToBiome = EEng_BiomeType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float TransitionWidth = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float BlendFactor = 0.5f;
};

/**
 * World Subsystem that manages biomes across the game world
 * Handles biome generation, transitions, and environmental effects
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_BiomeManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UEng_BiomeManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Biome Management
    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    EEng_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    FEng_BiomeData GetBiomeData(EEng_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void SetBiomeAtLocation(const FVector& WorldLocation, EEng_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    TArray<EEng_BiomeType> GetNearbyBiomes(const FVector& WorldLocation, float Radius) const;

    // Biome Properties
    UFUNCTION(BlueprintCallable, Category = "Biome Properties")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Properties")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Properties")
    float GetVegetationDensityAtLocation(const FVector& WorldLocation) const;

    // Biome Transitions
    UFUNCTION(BlueprintCallable, Category = "Biome Transitions")
    bool IsInTransitionZone(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Transitions")
    FEng_BiomeTransition GetTransitionData(const FVector& WorldLocation) const;

    // World Generation Support
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateBiomeMap(int32 WorldSizeX, int32 WorldSizeY, int32 BiomeResolution);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void ApplyBiomeEffectsToTerrain(const FVector& Location, float Radius);

    // Editor Tools
    UFUNCTION(CallInEditor, Category = "Debug")
    void DebugDrawBiomeMap();

    UFUNCTION(CallInEditor, Category = "Debug")
    void ValidateBiomeConfiguration();

protected:
    // Biome configuration data
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    TMap<EEng_BiomeType, FEng_BiomeData> BiomeDataMap;

    // Transition zones
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    TArray<FEng_BiomeTransition> BiomeTransitions;

    // Runtime biome map (2D grid)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    TArray<TArray<EEng_BiomeType>> BiomeGrid;

    // Grid parameters
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    int32 GridSizeX = 100;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    int32 GridSizeY = 100;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    float CellSize = 1000.0f;

    // World bounds
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    FVector WorldOrigin = FVector::ZeroVector;

private:
    // Internal helper functions
    FIntPoint WorldLocationToGridCoords(const FVector& WorldLocation) const;
    FVector GridCoordsToWorldLocation(const FIntPoint& GridCoords) const;
    bool IsValidGridCoords(const FIntPoint& GridCoords) const;
    
    void InitializeDefaultBiomeData();
    void SetupBiomeTransitions();
    
    // Noise generation for procedural biomes
    float GenerateBiomeNoise(float X, float Y, int32 Octaves = 4) const;
    EEng_BiomeType DetermineBiomeFromNoise(float NoiseValue, float Elevation) const;
};