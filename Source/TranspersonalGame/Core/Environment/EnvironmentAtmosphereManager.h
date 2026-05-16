#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/TriggerBox.h"
#include "EnvironmentAtmosphereManager.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"), 
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

UENUM(BlueprintType)
enum class EEnvArt_WeatherType : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Hazy        UMETA(DisplayName = "Hazy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Volcanic    UMETA(DisplayName = "Volcanic Ash")
};

USTRUCT(BlueprintType)
struct FEnvArt_AtmosphereSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 3.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FRotator SunRotation = FRotator(-15.0f, 45.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogInscatteringColor = FLinearColor(0.9f, 0.7f, 0.4f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogMaxOpacity = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric")
    bool bEnableVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric")
    float VolumetricScatteringIntensity = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric")
    FLinearColor VolumetricAlbedo = FLinearColor(0.9f, 0.8f, 0.6f, 1.0f);
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvironmentAtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    AEnvironmentAtmosphereManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Atmosphere Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    EEnvArt_TimeOfDay CurrentTimeOfDay = EEnvArt_TimeOfDay::Afternoon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    EEnvArt_WeatherType CurrentWeather = EEnvArt_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FEnvArt_AtmosphereSettings AtmosphereSettings;

    // References to scene lighting
    UPROPERTY(BlueprintReadOnly, Category = "Lighting")
    ADirectionalLight* SunLight;

    UPROPERTY(BlueprintReadOnly, Category = "Fog")
    AExponentialHeightFog* HeightFog;

    // Atmospheric zones
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zones")
    TArray<ATriggerBox*> WindZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zones")
    TArray<ATriggerBox*> MistZones;

public:
    virtual void Tick(float DeltaTime) override;

    // Atmosphere Control Functions
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetWeatherType(EEnvArt_WeatherType NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ApplyAtmosphereSettings(const FEnvArt_AtmosphereSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateSunLighting();

    UFUNCTION(BlueprintCallable, Category = "Fog")
    void UpdateFogSettings();

    UFUNCTION(BlueprintCallable, Category = "Zones")
    void CreateWindZone(FVector Location, FVector Scale = FVector(100.0f, 100.0f, 10.0f));

    UFUNCTION(BlueprintCallable, Category = "Zones")
    void CreateMistZone(FVector Location, FVector Scale = FVector(50.0f, 50.0f, 20.0f));

    // Cretaceous Presets
    UFUNCTION(BlueprintCallable, Category = "Presets")
    void ApplyCretaceousGoldenHour();

    UFUNCTION(BlueprintCallable, Category = "Presets")
    void ApplyCretaceousStorm();

    UFUNCTION(BlueprintCallable, Category = "Presets")
    void ApplyCretaceousVolcanicHaze();

protected:
    void FindSceneLightingActors();
    void InitializeDefaultSettings();
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float DayNightCycleDuration = 1200.0f; // 20 minutes real time

    UPROPERTY(BlueprintReadOnly, Category = "Timing")
    float CurrentDayTime = 0.5f; // 0.0 = midnight, 0.5 = noon
};