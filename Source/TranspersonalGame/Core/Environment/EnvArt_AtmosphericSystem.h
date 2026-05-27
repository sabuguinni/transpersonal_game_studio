#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "EnvArt_AtmosphericSystem.generated.h"

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
    FLinearColor LightColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float LightIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FRotator SunRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor;

    FEnvArt_LightingPreset()
    {
        LightColor = FLinearColor::White;
        LightIntensity = 3.0f;
        SunRotation = FRotator(-30.0f, 45.0f, 0.0f);
        FogDensity = 0.02f;
        FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_AtmosphericSystem : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_AtmosphericSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Lighting presets for different times of day
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    TMap<EEnvArt_TimeOfDay, FEnvArt_LightingPreset> LightingPresets;

    // Current time of day
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    EEnvArt_TimeOfDay CurrentTimeOfDay;

    // Auto cycle through day/night
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    bool bAutoCycleDayNight;

    // Day cycle duration in seconds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float DayCycleDuration;

    // References to world lighting actors
    UPROPERTY(BlueprintReadOnly, Category = "References")
    ADirectionalLight* SunLight;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    AExponentialHeightFog* AtmosphericFog;

    // Atmospheric effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    bool bEnableVolumetricFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    bool bEnableGodRays;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    float ParticleIntensity;

public:
    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ApplyLightingPreset(const FEnvArt_LightingPreset& Preset);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void CreateAtmosphericFog();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateSunPosition();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void EnableGoldenHourLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SpawnParticleEffects();

protected:
    // Internal functions
    void InitializeLightingPresets();
    void FindWorldLightingActors();
    void UpdateDayCycle(float DeltaTime);

private:
    float CurrentDayTime;
    bool bSystemInitialized;
};