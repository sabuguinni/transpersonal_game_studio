#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
#include "CretaceousLightingManager.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    GoldenHour  UMETA(DisplayName = "Golden Hour"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

USTRUCT(BlueprintType)
struct FLight_SunConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitch = -38.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.88f, 0.62f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    bool bAtmosphereSunLight = true;
};

USTRUCT(BlueprintType)
struct FLight_FogConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.03f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogInscatteringColor = FLinearColor(0.55f, 0.72f, 0.85f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    bool bVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float VolumetricFogExtinctionScale = 0.8f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACretaceousLightingManager : public AActor
{
    GENERATED_BODY()

public:
    ACretaceousLightingManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Current time of day
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::GoldenHour;

    // Day/night cycle speed (0 = static)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    float DayNightCycleSpeed = 0.0f;

    // Sun configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    FLight_SunConfig SunConfig;

    // Fog configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    FLight_FogConfig FogConfig;

    // Sky light intensity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sky")
    float SkyLightIntensity = 1.2f;

    // Bloom intensity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|PostProcess")
    float BloomIntensity = 0.4f;

    // Ambient occlusion intensity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|PostProcess")
    float AmbientOcclusionIntensity = 0.6f;

    // Apply current lighting configuration
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting")
    void ApplyLightingConfiguration();

    // Set time of day and update lighting
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(ELight_TimeOfDay NewTimeOfDay);

    // Get current sun pitch angle
    UFUNCTION(BlueprintPure, Category = "Lighting")
    float GetCurrentSunPitch() const;

    // Get current time of day name
    UFUNCTION(BlueprintPure, Category = "Lighting")
    FString GetTimeOfDayName() const;

    // Enable/disable volumetric fog
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetVolumetricFogEnabled(bool bEnabled);

private:
    // Cached references
    UPROPERTY()
    class ADirectionalLight* CachedSunActor;

    UPROPERTY()
    class ASkyLight* CachedSkyLightActor;

    UPROPERTY()
    class AExponentialHeightFog* CachedFogActor;

    UPROPERTY()
    APostProcessVolume* CachedPostProcessVolume;

    float CurrentDayProgress = 0.5f; // 0=midnight, 0.5=noon, 1=midnight

    void FindLightingActors();
    void UpdateSunForTimeOfDay(ELight_TimeOfDay TimeOfDay);
    float GetSunPitchForTimeOfDay(ELight_TimeOfDay TimeOfDay) const;
    FLinearColor GetSunColorForTimeOfDay(ELight_TimeOfDay TimeOfDay) const;
    float GetSunIntensityForTimeOfDay(ELight_TimeOfDay TimeOfDay) const;
};
