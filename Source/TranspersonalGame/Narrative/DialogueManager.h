#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DialogueManager.generated.h"

// ============================================================
// Enums — must be at global scope (UE5 UHT rule)
// ============================================================

UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Active      UMETA(DisplayName = "Active"),
    Waiting     UMETA(DisplayName = "Waiting For Choice"),
    Completed   UMETA(DisplayName = "Completed"),
    Interrupted UMETA(DisplayName = "Interrupted")
};

UENUM(BlueprintType)
enum class ENarr_SpeakerRole : uint8
{
    TribeElder  UMETA(DisplayName = "Tribe Elder"),
    Scout       UMETA(DisplayName = "Scout"),
    Hunter      UMETA(DisplayName = "Hunter"),
    Crafter     UMETA(DisplayName = "Crafter"),
    Narrator    UMETA(DisplayName = "Narrator")
};

// ============================================================
// Structs — must be at global scope (UE5 UHT rule)
// ============================================================

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString SpeakerID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_SpeakerRole SpeakerRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AnimationHint;

    FNarr_DialogueLine()
        : SpeakerRole(ENarr_SpeakerRole::Narrator)
        , DisplayDuration(4.0f)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_DialogueChoice
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString ChoiceText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString NextNodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString RequiredQuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bUnlocksQuest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString UnlockedQuestID;

    FNarr_DialogueChoice()
        : bUnlocksQuest(false)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_DialogueNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString NodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueChoice> Choices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AutoNextNodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bIsEndNode;

    FNarr_DialogueNode()
        : bIsEndNode(false)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_DialogueTree
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString TreeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString NPCID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString RootNodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueNode> Nodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LinkedQuestID;
};

// ============================================================
// ADialogueManager — main dialogue system actor
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADialogueManager : public AActor
{
    GENERATED_BODY()

public:
    ADialogueManager();

    // ---- State ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative")
    ENarr_DialogueState CurrentState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative")
    FString ActiveTreeID;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative")
    FString ActiveNodeID;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative")
    int32 ActiveLineIndex;

    // ---- Registered dialogue trees ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueTree> DialogueTrees;

    // ---- Public API ----
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool StartDialogue(const FString& TreeID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool AdvanceLine();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool MakeChoice(int32 ChoiceIndex);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueLine GetCurrentLine() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FNarr_DialogueChoice> GetCurrentChoices() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsDialogueActive() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterDialogueTree(const FNarr_DialogueTree& Tree);

protected:
    virtual void BeginPlay() override;

private:
    void InitialiseBuiltInDialogueTrees();
    FNarr_DialogueNode* FindNode(const FString& TreeID, const FString& NodeID);
    FNarr_DialogueTree* FindTree(const FString& TreeID);

    // Built-in NPC dialogue trees
    void BuildElderKaelTree();
    void BuildScoutMiraTree();
};
