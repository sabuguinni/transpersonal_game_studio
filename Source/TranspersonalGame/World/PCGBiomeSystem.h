#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "PCGBiomeSystem.generated.h"

// === BIOME TYPES ===
UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    RiverDelta       UMETA(DisplayName = "River Delta"),
    RockyHighlands   UMETA(DisplayName = "Rocky Highlands"),
    DenseForest      UMETA(DisplayName = "Dense Forest"),
    OpenSavanna      UMETA(DisplayName = "Open Savanna"),
    VolcanicPlains   UMETA(DisplayName = "Volcanic Plains"),
    CoastalMarsh     UMETA(DisplayName = "Coastal Marsh"),
    COUNT            UMETA(Hidden)
};

// === BIOME ZONE DATA ===
USTRUCT(BlueprintType)
struct FWorld_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::OpenSavanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float WaterLevel = -50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor FogColor = FLinearColor(0.5f, 0.6f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float AmbientTemperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bHasWater = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bHasVolcanicActivity = false;
};

// === WEATHER STATE ===
UENUM(BlueprintType)
enum class EWorld_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    LightRain   UMETA(DisplayName = "Light Rain"),
    HeavyRain   UMETA(DisplayName = "Heavy Rain"),
    Thunderstorm UMETA(DisplayName = "Thunderstorm"),
    Fog         UMETA(DisplayName = "Fog"),
    COUNT       UMETA(Hidden)
};

/**
 * APCGBiomeSystem
 * Manages procedural biome zones, weather transitions, and environmental audio triggers.
 * Agent #5 — Procedural World Generator
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = "TranspersonalGame|World")
class TRANSPERSONALGAME_API APCGBiomeSystem : public AActor
{
    GENERATED_BODY()

public:
    APCGBiomeSystem();

    // === BIOME ZONES ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Zones")
    TArray<FWorld_BiomeZone> BiomeZones;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome|State")
    EWorld_BiomeType CurrentPlayerBiome = EWorld_BiomeType::OpenSavanna;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome|State")
    int32 CurrentBiomeIndex = -1;

    // === WEATHER ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWorld_WeatherState CurrentWeather = EWorld_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionDuration = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float RainIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bEnableWeatherCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherCycleIntervalMin = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherCycleIntervalMax = 300.0f;

    // === DAY/NIGHT ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight")
    float DayDurationSeconds = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight")
    float CurrentTimeOfDay = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight")
    bool bEnableDayNightCycle = true;

    // === AUDIO TRIGGERS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TMap<EWorld_BiomeType, FName> BiomeAmbientSoundCues;

    // === FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Biome")
    void InitializeBiomeZones();

    UFUNCTION(BlueprintCallable, Category = "Biome")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    FWorld_BiomeZone GetBiomeZoneData(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void UpdatePlayerBiome(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherState(EWorld_WeatherState NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void TriggerRandomWeatherTransition();

    UFUNCTION(BlueprintCallable, Category = "DayNight")
    float GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintCallable, Category = "DayNight")
    bool IsNightTime() const;

    UFUNCTION(BlueprintCallable, Category = "DayNight")
    FLinearColor GetSkyColorForTime(float TimeOfDay) const;

    UFUNCTION(CallInEditor, Category = "Debug")
    void DebugDrawBiomeZones();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    float WeatherTimer = 0.0f;
    float NextWeatherChange = 0.0f;

    void TickDayNightCycle(float DeltaTime);
    void TickWeatherCycle(float DeltaTime);
    void ApplyBiomeAtmosphere(const FWorld_BiomeZone& Zone);
    void SetupDefaultBiomeZones();
};
