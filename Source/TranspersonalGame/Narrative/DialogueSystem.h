#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "Engine/DataTable.h"
#include "../SharedTypes.h"
#include "DialogueSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TSoftObjectPtr<USoundWave> VoiceAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> NextDialogueOptions;

    FNarr_DialogueEntry()
    {
        DialogueID = TEXT("");
        SpeakerName = TEXT("");
        DialogueText = TEXT("");
        Duration = 5.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_CharacterVoiceProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    FString VoiceDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    float PitchModifier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    float VolumeModifier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    TArray<TSoftObjectPtr<USoundWave>> VoiceLines;

    FNarr_CharacterVoiceProfile()
    {
        CharacterName = TEXT("");
        VoiceDescription = TEXT("");
        PitchModifier = 1.0f;
        VolumeModifier = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADialogueSystem : public AActor
{
    GENERATED_BODY()

public:
    ADialogueSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* AudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Database")
    UDataTable* DialogueDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Profiles")
    TMap<FString, FNarr_CharacterVoiceProfile> CharacterVoices;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    FNarr_DialogueEntry CurrentDialogue;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    bool bIsPlayingDialogue;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    float DialogueTimer;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool PlayDialogue(const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StopDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialogueActive() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueEntry GetCurrentDialogue() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    TArray<FString> GetAvailableDialogueOptions() const;

    UFUNCTION(BlueprintCallable, Category = "Voice System")
    void RegisterCharacterVoice(const FString& CharacterName, const FNarr_CharacterVoiceProfile& VoiceProfile);

    UFUNCTION(BlueprintCallable, Category = "Voice System")
    bool PlayCharacterLine(const FString& CharacterName, const FString& LineText, USoundWave* AudioClip = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerNarrativeEvent(const FString& EventID);

protected:
    UFUNCTION()
    void OnDialogueFinished();

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnDialogueStarted(const FNarr_DialogueEntry& DialogueEntry);

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnDialogueCompleted(const FNarr_DialogueEntry& DialogueEntry);

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnNarrativeEventTriggered(const FString& EventID);

private:
    FNarr_DialogueEntry* FindDialogueEntry(const FString& DialogueID);
    void ApplyVoiceProfile(const FString& CharacterName);
};