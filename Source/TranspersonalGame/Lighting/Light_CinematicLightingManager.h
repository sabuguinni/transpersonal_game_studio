#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/PostProcessComponent.h"
#include "Engine/PostProcessVolume.h"
#include "TranspersonalGame.h"
#include "Light_CinematicLightingManager.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Noon        UMETA(DisplayName = "Noon"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

UENUM(BlueprintType)
enum class ELight_WeatherType : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Dusty       UMETA(DisplayName = "Dusty")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_LightingPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FRotator SunRotation = FRotator(-45.0f, 30.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogDensity = 0.008f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor FogColor = FLinearColor(0.9f, 0.85f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float VolumetricScattering = 1.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    float WhiteTemperature = 6200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    float BloomIntensity = 0.8f;

    FLight_LightingPreset()
    {
        // Default constructor with preset values
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_CinematicLightingManager : public AActor
{
    GENERATED_BODY()

public:
    ALight_CinematicLightingManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core lighting components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting", meta = (AllowPrivateAccess = "true"))
    class UDirectionalLightComponent* MainSunLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting", meta = (AllowPrivateAccess = "true"))
    class USpotLightComponent* RimLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting", meta = (AllowPrivateAccess = "true"))
    class UExponentialHeightFogComponent* AtmosphericFog;

    // Time and weather system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Noon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    ELight_WeatherType CurrentWeather = ELight_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    float DayDurationMinutes = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    bool bAutoProgressTime = true;

    // Lighting presets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    TMap<ELight_TimeOfDay, FLight_LightingPreset> TimeOfDayPresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    TMap<ELight_WeatherType, FLight_LightingPreset> WeatherPresets;

    // Cinematic lighting controls
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
    bool bCinematicMode = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
    float CinematicTransitionDuration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
    FLight_LightingPreset CinematicPreset;

    // Public functions
    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void SetTimeOfDay(ELight_TimeOfDay NewTimeOfDay, bool bInstant = false);

    UFUNCTION(BlueprintCallable, Category = "Weather Control")
    void SetWeather(ELight_WeatherType NewWeather, bool bInstant = false);

    UFUNCTION(BlueprintCallable, Category = "Cinematic Control")
    void EnableCinematicMode(const FLight_LightingPreset& Preset, float TransitionTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Cinematic Control")
    void DisableCinematicMode(float TransitionTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void ApplyLightingPreset(const FLight_LightingPreset& Preset, float TransitionTime = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    FLight_LightingPreset GetCurrentLightingSettings() const;

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SetVolumetricFogDensity(float Density);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SetAtmosphericScattering(float ScatteringIntensity);

    UFUNCTION(BlueprintCallable, Category = "Post Process Control")
    void UpdatePostProcessSettings(float WhiteTemp, float BloomIntensity, float Saturation);

protected:
    // Internal state
    float CurrentTimeProgress = 0.0f;
    bool bIsTransitioning = false;
    float TransitionProgress = 0.0f;
    float TransitionDuration = 0.0f;
    FLight_LightingPreset TransitionStartPreset;
    FLight_LightingPreset TransitionTargetPreset;

    // Reference to post process volume
    UPROPERTY()
    class APostProcessVolume* MainPostProcessVolume;

    // Internal functions
    void InitializeLightingPresets();
    void UpdateTimeProgression(float DeltaTime);
    void UpdateLightingTransition(float DeltaTime);
    void ApplyPresetToComponents(const FLight_LightingPreset& Preset);
    FLight_LightingPreset InterpolateLightingPresets(const FLight_LightingPreset& A, const FLight_LightingPreset& B, float Alpha) const;
    void FindOrCreatePostProcessVolume();
    void SetupAtmosphericComponents();
    
    // Cinematic lighting functions
    void CreateCinematicRimLighting();
    void CreateAtmosphericFillLights();
    void EnhanceVolumetricFog();
    void ConfigureCinematicPostProcess();
};