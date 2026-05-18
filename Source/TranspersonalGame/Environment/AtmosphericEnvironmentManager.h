#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyAtmosphere.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "AtmosphericEnvironmentManager.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_AtmosphericTimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Noon        UMETA(DisplayName = "Noon"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

UENUM(BlueprintType)
enum class EEnvArt_WeatherCondition : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    LightRain   UMETA(DisplayName = "Light Rain"),
    HeavyRain   UMETA(DisplayName = "Heavy Rain"),
    Fog         UMETA(DisplayName = "Fog"),
    Storm       UMETA(DisplayName = "Storm")
};

USTRUCT(BlueprintType)
struct FEnvArt_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 3.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FRotator SunRotation = FRotator(-30.0f, 45.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    bool bVolumetricFogEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float VolumetricScatteringDistribution = 0.2f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAtmosphericEnvironmentManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAtmosphericEnvironmentManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    EEnvArt_AtmosphericTimeOfDay CurrentTimeOfDay = EEnvArt_AtmosphericTimeOfDay::Afternoon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    EEnvArt_WeatherCondition CurrentWeather = EEnvArt_WeatherCondition::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FEnvArt_AtmosphericSettings AtmosphericSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    AExponentialHeightFog* AtmosphericFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    ASkyAtmosphere* SkyAtmosphere;

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void SetTimeOfDay(EEnvArt_AtmosphericTimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void SetWeatherCondition(EEnvArt_WeatherCondition NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void ApplyAtmosphericSettings(const FEnvArt_AtmosphericSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void CreateCretaceousAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void UpdateLightingForBiome(const FString& BiomeName, const FVector& BiomeLocation);

private:
    void UpdateSunLighting();
    void UpdateFogSettings();
    void FindOrCreateAtmosphericActors();

    UPROPERTY()
    float TimeOfDayProgress = 0.5f;

    UPROPERTY()
    bool bAtmosphereInitialized = false;
};