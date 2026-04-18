#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Core/SharedTypes.h"
#include "AudioSystemManager.generated.h"

/**
 * TRANSPERSONAL GAME STUDIO - AUDIO SYSTEM MANAGER
 * Audio Agent #16
 * 
 * Manages all audio systems in the prehistoric survival game:
 * - Ambient soundscapes (forest, wind, prehistoric atmosphere)
 * - Dinosaur audio (footsteps, roars, breathing)
 * - Player feedback (heartbeat, survival warnings)
 * - Dynamic music system based on danger level
 * - 3D spatial audio for immersive prehistoric world
 */

UENUM(BlueprintType)
enum class EAudio_SoundType : uint8
{
    Ambient = 0         UMETA(DisplayName = "Ambient"),
    Dinosaur = 1        UMETA(DisplayName = "Dinosaur"),
    Player = 2          UMETA(DisplayName = "Player"),
    Environment = 3     UMETA(DisplayName = "Environment"),
    Music = 4           UMETA(DisplayName = "Music"),
    UI = 5              UMETA(DisplayName = "UI")
};

UENUM(BlueprintType)
enum class EAudio_DangerLevel : uint8
{
    Safe = 0            UMETA(DisplayName = "Safe"),
    Cautious = 1        UMETA(DisplayName = "Cautious"),
    Threatened = 2      UMETA(DisplayName = "Threatened"),
    Danger = 3          UMETA(DisplayName = "Danger"),
    Critical = 4        UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SoundSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Pitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeInTime = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeOutTime = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bLoop = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool b3D = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AttenuationRadius = 1000.0f;

    FAudio_SoundSettings()
    {
        Volume = 1.0f;
        Pitch = 1.0f;
        FadeInTime = 0.5f;
        FadeOutTime = 0.5f;
        bLoop = false;
        b3D = true;
        AttenuationRadius = 1000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_DynamicMusicState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    EAudio_DangerLevel CurrentDangerLevel = EAudio_DangerLevel::Safe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    float IntensityLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    float TransitionTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    bool bInCombat = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    float HeartRate = 60.0f;

    FAudio_DynamicMusicState()
    {
        CurrentDangerLevel = EAudio_DangerLevel::Safe;
        IntensityLevel = 0.0f;
        TransitionTime = 2.0f;
        bInCombat = false;
        HeartRate = 60.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudioSystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UAudioSystemManager();

    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core Audio Management
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlaySound(USoundCue* SoundCue, FVector Location, const FAudio_SoundSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlaySound2D(USoundCue* SoundCue, const FAudio_SoundSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StopSound(USoundCue* SoundCue);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StopAllSounds();

    // Ambient Audio System
    UFUNCTION(BlueprintCallable, Category = "Ambient Audio")
    void StartAmbientSoundscape(EEng_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Ambient Audio")
    void UpdateAmbientIntensity(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Ambient Audio")
    void TransitionToNewBiome(EEng_BiomeType NewBiome, float TransitionTime = 3.0f);

    // Dinosaur Audio System
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void PlayDinosaurFootstep(FVector Location, float DinosaurSize);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void PlayDinosaurRoar(FVector Location, EAudio_DangerLevel ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void PlayDinosaurBreathing(FVector Location, bool bAggressive);

    // Player Audio Feedback
    UFUNCTION(BlueprintCallable, Category = "Player Audio")
    void UpdateHeartbeat(float HeartRate);

    UFUNCTION(BlueprintCallable, Category = "Player Audio")
    void PlaySurvivalWarning(EAudio_DangerLevel DangerLevel);

    UFUNCTION(BlueprintCallable, Category = "Player Audio")
    void PlayPlayerFootstep(FVector Location, bool bSneaking);

    // Dynamic Music System
    UFUNCTION(BlueprintCallable, Category = "Dynamic Music")
    void UpdateMusicState(const FAudio_DynamicMusicState& NewState);

    UFUNCTION(BlueprintCallable, Category = "Dynamic Music")
    void SetDangerLevel(EAudio_DangerLevel DangerLevel);

    UFUNCTION(BlueprintCallable, Category = "Dynamic Music")
    void StartCombatMusic();

    UFUNCTION(BlueprintCallable, Category = "Dynamic Music")
    void EndCombatMusic();

    // Audio Settings
    UFUNCTION(BlueprintCallable, Category = "Audio Settings")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Settings")
    void SetSFXVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Settings")
    void SetMusicVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Settings")
    void SetAmbientVolume(float Volume);

protected:
    // Audio Components
    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    TMap<EAudio_SoundType, class UAudioComponent*> AudioComponents;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* AmbientAudioComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* MusicAudioComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* HeartbeatAudioComponent;

    // Audio Assets
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound Assets")
    TMap<EEng_BiomeType, USoundCue*> BiomeAmbientSounds;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound Assets")
    TMap<EAudio_DangerLevel, USoundCue*> MusicTracks;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound Assets")
    USoundCue* DinosaurFootstepSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound Assets")
    USoundCue* DinosaurRoarSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound Assets")
    USoundCue* PlayerHeartbeatSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound Assets")
    USoundCue* PlayerFootstepSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound Assets")
    USoundCue* SurvivalWarningSound;

    // Audio State
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    FAudio_DynamicMusicState CurrentMusicState;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EEng_BiomeType CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float MasterVolume;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float SFXVolume;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float MusicVolume;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float AmbientVolume;

private:
    // Internal Audio Management
    void InitializeAudioComponents();
    void UpdateAudioMixing();
    void ProcessMusicTransition();
    void CalculateAudioAttenuation(FVector ListenerLocation, FVector SoundLocation, float& OutVolume, float& OutPitch);
};