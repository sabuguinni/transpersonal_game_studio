#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Subsystems/WorldSubsystem.h"
#include "../Core/SharedTypes.h"
#include "AudioManager.generated.h"

// Audio system enums using Audio_ prefix
UENUM(BlueprintType)
enum class EAudio_EnvironmentType : uint8
{
    Forest = 0      UMETA(DisplayName = "Forest"),
    Swamp = 1       UMETA(DisplayName = "Swamp"),
    Grassland = 2   UMETA(DisplayName = "Grassland"),
    Desert = 3      UMETA(DisplayName = "Desert"),
    Mountains = 4   UMETA(DisplayName = "Mountains"),
    River = 5       UMETA(DisplayName = "River"),
    Cave = 6        UMETA(DisplayName = "Cave")
};

UENUM(BlueprintType)
enum class EAudio_IntensityLevel : uint8
{
    Calm = 0        UMETA(DisplayName = "Calm"),
    Ambient = 1     UMETA(DisplayName = "Ambient"),
    Tense = 2       UMETA(DisplayName = "Tense"),
    Danger = 3      UMETA(DisplayName = "Danger"),
    Combat = 4      UMETA(DisplayName = "Combat")
};

UENUM(BlueprintType)
enum class EAudio_DinosaurType : uint8
{
    TRex = 0        UMETA(DisplayName = "T-Rex"),
    Raptor = 1      UMETA(DisplayName = "Raptor"),
    Brachiosaurus = 2 UMETA(DisplayName = "Brachiosaurus"),
    Triceratops = 3 UMETA(DisplayName = "Triceratops"),
    Parasaurolophus = 4 UMETA(DisplayName = "Parasaurolophus")
};

// Audio configuration structs
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_EnvironmentSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Audio")
    EAudio_EnvironmentType EnvironmentType = EAudio_EnvironmentType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Audio")
    float AmbientVolume = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Audio")
    float ReverbIntensity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Audio")
    bool bEnableWindSounds = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Audio")
    bool bEnableBirdSounds = true;

    FAudio_EnvironmentSettings()
    {
        EnvironmentType = EAudio_EnvironmentType::Forest;
        AmbientVolume = 0.5f;
        ReverbIntensity = 0.3f;
        bEnableWindSounds = true;
        bEnableBirdSounds = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_DinosaurSoundProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    EAudio_DinosaurType DinosaurType = EAudio_DinosaurType::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float FootstepVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float VocalizationVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float FootstepRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float VocalizationRange = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    bool bCausesGroundShake = false;

    FAudio_DinosaurSoundProfile()
    {
        DinosaurType = EAudio_DinosaurType::TRex;
        FootstepVolume = 0.8f;
        VocalizationVolume = 1.0f;
        FootstepRange = 2000.0f;
        VocalizationRange = 5000.0f;
        bCausesGroundShake = false;
    }
};

/**
 * Audio Manager - Handles all audio systems in the prehistoric world
 * Manages environmental audio, dinosaur sounds, narrative triggers, and adaptive music
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudioManager : public AActor
{
    GENERATED_BODY()

public:
    AAudioManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core audio components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* EnvironmentAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* MusicAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* NarrativeAudioComponent;

    // Audio settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    FAudio_EnvironmentSettings CurrentEnvironmentSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    EAudio_IntensityLevel CurrentIntensity = EAudio_IntensityLevel::Ambient;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MusicVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float SFXVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float NarrativeVolume = 0.9f;

    // Dinosaur sound profiles
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TArray<FAudio_DinosaurSoundProfile> DinosaurSoundProfiles;

    // Audio references
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    class USoundCue* ForestAmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    class USoundCue* SwampAmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    class USoundCue* GrasslandAmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    class USoundCue* TRexFootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    class USoundCue* TRexRoarSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    class USoundCue* BackgroundMusicCalm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    class USoundCue* BackgroundMusicTense;

public:
    // Environment audio control
    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void SetEnvironmentType(EAudio_EnvironmentType NewEnvironment);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void SetIntensityLevel(EAudio_IntensityLevel NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void UpdateEnvironmentAudio();

    // Dinosaur audio
    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void PlayDinosaurFootstep(EAudio_DinosaurType DinosaurType, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void PlayDinosaurVocalization(EAudio_DinosaurType DinosaurType, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void TriggerGroundShake(FVector Location, float Intensity);

    // Narrative audio integration
    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void PlayNarrativeLine(const FString& AudioPath, FVector Location = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void StopNarrativeAudio();

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    bool IsNarrativeAudioPlaying() const;

    // Music control
    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void TransitionToMusic(class USoundCue* NewMusic, float FadeTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void SetMusicVolume(float Volume);

    // Volume controls
    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void SetSFXVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void SetNarrativeVolume(float Volume);

    // Spatial audio
    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void PlaySpatialSound(class USoundCue* Sound, FVector Location, float Volume = 1.0f);

    // System management
    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void InitializeAudioSystem();

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void ShutdownAudioSystem();

private:
    // Internal state
    bool bIsInitialized = false;
    float MusicFadeTimer = 0.0f;
    float MusicFadeDuration = 0.0f;
    class USoundCue* PendingMusic = nullptr;
    class USoundCue* CurrentMusic = nullptr;

    // Helper functions
    void UpdateMusicFade(float DeltaTime);
    void ApplyVolumeSettings();
    FAudio_DinosaurSoundProfile GetDinosaurProfile(EAudio_DinosaurType DinosaurType);
};

/**
 * Audio World Subsystem - Manages global audio state
 */
UCLASS()
class TRANSPERSONALGAME_API UAudioWorldSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Audio Subsystem")
    AAudioManager* GetAudioManager();

    UFUNCTION(BlueprintCallable, Category = "Audio Subsystem")
    void RegisterAudioManager(AAudioManager* Manager);

private:
    UPROPERTY()
    AAudioManager* AudioManagerInstance = nullptr;
};