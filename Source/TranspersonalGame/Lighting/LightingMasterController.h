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
enum class ETimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

UENUM(BlueprintType)
enum class EWeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    LightRain   UMETA(DisplayName = "Light Rain"),
    HeavyRain   UMETA(DisplayName = "Heavy Rain"),
    Storm       UMETA(DisplayName = "Storm"),
    Fog         UMETA(DisplayName = "Fog")
};

UENUM(BlueprintType)
enum class EMoodState : uint8
{
    Peaceful    UMETA(DisplayName = "Peaceful"),
    Tense       UMETA(DisplayName = "Tense"),
    Dangerous   UMETA(DisplayName = "Dangerous"),
    Terrifying  UMETA(DisplayName = "Terrifying"),
    Discovery   UMETA(DisplayName = "Discovery"),
    Safe        UMETA(DisplayName = "Safe")
};

USTRUCT(BlueprintType)
struct FLightingPreset
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
    virtual void Tick(float DeltaTime) override;

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
    EWeatherState CurrentWeather = EWeatherState::Clear;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionSpeed = 1.0f;
    
    // Mood System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mood")
    EMoodState CurrentMood = EMoodState::Peaceful;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mood")
    float MoodInfluenceStrength = 0.5f;
    
    // Lighting Presets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TMap<ETimeOfDay, FLightingPreset> TimePresets;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TMap<EWeatherState, FLightingPreset> WeatherPresets;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TMap<EMoodState, FLightingPreset> MoodPresets;

    // Public Functions
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(float Hours);
    
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetWeather(EWeatherState NewWeather);
    
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetMood(EMoodState NewMood);
    
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    ETimeOfDay GetCurrentTimeOfDay() const;
    
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateLighting();

private:
    // Internal state
    FLightingPreset CurrentPreset;
    FLightingPreset TargetPreset;
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
    void ApplyLightingPreset(const FLightingPreset& Preset);
    FLightingPreset BlendPresets(const FLightingPreset& A, const FLightingPreset& B, float Alpha);
    
    // Preset calculation helpers
    FLightingPreset GetTimeBasedPreset() const;
    FLightingPreset GetWeatherModifiedPreset(const FLightingPreset& BasePreset) const;
    FLightingPreset GetMoodModifiedPreset(const FLightingPreset& BasePreset) const;
};