#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
#include "LightingMasterController.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay_88A : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

UENUM(BlueprintType)
enum class ELight_WeatherState_88A : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    LightRain   UMETA(DisplayName = "Light Rain"),
    HeavyRain   UMETA(DisplayName = "Heavy Rain"),
    Storm       UMETA(DisplayName = "Storm"),
    Fog         UMETA(DisplayName = "Fog")
};

UENUM(BlueprintType)
enum class ELight_MoodState : uint8
{
    Peaceful    UMETA(DisplayName = "Peaceful"),
    Tense       UMETA(DisplayName = "Tense"),
    Dangerous   UMETA(DisplayName = "Dangerous"),
    Terrifying  UMETA(DisplayName = "Terrifying"),
    Discovery   UMETA(DisplayName = "Discovery"),
    Safe        UMETA(DisplayName = "Safe")
};

USTRUCT(BlueprintType)
struct FLight_LightingPreset_88A
{
    GENERATED_BODY()

    // Sun Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor SunColor = FLinearColor::White;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SunIntensity = 3.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SunAngle = 45.0f;
    
    // Sky Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor SkyColor = FLinearColor(0.2f, 0.5f, 1.0f);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SkyIntensity = 1.0f;
    
    // Fog Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor FogColor = FLinearColor(0.5f, 0.6f, 0.7f);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FogDensity = 0.02f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FogHeightFalloff = 0.2f;
    
    // Post Process
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Temperature = 6500.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Saturation = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Contrast = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor ColorGrading = FLinearColor::White;
};

/**
 * Master controller for all lighting and atmospheric systems
 * Handles day/night cycle, weather, and mood-based lighting
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALightingMasterController : public AActor
{
    GENERATED_BODY()

public:
    ALightingMasterController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime);

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class UDirectionalLightComponent* SunLight;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class USkyLightComponent* SkyLight;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class USkyAtmosphereComponent* SkyAtmosphere;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class UVolumetricCloudComponent* VolumetricClouds;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class UExponentialHeightFogComponent* HeightFog;

public:
    // Time System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float TimeOfDayHours = 12.0f; // 0-24 hours
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float DayDurationMinutes = 20.0f; // Real minutes for full day cycle
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    bool bAutoAdvanceTime = true;
    
    // Weather System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherState_88A CurrentWeather = ELight_WeatherState_88A::Clear;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionSpeed = 1.0f;
    
    // Mood System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mood")
    ELight_MoodState CurrentMood = ELight_MoodState::Peaceful;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mood")
    float MoodInfluenceStrength = 0.5f;
    
    // Lighting Presets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TMap<ELight_TimeOfDay_88A, FLight_LightingPreset_88A> TimePresets;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TMap<ELight_WeatherState_88A, FLight_LightingPreset_88A> WeatherPresets;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TMap<ELight_MoodState, FLight_LightingPreset_88A> MoodPresets;

    // Public Functions
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(float Hours);
    
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetWeather(ELight_WeatherState_88A NewWeather);
    
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetMood(ELight_MoodState NewMood);
    
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    ELight_TimeOfDay_88A GetCurrentTimeOfDay() const;
    
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateLighting();

private:
    // Internal state
    FLight_LightingPreset_88A CurrentPreset;
    FLight_LightingPreset_88A TargetPreset;
    float TransitionProgress = 0.0f;
    bool bIsTransitioning = false;
    
    // Reference to Post Process Volume
    UPROPERTY()
    class APostProcessVolume* MainPostProcessVolume;
    
    // Helper Functions
    void InitializeComponents();
    void InitializePresets();
    void UpdateTimeOfDay(float DeltaTime);
    void CalculateTargetPreset();
    void InterpolateLighting(float DeltaTime);
    void ApplyLightingPreset(const FLight_LightingPreset_88A& Preset);
    FLight_LightingPreset_88A BlendPresets(const FLight_LightingPreset_88A& A, const FLight_LightingPreset_88A& B, float Alpha);
    
    // Preset calculation helpers
    FLight_LightingPreset_88A GetTimeBasedPreset() const;
    FLight_LightingPreset_88A GetWeatherModifiedPreset(const FLight_LightingPreset_88A& BasePreset) const;
    FLight_LightingPreset_88A GetMoodModifiedPreset(const FLight_LightingPreset_88A& BasePreset) const;
};