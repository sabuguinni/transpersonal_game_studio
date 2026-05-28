#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "EnvArt_AtmosphericLighting.generated.h"

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
struct FEnvArt_LightingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FRotator SunRotation = FRotator(-30.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.5f, 0.6f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;
};

/**
 * Environment Artist atmospheric lighting system for dynamic time-of-day and weather effects
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEnvArt_AtmosphericLighting : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnvArt_AtmosphericLighting();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Time of day control
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    EEnvArt_TimeOfDay CurrentTimeOfDay = EEnvArt_TimeOfDay::Morning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float TimeOfDaySpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    bool bAutoAdvanceTime = false;

    // Lighting presets for each time period
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    FEnvArt_LightingSettings DawnSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    FEnvArt_LightingSettings MorningSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    FEnvArt_LightingSettings NoonSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    FEnvArt_LightingSettings AfternoonSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    FEnvArt_LightingSettings DuskSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    FEnvArt_LightingSettings NightSettings;

    // Component references
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    UExponentialHeightFogComponent* HeightFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    USkyAtmosphereComponent* SkyAtmosphere;

    // Transition settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    float TransitionDuration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    bool bSmoothTransitions = true;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void ApplyLightingSettings(const FEnvArt_LightingSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    FEnvArt_LightingSettings GetCurrentLightingSettings() const;

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void SetGoldenHourLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void SetVolumetricFogIntensity(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void InitializeAtmosphericComponents();

private:
    float CurrentTransitionTime = 0.0f;
    bool bIsTransitioning = false;
    FEnvArt_LightingSettings StartSettings;
    FEnvArt_LightingSettings TargetSettings;

    void UpdateTransition(float DeltaTime);
    FEnvArt_LightingSettings LerpLightingSettings(const FEnvArt_LightingSettings& A, const FEnvArt_LightingSettings& B, float Alpha) const;
    void FindAtmosphericComponents();
};

#include "EnvArt_AtmosphericLighting.generated.h"