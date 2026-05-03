#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/VolumetricClouds.h"
#include "Light_AtmosphericManager.generated.h"

UENUM(BlueprintType)
enum class ELight_WeatherType : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Cloudy      UMETA(DisplayName = "Cloudy"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Dusty       UMETA(DisplayName = "Dusty")
};

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

USTRUCT(BlueprintType)
struct FLight_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    FLinearColor SkyTint = FLinearColor(0.5f, 0.8f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float SkyIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds")
    float CloudCoverage = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float AtmosphereThickness = 1.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_AtmosphericManager : public AActor
{
    GENERATED_BODY()

public:
    ALight_AtmosphericManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Referências para actores de iluminação existentes
    UPROPERTY(BlueprintReadOnly, Category = "Lighting")
    ADirectionalLight* SunActor;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting")
    ASkyLight* SkyLightActor;

    UPROPERTY(BlueprintReadOnly, Category = "Atmosphere")
    ASkyAtmosphere* AtmosphereActor;

    UPROPERTY(BlueprintReadOnly, Category = "Atmosphere")
    AExponentialHeightFog* FogActor;

    UPROPERTY(BlueprintReadOnly, Category = "Atmosphere")
    AVolumetricClouds* CloudsActor;

public:
    // Configurações de tempo e clima
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float TimeOfDayHours = 12.0f; // 0-24 horas

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float DayDurationMinutes = 20.0f; // Duração de um dia em minutos reais

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherType CurrentWeather = ELight_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionSpeed = 1.0f;

    // Presets para diferentes momentos do dia
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    FLight_AtmosphericSettings DawnSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    FLight_AtmosphericSettings NoonSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    FLight_AtmosphericSettings DuskSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    FLight_AtmosphericSettings NightSettings;

    // Configurações de bioma
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bUseSwampLighting = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bUseForestLighting = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bUseDesertLighting = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bUseMountainLighting = false;

    // Funções públicas
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(float Hours);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeather(ELight_WeatherType NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyBiomeLighting(const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    ELight_TimeOfDay GetCurrentTimeOfDay() const;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void EnableDynamicTimeProgression(bool bEnable);

private:
    bool bDynamicTimeEnabled = true;
    float LastUpdateTime = 0.0f;

    void FindLightingActors();
    void UpdateSunPosition();
    void UpdateAtmosphericSettings();
    void InterpolateSettings(const FLight_AtmosphericSettings& From, const FLight_AtmosphericSettings& To, float Alpha);
    void ApplyWeatherEffects();
    FLight_AtmosphericSettings GetSettingsForTime(float Hours) const;
    float CalculateSunAngle(float Hours) const;
};