#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "Subsystems/WorldSubsystem.h"
#include "../SharedTypes.h"
#include "LightingManager.generated.h"

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
    Cloudy      UMETA(DisplayName = "Cloudy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Stormy      UMETA(DisplayName = "Stormy")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_BiomeLighting
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    EBiomeType BiomeType = EBiomeType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    FLinearColor FogColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    float FogDensity = 0.005f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    FLinearColor AmbientTint = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    float AmbientIntensity = 1.0f;

    FLight_BiomeLighting()
    {
        BiomeType = EBiomeType::Savanna;
        FogColor = FLinearColor(0.8f, 0.7f, 0.5f, 1.0f);
        FogDensity = 0.005f;
        FogHeightFalloff = 0.2f;
        AmbientTint = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
        AmbientIntensity = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_TimeOfDaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    ELight_TimeOfDay TimeOfDay = ELight_TimeOfDay::Noon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    FLinearColor SunColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    FRotator SunRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    FLinearColor SkyColor = FLinearColor::Blue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float SkyIntensity = 1.0f;

    FLight_TimeOfDaySettings()
    {
        TimeOfDay = ELight_TimeOfDay::Noon;
        SunColor = FLinearColor(1.0f, 0.85f, 0.6f, 1.0f);
        SunIntensity = 3.0f;
        SunRotation = FRotator(-45.0f, 135.0f, 0.0f);
        SkyColor = FLinearColor(0.4f, 0.6f, 1.0f, 1.0f);
        SkyIntensity = 1.2f;
    }
};

/**
 * Manages dynamic lighting and atmospheric effects for the Cretaceous world
 * Handles day/night cycle, weather systems, and biome-specific lighting
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ULightingManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    ULightingManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Core lighting functions
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void InitializeLightingSystem();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateTimeOfDay(ELight_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateWeather(ELight_WeatherType NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetBiomeLighting(EBiomeType BiomeType, const FLight_BiomeLighting& LightingSettings);

    // Day/night cycle
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void StartDayNightCycle(float CycleDurationMinutes = 20.0f);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void StopDayNightCycle();

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetTimeOfDayProgress(float Progress);

    // Atmospheric effects
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void CreateBiomeFog(EBiomeType BiomeType, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateGlobalFog(float Density, FLinearColor Color);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetVolumetricLighting(bool bEnabled);

    // Cinematic lighting
    UFUNCTION(BlueprintCallable, Category = "Cinematic")
    void SetCinematicLighting(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Cinematic")
    void ConfigurePostProcessing(float Saturation, float Contrast, float Gamma);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Lighting")
    ELight_TimeOfDay GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintPure, Category = "Lighting")
    ELight_WeatherType GetCurrentWeather() const { return CurrentWeather; }

    UFUNCTION(BlueprintPure, Category = "Lighting")
    float GetDayNightProgress() const { return DayNightProgress; }

    UFUNCTION(BlueprintPure, Category = "Lighting")
    bool IsDayNightCycleActive() const { return bDayNightCycleActive; }

protected:
    // Core lighting actors
    UPROPERTY(BlueprintReadOnly, Category = "Lighting")
    TObjectPtr<ADirectionalLight> SunLight;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting")
    TObjectPtr<ASkyLight> SkyLight;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting")
    TObjectPtr<APostProcessVolume> PostProcessVolume;

    // Current state
    UPROPERTY(BlueprintReadOnly, Category = "State")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Noon;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    ELight_WeatherType CurrentWeather = ELight_WeatherType::Clear;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float DayNightProgress = 0.5f;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bDayNightCycleActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float CycleDuration = 1200.0f; // 20 minutes in seconds

    // Biome lighting configurations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    TMap<EBiomeType, FLight_BiomeLighting> BiomeLightingSettings;

    // Time of day configurations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    TMap<ELight_TimeOfDay, FLight_TimeOfDaySettings> TimeOfDaySettings;

    // Fog actors per biome
    UPROPERTY(BlueprintReadOnly, Category = "Atmosphere")
    TMap<EBiomeType, TObjectPtr<AExponentialHeightFog>> BiomeFogActors;

private:
    // Internal functions
    void SetupDefaultLightingSettings();
    void SetupDefaultBiomeLighting();
    void SetupDefaultTimeOfDaySettings();
    
    void UpdateSunLighting(const FLight_TimeOfDaySettings& Settings);
    void UpdateSkyLighting(const FLight_TimeOfDaySettings& Settings);
    void UpdateAtmosphericFog(ELight_WeatherType Weather);
    
    void FindOrCreateLightingActors();
    ADirectionalLight* FindOrCreateSunLight();
    ASkyLight* FindOrCreateSkyLight();
    APostProcessVolume* FindOrCreatePostProcessVolume();

    // Timer for day/night cycle
    FTimerHandle DayNightTimerHandle;
    void TickDayNightCycle();
};