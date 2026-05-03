#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "SharedTypes.h"
#include "AudioManager.generated.h"

// Audio system for prehistoric survival game
// Manages adaptive music, ambient sounds, and spatial audio

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Caution     UMETA(DisplayName = "Caution"),
    Danger      UMETA(DisplayName = "Danger"),
    Critical    UMETA(DisplayName = "Critical")
};

UENUM(BlueprintType)
enum class EAudio_MusicType : uint8
{
    Ambient     UMETA(DisplayName = "Ambient"),
    Exploration UMETA(DisplayName = "Exploration"),
    Tension     UMETA(DisplayName = "Tension"),
    Combat      UMETA(DisplayName = "Combat"),
    Discovery   UMETA(DisplayName = "Discovery"),
    Narrative   UMETA(DisplayName = "Narrative")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EEng_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString AmbientSoundPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString MusicPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float BaseVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float ReverbIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float LowPassFilter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bHasWind;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bHasWater;

    FAudio_BiomeConfig()
    {
        BiomeType = EEng_BiomeType::Forest;
        AmbientSoundPath = "";
        MusicPath = "";
        BaseVolume = 0.7f;
        ReverbIntensity = 0.5f;
        LowPassFilter = 22000.0f;
        bHasWind = true;
        bHasWater = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ThreatConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_ThreatLevel ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString MusicPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MusicVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float HeartbeatIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float TransitionSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bEnableStingers;

    FAudio_ThreatConfig()
    {
        ThreatLevel = EAudio_ThreatLevel::Safe;
        MusicPath = "";
        MusicVolume = 0.5f;
        HeartbeatIntensity = 0.0f;
        TransitionSpeed = 2.0f;
        bEnableStingers = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_AudioManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_AudioManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core audio management
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void InitializeAudioSystem();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetCurrentBiome(EEng_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetThreatLevel(EAudio_ThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayMusicType(EAudio_MusicType MusicType, float FadeTime = 2.0f);

    // Spatial audio
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlaySpatialSound(const FString& SoundPath, const FVector& Location, float Volume = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayDinosaurSound(EEng_DinosaurSpecies Species, const FVector& Location, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayFootstepSound(const FVector& Location, bool bIsHeavy = false);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlaySeismicRumble(const FVector& EpicenterLocation, float Magnitude);

    // Environmental audio
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateWeatherAudio(EEng_WeatherType WeatherType);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateTimeOfDayAudio(EEng_TimeOfDay TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetWindIntensity(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetRainIntensity(float Intensity);

    // Dialogue integration
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayDialogueAudio(const FString& AudioPath, float Volume = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopDialogueAudio();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    bool IsDialoguePlaying() const;

    // Volume and mixing
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetMusicVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetSFXVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetAmbientVolume(float Volume);

protected:
    // Audio components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    UAudioComponent* MusicComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    UAudioComponent* AmbientComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    UAudioComponent* DialogueComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    UAudioComponent* SFXComponent;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TArray<FAudio_BiomeConfig> BiomeConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TArray<FAudio_ThreatConfig> ThreatConfigs;

    // Current state
    UPROPERTY(BlueprintReadOnly, Category = "Audio")
    EEng_BiomeType CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Audio")
    EAudio_ThreatLevel CurrentThreatLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Audio")
    EAudio_MusicType CurrentMusicType;

    UPROPERTY(BlueprintReadOnly, Category = "Audio")
    EEng_WeatherType CurrentWeather;

    UPROPERTY(BlueprintReadOnly, Category = "Audio")
    EEng_TimeOfDay CurrentTimeOfDay;

    // Volume settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MasterVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MusicVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float SFXVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AmbientVolume;

    // Transition state
    UPROPERTY(BlueprintReadOnly, Category = "Audio")
    bool bIsTransitioning;

    UPROPERTY(BlueprintReadOnly, Category = "Audio")
    float TransitionTimer;

    UPROPERTY(BlueprintReadOnly, Category = "Audio")
    float TransitionDuration;

private:
    // Internal audio logic
    void InitializeBiomeConfigs();
    void InitializeThreatConfigs();
    void TransitionToNewBiome(EEng_BiomeType NewBiome);
    void TransitionToNewThreat(EAudio_ThreatLevel NewThreat);
    void UpdateAudioMixing();
    FAudio_BiomeConfig GetBiomeConfig(EEng_BiomeType BiomeType);
    FAudio_ThreatConfig GetThreatConfig(EAudio_ThreatLevel ThreatLevel);
    void SpawnSpatialAudioComponent(const FString& SoundPath, const FVector& Location, float Volume);

    // Timer handles
    FTimerHandle TransitionTimerHandle;
    FTimerHandle SeismicTimerHandle;
};