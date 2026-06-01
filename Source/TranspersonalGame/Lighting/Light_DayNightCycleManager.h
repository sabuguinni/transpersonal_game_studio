#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Light_DayNightCycleManager.generated.h"

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

USTRUCT(BlueprintType)
struct FLight_TimeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    float SunAngle = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    FLinearColor SunColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    FLinearColor FogColor = FLinearColor(0.447f, 0.639f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    float AtmosphereExposure = 1.0f;

    FLight_TimeSettings()
    {
        SunAngle = 0.0f;
        SunIntensity = 3.0f;
        SunColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
        FogDensity = 0.02f;
        FogColor = FLinearColor(0.447f, 0.639f, 1.0f, 1.0f);
        AtmosphereExposure = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct FLight_BiomeLightingConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    float ExposureBias = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    float ContrastMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    float SaturationMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    float ColorTemperature = 6500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    FLinearColor AmbientTint = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);

    FLight_BiomeLightingConfig()
    {
        BiomeName = TEXT("Default");
        ExposureBias = 1.0f;
        ContrastMultiplier = 1.0f;
        SaturationMultiplier = 1.0f;
        ColorTemperature = 6500.0f;
        AmbientTint = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
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

    // Core lighting components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Components")
    class UDirectionalLightComponent* SunLightComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Components")
    class USkyAtmosphereComponent* SkyAtmosphereComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Components")
    class UExponentialHeightFogComponent* FogComponent;

    // Day/Night cycle settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    bool bEnableDayNightCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float CycleSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentTimeOfDay = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    ELight_TimeOfDay CurrentTimePhase = ELight_TimeOfDay::Midday;

    // Time-specific lighting configurations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    TMap<ELight_TimeOfDay, FLight_TimeSettings> TimeConfigurations;

    // Biome-specific lighting configurations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    TArray<FLight_BiomeLightingConfig> BiomeLightingConfigs;

    // Dynamic lighting properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Lighting")
    bool bEnableWeatherEffects = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Lighting")
    float WeatherIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Lighting")
    float CloudCoverage = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Lighting")
    bool bEnableVolumetricFog = true;

public:
    // Core functionality
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetTimeOfDay(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetTimePhase(ELight_TimeOfDay NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    float GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    ELight_TimeOfDay GetCurrentTimePhase() const { return CurrentTimePhase; }

    // Lighting control
    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void UpdateSunPosition();

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void UpdateAtmosphericSettings();

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void UpdateFogSettings();

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void ApplyBiomeLighting(const FString& BiomeName);

    // Weather and atmospheric effects
    UFUNCTION(BlueprintCallable, Category = "Weather Effects")
    void SetWeatherIntensity(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Weather Effects")
    void SetCloudCoverage(float Coverage);

    UFUNCTION(BlueprintCallable, Category = "Weather Effects")
    void TriggerLightningEffect();

    // Configuration and setup
    UFUNCTION(BlueprintCallable, Category = "Configuration")
    void InitializeTimeConfigurations();

    UFUNCTION(BlueprintCallable, Category = "Configuration")
    void InitializeBiomeConfigurations();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Configuration")
    void RefreshLightingSystem();

private:
    // Internal update functions
    void UpdateTimePhase();
    void InterpolateLightingSettings(float Alpha);
    FLight_TimeSettings GetInterpolatedSettings(ELight_TimeOfDay FromPhase, ELight_TimeOfDay ToPhase, float Alpha);
    
    // Helper functions
    float CalculateSunAngle(float TimeOfDay);
    FLinearColor CalculateSunColor(float SunAngle);
    float CalculateSunIntensity(float SunAngle);
    
    // Biome detection and application
    FString DetectCurrentBiome();
    void ApplyBiomeSpecificSettings(const FLight_BiomeLightingConfig& Config);
    
    // Weather system integration
    void UpdateWeatherEffects(float DeltaTime);
    void ProcessCloudShadows();
    
    // Performance optimization
    float LastUpdateTime = 0.0f;
    static constexpr float UpdateInterval = 0.1f; // Update every 100ms for performance
};