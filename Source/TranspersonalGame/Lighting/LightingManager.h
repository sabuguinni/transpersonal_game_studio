#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DirectionalLight.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "LightingManager.generated.h"

UENUM(BlueprintType)
enum class ETimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Noon        UMETA(DisplayName = "Noon"),
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
    Storm       UMETA(DisplayName = "Storm"),
    Fog         UMETA(DisplayName = "Fog")
};

UENUM(BlueprintType)
enum class EThreatLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Cautious    UMETA(DisplayName = "Cautious"),
    Danger      UMETA(DisplayName = "Danger"),
    Terror      UMETA(DisplayName = "Terror")
};

USTRUCT(BlueprintType)
struct FLightingPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor SunColor = FLinearColor::White;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SunIntensity = 10.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator SunRotation = FRotator::ZeroRotator;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor SkyColor = FLinearColor(0.2f, 0.4f, 0.8f, 1.0f);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FogDensity = 0.02f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CloudCoverage = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AtmosphericPerspective = 1.0f;
};

UCLASS()
class TRANSPERSONALGAME_API ALightingManager : public AActor
{
    GENERATED_BODY()

public:
    ALightingManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Lighting Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class ADirectionalLight* SunLight;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class ASkyLight* SkyLight;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmosphere")
    class ASkyAtmosphere* SkyAtmosphere;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmosphere")
    class AVolumetricCloud* VolumetricClouds;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmosphere")
    class AExponentialHeightFog* HeightFog;

    // Time and Weather System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    float TimeOfDaySpeed = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    float CurrentTimeOfDay = 12.0f; // 0-24 hours
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWeatherState CurrentWeather = EWeatherState::Clear;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    EThreatLevel CurrentThreatLevel = EThreatLevel::Safe;

    // Lighting Presets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TMap<ETimeOfDay, FLightingPreset> TimeOfDayPresets;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TMap<EWeatherState, FLightingPreset> WeatherPresets;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TMap<EThreatLevel, FLightingPreset> ThreatPresets;

    // Dynamic Lighting Control
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Lighting")
    float TransitionSpeed = 2.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Lighting")
    bool bEnableDynamicWeather = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Lighting")
    bool bEnableThreatLighting = true;

public:
    // Blueprint Callable Functions
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(float NewTime);
    
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherState(EWeatherState NewWeather);
    
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetThreatLevel(EThreatLevel NewThreatLevel);
    
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void TransitionToPreset(const FLightingPreset& TargetPreset, float Duration = 2.0f);
    
    UFUNCTION(BlueprintPure, Category = "Time System")
    ETimeOfDay GetCurrentTimeOfDayEnum() const;
    
    UFUNCTION(BlueprintPure, Category = "Lighting")
    FLightingPreset GetCurrentLightingState() const;

private:
    void InitializeLightingComponents();
    void UpdateTimeOfDay(float DeltaTime);
    void ApplyLightingPreset(const FLightingPreset& Preset);
    void InterpolateLighting(const FLightingPreset& From, const FLightingPreset& To, float Alpha);
    FLightingPreset BlendPresets(const FLightingPreset& Base, const FLightingPreset& Overlay, float Weight);
    
    // Current state tracking
    FLightingPreset CurrentPreset;
    FLightingPreset TargetPreset;
    bool bIsTransitioning = false;
    float TransitionProgress = 0.0f;
    float TransitionDuration = 2.0f;
};