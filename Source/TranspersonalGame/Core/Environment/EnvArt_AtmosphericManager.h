#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Engine/TriggerBox.h"
#include "EnvArt_AtmosphericManager.generated.h"

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
    Dusty       UMETA(DisplayName = "Dusty"),
    Stormy      UMETA(DisplayName = "Stormy")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_LightingPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunTemperature = 6500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FRotator SunRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SkyColor = FLinearColor::Blue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor::White;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_AudioZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString ZoneName = "Default_Zone";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FVector Scale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TArray<FString> AmbientSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float VolumeMultiplier = 1.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_AtmosphericManager : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_AtmosphericManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    EEnvArt_TimeOfDay CurrentTimeOfDay = EEnvArt_TimeOfDay::Midday;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    EEnvArt_WeatherType CurrentWeather = EEnvArt_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    TMap<EEnvArt_TimeOfDay, FEnvArt_LightingPreset> TimeOfDayPresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Presets")
    TMap<EEnvArt_WeatherType, FEnvArt_LightingPreset> WeatherPresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zones")
    TArray<FEnvArt_AudioZone> AudioZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float TransitionDuration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    bool bAutoTransition = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float DayDurationMinutes = 20.0f;

public:
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SetWeather(EEnvArt_WeatherType NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void TransitionToPreset(const FEnvArt_LightingPreset& TargetPreset);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void CreateAudioZone(const FEnvArt_AudioZone& ZoneData);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void UpdateAudioZones();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void ApplyGoldenHourLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void ApplyMysticalForestLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void ApplyVolcanicLighting();

protected:
    UFUNCTION(BlueprintImplementableEvent, Category = "Atmospheric Events")
    void OnTimeOfDayChanged(EEnvArt_TimeOfDay NewTime);

    UFUNCTION(BlueprintImplementableEvent, Category = "Atmospheric Events")
    void OnWeatherChanged(EEnvArt_WeatherType NewWeather);

private:
    void InitializeLightingPresets();
    void InitializeAudioZones();
    void FindLightingComponents();

    UPROPERTY()
    class UDirectionalLightComponent* SunLightComponent;

    UPROPERTY()
    class USkyLightComponent* SkyLightComponent;

    UPROPERTY()
    class UVolumetricCloudComponent* CloudComponent;

    UPROPERTY()
    TArray<class ATriggerBox*> SpawnedAudioTriggers;

    float CurrentTransitionTime = 0.0f;
    bool bIsTransitioning = false;
    FEnvArt_LightingPreset StartPreset;
    FEnvArt_LightingPreset TargetPreset;
};