#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "../SharedTypes.h"
#include "DialogueManager.generated.h"

// Forward declarations
class UDialogueNode;
class ANarrativeNPC;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString VoiceClipPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> PlayerResponseOptions;

    FNarr_DialogueLine()
    {
        SpeakerName = TEXT("");
        DialogueText = FText::GetEmpty();
        VoiceClipPath = TEXT("");
        DisplayDuration = 3.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueTree
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString TreeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsQuestRelated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AssociatedQuestID;

    FNarr_DialogueTree()
    {
        TreeID = TEXT("");
        NPCName = TEXT("");
        bIsQuestRelated = false;
        AssociatedQuestID = TEXT("");
    }
};

UENUM(BlueprintType)
enum class ENarr_StoryPhase : uint8
{
    Introduction     UMETA(DisplayName = "Introduction"),
    EarlyExploration UMETA(DisplayName = "Early Exploration"),
    FirstHunt        UMETA(DisplayName = "First Hunt"),
    TribeBuilding    UMETA(DisplayName = "Tribe Building"),
    ApexPredator     UMETA(DisplayName = "Apex Predator"),
    Mastery          UMETA(DisplayName = "Mastery")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogueStarted, const FString&, NPCName, const FString&, DialogueTreeID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueEnded, const FString&, DialogueTreeID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStoryPhaseChanged, ENarr_StoryPhase, OldPhase, ENarr_StoryPhase, NewPhase);

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDialogueManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UDialogueManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Dialogue management
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StartDialogue(const FString& NPCName, const FString& DialogueTreeID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsDialogueActive() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueLine GetCurrentDialogueLine() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceDialogue(int32 ResponseIndex = 0);

    // Dialogue tree management
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterDialogueTree(const FNarr_DialogueTree& DialogueTree);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueTree GetDialogueTree(const FString& TreeID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FString> GetAvailableDialogues(const FString& NPCName) const;

    // Story progression
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetStoryPhase(ENarr_StoryPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    ENarr_StoryPhase GetCurrentStoryPhase() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void UpdateStoryProgress(const FString& EventName, float ProgressValue);

    // Quest integration
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnQuestCompleted(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnQuestStarted(const FString& QuestID);

    // Character development tracking
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TrackPlayerAction(const FString& ActionType, const FString& ActionData);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    int32 GetPlayerActionCount(const FString& ActionType) const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Narrative")
    FOnDialogueStarted OnDialogueStarted;

    UPROPERTY(BlueprintAssignable, Category = "Narrative")
    FOnDialogueEnded OnDialogueEnded;

    UPROPERTY(BlueprintAssignable, Category = "Narrative")
    FOnStoryPhaseChanged OnStoryPhaseChanged;

protected:
    // Current dialogue state
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    bool bIsDialogueActive;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    FString CurrentNPCName;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    FString CurrentDialogueTreeID;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    int32 CurrentDialogueIndex;

    // Dialogue storage
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    TMap<FString, FNarr_DialogueTree> DialogueTrees;

    // Story progression
    UPROPERTY(BlueprintReadOnly, Category = "Story")
    ENarr_StoryPhase CurrentStoryPhase;

    UPROPERTY(BlueprintReadOnly, Category = "Story")
    TMap<FString, float> StoryProgressValues;

    // Player action tracking
    UPROPERTY(BlueprintReadOnly, Category = "Character")
    TMap<FString, int32> PlayerActionCounts;

    // Helper methods
    void InitializeDefaultDialogues();
    void CreateSurvivalDialogues();
    void CreateQuestDialogues();
    bool ValidateDialogueTree(const FNarr_DialogueTree& DialogueTree) const;
};