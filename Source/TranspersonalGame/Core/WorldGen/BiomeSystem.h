// BiomeSystem.h — Agent #5 Procedural World Generator
// Biome detection, parameters, and audio/visual configuration per biome type
// Cycle: PROD_CYCLE_AUTO_20260701_002

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "BiomeSystem.generated.h"

// ============================================================
// ENUMS — global scope (RULE 1: USTRUCT/UENUM at global scope)
// ============================================================

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    JungleForest       UMETA(DisplayName = "Jungle Forest"),
    VolcanicHighlands  UMETA(DisplayName = "Volcanic Highlands"),
    RiverWetlands      UMETA(DisplayName = "River Wetlands"),
    OpenSavanna        UMETA(DisplayName = "Open Savanna"),
    RockyHighlands     UMETA(DisplayName = "Rocky Highlands"),
    Unknown            UMETA(DisplayName = "Unknown")
};

UENUM(BlueprintType)
enum class EWorld_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Rain        UMETA(DisplayName = "Rain"),
    Thunderstorm UMETA(DisplayName = "Thunderstorm"),
    Volcanic    UMETA(DisplayName = "Volcanic Ash"),
    Fog         UMETA(DisplayName = "Dense Fog")
};

// ============================================================
// STRUCTS — global scope
// ============================================================

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate")
    float Temperature = 25.0f;  // Celsius

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate")
    float Humidity = 0.5f;  // 0.0 - 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Vegetation")
    float VegetationDensity = 0.5f;  // 0.0 - 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Danger")
    float DangerLevel = 0.5f;  // 0.0 - 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Visual")
    FLinearColor FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Visual")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Visual")
    FLinearColor AmbientColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Audio")
    FName AmbientSoundTag = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Audio")
    float AmbientVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Terrain")
    float MinElevation = -50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Terrain")
    float MaxElevation = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Terrain")
    float RainfallMM = 1200.0f;  // Annual rainfall in mm

    FWorld_BiomeParameters() {}
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FWorld_BiomeParameters Parameters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BlendRadius = 500.0f;  // Transition zone width

    FWorld_BiomeZone() {}
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_WeatherParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWorld_WeatherState CurrentWeather = EWorld_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float RainIntensity = 0.0f;  // 0.0 - 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WindSpeed = 5.0f;  // m/s

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FVector WindDirection = FVector(1.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float LightningFrequency = 0.0f;  // strikes per minute

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float CloudCoverage = 0.0f;  // 0.0 - 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float TemperatureModifier = 0.0f;  // Delta from biome base

    FWorld_WeatherParameters() {}
};

// ============================================================
// UBiomeDetectorComponent — detects current biome for an actor
// ============================================================

UCLASS(ClassGroup = "WorldGen", meta = (BlueprintSpawnableComponent), DisplayName = "Biome Detector Component")
class TRANSPERSONALGAME_API UBiomeDetectorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UBiomeDetectorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    EWorld_BiomeType GetCurrentBiome() const { return CurrentBiome; }

    UFUNCTION(BlueprintCallable, Category = "Biome")
    FWorld_BiomeParameters GetCurrentBiomeParameters() const { return CurrentParameters; }

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetBiomeTransitionAlpha() const { return TransitionAlpha; }

    UPROPERTY(BlueprintReadOnly, Category = "Biome", meta = (AllowPrivateAccess = "true"))
    EWorld_BiomeType CurrentBiome = EWorld_BiomeType::Unknown;

    UPROPERTY(BlueprintReadOnly, Category = "Biome", meta = (AllowPrivateAccess = "true"))
    EWorld_BiomeType PreviousBiome = EWorld_BiomeType::Unknown;

    UPROPERTY(BlueprintReadOnly, Category = "Biome", meta = (AllowPrivateAccess = "true"))
    float TransitionAlpha = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float DetectionInterval = 1.0f;  // Seconds between biome checks

private:
    FWorld_BiomeParameters CurrentParameters;
    float TimeSinceLastCheck = 0.0f;
};

// ============================================================
// ABiomeManager — world actor that owns all biome zones
// ============================================================

UCLASS(BlueprintType, Blueprintable, DisplayName = "Biome Manager")
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    FWorld_BiomeParameters GetBiomeParametersAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    bool GetBlendedBiomeParameters(const FVector& WorldLocation, FWorld_BiomeParameters& OutParams) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void RegisterBiomeZone(const FWorld_BiomeZone& Zone);

    UFUNCTION(BlueprintCallable, Category = "Biome")
    int32 GetBiomeZoneCount() const { return BiomeZones.Num(); }

    UFUNCTION(BlueprintCallable, Category = "Biome")
    FWorld_WeatherParameters GetCurrentWeather() const { return CurrentWeather; }

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void SetWeather(EWorld_WeatherState NewWeather, float TransitionDuration = 30.0f);

    UFUNCTION(CallInEditor, Category = "Biome|Debug")
    void InitializeDefaultBiomes();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Zones")
    TArray<FWorld_BiomeZone> BiomeZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Weather")
    FWorld_WeatherParameters CurrentWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Weather")
    float WeatherChangeInterval = 300.0f;  // Seconds between weather changes

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Debug")
    bool bDrawDebugZones = false;

private:
    float WeatherTimer = 0.0f;
    float WeatherTransitionAlpha = 0.0f;
    float WeatherTransitionDuration = 30.0f;
    EWorld_WeatherState TargetWeather = EWorld_WeatherState::Clear;

    FWorld_BiomeParameters BlendBiomeParameters(
        const FWorld_BiomeParameters& A,
        const FWorld_BiomeParameters& B,
        float Alpha) const;

    void TickWeatherTransition(float DeltaTime);
};
