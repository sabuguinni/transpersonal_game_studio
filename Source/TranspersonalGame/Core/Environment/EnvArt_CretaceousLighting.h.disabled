#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "EnvArt_CretaceousLighting.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Noon        UMETA(DisplayName = "Noon"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

USTRUCT(BlueprintType)
struct FEnvArt_LightingPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunTemperature = 5500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FRotator SunRotation = FRotator(-45.0f, 225.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor FogInscatteringColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float VolumetricScatteringIntensity = 2.0f;

    FEnvArt_LightingPreset()
    {
        SunIntensity = 8.0f;
        SunTemperature = 5500.0f;
        SunColor = FLinearColor::White;
        SunRotation = FRotator(-45.0f, 225.0f, 0.0f);
        FogDensity = 0.02f;
        FogHeightFalloff = 0.2f;
        FogInscatteringColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
        VolumetricScatteringIntensity = 2.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_CretaceousLighting : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_CretaceousLighting();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core lighting components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Current time of day
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    EEnvArt_TimeOfDay CurrentTimeOfDay = EEnvArt_TimeOfDay::Afternoon;

    // Lighting presets for different times of day
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    TMap<EEnvArt_TimeOfDay, FEnvArt_LightingPreset> LightingPresets;

    // Dynamic lighting control
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Lighting", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float TimeOfDayHours = 15.0f; // 3 PM default (golden hour)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Lighting")
    bool bEnableDynamicTimeOfDay = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Lighting", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float TimeOfDaySpeed = 1.0f;

    // Cretaceous-specific atmospheric settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Atmosphere")
    bool bEnableHumidTropicalAtmosphere = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Atmosphere")
    bool bEnableVolcanicHaze = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Atmosphere", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AtmosphericDensity = 0.3f;

    // Weather integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bAdaptToWeatherConditions = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float StormLightingIntensityMultiplier = 0.4f;

    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void SetTimeOfDayByHours(float Hours);

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void ApplyLightingPreset(const FEnvArt_LightingPreset& Preset);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void EnableCretaceousAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void EnableVolcanicAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void AdaptToWeatherCondition(float WeatherIntensity, bool bIsStormy);

    // Editor functions
    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void SetupCretaceousGoldenHour();

    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void SetupTropicalNoon();

    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void SetupMorningMist();

protected:
    // Internal lighting management
    void UpdateDynamicLighting(float DeltaTime);
    void FindAndCacheLightingActors();
    void ApplyLightingToScene(const FEnvArt_LightingPreset& Preset);
    FEnvArt_LightingPreset InterpolateLightingPresets(const FEnvArt_LightingPreset& PresetA, const FEnvArt_LightingPreset& PresetB, float Alpha);

    // Cached lighting actors
    UPROPERTY()
    class ADirectionalLight* CachedSunLight;

    UPROPERTY()
    class ASkyLight* CachedSkyLight;

    UPROPERTY()
    class AExponentialHeightFog* CachedFogActor;

    // Internal state
    bool bLightingActorsCached = false;
    float LastUpdateTime = 0.0f;

private:
    void InitializeLightingPresets();
    EEnvArt_TimeOfDay GetTimeOfDayFromHours(float Hours);
};