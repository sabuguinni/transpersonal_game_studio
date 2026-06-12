#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Audio_EnvironmentalController.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_EnvironmentZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Zone")
    FString ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Zone")
    FVector ZoneCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Zone")
    float ZoneRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Zone")
    TSoftObjectPtr<class USoundBase> AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Zone")
    float BaseVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Zone")
    bool bIsActive;

    FAudio_EnvironmentZone()
        : ZoneCenter(FVector::ZeroVector)
        , ZoneRadius(1000.0f)
        , BaseVolume(0.5f)
        , bIsActive(true)
    {
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_WeatherState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float RainIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WindSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float ThunderProbability;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float TemperatureLevel;

    FAudio_WeatherState()
        : RainIntensity(0.0f)
        , WindSpeed(0.3f)
        , ThunderProbability(0.0f)
        , TemperatureLevel(0.5f)
    {
    }
};

UCLASS()
class TRANSPERSONALGAME_API UAudio_EnvironmentalController : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Environment zone management
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void RegisterEnvironmentZone(const FAudio_EnvironmentZone& Zone);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void UpdatePlayerPosition(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void SetTimeOfDay(float TimeNormalized);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void UpdateWeatherState(const FAudio_WeatherState& WeatherState);

    // Prehistoric-specific environmental audio
    UFUNCTION(BlueprintCallable, Category = "Prehistoric Environment")
    void TriggerPrehistoricEvent(const FString& EventType, const FVector& Location, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Environment")
    void UpdateDinosaurPresence(const FVector& DinosaurLocation, float ThreatLevel, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Environment")
    void PlayVolcanicActivity(float Intensity, const FVector& VolcanoLocation);

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Environment")
    void UpdateForestDensity(float Density);

protected:
    // Environment zones
    UPROPERTY()
    TArray<FAudio_EnvironmentZone> EnvironmentZones;

    // Active audio components
    UPROPERTY()
    TMap<FString, class UAudioComponent*> ZoneAudioComponents;

    UPROPERTY()
    class UAudioComponent* WeatherAudioComponent;

    UPROPERTY()
    class UAudioComponent* DinosaurPresenceComponent;

    UPROPERTY()
    class UAudioComponent* VolcanicAudioComponent;

    // Current state
    UPROPERTY()
    FVector CurrentPlayerLocation;

    UPROPERTY()
    float CurrentTimeOfDay;

    UPROPERTY()
    FAudio_WeatherState CurrentWeatherState;

    UPROPERTY()
    FString CurrentActiveZone;

    // Audio configuration
    UPROPERTY(EditAnywhere, Category = "Environmental Audio")
    float ZoneTransitionSpeed;

    UPROPERTY(EditAnywhere, Category = "Environmental Audio")
    float MaxAudibleDistance;

    UPROPERTY(EditAnywhere, Category = "Environmental Audio")
    float VolumeInterpolationSpeed;

private:
    void UpdateActiveZone();
    void TransitionToZone(const FString& NewZoneName);
    void UpdateZoneVolumes(float DeltaTime);
    void ProcessWeatherAudio();
    void UpdateDayNightCycle();
    
    UAudioComponent* GetOrCreateAudioComponent(const FString& ComponentName);
    float CalculateDistanceAttenuation(const FVector& SourceLocation, const FVector& ListenerLocation, float MaxDistance);
    
    // Timers for environmental updates
    FTimerHandle EnvironmentUpdateTimer;
    void EnvironmentUpdateTick();
};