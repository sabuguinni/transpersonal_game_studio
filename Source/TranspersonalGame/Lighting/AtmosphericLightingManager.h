#pragma once

#include "CoreMinimal.h"
#include "Engine/DirectionalLight.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/ExponentialHeightFog.h"
#include "Subsystems/WorldSubsystem.h"
#include "AtmosphericLightingManager.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

UENUM(BlueprintType)
enum class ELight_WeatherType : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Storm       UMETA(DisplayName = "Storm")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunTemperature = 5500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float RayleighScattering = 0.0331f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float MieScattering = 0.003996f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float MieAbsorption = 0.000444f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
    float WhiteTemperature = 6500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
    FVector4 ColorSaturation = FVector4(1.2f, 1.2f, 1.2f, 1.0f);
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_DayNightCycle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float CurrentTimeOfDay = 12.0f; // 0-24 hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float DayDurationMinutes = 30.0f; // Real minutes for full day cycle

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    bool bAutoAdvanceTime = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    UCurveFloat* SunIntensityCurve;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    UCurveLinearColor* SunColorCurve;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    UCurveFloat* SkyBrightnessCurve;
};

/**
 * Manages atmospheric lighting, day/night cycles, and weather effects for the Cretaceous world
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAtmosphericLightingManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UAtmosphericLightingManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Tick function
    virtual void Tick(float DeltaTime) override;
    virtual bool IsTickable() const override { return true; }
    virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UAtmosphericLightingManager, STATGROUP_Tickables); }

    // Day/Night Cycle
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(float Hours);

    UFUNCTION(BlueprintPure, Category = "Lighting")
    float GetTimeOfDay() const { return DayNightSettings.CurrentTimeOfDay; }

    UFUNCTION(BlueprintPure, Category = "Lighting")
    ELight_TimeOfDay GetCurrentTimeOfDayEnum() const;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetDayDuration(float Minutes);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void PauseTimeAdvancement(bool bPause);

    // Weather Control
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherType(ELight_WeatherType NewWeather, float TransitionDuration = 5.0f);

    UFUNCTION(BlueprintPure, Category = "Weather")
    ELight_WeatherType GetCurrentWeather() const { return CurrentWeather; }

    // Atmospheric Settings
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ApplyAtmosphericSettings(const FLight_AtmosphericSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ResetToCretaceousDefaults();

    // Manual Lighting Control
    UFUNCTION(BlueprintCallable, Category = "Lighting", CallInEditor = true)
    void RefreshLightingActors();

    UFUNCTION(BlueprintCallable, Category = "Lighting", CallInEditor = true)
    void ApplyCretaceousLighting();

protected:
    // Core atmospheric settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FLight_AtmosphericSettings AtmosphericSettings;

    // Day/night cycle settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night")
    FLight_DayNightCycle DayNightSettings;

    // Current weather state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather")
    ELight_WeatherType CurrentWeather = ELight_WeatherType::Clear;

    // Actor references
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Actors")
    ADirectionalLight* SunLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Actors")
    class ASkyAtmosphere* SkyAtmosphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Actors")
    APostProcessVolume* PostProcessVolume;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Actors")
    AExponentialHeightFog* HeightFog;

private:
    // Internal methods
    void UpdateDayNightCycle(float DeltaTime);
    void UpdateSunPosition();
    void UpdateSunLighting();
    void UpdateSkyAtmosphere();
    void UpdatePostProcessing();
    void UpdateFog();
    
    void FindLightingActors();
    void CacheLightingComponents();
    
    // Weather transition
    float WeatherTransitionTime = 0.0f;
    float WeatherTransitionDuration = 5.0f;
    ELight_WeatherType TargetWeather = ELight_WeatherType::Clear;
    bool bIsTransitioningWeather = false;
    
    // Component caches
    class UDirectionalLightComponent* SunLightComponent;
    class USkyAtmosphereComponent* SkyAtmosphereComponent;
    class UPostProcessComponent* PostProcessComponent;
    class UExponentialHeightFogComponent* FogComponent;
};