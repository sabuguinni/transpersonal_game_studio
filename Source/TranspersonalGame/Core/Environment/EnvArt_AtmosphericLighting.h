#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/World.h"
#include "EnvArt_AtmosphericLighting.generated.h"

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
    Overcast    UMETA(DisplayName = "Overcast"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Dusty       UMETA(DisplayName = "Dusty")
};

USTRUCT(BlueprintType)
struct FEnvArt_LightingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Light")
    float SunIntensity = 3.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Light")
    FLinearColor SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Light")
    FRotator SunRotation = FRotator(-45.0f, 45.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky Light")
    float SkyLightIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky Light")
    FLinearColor SkyLightColor = FLinearColor(0.4f, 0.6f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    bool bVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogInscatteringColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEnvArt_AtmosphericLighting : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnvArt_AtmosphericLighting();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    EEnvArt_TimeOfDay CurrentTimeOfDay = EEnvArt_TimeOfDay::Afternoon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EEnvArt_WeatherType CurrentWeather = EEnvArt_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    TMap<EEnvArt_TimeOfDay, FEnvArt_LightingSettings> TimeOfDayPresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Modifiers")
    TMap<EEnvArt_WeatherType, FEnvArt_LightingSettings> WeatherModifiers;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    ADirectionalLight* SunLight;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    class ASkyLight* SkyLight;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    class AExponentialHeightFog* HeightFog;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void SetWeather(EEnvArt_WeatherType NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void ApplyLightingSettings(const FEnvArt_LightingSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void TransitionToTimeOfDay(EEnvArt_TimeOfDay TargetTime, float TransitionDuration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void CreateGoldenHourLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void CreateMysticalForestLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void CreateVolcanicAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void RefreshLightingReferences();

    UFUNCTION(BlueprintPure, Category = "Atmospheric Lighting")
    FEnvArt_LightingSettings GetCurrentLightingSettings() const;

private:
    void InitializeTimeOfDayPresets();
    void InitializeWeatherModifiers();
    void FindLightingActors();
    void ApplySettingsToActors(const FEnvArt_LightingSettings& Settings);

    FEnvArt_LightingSettings CurrentSettings;
    bool bIsTransitioning = false;
    float TransitionTimer = 0.0f;
    float TransitionDuration = 5.0f;
    FEnvArt_LightingSettings TransitionStartSettings;
    FEnvArt_LightingSettings TransitionTargetSettings;
};