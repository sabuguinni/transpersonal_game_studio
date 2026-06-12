#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "SharedTypes.h"
#include "Audio_AudioManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SoundLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Layer")
    USoundBase* SoundAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Layer")
    float Volume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Layer")
    float FadeTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Layer")
    bool bIsLooping;

    FAudio_SoundLayer()
    {
        SoundAsset = nullptr;
        Volume = 1.0f;
        FadeTime = 2.0f;
        bIsLooping = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_AudioZoneData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    TArray<FAudio_SoundLayer> AmbientLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    EBiomeType BiomeType;

    FAudio_AudioZoneData()
    {
        Location = FVector::ZeroVector;
        Radius = 1000.0f;
        BiomeType = EBiomeType::Forest;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UAudio_AudioManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UAudio_AudioManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Audio zone management
    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void RegisterAudioZone(const FAudio_AudioZoneData& ZoneData);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void UpdatePlayerLocation(const FVector& PlayerLocation);

    // Adaptive music system
    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void SetMusicIntensity(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void TriggerStingerSound(USoundBase* StingerSound);

    // Environmental audio
    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void SetWeatherAudio(EWeatherType WeatherType, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void SetTimeOfDayAudio(float TimeOfDay);

    // Survival audio feedback
    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void PlayHeartbeatEffect(float FearLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void PlayBreathingEffect(float StaminaLevel);

    // Dinosaur audio system
    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void PlayDinosaurAudio(EDinosaurSpecies Species, const FVector& Location, float Distance);

protected:
    UPROPERTY()
    TArray<FAudio_AudioZoneData> AudioZones;

    UPROPERTY()
    UAudioComponent* MusicComponent;

    UPROPERTY()
    UAudioComponent* AmbienceComponent;

    UPROPERTY()
    UAudioComponent* HeartbeatComponent;

    UPROPERTY()
    UAudioComponent* BreathingComponent;

    UPROPERTY()
    TMap<EDinosaurSpecies, USoundCue*> DinosaurSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings", meta = (AllowPrivateAccess = "true"))
    float MasterVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings", meta = (AllowPrivateAccess = "true"))
    float MusicVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings", meta = (AllowPrivateAccess = "true"))
    float SFXVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings", meta = (AllowPrivateAccess = "true"))
    float AmbienceVolume;

private:
    FVector LastPlayerLocation;
    float CurrentMusicIntensity;
    EWeatherType CurrentWeather;
    float CurrentTimeOfDay;

    void UpdateAmbientAudio();
    void CrossfadeToZone(const FAudio_AudioZoneData& NewZone);
    void LoadDinosaurSounds();
};