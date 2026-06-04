#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Light_DesertAtmosphereSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_DesertLightingConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desert Lighting")
    float SunIntensity = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desert Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.92f, 0.71f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desert Lighting")
    float HeatShimmerDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desert Lighting")
    float AtmosphereClarity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desert Lighting")
    float DustParticleScale = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desert Lighting")
    float ShadowIntensity = 0.9f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_TimeOfDaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float SunAngle = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float SunAzimuth = 180.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    FLinearColor SkyColor = FLinearColor(0.5f, 0.7f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    bool bIsNightTime = false;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_DesertAtmosphereSystem : public AActor
{
    GENERATED_BODY()

public:
    ALight_DesertAtmosphereSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Lighting Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desert Lighting")
    FLight_DesertLightingConfig DesertConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    FLight_TimeOfDaySettings TimeOfDaySettings;

    // Day/Night Cycle
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float DayDuration = 1200.0f; // 20 minutes real time

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float CurrentTimeOfDay = 0.5f; // 0 = midnight, 0.5 = noon, 1 = midnight

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    bool bEnableDayNightCycle = true;

    // Heat Shimmer Effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desert Effects")
    float HeatShimmerIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desert Effects")
    float TemperatureInfluence = 1.0f;

    // Shadow Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows")
    bool bEnableVolumetricShadows = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows")
    float ShadowDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows")
    float ShadowCascadeDistribution = 0.8f;

    // Public Functions
    UFUNCTION(BlueprintCallable, Category = "Desert Lighting")
    void SetDesertLightingConfig(const FLight_DesertLightingConfig& NewConfig);

    UFUNCTION(BlueprintCallable, Category = "Desert Lighting")
    void SetTimeOfDay(float NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Desert Lighting")
    void EnableHeatShimmer(bool bEnable, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Desert Lighting")
    void ConfigureDesertAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Desert Lighting")
    void UpdateSunPosition();

    UFUNCTION(BlueprintCallable, Category = "Desert Lighting")
    void UpdateFogSettings();

    UFUNCTION(CallInEditor, Category = "Desert Lighting")
    void ApplyDesertLighting();

    UFUNCTION(CallInEditor, Category = "Desert Lighting")
    void ResetToDefaultLighting();

private:
    // Internal Functions
    void UpdateDayNightCycle(float DeltaTime);
    void UpdateAtmosphericSettings();
    void UpdateDirectionalLight();
    void UpdateHeightFog();
    void CalculateSunAngles(float TimeOfDay, float& OutSunAngle, float& OutSunAzimuth);
    FLinearColor CalculateSunColor(float TimeOfDay);
    float CalculateFogDensity(float TimeOfDay);

    // Cached References
    UPROPERTY()
    class ADirectionalLight* CachedSunLight;

    UPROPERTY()
    class ASkyAtmosphere* CachedSkyAtmosphere;

    UPROPERTY()
    class AExponentialHeightFog* CachedHeightFog;

    // Internal State
    float LastTimeOfDay;
    bool bNeedsAtmosphereUpdate;
    float TemperatureAccumulator;
};