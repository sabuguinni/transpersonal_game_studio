#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Engine/World.h"
#include "Light_AtmosphericSystem.generated.h"

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
    Overcast    UMETA(DisplayName = "Overcast"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Volcanic    UMETA(DisplayName = "Volcanic")
};

USTRUCT(BlueprintType)
struct FLight_LightingPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunTemperature = 5400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float SkyLightIntensity = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds")
    float CloudCoverage = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float VolumetricScattering = 1.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_AtmosphericSystem : public AActor
{
    GENERATED_BODY()

public:
    ALight_AtmosphericSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core lighting components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Time and weather system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentTimeOfDay = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float DayDurationMinutes = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherType CurrentWeather = ELight_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionSpeed = 1.0f;

    // Lighting presets for different times and weather
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TMap<ELight_TimeOfDay, FLight_LightingPreset> TimePresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TMap<ELight_WeatherType, FLight_LightingPreset> WeatherPresets;

    // Dynamic lighting controls
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Lighting")
    bool bEnableTimeProgression = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Lighting")
    bool bEnableWeatherChanges = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Lighting")
    float AtmosphericIntensityMultiplier = 1.0f;

    // Prehistoric atmosphere settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prehistoric")
    bool bEnableVolcanicGlow = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prehistoric")
    FLinearColor VolcanicGlowColor = FLinearColor(1.0f, 0.3f, 0.1f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prehistoric")
    float VolcanicGlowIntensity = 2000.0f;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void SetTimeOfDay(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void SetWeatherType(ELight_WeatherType NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void TransitionToPreset(const FLight_LightingPreset& TargetPreset, float TransitionDuration = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    ELight_TimeOfDay GetCurrentTimeOfDayEnum() const;

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void CreateAtmosphericLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void UpdateSunPosition();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void UpdateFogSettings();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void CreateVolcanicAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System", CallInEditor)
    void InitializePresets();

private:
    // Internal lighting references
    UPROPERTY()
    class ADirectionalLight* SunLight;

    UPROPERTY()
    class ASkyLight* SkyLightActor;

    UPROPERTY()
    class AExponentialHeightFog* FogActor;

    UPROPERTY()
    class AVolumetricCloud* CloudActor;

    // Transition system
    FLight_LightingPreset CurrentPreset;
    FLight_LightingPreset TargetPreset;
    float TransitionProgress = 0.0f;
    float TransitionDuration = 2.0f;
    bool bIsTransitioning = false;

    // Internal functions
    void UpdateLightingTransition(float DeltaTime);
    void ApplyLightingPreset(const FLight_LightingPreset& Preset);
    void FindLightingActors();
    FLight_LightingPreset InterpolateLightingPresets(const FLight_LightingPreset& A, const FLight_LightingPreset& B, float Alpha) const;
    void UpdateTimeBasedLighting();
    void UpdateWeatherEffects();
};