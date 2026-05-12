#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "Eng_BiomeArchitecture.generated.h"

/**
 * ENGINE ARCHITECT CYCLE 008 - BIOME ARCHITECTURE FOUNDATION
 * 
 * Core architectural system for biome management and environmental systems.
 * Defines the foundational rules and interfaces that all biome-related
 * systems must follow across the entire game.
 * 
 * ARCHITECTURAL PRINCIPLES:
 * - Biomes are data-driven and configurable
 * - Weather systems integrate seamlessly with biomes
 * - Performance is maintained through LOD and streaming
 * - All biome changes are event-driven for system coordination
 * - Biome transitions are smooth and predictable
 */

// Biome Performance Profile
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomePerformanceProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxVegetationActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxDinosaurActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistanceMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseInstancedFoliage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float WeatherUpdateFrequency;

    FEng_BiomePerformanceProfile()
    {
        MaxVegetationActors = 1000;
        MaxDinosaurActors = 50;
        LODDistanceMultiplier = 1.0f;
        bUseInstancedFoliage = true;
        WeatherUpdateFrequency = 1.0f;
    }
};

// Biome Transition Configuration
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeTransitionConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float TransitionDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float BlendRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    bool bSmoothWeatherTransition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float WeatherTransitionTime;

    FEng_BiomeTransitionConfig()
    {
        TransitionDistance = 500.0f;
        BlendRadius = 200.0f;
        bSmoothWeatherTransition = true;
        WeatherTransitionTime = 30.0f;
    }
};

// Extended Biome Configuration
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ExtendedBiomeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FEng_BiomeData BaseData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FEng_BiomePerformanceProfile PerformanceProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    FEng_BiomeTransitionConfig TransitionConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    TArray<EEng_WeatherType> AllowedWeatherTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    TMap<EEng_DinosaurSpecies, float> DinosaurSpawnProbabilities;

    FEng_ExtendedBiomeConfig()
    {
        // Default allowed weather for all biomes
        AllowedWeatherTypes = {EEng_WeatherType::Clear, EEng_WeatherType::Cloudy};
    }
};

/**
 * Biome Architecture Subsystem - Game Instance Level
 * Manages global biome configuration and architectural rules
 */
UCLASS()
class TRANSPERSONALGAME_API UEng_BiomeArchitectureSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core Architecture Interface
    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    bool RegisterBiomeConfiguration(EEng_BiomeType BiomeType, const FEng_ExtendedBiomeConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    FEng_ExtendedBiomeConfig GetBiomeConfiguration(EEng_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    TArray<EEng_BiomeType> GetAllRegisteredBiomes() const;

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetGlobalPerformanceProfile(const FEng_BiomePerformanceProfile& Profile);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FEng_BiomePerformanceProfile GetGlobalPerformanceProfile() const;

    // Validation
    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateBiomeConfiguration(const FEng_ExtendedBiomeConfig& Config) const;

    UFUNCTION(BlueprintCallable, Category = "Validation")
    TArray<FString> GetConfigurationErrors(const FEng_ExtendedBiomeConfig& Config) const;

protected:
    // Biome Configuration Registry
    UPROPERTY()
    TMap<EEng_BiomeType, FEng_ExtendedBiomeConfig> BiomeConfigurations;

    // Global Performance Settings
    UPROPERTY()
    FEng_BiomePerformanceProfile GlobalPerformanceProfile;

    // Architectural Rules
    void InitializeDefaultConfigurations();
    bool ValidatePerformanceProfile(const FEng_BiomePerformanceProfile& Profile) const;
    bool ValidateTransitionConfig(const FEng_BiomeTransitionConfig& Config) const;
};

/**
 * World Biome Manager - World Level
 * Manages active biomes in the current world/level
 */
UCLASS()
class TRANSPERSONALGAME_API UEng_WorldBiomeManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Active Biome Management
    UFUNCTION(BlueprintCallable, Category = "World Biomes")
    void RegisterActiveBiome(FVector Location, EEng_BiomeType BiomeType, float Radius);

    UFUNCTION(BlueprintCallable, Category = "World Biomes")
    EEng_BiomeType GetBiomeAtLocation(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "World Biomes")
    TArray<EEng_BiomeType> GetNearbyBiomes(FVector Location, float SearchRadius) const;

    UFUNCTION(BlueprintCallable, Category = "World Biomes")
    bool IsInBiomeTransitionZone(FVector Location) const;

    // Weather Integration
    UFUNCTION(BlueprintCallable, Category = "Weather")
    EEng_WeatherType GetCurrentWeather(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherForBiome(EEng_BiomeType BiomeType, EEng_WeatherType WeatherType);

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetActiveBiomeCount() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetBiomeSystemPerformanceMetric() const;

protected:
    // Active Biome Data
    USTRUCT()
    struct FActiveBiomeData
    {
        GENERATED_BODY()

        UPROPERTY()
        FVector Location;

        UPROPERTY()
        EEng_BiomeType BiomeType;

        UPROPERTY()
        float Radius;

        UPROPERTY()
        EEng_WeatherType CurrentWeather;

        FActiveBiomeData()
        {
            Location = FVector::ZeroVector;
            BiomeType = EEng_BiomeType::Forest;
            Radius = 1000.0f;
            CurrentWeather = EEng_WeatherType::Clear;
        }
    };

    UPROPERTY()
    TArray<FActiveBiomeData> ActiveBiomes;

    UPROPERTY()
    UEng_BiomeArchitectureSubsystem* ArchitectureSubsystem;

    // Internal Methods
    void InitializeWorldBiomes();
    FActiveBiomeData* FindClosestBiome(FVector Location);
    bool IsLocationInBiome(FVector Location, const FActiveBiomeData& BiomeData) const;
};

/**
 * Biome Event Dispatcher
 * Handles all biome-related events for system coordination
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBiomeChanged, EEng_BiomeType, OldBiome, EEng_BiomeType, NewBiome);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeatherChanged, EEng_WeatherType, OldWeather, EEng_WeatherType, NewWeather);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBiomeTransitionStarted, EEng_BiomeType, TargetBiome);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBiomeTransitionCompleted);

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_BiomeEventDispatcher : public UObject
{
    GENERATED_BODY()

public:
    // Event Delegates
    UPROPERTY(BlueprintAssignable, Category = "Biome Events")
    FOnBiomeChanged OnBiomeChanged;

    UPROPERTY(BlueprintAssignable, Category = "Weather Events")
    FOnWeatherChanged OnWeatherChanged;

    UPROPERTY(BlueprintAssignable, Category = "Transition Events")
    FOnBiomeTransitionStarted OnBiomeTransitionStarted;

    UPROPERTY(BlueprintAssignable, Category = "Transition Events")
    FOnBiomeTransitionCompleted OnBiomeTransitionCompleted;

    // Event Broadcasting
    UFUNCTION(BlueprintCallable, Category = "Events")
    void BroadcastBiomeChanged(EEng_BiomeType OldBiome, EEng_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Events")
    void BroadcastWeatherChanged(EEng_WeatherType OldWeather, EEng_WeatherType NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Events")
    void BroadcastTransitionStarted(EEng_BiomeType TargetBiome);

    UFUNCTION(BlueprintCallable, Category = "Events")
    void BroadcastTransitionCompleted();

    // Static Access
    UFUNCTION(BlueprintCallable, Category = "Events", meta = (CallInEditor = "true"))
    static UEng_BiomeEventDispatcher* GetBiomeEventDispatcher(const UObject* WorldContext);

protected:
    static UEng_BiomeEventDispatcher* Instance;
};