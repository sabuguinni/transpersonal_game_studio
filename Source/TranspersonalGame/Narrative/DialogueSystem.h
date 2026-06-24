#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DialogueSystem.generated.h"

// ============================================================
// Narrative & Dialogue Agent #15 — DialogueSystem
// Dialogue tree component for NPC conversation in survival game
// ============================================================

UENUM(BlueprintType)
enum class ENarr_DialogueSpeaker : uint8
{
    Player          UMETA(DisplayName = "Player"),
    TribeElder      UMETA(DisplayName = "Tribe Elder"),
    Scout           UMETA(DisplayName = "Scout"),
    Stranger        UMETA(DisplayName = "Stranger"),
    NarratorInner   UMETA(DisplayName = "Inner Narrator")
};

UENUM(BlueprintType)
enum class ENarr_DialogueCondition : uint8
{
    None            UMETA(DisplayName = "None"),
    HasItem         UMETA(DisplayName = "Has Item"),
    QuestActive     UMETA(DisplayName = "Quest Active"),
    QuestComplete   UMETA(DisplayName = "Quest Complete"),
    FirstMeeting    UMETA(DisplayName = "First Meeting"),
    NearDanger      UMETA(DisplayName = "Near Danger")
};

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_DialogueSpeaker Speaker = ENarr_DialogueSpeaker::TribeElder;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FText LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    float DisplayDuration = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    bool bWaitForInput = true;
};

USTRUCT(BlueprintType)
struct FNarr_DialogueChoice
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FText ChoiceText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    int32 NextNodeIndex = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_DialogueCondition RequiredCondition = ENarr_DialogueCondition::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName ConditionParam;
};

USTRUCT(BlueprintType)
struct FNarr_DialogueNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    int32 NodeIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueChoice> Choices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    bool bIsEndNode = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName QuestToActivate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName QuestToComplete;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNarr_OnDialogueStarted, FName, DialogueID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNarr_OnDialogueEnded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNarr_OnNodeAdvanced, int32, NewNodeIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNarr_OnChoiceSelected, int32, ChoiceIndex);

UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent), DisplayName = "Dialogue System")
class TRANSPERSONALGAME_API UDialogueSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UDialogueSystem();

    // ---- Delegates ----
    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FNarr_OnDialogueStarted OnDialogueStarted;

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FNarr_OnDialogueEnded OnDialogueEnded;

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FNarr_OnNodeAdvanced OnNodeAdvanced;

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FNarr_OnChoiceSelected OnChoiceSelected;

    // ---- Dialogue Data ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Data")
    FName DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Data")
    TArray<FNarr_DialogueNode> DialogueNodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Data")
    ENarr_DialogueSpeaker NPCSpeaker = ENarr_DialogueSpeaker::TribeElder;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Data")
    float InteractionRadius = 300.0f;

    // ---- State ----
    UPROPERTY(BlueprintReadOnly, Category = "Narrative|State", meta = (AllowPrivateAccess = "true"))
    bool bDialogueActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|State", meta = (AllowPrivateAccess = "true"))
    int32 CurrentNodeIndex = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|State", meta = (AllowPrivateAccess = "true"))
    int32 CurrentLineIndex = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|State", meta = (AllowPrivateAccess = "true"))
    bool bHasSpokenBefore = false;

    // ---- Functions ----
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void StartDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void AdvanceLine();

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void SelectChoice(int32 ChoiceIndex);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintPure, Category = "Narrative|Dialogue")
    bool IsDialogueActive() const { return bDialogueActive; }

    UFUNCTION(BlueprintPure, Category = "Narrative|Dialogue")
    FNarr_DialogueLine GetCurrentLine() const;

    UFUNCTION(BlueprintPure, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueChoice> GetCurrentChoices() const;

    UFUNCTION(BlueprintPure, Category = "Narrative|Dialogue")
    bool IsAtChoicePoint() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void InitializeTribeElderDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void InitializeScoutDialogue();

    UFUNCTION(CallInEditor, Category = "Narrative|Debug")
    void DebugPrintDialogueTree();

protected:
    virtual void BeginPlay() override;

private:
    void GoToNode(int32 NodeIndex);
    bool CheckCondition(ENarr_DialogueCondition Condition, FName Param) const;
};
