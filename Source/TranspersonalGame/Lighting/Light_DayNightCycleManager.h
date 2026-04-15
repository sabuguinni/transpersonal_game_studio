#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "TimerManager.h"
#include "Light_DayNightCycleManager.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Noon        UMETA(DisplayName = "Noon"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night"),
    Midnight    UMETA(DisplayName = "Midnight")
};

USTRUCT(BlueprintType)
struct FLight_TimeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FRotator SunRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SkyColor = FLinearColor::Blue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.02f;

    FLight_TimeSettings()
    {
        SunColor = FLinearColor::White;
        SunIntensity = 3.0f;
        SunRotation = FRotator::ZeroRotator;
        SkyColor = FLinearColor::Blue;
        SkyIntensity = 1.0f;
        FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
        FogDensity = 0.02f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_DayNightCycleManager : public AActor
{
    GENERATED_BODY()

public:
    ALight_DayNightCycleManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Time Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentTimeOfDay = 12.0f; // 0-24 hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time", meta = (ClampMin = "0.1", ClampMax = "100.0"))
    float TimeSpeed = 1.0f; // Multiplier for time progression

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    bool bAutoProgress = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float DayLengthInMinutes = 20.0f; // Real-time minutes for full day cycle

    // Lighting References
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    ASkyLight* SkyLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    AExponentialHeightFog* HeightFog;

    // Time Period Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    FLight_TimeSettings DawnSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    FLight_TimeSettings MorningSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    FLight_TimeSettings NoonSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    FLight_TimeSettings AfternoonSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    FLight_TimeSettings DuskSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    FLight_TimeSettings NightSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    FLight_TimeSettings MidnightSettings;

    // Transition Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    float TransitionSpeed = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    bool bSmoothTransitions = true;

public:
    // Public Functions
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetTimeOfDay(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    float GetTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    ELight_TimeOfDay GetCurrentTimePeriod() const;

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetTimeSpeed(float NewSpeed);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void PauseTimeProgression();

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void ResumeTimeProgression();

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void FindLightingActors();

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void ApplyTimeSettings(const FLight_TimeSettings& Settings);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Day Night Cycle")
    void InitializePresets();

protected:
    // Internal Functions
    void UpdateLighting();
    void InterpolateLighting(const FLight_TimeSettings& From, const FLight_TimeSettings& To, float Alpha);
    FLight_TimeSettings GetCurrentTimeSettings() const;
    FLight_TimeSettings GetTimeSettingsForPeriod(ELight_TimeOfDay Period) const;
    float GetTransitionAlpha(float TimeA, float TimeB, float CurrentTime) const;
    void UpdateSunPosition();
    void UpdateSkyLighting();
    void UpdateFogSettings();
    
private:
    // Internal state
    bool bIsInitialized = false;
    FTimerHandle TimeProgressionTimer;
    FLight_TimeSettings LastAppliedSettings;
};