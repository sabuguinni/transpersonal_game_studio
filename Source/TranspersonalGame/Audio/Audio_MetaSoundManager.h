#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "MetasoundSource.h"
#include "Sound/SoundBase.h"
#include "../Core/SharedTypes.h"
#include "Audio_MetaSoundManager.generated.h"

/**
 * AUDIO AGENT #16 - METASOUND MANAGER
 * 
 * Manages adaptive audio system using UE5 MetaSounds for dynamic prehistoric
 * soundscapes. Responds to player state, dinosaur proximity, and environmental
 * conditions to create immersive audio experience.
 */

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Forest = 0          UMETA(DisplayName = "Forest"),
    Swamp = 1           UMETA(DisplayName = "Swamp"),
    Savanna = 2         UMETA(DisplayName = "Savanna"),
    Desert = 3          UMETA(DisplayName = "Desert"),
    SnowMountain = 4    UMETA(DisplayName = "Snow Mountain")
};

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Safe = 0            UMETA(DisplayName = "Safe"),
    Cautious = 1        UMETA(DisplayName = "Cautious"),
    Danger = 2          UMETA(DisplayName = "Danger"),
    Panic = 3           UMETA(DisplayName = "Panic")
};

UENUM(BlueprintType)
enum class EAudio_TimeOfDay : uint8
{
    Dawn = 0            UMETA(DisplayName = "Dawn"),
    Day = 1             UMETA(DisplayName = "Day"),
    Dusk = 2            UMETA(DisplayName = "Dusk"),
    Night = 3           UMETA(DisplayName = "Night")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SoundscapeParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soundscape")
    EAudio_BiomeType CurrentBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soundscape")
    EAudio_ThreatLevel ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soundscape")
    EAudio_TimeOfDay TimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soundscape")
    float WeatherIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soundscape")
    float DinosaurProximity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soundscape")
    float PlayerStressLevel;

    FAudio_SoundscapeParameters()
    {
        CurrentBiome = EAudio_BiomeType::Forest;
        ThreatLevel = EAudio_ThreatLevel::Safe;
        TimeOfDay = EAudio_TimeOfDay::Day;
        WeatherIntensity = 0.0f;
        DinosaurProximity = 0.0f;
        PlayerStressLevel = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_DinosaurAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    FString DinosaurType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TSoftObjectPtr<USoundBase> IdleSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TSoftObjectPtr<USoundBase> AlertSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TSoftObjectPtr<USoundBase> AttackSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TSoftObjectPtr<USoundBase> FootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float VolumeMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float PitchVariation;

    FAudio_DinosaurAudioData()
    {
        DinosaurType = TEXT("");
        IdleSound = nullptr;
        AlertSound = nullptr;
        AttackSound = nullptr;
        FootstepSound = nullptr;
        VolumeMultiplier = 1.0f;
        PitchVariation = 0.1f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_MetaSoundManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_MetaSoundManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core audio components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* AmbienceAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* MusicAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* NarrativeAudioComponent;

    // MetaSound assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSound Assets")
    TSoftObjectPtr<UMetaSoundSource> AdaptiveMusicMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSound Assets")
    TSoftObjectPtr<UMetaSoundSource> AmbienceMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSound Assets")
    TSoftObjectPtr<UMetaSoundSource> DinosaurMetaSound;

    // Soundscape management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soundscape")
    FAudio_SoundscapeParameters CurrentSoundscape;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soundscape")
    TArray<FAudio_DinosaurAudioData> DinosaurAudioDatabase;

    // Audio settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MasterVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float AmbienceVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MusicVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float SFXVolume;

public:
    // Soundscape control
    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void UpdateSoundscapeParameters(const FAudio_SoundscapeParameters& NewParameters);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void SetBiome(EAudio_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void SetThreatLevel(EAudio_ThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void SetTimeOfDay(EAudio_TimeOfDay NewTimeOfDay);

    // Dinosaur audio
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void PlayDinosaurSound(const FString& DinosaurType, const FString& SoundType, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void RegisterDinosaurProximity(const FString& DinosaurType, float Distance);

    // Narrative audio
    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void PlayNarrativeClip(USoundBase* AudioClip);

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void StopNarrativeAudio();

    // Environmental audio
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void TriggerWeatherAudio(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void PlayFootstepAudio(FVector Location, const FString& SurfaceType);

    // Volume control
    UFUNCTION(BlueprintCallable, Category = "Audio Settings")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Settings")
    void SetAmbienceVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Settings")
    void SetMusicVolume(float Volume);

    // Query functions
    UFUNCTION(BlueprintPure, Category = "Audio Query")
    EAudio_BiomeType GetCurrentBiome() const { return CurrentSoundscape.CurrentBiome; }

    UFUNCTION(BlueprintPure, Category = "Audio Query")
    EAudio_ThreatLevel GetCurrentThreatLevel() const { return CurrentSoundscape.ThreatLevel; }

    UFUNCTION(BlueprintPure, Category = "Audio Query")
    float GetPlayerStressLevel() const { return CurrentSoundscape.PlayerStressLevel; }

private:
    // Internal audio management
    void InitializeAudioComponents();
    void InitializeDinosaurAudioDatabase();
    void UpdateAdaptiveMusic();
    void UpdateAmbienceAudio();
    void ProcessAudioParameters();
    
    // MetaSound parameter updates
    void UpdateMetaSoundParameters();
    void SetMetaSoundParameter(UAudioComponent* AudioComp, const FName& ParameterName, float Value);
    
    // Audio transition management
    void CrossfadeToNewBiome(EAudio_BiomeType NewBiome);
    void TransitionThreatLevel(EAudio_ThreatLevel NewLevel);
    
    // Timer for audio updates
    float AudioUpdateTimer;
    static constexpr float AudioUpdateInterval = 0.5f;
    
    // Crossfade state
    bool bIsTransitioning;
    float TransitionProgress;
    static constexpr float TransitionDuration = 2.0f;
};