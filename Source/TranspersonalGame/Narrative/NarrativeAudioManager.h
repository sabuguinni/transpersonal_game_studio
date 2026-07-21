#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "SharedTypes.h"
#include "NarrativeAudioManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_NarrativeAudioClip
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    ENarr_NarrativeContext Context;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    bool bIsLooping;

    FNarr_NarrativeAudioClip()
    {
        CharacterName = TEXT("");
        AudioURL = TEXT("");
        DialogueText = TEXT("");
        Context = ENarr_NarrativeContext::Exploration;
        Duration = 0.0f;
        bIsLooping = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_AudioTriggerZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Trigger")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Trigger")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Trigger")
    FString TriggerID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Trigger")
    TArray<FNarr_NarrativeAudioClip> AudioClips;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Trigger")
    bool bHasBeenTriggered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Trigger")
    bool bCanRetrigger;

    FNarr_AudioTriggerZone()
    {
        Location = FVector::ZeroVector;
        TriggerRadius = 1000.0f;
        TriggerID = TEXT("");
        bHasBeenTriggered = false;
        bCanRetrigger = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarrativeAudioManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarrativeAudioManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Audio management
    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void PlayNarrativeAudio(const FString& CharacterName, const FString& AudioURL, const FString& DialogueText);

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void StopNarrativeAudio();

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void RegisterAudioClip(const FNarr_NarrativeAudioClip& AudioClip);

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void CreateAudioTriggerZone(const FVector& Location, float Radius, const FString& TriggerID, const TArray<FNarr_NarrativeAudioClip>& Clips);

    // Trigger system
    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void CheckPlayerProximity(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    bool IsAudioPlaying() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    FString GetCurrentCharacterSpeaking() const;

    // Context-based audio
    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void PlayContextualAudio(ENarr_NarrativeContext Context, const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void SetNarrativeVolume(float Volume);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Narrative Audio")
    TArray<FNarr_NarrativeAudioClip> RegisteredAudioClips;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative Audio")
    TArray<FNarr_AudioTriggerZone> AudioTriggerZones;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative Audio")
    UAudioComponent* CurrentAudioComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative Audio")
    FString CurrentSpeaker;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative Audio")
    float NarrativeVolume;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative Audio")
    bool bIsCurrentlyPlaying;

private:
    void InitializeDefaultAudioClips();
    void TriggerAudioZone(const FNarr_AudioTriggerZone& Zone);
    FNarr_NarrativeAudioClip* FindAudioClipByContext(ENarr_NarrativeContext Context);
};