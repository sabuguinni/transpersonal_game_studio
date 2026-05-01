#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "Eng_BiomeManager.generated.h"

// Forward declarations
class UBiomeData;
class ABiomeZone;
class UWeatherSystem;

/**
 * Biome configuration data structure
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName = TEXT("Unknown");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float TemperatureMin = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float TemperatureMax = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float HumidityLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<TSubclassOf<class ADinosaurBase>> NativeDinosaurs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor FogColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FogDensity = 0.02f;

    FEng_BiomeConfig()
    {
        BiomeType = EBiomeType::Forest;
        BiomeName = TEXT("Forest");
        TemperatureMin = 15.0f;
        TemperatureMax = 25.0f;
        HumidityLevel = 0.7f;
        VegetationDensity = 0.8f;
        FogColor = FLinearColor(0.5f, 0.7f, 0.3f, 1.0f);
        FogDensity = 0.01f;
    }
};

/**
 * Biome transition data
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeTransition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    EBiomeType FromBiome = EBiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    EBiomeType ToBiome = EBiomeType::Swamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float TransitionDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float BlendFactor = 0.5f;

    FEng_BiomeTransition()
    {
        FromBiome = EBiomeType::Forest;
        ToBiome = EBiomeType::Swamp;
        TransitionDistance = 1000.0f;
        BlendFactor = 0.5f;
    }
};

/**
 * Central biome management system for the prehistoric world
 * Manages 5 biomes: Swamp, Forest, Savanna, Desert, Snow Mountain
 * Handles biome transitions, weather, temperature, and ecosystem rules
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

    // Core biome functionality
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EBiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FEng_BiomeConfig GetBiomeConfig(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    bool IsLocationInBiomeTransition(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FEng_BiomeTransition GetBiomeTransitionData(const FVector& WorldLocation) const;

    // Biome zone management
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void RegisterBiomeZone(ABiomeZone* BiomeZone);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void UnregisterBiomeZone(ABiomeZone* BiomeZone);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    TArray<ABiomeZone*> GetBiomeZonesInRadius(const FVector& Center, float Radius) const;

    // Weather integration
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void UpdateWeatherForBiome(EBiomeType BiomeType, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    bool CanWeatherOccurInBiome(EWeatherType WeatherType, EBiomeType BiomeType) const;

    // Ecosystem rules
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    TArray<TSubclassOf<class ADinosaurBase>> GetNativeDinosaursForBiome(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetDinosaurSpawnChanceInBiome(TSubclassOf<class ADinosaurBase> DinosaurClass, EBiomeType BiomeType) const;

    // Debug and editor tools
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void DebugPrintBiomeInfo(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void ValidateBiomeConfiguration() const;

protected:
    // Biome configuration storage
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TMap<EBiomeType, FEng_BiomeConfig> BiomeConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TArray<FEng_BiomeTransition> BiomeTransitions;

    // Runtime biome zones
    UPROPERTY()
    TArray<ABiomeZone*> RegisteredBiomeZones;

    // Weather system reference
    UPROPERTY()
    UWeatherSystem* WeatherSystem;

    // Biome world boundaries (in UE5 units)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Layout")
    float WorldSizeX = 20000.0f; // 20km

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Layout")
    float WorldSizeY = 20000.0f; // 20km

    // Biome distribution settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Layout")
    float SwampRegionSize = 4000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Layout")
    float ForestRegionSize = 6000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Layout")
    float SavannaRegionSize = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Layout")
    float DesertRegionSize = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Layout")
    float MountainRegionSize = 2000.0f;

private:
    // Internal helper functions
    void InitializeBiomeConfigs();
    void SetupBiomeTransitions();
    EBiomeType CalculateBiomeFromWorldPosition(const FVector& WorldLocation) const;
    float CalculateDistanceToBiomeBorder(const FVector& Location, EBiomeType BiomeType) const;
    FEng_BiomeConfig BlendBiomeConfigs(const FEng_BiomeConfig& BiomeA, const FEng_BiomeConfig& BiomeB, float BlendFactor) const;

    // Performance optimization
    mutable TMap<FVector, EBiomeType> BiomeLocationCache;
    mutable float LastCacheCleanupTime = 0.0f;
    static constexpr float CACHE_CLEANUP_INTERVAL = 30.0f;
    static constexpr int32 MAX_CACHE_ENTRIES = 1000;

    void CleanupLocationCache() const;
};