#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DialogueSystem.generated.h"

// ============================================================
// Enums — global scope (RULE 1)
// ============================================================

UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Active      UMETA(DisplayName = "Active"),
    Completed   UMETA(DisplayName = "Completed"),
    Locked      UMETA(DisplayName = "Locked")
};

UENUM(BlueprintType)
enum class ENarr_SpeakerType : uint8
{
    Player      UMETA(DisplayName = "Player"),
    Elder       UMETA(DisplayName = "Elder"),
    Crafter     UMETA(DisplayName = "Crafter"),
    Scout       UMETA(DisplayName = "Scout"),
    Narrator    UMETA(DisplayName = "Narrator")
};

// ============================================================
// Structs — global scope (RULE 1)
// ============================================================

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_SpeakerType Speaker = ENarr_SpeakerType::Narrator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration = 4.0f;

    FNarr_DialogueLine() {}
    FNarr_DialogueLine(ENarr_SpeakerType InSpeaker, const FString& InText, float InDuration = 4.0f)
        : Speaker(InSpeaker), LineText(InText), DisplayDuration(InDuration) {}
};

USTRUCT(BlueprintType)
struct FNarr_DialogueNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString NodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> Lines;

    // IDs of nodes that follow this one (player choices or auto-advance)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FString> NextNodeIDs;

    // Player-facing choice labels (empty = auto-advance)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FString> ChoiceLabels;

    // Quest ID to activate when this node completes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString ActivatesQuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueState State = ENarr_DialogueState::Idle;

    FNarr_DialogueNode() {}
};

USTRUCT(BlueprintType)
struct FNarr_DialogueTree
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString TreeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString EntryNodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueNode> Nodes;

    FNarr_DialogueTree() {}
};

// ============================================================
// ADialogueSystem — manages all NPC dialogue trees
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADialogueSystem : public AActor
{
    GENERATED_BODY()

public:
    ADialogueSystem();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ---- Active dialogue state ----
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    bool bDialogueActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    FString ActiveTreeID;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    FString ActiveNodeID;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    int32 ActiveLineIndex = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    float LineTimer = 0.0f;

    // ---- All registered dialogue trees ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueTree> DialogueTrees;

    // ---- Public API ----

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StartDialogue(const FString& TreeID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceDialogue(int32 ChoiceIndex = 0);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueLine GetCurrentLine() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FString> GetCurrentChoices() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsDialogueActive() const { return bDialogueActive; }

    // ---- Tree registration ----
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterDialogueTree(const FNarr_DialogueTree& Tree);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void InitialiseDefaultTrees();

private:
    FNarr_DialogueNode* FindNode(const FString& TreeID, const FString& NodeID);
    FNarr_DialogueTree* FindTree(const FString& TreeID);

    void BuildElderKaelTree();
    void BuildCrafterMiraTree();
    void BuildScoutTaraTree();
};
