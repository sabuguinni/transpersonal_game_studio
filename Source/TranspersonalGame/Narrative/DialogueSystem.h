#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DialogueSystem.generated.h"

// ============================================================
// Enums — Narr_ prefix to avoid collision with other agents
// ============================================================

UENUM(BlueprintType)
enum class ENarr_DialogueTone : uint8
{
    Neutral     UMETA(DisplayName = "Neutral"),
    Urgent      UMETA(DisplayName = "Urgent"),
    Warning     UMETA(DisplayName = "Warning"),
    Informative UMETA(DisplayName = "Informative"),
    Hostile     UMETA(DisplayName = "Hostile")
};

UENUM(BlueprintType)
enum class ENarr_NPCRole : uint8
{
    TribeElder      UMETA(DisplayName = "Tribe Elder"),
    Scout           UMETA(DisplayName = "Scout"),
    Hunter          UMETA(DisplayName = "Hunter"),
    Quartermaster   UMETA(DisplayName = "Quartermaster"),
    TribeLeader     UMETA(DisplayName = "Tribe Leader"),
    Survivor        UMETA(DisplayName = "Survivor")
};

// ============================================================
// Structs
// ============================================================

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FString SpeakerID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FText LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_DialogueTone Tone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FString AudioAssetPath;

    FNarr_DialogueLine()
        : SpeakerID(TEXT("Unknown"))
        , Tone(ENarr_DialogueTone::Neutral)
        , DisplayDuration(4.0f)
        , AudioAssetPath(TEXT(""))
    {}
};

USTRUCT(BlueprintType)
struct FNarr_DialogueNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FString NodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FString> PlayerResponseIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FString NextNodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    bool bEndsConversation;

    FNarr_DialogueNode()
        : NodeID(TEXT(""))
        , bEndsConversation(false)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_ConversationTree
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FString ConversationID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_NPCRole NPCRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FString RootNodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueNode> Nodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    bool bRequiresQuestActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FString RequiredQuestID;

    FNarr_ConversationTree()
        : ConversationID(TEXT(""))
        , NPCRole(ENarr_NPCRole::Survivor)
        , RootNodeID(TEXT("root"))
        , bRequiresQuestActive(false)
        , RequiredQuestID(TEXT(""))
    {}
};

// ============================================================
// Component
// ============================================================

UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_DialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueComponent();

    virtual void BeginPlay() override;

    // The conversation tree assigned to this NPC
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FNarr_ConversationTree ConversationTree;

    // Role of this NPC
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_NPCRole NPCRole;

    // Whether this NPC is currently in dialogue with the player
    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue")
    bool bIsInDialogue;

    // Interaction radius — player must be within this range to trigger dialogue
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    float InteractionRadius;

    // Start dialogue from root node
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void StartDialogue();

    // Advance to next node
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void AdvanceDialogue(const FString& ChosenResponseID);

    // End dialogue
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void EndDialogue();

    // Get current active node
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    FNarr_DialogueNode GetCurrentNode() const;

    // Get current line index within active node
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    int32 GetCurrentLineIndex() const;

    // Advance to next line within current node
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool AdvanceLine();

private:
    FString CurrentNodeID;
    int32 CurrentLineIndex;

    FNarr_DialogueNode* FindNode(const FString& NodeID);
};
