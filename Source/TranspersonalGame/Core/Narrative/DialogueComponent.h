#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "NarrativeManager.h"
#include "../SharedTypes.h"
#include "DialogueComponent.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueOption
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText OptionText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString NextDialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> RequiredFlags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> SetFlags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bEndDialogue;

    FNarr_DialogueOption()
    {
        OptionText = FText::GetEmpty();
        NextDialogueID = TEXT("");
        bEndDialogue = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_NPCDialogue
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TSoftObjectPtr<USoundCue> VoiceClip;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueOption> PlayerOptions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> RequiredFlags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float InteractionRange;

    FNarr_NPCDialogue()
    {
        DialogueID = TEXT("");
        NPCName = TEXT("");
        DialogueText = FText::GetEmpty();
        InteractionRange = 200.0f;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueInteractionAvailable, AActor*, InteractingActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueInteractionUnavailable, AActor*, InteractingActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogueOptionSelected, int32, OptionIndex, const FString&, NextDialogueID);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDialogueComponent();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Dialogue Management
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StartDialogue(AActor* InteractingActor);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SelectDialogueOption(int32 OptionIndex);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool CanStartDialogue(AActor* InteractingActor) const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsInDialogue() const { return bIsInDialogue; }

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SetCurrentDialogue(const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AddDialogue(const FNarr_NPCDialogue& NewDialogue);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_NPCDialogue GetCurrentDialogueData() const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Dialogue")
    FOnDialogueInteractionAvailable OnDialogueInteractionAvailable;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue")
    FOnDialogueInteractionUnavailable OnDialogueInteractionUnavailable;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue")
    FOnDialogueOptionSelected OnDialogueOptionSelected;

protected:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_NPCDialogue> AvailableDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DefaultDialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float InteractionCheckInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bAutoDetectPlayers;

    // Runtime State
    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsInDialogue;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FString CurrentDialogueID;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    AActor* CurrentInteractingActor;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FNarr_NPCDialogue CurrentDialogueData;

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* VoiceAudioComponent;

    // Internal Functions
    void CheckForNearbyPlayers();
    bool CheckDialogueConditions(const FNarr_NPCDialogue& Dialogue) const;
    void ProcessDialogueFlags(const TArray<FString>& Flags);
    void PlayVoiceClip(TSoftObjectPtr<USoundCue> VoiceClip);

private:
    FTimerHandle InteractionCheckTimer;
    TSet<AActor*> NearbyActors;
    UNarrativeManager* NarrativeManager;
};