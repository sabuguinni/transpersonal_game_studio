#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/VolumetricFogComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/VolumetricFog.h"
#include "Sound/SoundCue.h"
#include "EnvArt_AtmosphericLightingManager.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"), 
    Noon        UMETA(DisplayName = "Noon"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

UENUM(BlueprintType)
enum class EEnvArt_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Stormy      UMETA(DisplayName = "Stormy")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FRotator SunRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    FLinearColor SkyTint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AmbientVolume;

    FEnvArt_AtmosphericSettings()
    {
        SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
        SunIntensity = 3.5f;
        SunRotation = FRotator(-15.0f, 45.0f, 0.0f);
        FogDensity = 0.02f;
        FogColor = FLinearColor(0.8f, 0.86f, 0.7f, 1.0f);
        FogHeightFalloff = 0.2f;
        SkyTint = FLinearColor(0.9f, 0.95f, 1.0f, 1.0f);
        AmbientVolume = 0.5f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_AtmosphericLightingManager : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_AtmosphericLightingManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    EEnvArt_TimeOfDay CurrentTimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    EEnvArt_WeatherState CurrentWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Settings")
    TMap<EEnvArt_TimeOfDay, FEnvArt_AtmosphericSettings> TimeOfDaySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Settings")
    TMap<EEnvArt_WeatherState, FEnvArt_AtmosphericSettings> WeatherSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float TransitionSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    bool bAutoTransition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float DayDurationMinutes;

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SetWeatherState(EEnvArt_WeatherState NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void TransitionToSettings(const FEnvArt_AtmosphericSettings& TargetSettings);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void ApplyGoldenHourLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void CreateVolumetricFogZone(const FVector& Location, float Radius, float Density);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void UpdateDirectionalLight(const FEnvArt_AtmosphericSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void UpdateVolumetricFog(const FEnvArt_AtmosphericSettings& Settings);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    TObjectPtr<ADirectionalLight> MainDirectionalLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    TObjectPtr<ASkyLight> MainSkyLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    TArray<TObjectPtr<AVolumetricFog>> VolumetricFogActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TObjectPtr<UAudioComponent> AmbientAudioComponent;

private:
    void InitializeAtmosphericSettings();
    void FindLightingActors();
    void UpdateAmbientAudio(const FEnvArt_AtmosphericSettings& Settings);

    FEnvArt_AtmosphericSettings CurrentSettings;
    FEnvArt_AtmosphericSettings TargetSettings;
    bool bTransitioning;
    float TransitionProgress;
    float CurrentDayTime;
};