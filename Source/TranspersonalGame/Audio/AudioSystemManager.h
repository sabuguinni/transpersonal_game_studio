#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "Core/SharedTypes.h"
#include "AudioSystemManager.generated.h"

/**
 * TRANSPERSONAL GAME STUDIO - AUDIO SYSTEM MANAGER
 * Audio Agent #16
 * 
 * Manages all audio systems for the prehistoric survival game:
 * - Dynamic music system with fear/tension states
 * - 3D positioned ambient sounds (forest, wind, water)
 * - Dinosaur audio with distance-based intensity
 * - Player feedback sounds (heartbeat, breathing, footsteps)
 * - Environmental audio zones with smooth transitions
 */

UENUM(BlueprintType)
enum class EAudio_MusicState : uint8
{
    Calm = 0        UMETA(DisplayName = "Calm"),
    Tension = 1     UMETA(DisplayName = "Tension"),
    Danger = 2      UMETA(DisplayName = "Danger"),
    Combat = 3      UMETA(DisplayName = "Combat"),
    Victory = 4     UMETA(DisplayName = "Victory"),
    Death = 5       UMETA(DisplayName = "Death")
};

UENUM(BlueprintType)
enum class EAudio_AmbienceType : uint8
{
    Forest = 0      UMETA(DisplayName = "Forest"),
    Grassland = 1   UMETA(DisplayName = "Grassland"),
    Desert = 2      UMETA(DisplayName = "Desert"),
    Swamp = 3       UMETA(DisplayName = "Swamp"),
    Mountains = 4   UMETA(DisplayName = "Mountains"),
    River = 5       UMETA(DisplayName = "River"),
    Cave = 6        UMETA(DisplayName = "Cave"),
    Night = 7       UMETA(DisplayName = "Night")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SoundSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MusicVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float SFXVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AmbienceVolume = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float VoiceVolume = 0.9f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bEnableDynamicRange = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MaxAudioDistance = 5000.0f;

    FAudio_SoundSettings()
    {
        MasterVolume = 1.0f;
        MusicVolume = 0.7f;
        SFXVolume = 0.8f;
        AmbienceVolume = 0.6f;
        VoiceVolume = 0.9f;
        bEnableDynamicRange = true;
        MaxAudioDistance = 5000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_MusicLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    TSoftObjectPtr<USoundCue> SoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    EAudio_MusicState MusicState = EAudio_MusicState::Calm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    float FadeOutTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    bool bIsLooping = true;

    FAudio_MusicLayer()
    {
        MusicState = EAudio_MusicState::Calm;
        Volume = 1.0f;
        FadeInTime = 2.0f;
        FadeOutTime = 2.0f;
        bIsLooping = true;
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

    // Music System
    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void SetMusicState(EAudio_MusicState NewState, float TransitionTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    EAudio_MusicState GetCurrentMusicState() const { return CurrentMusicState; }

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void StopAllMusic(float FadeOutTime = 2.0f);

    // Ambience System
    UFUNCTION(BlueprintCallable, Category = "Audio|Ambience")
    void SetAmbienceType(EAudio_AmbienceType NewType, float TransitionTime = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambience")
    void PlayAmbienceAtLocation(EAudio_AmbienceType AmbienceType, FVector Location, float Radius = 1000.0f);

    // Sound Effects
    UFUNCTION(BlueprintCallable, Category = "Audio|SFX")
    void PlaySoundAtLocation(USoundCue* SoundCue, FVector Location, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|SFX")
    void PlayDinosaurSound(const FString& DinosaurType, FVector Location, float Intensity = 1.0f);

    // Player Audio Feedback
    UFUNCTION(BlueprintCallable, Category = "Audio|Player")
    void SetPlayerFearLevel(float FearLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio|Player")
    void PlayPlayerHeartbeat(float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Player")
    void PlayPlayerBreathing(float Intensity = 1.0f);

    // Settings
    UFUNCTION(BlueprintCallable, Category = "Audio|Settings")
    void ApplyAudioSettings(const FAudio_SoundSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Audio|Settings")
    FAudio_SoundSettings GetAudioSettings() const { return AudioSettings; }

    // Distance-based Audio
    UFUNCTION(BlueprintCallable, Category = "Audio|Distance")
    float CalculateVolumeByDistance(FVector SourceLocation, FVector ListenerLocation, float MaxDistance = 1000.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Distance")
    void UpdateListenerPosition(FVector NewPosition, FRotator NewRotation);

protected:
    // Music System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    TArray<FAudio_MusicLayer> MusicLayers;

    UPROPERTY()
    TMap<EAudio_MusicState, UAudioComponent*> MusicComponents;

    UPROPERTY()
    EAudio_MusicState CurrentMusicState = EAudio_MusicState::Calm;

    UPROPERTY()
    EAudio_MusicState TargetMusicState = EAudio_MusicState::Calm;

    // Ambience System
    UPROPERTY()
    TMap<EAudio_AmbienceType, UAudioComponent*> AmbienceComponents;

    UPROPERTY()
    EAudio_AmbienceType CurrentAmbienceType = EAudio_AmbienceType::Forest;

    // Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FAudio_SoundSettings AudioSettings;

    // Player Audio
    UPROPERTY()
    UAudioComponent* HeartbeatComponent;

    UPROPERTY()
    UAudioComponent* BreathingComponent;

    UPROPERTY()
    float CurrentFearLevel = 0.0f;

    // Listener Position
    UPROPERTY()
    FVector ListenerPosition = FVector::ZeroVector;

    UPROPERTY()
    FRotator ListenerRotation = FRotator::ZeroRotator;

private:
    void InitializeMusicSystem();
    void InitializeAmbienceSystem();
    void InitializePlayerAudio();
    void UpdateMusicTransition(float DeltaTime);
    void UpdatePlayerAudioFeedback(float DeltaTime);

    UPROPERTY()
    float MusicTransitionTimer = 0.0f;

    UPROPERTY()
    float MusicTransitionDuration = 2.0f;

    UPROPERTY()
    bool bIsMusicTransitioning = false;
};