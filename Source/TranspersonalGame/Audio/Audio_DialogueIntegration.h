#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Audio_DialogueIntegration.generated.h"

// Forward declarations
class UNarr_DialogueSystem;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_DialogueAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Audio")
    TSoftObjectPtr<USoundWave> AudioClip;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Audio")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Audio")
    float Volume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Audio")
    bool bIs3D;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Audio")
    float AttenuationRadius;

    FAudio_DialogueAudioData()
    {
        Duration = 0.0f;
        Volume = 1.0f;
        bIs3D = true;
        AttenuationRadius = 1000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_VoiceProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Profile")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Profile")
    TMap<FString, FAudio_DialogueAudioData> DialogueAudioMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Profile")
    float BasePitch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Profile")
    float BaseVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Profile")
    bool bUseDistanceAttenuation;

    FAudio_VoiceProfile()
    {
        BasePitch = 1.0f;
        BaseVolume = 1.0f;
        bUseDistanceAttenuation = true;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_DialogueIntegration : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_DialogueIntegration();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Voice profile management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Audio")
    TMap<FString, FAudio_VoiceProfile> VoiceProfiles;

    // Audio components for dialogue playback
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> DialogueAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> NarrationAudioComponent;

    // Current dialogue state
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue State")
    bool bIsPlayingDialogue;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue State")
    FString CurrentSpeaker;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue State")
    float CurrentDialogueTime;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue State")
    float CurrentDialogueDuration;

    // Audio mixing settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Mixing")
    float DialogueVolumeMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Mixing")
    float BackgroundMusicDucking;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Mixing")
    float EnvironmentSoundDucking;

    // Dialogue audio methods
    UFUNCTION(BlueprintCallable, Category = "Dialogue Audio")
    bool PlayDialogueAudio(const FString& SpeakerName, const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue Audio")
    void StopDialogueAudio();

    UFUNCTION(BlueprintCallable, Category = "Dialogue Audio")
    void PauseDialogueAudio();

    UFUNCTION(BlueprintCallable, Category = "Dialogue Audio")
    void ResumeDialogueAudio();

    // Voice profile management
    UFUNCTION(BlueprintCallable, Category = "Voice Profiles")
    void RegisterVoiceProfile(const FString& CharacterName, const FAudio_VoiceProfile& VoiceProfile);

    UFUNCTION(BlueprintCallable, Category = "Voice Profiles")
    bool GetVoiceProfile(const FString& CharacterName, FAudio_VoiceProfile& OutVoiceProfile);

    UFUNCTION(BlueprintCallable, Category = "Voice Profiles")
    void AddDialogueAudioToProfile(const FString& CharacterName, const FString& DialogueID, const FAudio_DialogueAudioData& AudioData);

    // Audio mixing control
    UFUNCTION(BlueprintCallable, Category = "Audio Mixing")
    void SetDialogueVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Mixing")
    void DuckBackgroundAudio(float DuckingAmount);

    UFUNCTION(BlueprintCallable, Category = "Audio Mixing")
    void RestoreBackgroundAudio();

    // Integration with narrative system
    UFUNCTION(BlueprintCallable, Category = "Narrative Integration")
    void OnDialogueStarted(const FString& SpeakerName, const FString& DialogueText);

    UFUNCTION(BlueprintCallable, Category = "Narrative Integration")
    void OnDialogueFinished();

    UFUNCTION(BlueprintCallable, Category = "Narrative Integration")
    void OnDialogueNodeChanged(const FString& NewSpeaker, const FString& NewDialogueID);

    // Spatial audio for NPCs
    UFUNCTION(BlueprintCallable, Category = "Spatial Audio")
    void UpdateSpatialAudioPosition(const FVector& NewPosition);

    UFUNCTION(BlueprintCallable, Category = "Spatial Audio")
    void SetSpatialAudioAttenuation(float MaxDistance, float FalloffDistance);

protected:
    // Internal audio management
    void InitializeAudioComponents();
    void SetupVoiceProfiles();
    void UpdateDialoguePlayback(float DeltaTime);
    bool LoadDialogueAudio(const FString& AudioPath, USoundWave*& OutSoundWave);

    // Audio ducking system
    void ApplyAudioDucking();
    void RemoveAudioDucking();

    // Reference to dialogue system
    UPROPERTY()
    TWeakObjectPtr<UNarr_DialogueSystem> DialogueSystemRef;

    // Audio timing
    float DialogueStartTime;
    bool bDialogueAudioLoaded;
};