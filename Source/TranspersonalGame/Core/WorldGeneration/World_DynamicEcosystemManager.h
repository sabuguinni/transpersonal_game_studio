#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/TriggerBox.h"
#include "SharedTypes.h"
#include "World_DynamicEcosystemManager.generated.h"

UENUM(BlueprintType)
enum class EWorld_WeatherPattern : uint8
{
    Clear = 0,
    LightRain = 1,
    HeavyRain = 2,
    Storm = 3,
    Drought = 4,
    Snow = 5,
    Fog = 6,
    WindStorm = 7
};

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    TropicalRainforest = 0,
    TemperateForest = 1,
    Savanna = 2,
    MountainTundra = 3,
    CoastalWetlands = 4,
    Desert = 5,
    Swampland = 6
};

UENUM(BlueprintType)
enum class EWorld_ResourceType : uint8
{
    Water = 0,
    SeasonalWater = 1,
    SeasonalFood = 2,
    YearRoundFood = 3,
    PermanentShelter = 4,
    SeasonalShelter = 5,
    MineralDeposits = 6
};

USTRUCT(BlueprintType)
struct FWorld_BiomeZoneData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector ZoneCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float ZoneRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_WeatherPattern PreferredWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float ResourceDensity;

    FWorld_BiomeZoneData()
    {
        BiomeType = EWorld_BiomeType::TemperateForest;
        ZoneCenter = FVector::ZeroVector;
        ZoneRadius = 1500.0f;
        PreferredWeather = EWorld_WeatherPattern::Clear;
        Temperature = 20.0f;
        Humidity = 50.0f;
        ResourceDensity = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct FWorld_ResourceNodeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    EWorld_ResourceType ResourceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    FVector NodeLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    float AvailabilityRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    bool bIsSeasonallyAvailable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    float CurrentAvailability;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    float RegenerationRate;

    FWorld_ResourceNodeData()
    {
        ResourceType = EWorld_ResourceType::Water;
        NodeLocation = FVector::ZeroVector;
        AvailabilityRadius = 300.0f;
        bIsSeasonallyAvailable = false;
        CurrentAvailability = 100.0f;
        RegenerationRate = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct FWorld_WeatherSystemData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWorld_WeatherPattern CurrentPattern;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FVector AffectedArea;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float TimeRemaining;

    FWorld_WeatherSystemData()
    {
        CurrentPattern = EWorld_WeatherPattern::Clear;
        Intensity = 1.0f;
        AffectedArea = FVector(5000.0f, 5000.0f, 1000.0f);
        Duration = 300.0f;
        TimeRemaining = 300.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_DynamicEcosystemManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_DynamicEcosystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core ecosystem data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem", meta = (AllowPrivateAccess = "true"))
    TArray<FWorld_BiomeZoneData> BiomeZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem", meta = (AllowPrivateAccess = "true"))
    TArray<FWorld_ResourceNodeData> ResourceNodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (AllowPrivateAccess = "true"))
    TArray<FWorld_WeatherSystemData> WeatherSystems;

    // Ecosystem parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem", meta = (AllowPrivateAccess = "true"))
    float SeasonCycleDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem", meta = (AllowPrivateAccess = "true"))
    float CurrentSeasonTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem", meta = (AllowPrivateAccess = "true"))
    int32 CurrentSeason; // 0=Spring, 1=Summer, 2=Autumn, 3=Winter

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration", meta = (AllowPrivateAccess = "true"))
    bool bEnableAnimalMigration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration", meta = (AllowPrivateAccess = "true"))
    float MigrationTriggerDistance;

public:
    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void InitializeEcosystem();

    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void UpdateSeasonalCycle(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void UpdateWeatherSystems(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Resources")
    void UpdateResourceAvailability(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Migration")
    void TriggerAnimalMigration(EWorld_WeatherPattern WeatherTrigger);

    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Weather")
    EWorld_WeatherPattern GetWeatherAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Resources")
    float GetResourceAvailabilityAtLocation(const FVector& Location, EWorld_ResourceType ResourceType) const;

    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void SpawnBiomeSpecificVegetation(EWorld_BiomeType BiomeType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void CreateWeatherEvent(EWorld_WeatherPattern Pattern, const FVector& Location, float Duration);

    // Editor functions
    UFUNCTION(CallInEditor, Category = "Debug")
    void DebugShowBiomeZones();

    UFUNCTION(CallInEditor, Category = "Debug")
    void DebugShowResourceNodes();

    UFUNCTION(CallInEditor, Category = "Debug")
    void DebugShowWeatherSystems();

protected:
    void InitializeBiomeZones();
    void InitializeResourceNodes();
    void InitializeWeatherSystems();
    void ProcessSeasonalChanges();
    void ProcessWeatherTransitions();
    void ProcessResourceRegeneration();
    void ProcessAnimalBehaviorChanges();

private:
    float LastSeasonUpdate;
    float LastWeatherUpdate;
    float LastResourceUpdate;
    bool bEcosystemInitialized;
};