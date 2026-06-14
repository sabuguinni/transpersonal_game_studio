#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "SharedTypes.h"
#include "Light_AtmosphericManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_AtmosphericPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunAngle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SkyColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float AtmosphereHaze;

    FLight_AtmosphericPreset()
    {
        SunColor = FLinearColor::White;
        SunIntensity = 8.0f;
        SunAngle = 45.0f;
        SkyColor = FLinearColor(0.2f, 0.4f, 0.8f, 1.0f);
        SkyIntensity = 1.0f;
        FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
        FogDensity = 0.02f;
        FogHeight = 1000.0f;
        AtmosphereHaze = 0.8f;
    }
};

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
    Humid       UMETA(DisplayName = "Humid")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_AtmosphericManager : public AActor
{
    GENERATED_BODY()

public:
    ALight_AtmosphericManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core lighting components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Components")
    class ADirectionalLight* SunLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Components")
    class ASkyLight* SkyLightActor;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Components")
    class AExponentialHeightFog* FogActor;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Components")
    USceneComponent* RootSceneComponent;

    // Atmospheric presets for different times
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Presets")
    FLight_AtmosphericPreset DawnPreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Presets")
    FLight_AtmosphericPreset MorningPreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Presets")
    FLight_AtmosphericPreset NoonPreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Presets")
    FLight_AtmosphericPreset AfternoonPreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Presets")
    FLight_AtmosphericPreset DuskPreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Presets")
    FLight_AtmosphericPreset NightPreset;

    // Current state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Current State")
    ELight_TimeOfDay CurrentTimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Current State")
    ELight_WeatherType CurrentWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Current State")
    float TimeOfDayProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Current State")
    float DayDurationMinutes;

    // Interior lighting modifiers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Lighting")
    float InteriorLightReduction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Lighting")
    FLinearColor InteriorAmbientColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Lighting")
    float FirePitLightIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Lighting")
    FLinearColor FirePitLightColor;

public:
    // Main atmospheric control functions
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SetTimeOfDay(ELight_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SetWeatherType(ELight_WeatherType NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SetDayNightCycleEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SetDayDuration(float DurationMinutes);

    // Interior lighting integration
    UFUNCTION(BlueprintCallable, Category = "Interior Lighting")
    void OnPlayerEnterInterior(bool bHasFirePit, float ShelterQuality);

    UFUNCTION(BlueprintCallable, Category = "Interior Lighting")
    void OnPlayerExitInterior();

    UFUNCTION(BlueprintCallable, Category = "Interior Lighting")
    void UpdateInteriorLighting(bool bHasFirePit, float Temperature, float Humidity);

    // Atmospheric effects
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Effects")
    void CreateVolumetricLightBeams();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Effects")
    void UpdateAtmosphericHaze(float HazeIntensity);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Effects")
    void SetLumenGlobalIllumination(bool bEnabled);

    // Cretaceous period specific settings
    UFUNCTION(BlueprintCallable, Category = "Cretaceous Atmosphere")
    void ApplyCretaceousAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Cretaceous Atmosphere")
    void SetPrehistoricLightingConditions();

    // Editor utility functions
    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void TestDayNightCycle();

    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void ResetToDefaultLighting();

    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void PreviewTimeOfDay();

protected:
    // Internal update functions
    void UpdateLightingFromPreset(const FLight_AtmosphericPreset& Preset);
    void InterpolateBetweenPresets(const FLight_AtmosphericPreset& PresetA, const FLight_AtmosphericPreset& PresetB, float Alpha);
    FLight_AtmosphericPreset GetPresetForTimeOfDay(ELight_TimeOfDay TimeOfDay);
    void UpdateDayNightCycle(float DeltaTime);
    void ApplyWeatherModifications();
    void SetupCretaceousPresets();

private:
    bool bDayNightCycleEnabled;
    bool bIsInInterior;
    float CurrentDayTime;
    FLight_AtmosphericPreset CurrentPreset;
    FLight_AtmosphericPreset TargetPreset;
    float TransitionSpeed;
};