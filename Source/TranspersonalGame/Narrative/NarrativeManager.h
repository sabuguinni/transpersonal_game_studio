#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TriggerBox.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "SharedTypes.h"
#include "NarrativeManager.generated.h"

class UNarrativeDialogueSystem;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_NarrativeEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString EventID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bIsOneShot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bHasBeenTriggered;

    FNarr_NarrativeEvent()
    {
        EventID = TEXT("");
        DialogueText = TEXT("");
        CharacterName = TEXT("");
        TriggerRadius = 500.0f;
        bIsOneShot = true;
        bHasBeenTriggered = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString Speaker;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString Text;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TSoftObjectPtr<USoundCue> VoiceClip;

    FNarr_DialogueEntry()
    {
        Speaker = TEXT("");
        Text = TEXT("");
        DisplayDuration = 3.0f;
        VoiceClip = nullptr;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarrativeManager : public AActor
{
    GENERATED_BODY()

public:
    ANarrativeManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UNarrativeDialogueSystem* DialogueSystem;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* NarratorAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Events")
    TArray<FNarr_NarrativeEvent> NarrativeEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueEntry> ActiveDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float PlayerDetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bNarrativeSystemActive;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    class ATranspersonalCharacter* PlayerCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float CurrentDialogueTimer;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    int32 CurrentDialogueIndex;

public:
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerNarrativeEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StartDialogueSequence(const TArray<FNarr_DialogueEntry>& DialogueEntries);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StopCurrentDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsDialoguePlaying() const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Narrative")
    void OnNarrativeEventTriggered(const FString& EventID, const FString& DialogueText);

    UFUNCTION(BlueprintImplementableEvent, Category = "Narrative")
    void OnDialogueStarted(const FString& Speaker, const FString& Text);

    UFUNCTION(BlueprintImplementableEvent, Category = "Narrative")
    void OnDialogueEnded();

protected:
    UFUNCTION()
    void CheckPlayerProximity();

    UFUNCTION()
    void UpdateDialoguePlayback(float DeltaTime);

    UFUNCTION()
    void InitializeNarrativeEvents();

    UFUNCTION()
    void FindPlayerCharacter();
};