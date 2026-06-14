#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "GameFramework/Actor.h"
#include "UObject/NoExportTypes.h"
#include "EnvArt_AtmosphericLightingManager.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

UENUM(BlueprintType)
enum class EEnvArt_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Volcanic    UMETA(DisplayName = "Volcanic")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_LightingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Light")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Light")
    FLinearColor SunColor = FLinearColor(1.0f, 0.9f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Light")
    FRotator SunRotation = FRotator(-30.0f, 45.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky Light")
    float SkyIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky Light")
    FLinearColor SkyColor = FLinearColor(0.5f, 0.7f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogInscatteringColor = FLinearColor(0.9f, 0.7f, 0.5f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    bool bVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float VolumetricFogScatteringDistribution = 0.2f;

    FEnvArt_LightingSettings()
    {
        // Default constructor with initialization list above
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_AtmosphericLightingManager : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_AtmosphericLightingManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Current atmospheric state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    EEnvArt_TimeOfDay CurrentTimeOfDay = EEnvArt_TimeOfDay::Morning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    EEnvArt_WeatherState CurrentWeatherState = EEnvArt_WeatherState::Clear;

    // Time progression
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    float DayDurationMinutes = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    float CurrentTimeNormalized = 0.25f; // 0.0 = midnight, 0.5 = noon

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    bool bAutoProgressTime = true;

    // Lighting presets for different times of day
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    FEnvArt_LightingSettings DawnSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    FEnvArt_LightingSettings MorningSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    FEnvArt_LightingSettings MiddaySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    FEnvArt_LightingSettings AfternoonSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    FEnvArt_LightingSettings DuskSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    FEnvArt_LightingSettings NightSettings;

    // Actor references
    UPROPERTY(BlueprintReadOnly, Category = "Light References", meta = (AllowPrivateAccess = "true"))
    class ADirectionalLight* SunLight;

    UPROPERTY(BlueprintReadOnly, Category = "Light References", meta = (AllowPrivateAccess = "true"))
    class ASkyLight* SkyLight;

    UPROPERTY(BlueprintReadOnly, Category = "Light References", meta = (AllowPrivateAccess = "true"))
    class AExponentialHeightFog* HeightFog;

    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void SetWeatherState(EEnvArt_WeatherState NewWeatherState);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void SetTimeNormalized(float NormalizedTime);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void ApplyLightingSettings(const FEnvArt_LightingSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void TransitionToTimeOfDay(EEnvArt_TimeOfDay TargetTimeOfDay, float TransitionDuration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    FEnvArt_LightingSettings GetCurrentLightingSettings() const;

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void FindAndCacheLightingActors();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void CreateDefaultLightingSetup();

private:
    // Internal transition system
    bool bIsTransitioning = false;
    float TransitionProgress = 0.0f;
    float TransitionDuration = 5.0f;
    FEnvArt_LightingSettings TransitionStartSettings;
    FEnvArt_LightingSettings TransitionTargetSettings;

    // Helper functions
    void UpdateTimeOfDayFromNormalizedTime();
    void UpdateLightingForCurrentTime();
    FEnvArt_LightingSettings InterpolateLightingSettings(const FEnvArt_LightingSettings& A, const FEnvArt_LightingSettings& B, float Alpha) const;
    FEnvArt_LightingSettings GetLightingSettingsForTimeOfDay(EEnvArt_TimeOfDay TimeOfDay) const;
    void InitializeDefaultLightingPresets();
};