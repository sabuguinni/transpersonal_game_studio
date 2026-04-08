#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
#include "LightingMasterController.generated.h"

UENUM(BlueprintType)
enum class EAtmosphericMood : uint8
{
    SafeExploration     UMETA(DisplayName = "Safe Exploration"),
    TensionBuilding     UMETA(DisplayName = "Tension Building"), 
    DangerImminent      UMETA(DisplayName = "Danger Imminent"),
    PredatorHunting     UMETA(DisplayName = "Predator Hunting"),
    NightTerror         UMETA(DisplayName = "Night Terror"),
    FalseCalm          UMETA(DisplayName = "False Calm")
};

UENUM(BlueprintType)
enum class ETimeOfDay : uint8
{
    Dawn                UMETA(DisplayName = "Dawn"),
    Morning             UMETA(DisplayName = "Morning"),
    Noon                UMETA(DisplayName = "Noon"),
    Afternoon           UMETA(DisplayName = "Afternoon"),
    Dusk                UMETA(DisplayName = "Dusk"),
    Night               UMETA(DisplayName = "Night"),
    DeepNight           UMETA(DisplayName = "Deep Night")
};

USTRUCT(BlueprintType)
struct FAtmosphericState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SkyLightIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor FogInscatteringColor = FLinearColor(0.4f, 0.6f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CloudCoverage = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VolumetricScatteringIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ShadowSharpness = 0.8f;
};

/**
 * Master controller for dynamic lighting and atmosphere system
 * Manages time of day, weather, and emotional lighting states
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALightingMasterController : public AActor
{
    GENERATED_BODY()

public:
    ALightingMasterController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Core")
    class UDirectionalLightComponent* SunLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Core")
    class USkyAtmosphereComponent* SkyAtmosphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Core")
    class USkyLightComponent* SkyLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Core")
    class UVolumetricCloudComponent* VolumetricClouds;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Core")
    class UExponentialHeightFogComponent* HeightFog;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Core")
    class UPostProcessComponent* PostProcessComponent;

public:
    // Time of Day System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day", meta = (ClampMin = 0.0f, ClampMax = 24.0f))
    float CurrentTimeOfDay = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float DayDurationMinutes = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    bool bAutoAdvanceTime = true;

    // Atmospheric Mood System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Mood")
    EAtmosphericMood CurrentMood = EAtmosphericMood::SafeExploration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Mood")
    float MoodTransitionSpeed = 2.0f;

    // Atmospheric States Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric States")
    TMap<EAtmosphericMood, FAtmosphericState> MoodStates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric States")
    TMap<ETimeOfDay, FAtmosphericState> TimeOfDayStates;

    // Weather System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bStormApproaching = false;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void SetAtmosphericMood(EAtmosphericMood NewMood, float TransitionTime = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void SetTimeOfDay(float NewTime, bool bInstant = false);

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void TriggerStormSequence(float Duration = 300.0f);

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void CreateTensionLighting(FVector PlayerLocation, float Intensity = 1.0f);

    UFUNCTION(BlueprintPure, Category = "Lighting Info")
    ETimeOfDay GetCurrentTimeOfDayEnum() const;

    UFUNCTION(BlueprintPure, Category = "Lighting Info")
    float GetSunAngle() const;

    UFUNCTION(BlueprintPure, Category = "Lighting Info")
    bool IsNightTime() const;

private:
    void InitializeAtmosphericStates();
    void UpdateTimeOfDay(float DeltaTime);
    void UpdateAtmosphericMood(float DeltaTime);
    void ApplyAtmosphericState(const FAtmosphericState& State, float BlendWeight);
    void UpdateSunPosition();
    void UpdateSkyAtmosphere();
    void UpdateVolumetricClouds();
    void UpdateHeightFog();
    void UpdatePostProcessEffects();

    // Internal state
    FAtmosphericState CurrentState;
    FAtmosphericState TargetState;
    float MoodBlendAlpha = 0.0f;
    bool bMoodTransitioning = false;
};