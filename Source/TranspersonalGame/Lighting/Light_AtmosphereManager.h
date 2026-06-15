#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/AtmosphericFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Subsystems/WorldSubsystem.h"
#include "Light_AtmosphereManager.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Noon        UMETA(DisplayName = "Noon"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

UENUM(BlueprintType)
enum class ELight_WeatherType : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Cloudy      UMETA(DisplayName = "Cloudy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Misty       UMETA(DisplayName = "Misty")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_AtmosphereSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.92f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunTemperature = 4200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float SkyLightIntensity = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    FLinearColor SkyLightColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.03f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogInscatteringColor = FLinearColor(0.9f, 0.7f, 0.5f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    float BloomIntensity = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    FVector4 ColorSaturation = FVector4(1.1f, 1.05f, 0.95f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    FVector4 ColorContrast = FVector4(1.05f, 1.02f, 1.0f, 1.0f);

    FLight_AtmosphereSettings()
    {
        // Default constructor with preset values
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_DayNightCycle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float CurrentTimeOfDay = 12.0f; // 0-24 hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float DayDurationMinutes = 20.0f; // Real-time minutes for full day

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    bool bAutoAdvanceTime = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float TimeMultiplier = 1.0f;

    FLight_DayNightCycle()
    {
        // Default constructor
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_AtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    ALight_AtmosphereManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLight_AtmosphereSettings AtmosphereSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    FLight_DayNightCycle DayNightCycle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherType CurrentWeather = ELight_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    class ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    class ASkyLight* SkyLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    class APostProcessVolume* PostProcessVolume;

public:
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetTimeOfDay(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetWeather(ELight_WeatherType NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ApplyAtmosphereSettings(const FLight_AtmosphereSettings& NewSettings);

    UFUNCTION(BlueprintPure, Category = "Atmosphere")
    ELight_TimeOfDay GetCurrentTimeOfDay() const;

    UFUNCTION(BlueprintPure, Category = "Atmosphere")
    float GetSunAngle() const;

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void CreateVolumetricLightShafts();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateSunPosition();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateFogSettings();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdatePostProcessSettings();

private:
    void InitializeLightingReferences();
    void CalculateSunRotation(float TimeOfDay, FRotator& OutRotation);
    void InterpolateLightingForTime(float TimeOfDay);
    void ApplyWeatherEffects();

    // Cached lighting values for smooth transitions
    float LastUpdateTime;
    FLinearColor CachedSunColor;
    float CachedSunIntensity;
    FLinearColor CachedSkyColor;
    float CachedSkyIntensity;
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API ULight_AtmosphereSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Atmosphere System")
    ALight_AtmosphereManager* GetAtmosphereManager();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere System")
    void RegisterAtmosphereManager(ALight_AtmosphereManager* Manager);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere System")
    void SetGlobalTimeOfDay(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere System")
    void SetGlobalWeather(ELight_WeatherType NewWeather);

protected:
    UPROPERTY()
    ALight_AtmosphereManager* AtmosphereManager;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Settings")
    bool bEnableGlobalLighting = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Settings")
    float GlobalLightingUpdateInterval = 0.1f;

private:
    FTimerHandle UpdateTimer;
    void UpdateGlobalLighting();
};