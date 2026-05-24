#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "SurvivalDialogueComponent.generated.h"

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ESurvivalContext Context;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString AudioURL;

    FNarr_DialogueLine()
    {
        SpeakerName = TEXT("");
        DialogueText = TEXT("");
        DisplayDuration = 3.0f;
        Context = ESurvivalContext::Neutral;
        AudioURL = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FNarr_DialogueSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SequenceName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triggers")
    ESurvivalThreat TriggerThreat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triggers")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conditions")
    bool bRequiresLineOfSight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conditions")
    float MinPlayerHealth;

    FNarr_DialogueSequence()
    {
        SequenceName = TEXT("");
        TriggerThreat = ESurvivalThreat::None;
        TriggerRadius = 500.0f;
        bRequiresLineOfSight = false;
        MinPlayerHealth = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API USurvivalDialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USurvivalDialogueComponent();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    TArray<FNarr_DialogueSequence> DialogueSequences;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TMap<FString, FString> CharacterVoiceURLs;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    bool bIsDialogueActive;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    int32 CurrentSequenceIndex;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    int32 CurrentLineIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float DialogueCheckInterval;

    FTimerHandle DialogueTimerHandle;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StartDialogueSequence(const FString& SequenceName);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StopCurrentDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AddDialogueSequence(const FNarr_DialogueSequence& NewSequence);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialogueActive() const { return bIsDialogueActive; }

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueLine GetCurrentDialogueLine() const;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetCharacterVoiceURL(const FString& CharacterName, const FString& AudioURL);

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnDialogueStarted(const FNarr_DialogueSequence& Sequence);

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnDialogueLineChanged(const FNarr_DialogueLine& Line);

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnDialogueEnded();

protected:
    UFUNCTION()
    void CheckDialogueTriggers();

    UFUNCTION()
    void AdvanceDialogue();

    bool ShouldTriggerSequence(const FNarr_DialogueSequence& Sequence) const;
    
    class ATranspersonalCharacter* GetPlayerCharacter() const;
    
    float GetDistanceToPlayer() const;
    
    bool HasLineOfSightToPlayer() const;
};