#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/World.h"
#include "EnvArt_AtmosphereManager.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_TimeOfDay : uint8
{
    Dawn       UMETA(DisplayName = "Dawn"),
    Morning    UMETA(DisplayName = "Morning"),
    Midday     UMETA(DisplayName = "Midday"),
    Afternoon  UMETA(DisplayName = "Afternoon"),
    Dusk       UMETA(DisplayName = "Dusk"),
    Night      UMETA(DisplayName = "Night")
};

UENUM(BlueprintType)
enum class EEnvArt_WeatherType : uint8
{
    Clear      UMETA(DisplayName = "Clear"),
    Cloudy     UMETA(DisplayName = "Cloudy"),
    Foggy      UMETA(DisplayName = "Foggy"),
    Stormy     UMETA(DisplayName = "Stormy"),
    Dusty      UMETA(DisplayName = "Dusty")
};

USTRUCT(BlueprintType)
struct FEnvArt_AtmosphereSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FRotator SunRotation = FRotator(-30.0f, 45.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float AtmosphereThickness = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    FLinearColor SkyTint = FLinearColor(0.5f, 0.8f, 1.0f, 1.0f);
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEnvArt_AtmosphereManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnvArt_AtmosphereManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    EEnvArt_TimeOfDay CurrentTimeOfDay = EEnvArt_TimeOfDay::Morning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EEnvArt_WeatherType CurrentWeather = EEnvArt_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    TMap<EEnvArt_TimeOfDay, FEnvArt_AtmosphereSettings> TimeOfDaySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    TMap<EEnvArt_WeatherType, FEnvArt_AtmosphereSettings> WeatherSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float TransitionSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    bool bAutoTransition = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float TimeOfDayDuration = 300.0f; // 5 minutes per time period

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetWeather(EEnvArt_WeatherType NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void TransitionToSettings(const FEnvArt_AtmosphereSettings& TargetSettings);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ApplyGoldenHourLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ApplyVolumetricFog();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void CreateAtmosphericParticles();

private:
    UPROPERTY()
    ADirectionalLight* SunLight;

    UPROPERTY()
    UExponentialHeightFogComponent* HeightFog;

    UPROPERTY()
    USkyAtmosphereComponent* SkyAtmosphere;

    FEnvArt_AtmosphereSettings CurrentSettings;
    FEnvArt_AtmosphereSettings TargetSettings;
    bool bIsTransitioning = false;
    float TransitionProgress = 0.0f;
    float TimeOfDayTimer = 0.0f;

    void InitializeAtmosphereComponents();
    void UpdateAtmosphere(float DeltaTime);
    void InterpolateSettings(float Alpha);
    void FindAtmosphereActors();
    void SetupDefaultSettings();
};