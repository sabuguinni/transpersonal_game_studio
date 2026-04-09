// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/TimelineComponent.h"
#include "DynamicLightingSystem.generated.h"

UENUM(BlueprintType)
enum class ETimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night"),
    Midnight    UMETA(DisplayName = "Midnight")
};

UENUM(BlueprintType)
enum class EWeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Rainy       UMETA(DisplayName = "Rainy")
};

UENUM(BlueprintType)
enum class EEmotionalTone : uint8
{
    Peaceful    UMETA(DisplayName = "Peaceful"),
    Mysterious  UMETA(DisplayName = "Mysterious"),
    Threatening UMETA(DisplayName = "Threatening"),
    Magical     UMETA(DisplayName = "Magical"),
    Melancholic UMETA(DisplayName = "Melancholic"),
    Hopeful     UMETA(DisplayName = "Hopeful")
};

USTRUCT(BlueprintType)
struct FLightingState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor SunColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator SunRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor SkyColor = FLinearColor(0.3f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor FogColor = FLinearColor(0.5f, 0.6f, 0.7f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CloudCoverage = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AtmosphereIntensity = 1.0f;
};

USTRUCT(BlueprintType)
struct FEmotionalLightingProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEmotionalTone EmotionalTone = EEmotionalTone::Peaceful;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLightingState LightingState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TransitionDuration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UCurveFloat* TransitionCurve = nullptr;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTimeOfDayChanged, ETimeOfDay, NewTimeOfDay, float, TimeOfDayNormalized);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeatherChanged, EWeatherState, NewWeather, float, TransitionProgress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEmotionalToneChanged, EEmotionalTone, NewTone, float, Intensity);

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADynamicLightingSystem : public AActor
{
    GENERATED_BODY()

public:
    ADynamicLightingSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Lighting Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class ADirectionalLight* SunLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class ASkyLight* SkyLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class AExponentialHeightFog* HeightFog;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmosphere")
    class USkyAtmosphereComponent* SkyAtmosphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmosphere")
    class UVolumetricCloudComponent* VolumetricClouds;

    // Time of Day System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day", meta = (ClampMin = 0.0, ClampMax = 24.0))
    float CurrentTimeOfDay = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day", meta = (ClampMin = 0.1, ClampMax = 100.0))
    float DayDurationMinutes = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    bool bAutoAdvanceTime = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    UCurveFloat* SunPathCurve;

    // Weather System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWeatherState CurrentWeatherState = EWeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionDuration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    TMap<EWeatherState, FLightingState> WeatherProfiles;

    // Emotional Lighting System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Lighting")
    EEmotionalTone CurrentEmotionalTone = EEmotionalTone::Peaceful;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Lighting")
    TMap<EEmotionalTone, FEmotionalLightingProfile> EmotionalProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Lighting")
    float EmotionalIntensity = 1.0f;

    // Lumen Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
    bool bUseLumenGI = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
    float LumenSceneViewDistance = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
    float LumenFinalGatherQuality = 1.0f;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseVirtualShadowMaps = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float ShadowDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 CascadeShadowMapCount = 4;

    // Timeline Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
    UTimelineComponent* WeatherTransitionTimeline;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
    UTimelineComponent* EmotionalTransitionTimeline;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnTimeOfDayChanged OnTimeOfDayChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnWeatherChanged OnWeatherChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnEmotionalToneChanged OnEmotionalToneChanged;

public:
    // Time of Day Functions
    UFUNCTION(BlueprintCallable, Category = "Time of Day")
    void SetTimeOfDay(float NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Time of Day")
    ETimeOfDay GetCurrentTimeOfDayEnum() const;

    UFUNCTION(BlueprintCallable, Category = "Time of Day")
    float GetTimeOfDayNormalized() const;

    UFUNCTION(BlueprintCallable, Category = "Time of Day")
    void SetDayDuration(float NewDurationMinutes);

    // Weather Functions
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherState(EWeatherState NewWeatherState, bool bInstant = false);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void TransitionToWeather(EWeatherState NewWeatherState, float Duration = 5.0f);

    // Emotional Lighting Functions
    UFUNCTION(BlueprintCallable, Category = "Emotional Lighting")
    void SetEmotionalTone(EEmotionalTone NewTone, float Intensity = 1.0f, bool bInstant = false);

    UFUNCTION(BlueprintCallable, Category = "Emotional Lighting")
    void BlendEmotionalTones(EEmotionalTone PrimaryTone, EEmotionalTone SecondaryTone, float BlendWeight);

    // Lighting Control Functions
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateSunPosition();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateAtmosphereSettings();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateFogSettings();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateCloudSettings();

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    FLightingState GetCurrentLightingState() const;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyLightingState(const FLightingState& NewState, float TransitionTime = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SaveLightingPreset(const FString& PresetName);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    bool LoadLightingPreset(const FString& PresetName);

private:
    // Internal Functions
    void InitializeLightingComponents();
    void InitializeWeatherProfiles();
    void InitializeEmotionalProfiles();
    void UpdateTimeOfDay(float DeltaTime);
    void InterpolateLightingStates(const FLightingState& FromState, const FLightingState& ToState, float Alpha);
    FRotator CalculateSunRotation(float TimeOfDay) const;
    FLinearColor CalculateSunColor(float TimeOfDay) const;
    float CalculateSunIntensity(float TimeOfDay) const;

    // Timeline Callbacks
    UFUNCTION()
    void OnWeatherTransitionUpdate(float Value);

    UFUNCTION()
    void OnWeatherTransitionFinished();

    UFUNCTION()
    void OnEmotionalTransitionUpdate(float Value);

    UFUNCTION()
    void OnEmotionalTransitionFinished();

    // Internal State
    FLightingState TargetLightingState;
    FLightingState CurrentLightingState;
    EWeatherState TargetWeatherState;
    EEmotionalTone TargetEmotionalTone;
    float WeatherTransitionProgress = 0.0f;
    float EmotionalTransitionProgress = 0.0f;
    bool bIsTransitioning = false;
};