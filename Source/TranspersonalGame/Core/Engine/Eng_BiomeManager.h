#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "../SharedTypes.h"
#include "Eng_BiomeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Grassland;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Elevation = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<TSubclassOf<class AActor>> FoliageTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<TSubclassOf<class APawn>> DinosaurTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 MaxActorsPerBiome = 4000;

    FEng_BiomeData()
    {
        Temperature = 25.0f;
        Humidity = 0.5f;
        Elevation = 0.0f;
        MaxActorsPerBiome = 4000;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeRegion
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
    FVector Center = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
    float Radius = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
    FEng_BiomeData BiomeData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
    TArray<AActor*> SpawnedActors;

    FEng_BiomeRegion()
    {
        Center = FVector::ZeroVector;
        Radius = 10000.0f;
    }
};

/**
 * Engine Architect's Biome Management System
 * Manages biome distribution, environmental parameters, and actor spawning limits
 * Critical for maintaining performance with max 20,000 total actors across 5 biomes
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_BiomeManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UEng_BiomeManager();

    // UWorldSubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Biome Management
    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    EBiomeType GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    FEng_BiomeData GetBiomeData(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    bool CanSpawnActorInBiome(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void RegisterActorToBiome(AActor* Actor, EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void UnregisterActorFromBiome(AActor* Actor, EBiomeType BiomeType);

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetTotalActorCount() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetBiomeActorCount(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CleanupExcessActors();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsWithinActorLimits() const;

    // Environmental Parameters
    UFUNCTION(BlueprintCallable, Category = "Environment")
    float GetTemperatureAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Environment")
    float GetHumidityAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Environment")
    float GetElevationAtLocation(const FVector& Location) const;

    // Biome Transitions
    UFUNCTION(BlueprintCallable, Category = "Transitions")
    float GetBiomeTransitionWeight(const FVector& Location, EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Transitions")
    TArray<EBiomeType> GetNearbyBiomes(const FVector& Location, float Radius) const;

protected:
    // Biome Regions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TMap<EBiomeType, FEng_BiomeRegion> BiomeRegions;

    // Performance Limits
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxTotalActors = 20000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActorsPerBiome = 4000;

    // Biome Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TMap<EBiomeType, FEng_BiomeData> BiomeConfigurations;

    // Internal Methods
    void SetupDefaultBiomes();
    void CreateBiomeRegion(EBiomeType BiomeType, const FVector& Center, float Radius);
    void ValidateActorLimits();
    void RemoveOldestActorsFromBiome(EBiomeType BiomeType, int32 CountToRemove);

private:
    // Actor tracking per biome
    TMap<EBiomeType, TArray<TWeakObjectPtr<AActor>>> BiomeActorRegistry;
    
    // Performance monitoring
    float LastCleanupTime = 0.0f;
    const float CleanupInterval = 30.0f; // seconds
};