#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DialogueManager.generated.h"

// ============================================================
// ENarr_DialogueState — state of a dialogue conversation
// ============================================================
UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Active      UMETA(DisplayName = "Active"),
    Waiting     UMETA(DisplayName = "Waiting For Input"),
    Completed   UMETA(DisplayName = "Completed"),
    Locked      UMETA(DisplayName = "Locked")
};

// ============================================================
// ENarr_DialogueType — category of dialogue line
// ============================================================
UENUM(BlueprintType)
enum class ENarr_DialogueType : uint8
{
    QuestGive       UMETA(DisplayName = "Quest Give"),
    QuestComplete   UMETA(DisplayName = "Quest Complete"),
    QuestFail       UMETA(DisplayName = "Quest Fail"),
    Tutorial        UMETA(DisplayName = "Tutorial"),
    Ambient         UMETA(DisplayName = "Ambient"),
    Warning         UMETA(DisplayName = "Warning"),
    Lore            UMETA(DisplayName = "Lore")
};

// ============================================================
// FNarr_DialogueLine — a single line of NPC dialogue
// ============================================================
USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LineID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueType LineType;

    // Audio URL from ElevenLabs TTS
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioURL;

    // Duration in seconds for subtitle display
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration;

    // ID of next line (empty = end of conversation)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString NextLineID;

    // Quest ID this line triggers (empty = no quest)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString TriggersQuestID;

    FNarr_DialogueLine()
        : LineID(TEXT(""))
        , SpeakerName(TEXT(""))
        , LineText(TEXT(""))
        , LineType(ENarr_DialogueType::Ambient)
        , AudioURL(TEXT(""))
        , DisplayDuration(4.0f)
        , NextLineID(TEXT(""))
        , TriggersQuestID(TEXT(""))
    {}
};

// ============================================================
// FNarr_DialogueTree — a full conversation tree for one NPC
// ============================================================
USTRUCT(BlueprintType)
struct FNarr_DialogueTree
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString TreeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString FirstLineID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueState State;

    FNarr_DialogueTree()
        : TreeID(TEXT(""))
        , NPCName(TEXT(""))
        , FirstLineID(TEXT(""))
        , State(ENarr_DialogueState::Idle)
    {}
};

// ============================================================
// ADialogueManager — manages all NPC dialogue in the world
// ============================================================
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADialogueManager : public AActor
{
    GENERATED_BODY()

public:
    ADialogueManager();

protected:
    virtual void BeginPlay() override;

public:
    // All dialogue trees registered in this level
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueTree> DialogueTrees;

    // Currently active tree ID (empty = no active dialogue)
    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue")
    FString ActiveTreeID;

    // Currently active line ID
    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue")
    FString ActiveLineID;

    // Start a dialogue tree by ID
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool StartDialogue(const FString& TreeID);

    // Advance to next line in active dialogue
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool AdvanceDialogue();

    // End active dialogue
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void EndDialogue();

    // Get current line data
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    FNarr_DialogueLine GetCurrentLine() const;

    // Get dialogue tree by ID
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool GetDialogueTree(const FString& TreeID, FNarr_DialogueTree& OutTree) const;

    // Register a new dialogue tree at runtime
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void RegisterDialogueTree(const FNarr_DialogueTree& Tree);

    // Mark a tree as completed (won't replay)
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void MarkTreeCompleted(const FString& TreeID);

    // Check if dialogue is currently active
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool IsDialogueActive() const;

private:
    void InitializeDefaultDialogueTrees();
    FNarr_DialogueLine* FindLine(const FString& TreeID, const FString& LineID);
};
