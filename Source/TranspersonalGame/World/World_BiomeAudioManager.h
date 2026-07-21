#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "World_BiomeAudioManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundCue* AmbientLoop;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TArray<USoundWave*> RandomSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AmbientVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float RandomSoundChance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MinRandomInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MaxRandomInterval;

    FWorld_BiomeAudioData()
    {
        AmbientLoop = nullptr;
        AmbientVolume = 0.5f;
        RandomSoundChance = 0.3f;
        MinRandomInterval = 10.0f;
        MaxRandomInterval = 30.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_WeatherAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    USoundCue* RainSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    USoundCue* WindSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    USoundCue* ThunderSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    float WeatherVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    float ThunderChance;

    FWorld_WeatherAudioData()
    {
        RainSound = nullptr;
        WindSound = nullptr;
        ThunderSound = nullptr;
        WeatherVolume = 0.7f;
        ThunderChance = 0.1f;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UWorld_BiomeAudioManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UWorld_BiomeAudioManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core audio management
    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void UpdatePlayerBiome(EBiomeType NewBiome, const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void SetWeatherState(EWeatherType WeatherType, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void PlayRandomBiomeSound(EBiomeType BiomeType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void SetGlobalAudioVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void EnableDynamicAudio(bool bEnabled);

    // Audio zone management
    UFUNCTION(BlueprintCallable, Category = "Audio Zones")
    void CreateAudioZone(const FVector& Center, float Radius, EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Audio Zones")
    void RemoveAudioZone(const FVector& Center);

    UFUNCTION(BlueprintCallable, Category = "Audio Zones")
    EBiomeType GetBiomeAtLocation(const FVector& Location);

    // Day/night cycle audio
    UFUNCTION(BlueprintCallable, Category = "Day Night Audio")
    void UpdateTimeOfDay(float TimeOfDay); // 0.0 = midnight, 0.5 = noon

    UFUNCTION(BlueprintCallable, Category = "Day Night Audio")
    void SetNightAudioEnabled(bool bEnabled);

protected:
    // Biome audio data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TMap<EBiomeType, FWorld_BiomeAudioData> BiomeAudioMap;

    // Weather audio data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    FWorld_WeatherAudioData WeatherAudio;

    // Audio components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* AmbientAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* WeatherAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* RandomSoundComponent;

    // Current state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    EBiomeType CurrentBiome;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    EWeatherType CurrentWeather;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    float CurrentTimeOfDay;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    bool bDynamicAudioEnabled;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    bool bNightAudioEnabled;

    // Audio zones
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Zones")
    TArray<FVector> AudioZoneCenters;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Zones")
    TArray<float> AudioZoneRadii;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Zones")
    TArray<EBiomeType> AudioZoneBiomes;

    // Timers
    FTimerHandle RandomSoundTimer;
    FTimerHandle WeatherUpdateTimer;

private:
    // Internal methods
    void InitializeBiomeAudioData();
    void InitializeWeatherAudioData();
    void StartRandomSoundTimer();
    void PlayRandomSound();
    void UpdateWeatherAudio();
    void TransitionBiomeAudio(EBiomeType FromBiome, EBiomeType ToBiome);
    void ApplyTimeOfDayModifiers();
    float CalculateDistanceToNearestZone(const FVector& Location, EBiomeType& OutNearestBiome);
};