#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Audio_SoundManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Savana UMETA(DisplayName = "Savana"),
    Pantano UMETA(DisplayName = "Pantano"),
    Floresta UMETA(DisplayName = "Floresta"),
    Deserto UMETA(DisplayName = "Deserto"),
    Montanha UMETA(DisplayName = "Montanha")
};

UENUM(BlueprintType)
enum class EAudio_DinosaurType : uint8
{
    TRex UMETA(DisplayName = "T-Rex"),
    Velociraptor UMETA(DisplayName = "Velociraptor"),
    Triceratops UMETA(DisplayName = "Triceratops"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus UMETA(DisplayName = "Ankylosaurus")
};

USTRUCT(BlueprintType)
struct FAudio_SoundProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Profile")
    TSoftObjectPtr<USoundCue> AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Profile")
    TSoftObjectPtr<USoundCue> FootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Profile")
    TSoftObjectPtr<USoundCue> VocalizationSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Profile")
    float VolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Profile")
    float PitchMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Profile")
    float AttenuationRadius = 5000.0f;

    FAudio_SoundProfile()
    {
        VolumeMultiplier = 1.0f;
        PitchMultiplier = 1.0f;
        AttenuationRadius = 5000.0f;
    }
};

USTRUCT(BlueprintType)
struct FAudio_BiomeAmbience
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    EAudio_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TSoftObjectPtr<USoundCue> DayAmbience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TSoftObjectPtr<USoundCue> NightAmbience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TSoftObjectPtr<USoundCue> WeatherAmbience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float TransitionDuration = 5.0f;

    FAudio_BiomeAmbience()
    {
        BiomeType = EAudio_BiomeType::Savana;
        TransitionDuration = 5.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_SoundManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_SoundManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core audio components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* MasterAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* AmbienceAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* MusicAudioComponent;

    // Audio profiles for different dinosaur types
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TMap<EAudio_DinosaurType, FAudio_SoundProfile> DinosaurSoundProfiles;

    // Biome ambience settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TArray<FAudio_BiomeAmbience> BiomeAmbienceSettings;

    // Current audio state
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudio_BiomeType CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    bool bIsNightTime;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    bool bIsWeatherActive;

    // Audio settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float AmbienceVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MusicVolume = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float SFXVolume = 0.8f;

public:
    // Main audio control functions
    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void SetCurrentBiome(EAudio_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void SetTimeOfDay(bool bNightTime);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void SetWeatherState(bool bWeatherActive);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void PlayDinosaurSound(EAudio_DinosaurType DinosaurType, FVector Location, bool bIsFootstep = false);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void PlayAmbienceForBiome(EAudio_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void StopAllAmbience();

    // Volume controls
    UFUNCTION(BlueprintCallable, Category = "Audio Settings")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Settings")
    void SetAmbienceVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Settings")
    void SetMusicVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Settings")
    void SetSFXVolume(float Volume);

    // Audio state queries
    UFUNCTION(BlueprintPure, Category = "Audio State")
    EAudio_BiomeType GetCurrentBiome() const { return CurrentBiome; }

    UFUNCTION(BlueprintPure, Category = "Audio State")
    bool IsNightTime() const { return bIsNightTime; }

    UFUNCTION(BlueprintPure, Category = "Audio State")
    bool IsWeatherActive() const { return bIsWeatherActive; }

private:
    // Internal audio management
    void UpdateAmbienceAudio();
    void CrossfadeToNewAmbience(USoundCue* NewAmbience);
    FAudio_SoundProfile GetDinosaurSoundProfile(EAudio_DinosaurType DinosaurType);
    FAudio_BiomeAmbience GetBiomeAmbienceSettings(EAudio_BiomeType BiomeType);

    // Audio transition state
    bool bIsTransitioning;
    float TransitionTimer;
    float TransitionDuration;
    USoundCue* PreviousAmbience;
    USoundCue* TargetAmbience;
};