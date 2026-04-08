#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "LightingSystemManager.generated.h"

UENUM(BlueprintType)
enum class ETimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"), 
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night"),
    DeepNight   UMETA(DisplayName = "Deep Night")
};

UENUM(BlueprintType)
enum class EWeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    PartlyCloudy UMETA(DisplayName = "Partly Cloudy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    LightRain   UMETA(DisplayName = "Light Rain"),
    HeavyRain   UMETA(DisplayName = "Heavy Rain"),
    Storm       UMETA(DisplayName = "Storm"),
    Fog         UMETA(DisplayName = "Fog")
};

UENUM(BlueprintType)
enum class EEmotionalTone : uint8
{
    Peaceful    UMETA(DisplayName = "Peaceful"),
    Tense       UMETA(DisplayName = "Tense"),
    Dangerous   UMETA(DisplayName = "Dangerous"),
    Terrifying  UMETA(DisplayName = "Terrifying"),
    Mysterious  UMETA(DisplayName = "Mysterious"),
    Hopeful     UMETA(DisplayName = "Hopeful")
};

USTRUCT(BlueprintType)
struct FLightingState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SunAngle = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor SunColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SunIntensity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor SkyColor = FLinearColor(0.5f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CloudDensity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor FogColor = FLinearColor(0.8f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AtmosphericPerspective = 1.0f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ULightingSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    ULightingSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === CORE SYSTEM ===
    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void InitializeLightingSystem();

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void SetTimeOfDay(ETimeOfDay NewTimeOfDay, float TransitionDuration = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void SetWeatherState(EWeatherState NewWeather, float TransitionDuration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void SetEmotionalTone(EEmotionalTone NewTone, float TransitionDuration = 3.0f);

    // === TIME PROGRESSION ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    bool bAutoProgressTime = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float TimeProgressionSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    float DayLengthInMinutes = 20.0f;

    // === EMOTIONAL LIGHTING ===
    UFUNCTION(BlueprintCallable, Category = "Emotional Lighting")
    void TriggerTensionLighting(float Intensity = 1.0f, float Duration = 10.0f);

    UFUNCTION(BlueprintCallable, Category = "Emotional Lighting")
    void TriggerDangerLighting(float Intensity = 1.0f, float Duration = 15.0f);

    UFUNCTION(BlueprintCallable, Category = "Emotional Lighting")
    void RestoreNormalLighting(float TransitionDuration = 5.0f);

    // === WEATHER EFFECTS ===
    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void StartRainEffect(float Intensity = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void StopRainEffect(float TransitionDuration = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void StartStormEffect();

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void StopStormEffect();

    // === GETTERS ===
    UFUNCTION(BlueprintPure, Category = "Lighting System")
    ETimeOfDay GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintPure, Category = "Lighting System")
    EWeatherState GetCurrentWeather() const { return CurrentWeather; }

    UFUNCTION(BlueprintPure, Category = "Lighting System")
    EEmotionalTone GetCurrentEmotionalTone() const { return CurrentEmotionalTone; }

    UFUNCTION(BlueprintPure, Category = "Lighting System")
    float GetCurrentTimeOfDayFloat() const { return CurrentTimeFloat; }

protected:
    // === LIGHTING COMPONENTS ===
    UPROPERTY()
    class ADirectionalLight* SunLight;

    UPROPERTY()
    class ASkyLight* SkyLightActor;

    UPROPERTY()
    class AExponentialHeightFog* FogActor;

    UPROPERTY()
    class AVolumetricCloud* CloudActor;

    UPROPERTY()
    class ASkyAtmosphere* AtmosphereActor;

    // === STATE MANAGEMENT ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    ETimeOfDay CurrentTimeOfDay = ETimeOfDay::Morning;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    EWeatherState CurrentWeather = EWeatherState::Clear;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    EEmotionalTone CurrentEmotionalTone = EEmotionalTone::Peaceful;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    float CurrentTimeFloat = 0.5f; // 0.0 = midnight, 0.5 = noon, 1.0 = midnight

    // === LIGHTING PRESETS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    TMap<ETimeOfDay, FLightingState> TimeOfDayPresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    TMap<EWeatherState, FLightingState> WeatherPresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    TMap<EEmotionalTone, FLightingState> EmotionalPresets;

    // === TRANSITION SYSTEM ===
    UPROPERTY()
    bool bIsTransitioning = false;

    UPROPERTY()
    float TransitionProgress = 0.0f;

    UPROPERTY()
    float TransitionDuration = 2.0f;

    UPROPERTY()
    FLightingState TransitionStartState;

    UPROPERTY()
    FLightingState TransitionTargetState;

    // === INTERNAL FUNCTIONS ===
    void UpdateLightingTransition(float DeltaTime);
    void ApplyLightingState(const FLightingState& State);
    void FindLightingActors();
    void SetupDefaultPresets();
    FLightingState BlendLightingStates(const FLightingState& StateA, const FLightingState& StateB, float Alpha);
    void UpdateTimeProgression(float DeltaTime);
    FRotator CalculateSunRotation(float TimeOfDay);
};