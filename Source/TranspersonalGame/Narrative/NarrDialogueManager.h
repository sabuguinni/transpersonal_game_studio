#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Components/ActorComponent.h"
#include "../SharedTypes.h"
#include "NarrDialogueManager.generated.h"

// Forward declarations
class UNarrDialogueComponent;
class UNarrQuestManager;

UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Inactive,
    WaitingForPlayer,
    NPCSpeaking,
    PlayerChoosing,
    Completed,
    Interrupted
};

UENUM(BlueprintType)
enum class ENarr_DialogueTrigger : uint8
{
    Proximity,
    Interaction,
    QuestStart,
    QuestComplete,
    ItemGiven,
    SpecialEvent
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioAssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> PlayerResponseOptions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> NextDialogueIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bRequiresQuestComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString RequiredQuestID;

    FNarr_DialogueLine()
    {
        SpeakerName = TEXT("");
        DialogueText = FText::GetEmpty();
        AudioAssetPath = TEXT("");
        Duration = 3.0f;
        bRequiresQuestComplete = false;
        RequiredQuestID = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueTree
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueTrigger TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 MaxRepetitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float CooldownTime;

    FNarr_DialogueTree()
    {
        DialogueID = TEXT("");
        NPCName = TEXT("");
        TriggerType = ENarr_DialogueTrigger::Proximity;
        bRepeatable = true;
        MaxRepetitions = -1;
        CooldownTime = 0.0f;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogueStarted, const FString&, DialogueID, const FString&, NPCName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueEnded, const FString&, DialogueID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDialogueLineSpoken, const FString&, SpeakerName, const FText&, DialogueText, float, Duration);

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarrDialogueManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarrDialogueManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core dialogue functions
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool StartDialogue(const FString& DialogueID, AActor* NPCActor, AActor* PlayerActor);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SelectPlayerResponse(int32 ResponseIndex);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AdvanceDialogue();

    // Dialogue management
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void RegisterDialogueTree(const FNarr_DialogueTree& DialogueTree);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool HasDialogue(const FString& DialogueID) const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueTree GetDialogueTree(const FString& DialogueID) const;

    // State queries
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dialogue")
    bool IsDialogueActive() const { return CurrentState != ENarr_DialogueState::Inactive; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dialogue")
    ENarr_DialogueState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dialogue")
    FString GetCurrentDialogueID() const { return CurrentDialogueID; }

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
    FOnDialogueStarted OnDialogueStarted;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
    FOnDialogueEnded OnDialogueEnded;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
    FOnDialogueLineSpoken OnDialogueLineSpoken;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue State")
    ENarr_DialogueState CurrentState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue State")
    FString CurrentDialogueID;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue State")
    int32 CurrentLineIndex;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue State")
    TWeakObjectPtr<AActor> CurrentNPCActor;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue State")
    TWeakObjectPtr<AActor> CurrentPlayerActor;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue Data")
    TMap<FString, FNarr_DialogueTree> DialogueTrees;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue Data")
    TMap<FString, int32> DialogueUsageCounts;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue Data")
    TMap<FString, float> DialogueCooldowns;

private:
    void ProcessCurrentLine();
    bool CanStartDialogue(const FString& DialogueID) const;
    void UpdateCooldowns(float DeltaTime);
    
    FTimerHandle CooldownTimerHandle;
};