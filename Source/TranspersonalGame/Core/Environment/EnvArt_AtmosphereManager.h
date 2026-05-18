#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "EnvArt_AtmosphereManager.generated.h"

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
struct TRANSPERSONALGAME_API FEnvArt_LightingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.86f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunTemperature = 3200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FRotator SunRotation = FRotator(-15.0f, 45.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float VolumetricScatteringIntensity = 1.5f;

    FEnvArt_LightingSettings()
    {
        SunColor = FLinearColor(1.0f, 0.86f, 0.7f, 1.0f);
        SunIntensity = 3.0f;
        SunTemperature = 3200.0f;
        SunRotation = FRotator(-15.0f, 45.0f, 0.0f);
        VolumetricScatteringIntensity = 1.5f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_FogSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogInscatteringColor = FLinearColor(0.9f, 0.9f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogMaxOpacity = 1.0f;

    FEnvArt_FogSettings()
    {
        FogDensity = 0.02f;
        FogHeightFalloff = 0.2f;
        FogInscatteringColor = FLinearColor(0.9f, 0.9f, 1.0f, 1.0f);
        FogMaxOpacity = 1.0f;
    }
};

/**
 * Environment Artist Atmosphere Manager
 * Manages lighting, fog, and atmospheric effects for Cretaceous period ambiance
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_AtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_AtmosphereManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Current time of day
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    EEnvArt_TimeOfDay CurrentTimeOfDay = EEnvArt_TimeOfDay::Afternoon;

    // Enable automatic day/night cycle
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    bool bEnableDayNightCycle = false;

    // Day cycle duration in seconds (default 20 minutes)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere", meta = (ClampMin = "60.0", ClampMax = "7200.0"))
    float DayCycleDuration = 1200.0f;

    // Lighting settings for different times of day
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    TMap<EEnvArt_TimeOfDay, FEnvArt_LightingSettings> LightingPresets;

    // Fog settings for different times of day
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    TMap<EEnvArt_TimeOfDay, FEnvArt_FogSettings> FogPresets;

    // References to world lighting actors
    UPROPERTY(BlueprintReadOnly, Category = "References")
    ADirectionalLight* SunLight;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    AExponentialHeightFog* AtmosphericFog;

    // Cretaceous atmosphere settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Settings")
    bool bEnableCretaceousAtmosphere = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Settings")
    float CretaceousHumidity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Settings")
    float CretaceousTemperature = 25.0f;

    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ApplyGoldenHourLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ApplyCretaceousAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateFog();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void FindWorldLightingActors();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void CreateDefaultLighting();

private:
    // Internal time tracking for day/night cycle
    float CurrentDayTime = 0.0f;

    // Initialize default presets
    void InitializePresets();

    // Interpolate between lighting settings
    FEnvArt_LightingSettings InterpolateLightingSettings(const FEnvArt_LightingSettings& A, const FEnvArt_LightingSettings& B, float Alpha);

    // Interpolate between fog settings
    FEnvArt_FogSettings InterpolateFogSettings(const FEnvArt_FogSettings& A, const FEnvArt_FogSettings& B, float Alpha);
};