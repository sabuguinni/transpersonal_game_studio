#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Engine/World.h"
#include "TranspersonalGame.h"
#include "Light_AtmosphereManager.generated.h"

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
struct TRANSPERSONALGAME_API FLight_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunAngle = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambient")
    float SkyLightIntensity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambient")
    FLinearColor SkyLightColor = FLinearColor(0.78f, 0.86f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogInscatteringColor = FLinearColor(1.0f, 0.86f, 0.70f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric")
    float VolumetricScatteringIntensity = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric")
    bool bEnableVolumetricFog = true;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_InteriorLightConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    float TorchIntensity = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FLinearColor TorchColor = FLinearColor(1.0f, 0.70f, 0.47f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    float TorchRadius = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    float FireplaceIntensity = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FLinearColor FireplaceColor = FLinearColor(1.0f, 0.60f, 0.30f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    bool bCastShadows = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    float VolumetricScattering = 2.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_AtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    ALight_AtmosphereManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Atmospheric Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLight_AtmosphericSettings AtmosphericSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FLight_InteriorLightConfig InteriorLightConfig;

    // Time and Weather
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Noon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherType CurrentWeather = ELight_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float DayDurationMinutes = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    bool bEnableDayNightCycle = true;

    // Light References
    UPROPERTY(BlueprintReadOnly, Category = "Lights")
    ADirectionalLight* SunLight;

    UPROPERTY(BlueprintReadOnly, Category = "Lights")
    ASkyLight* AmbientLight;

    UPROPERTY(BlueprintReadOnly, Category = "Lights")
    AExponentialHeightFog* AtmosphericFog;

    UPROPERTY(BlueprintReadOnly, Category = "Lights")
    TArray<APointLight*> InteriorTorches;

    UPROPERTY(BlueprintReadOnly, Category = "Lights")
    TArray<ASpotLight*> StructureRimLights;

private:
    float CurrentTimeOfDayFloat = 12.0f; // 0-24 hours
    float TimeAccumulator = 0.0f;

public:
    // Core Lighting Functions
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void InitializeAtmosphericLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateAtmosphericSettings(const FLight_AtmosphericSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Time")
    void SetTimeOfDay(ELight_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherType(ELight_WeatherType NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Time")
    void SetDayNightCycleEnabled(bool bEnabled);

    // Interior Lighting
    UFUNCTION(BlueprintCallable, Category = "Interior")
    void CreateInteriorLighting(FVector StructureLocation, const FString& StructureName);

    UFUNCTION(BlueprintCallable, Category = "Interior")
    void UpdateInteriorLightConfig(const FLight_InteriorLightConfig& NewConfig);

    UFUNCTION(BlueprintCallable, Category = "Interior")
    APointLight* SpawnTorchLight(FVector Location, const FString& LightName);

    UFUNCTION(BlueprintCallable, Category = "Interior")
    ASpotLight* SpawnRimLight(FVector Location, FRotator Rotation, const FString& LightName);

    // Lumen Configuration
    UFUNCTION(BlueprintCallable, Category = "Lumen")
    void ConfigureLumenGlobalIllumination();

    UFUNCTION(BlueprintCallable, Category = "Lumen")
    void EnableVolumetricFog();

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void FindExistingLightActors();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void CleanupLegacyAtmosphericFog();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere", CallInEditor = true)
    void ApplyCretaceousAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogCurrentLightingState();

    // Getters
    UFUNCTION(BlueprintPure, Category = "Time")
    float GetCurrentTimeOfDayFloat() const { return CurrentTimeOfDayFloat; }

    UFUNCTION(BlueprintPure, Category = "Time")
    ELight_TimeOfDay GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintPure, Category = "Weather")
    ELight_WeatherType GetCurrentWeather() const { return CurrentWeather; }

private:
    void UpdateDayNightCycle(float DeltaTime);
    void ApplyTimeOfDayLighting();
    void ApplyWeatherEffects();
    FLight_AtmosphericSettings GetSettingsForTimeOfDay(ELight_TimeOfDay TimeOfDay);
    FLight_AtmosphericSettings GetSettingsForWeather(ELight_WeatherType Weather);
};