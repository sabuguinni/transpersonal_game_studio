#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "PrehistoricAudioManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Forest,
    Plains,
    Swamp,
    Mountain,
    Desert
};

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Safe,
    Cautious,
    Danger,
    Panic
};

USTRUCT(BlueprintType)
struct FAudio_AmbienceSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambience")
    USoundCue* ForestAmbience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambience")
    USoundCue* PlainsAmbience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambience")
    USoundCue* SwampAmbience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambience")
    float BaseVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambience")
    float FadeTime;

    FAudio_AmbienceSettings()
    {
        ForestAmbience = nullptr;
        PlainsAmbience = nullptr;
        SwampAmbience = nullptr;
        BaseVolume = 0.7f;
        FadeTime = 2.0f;
    }
};

USTRUCT(BlueprintType)
struct FAudio_DinosaurSoundProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    USoundCue* IdleSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    USoundCue* AlertSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    USoundCue* AttackSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    USoundCue* FootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float MinPitch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float MaxPitch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float MaxAudibleDistance;

    FAudio_DinosaurSoundProfile()
    {
        IdleSound = nullptr;
        AlertSound = nullptr;
        AttackSound = nullptr;
        FootstepSound = nullptr;
        MinPitch = 0.8f;
        MaxPitch = 1.2f;
        MaxAudibleDistance = 2000.0f;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UPrehistoricAudioManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPrehistoricAudioManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Ambience Management
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetBiomeAmbience(EAudio_BiomeType BiomeType, float FadeTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetThreatLevel(EAudio_ThreatLevel ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateTimeOfDay(float TimeOfDay);

    // Dinosaur Audio
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayDinosaurSound(const FString& DinosaurType, const FString& SoundType, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void RegisterDinosaurSoundProfile(const FString& DinosaurType, const FAudio_DinosaurSoundProfile& SoundProfile);

    // Survival Audio
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlaySurvivalSound(const FString& SoundName, FVector Location, float Volume = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayPlayerHeartbeat(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopPlayerHeartbeat();

    // Environmental Audio
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayWeatherSound(const FString& WeatherType, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateWindIntensity(float WindSpeed);

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio Settings")
    FAudio_AmbienceSettings AmbienceSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudio_BiomeType CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudio_ThreatLevel CurrentThreatLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float CurrentTimeOfDay;

private:
    UPROPERTY()
    UAudioComponent* AmbienceAudioComponent;

    UPROPERTY()
    UAudioComponent* MusicAudioComponent;

    UPROPERTY()
    UAudioComponent* HeartbeatAudioComponent;

    UPROPERTY()
    TMap<FString, FAudio_DinosaurSoundProfile> DinosaurSoundProfiles;

    UPROPERTY()
    TArray<UAudioComponent*> ActiveSoundComponents;

    void InitializeDefaultSoundProfiles();
    void CleanupInactiveSounds();
    UAudioComponent* CreateSpatialAudioComponent(FVector Location);
};