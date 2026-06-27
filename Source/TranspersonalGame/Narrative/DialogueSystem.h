#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TriggerBox.h"
#include "DialogueSystem.generated.h"

// ============================================================
// Enums — global scope (UHT requirement)
// ============================================================

UENUM(BlueprintType)
enum class ENarr_DialogueZone : uint8
{
    None            UMETA(DisplayName = "None"),
    RiverCrossing   UMETA(DisplayName = "River Crossing"),
    RaptorTerritory UMETA(DisplayName = "Raptor Territory"),
    CraftingCamp    UMETA(DisplayName = "Crafting Camp"),
    HerdMigration   UMETA(DisplayName = "Herd Migration")
};

UENUM(BlueprintType)
enum class ENarr_NPCType : uint8
{
    Elder   UMETA(DisplayName = "Elder"),
    Scout   UMETA(DisplayName = "Scout"),
    Hunter  UMETA(DisplayName = "Hunter"),
    Crafter UMETA(DisplayName = "Crafter")
};

UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Playing     UMETA(DisplayName = "Playing"),
    WaitingInput UMETA(DisplayName = "Waiting For Input"),
    Completed   UMETA(DisplayName = "Completed")
};

// ============================================================
// Structs — global scope
// ============================================================

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioAssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_NPCType SpeakerType;

    FNarr_DialogueLine()
        : SpeakerName(TEXT("Unknown"))
        , DisplayDuration(5.0f)
        , SpeakerType(ENarr_NPCType::Elder)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_DialogueTree
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueZone Zone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bHasBeenTriggered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LinkedQuestID;

    FNarr_DialogueTree()
        : Zone(ENarr_DialogueZone::None)
        , bHasBeenTriggered(false)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_QuestObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FText ObjectiveText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FText HintText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    int32 CurrentCount;

    FNarr_QuestObjective()
        : bIsCompleted(false)
        , RequiredCount(1)
        , CurrentCount(0)
    {}
};

// ============================================================
// ANarr_DialogueTriggerActor — fires dialogue when player enters
// ============================================================

UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ANarr_DialogueTriggerActor : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueTriggerActor();

    // The dialogue zone this trigger belongs to
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueZone DialogueZone;

    // All dialogue lines for this zone
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FNarr_DialogueTree DialogueTree;

    // Trigger radius in cm
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float TriggerRadius;

    // Whether this dialogue fires only once
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bOneShot;

    // Current state
    UPROPERTY(BlueprintReadOnly, Category = "Narrative", meta = (AllowPrivateAccess = "true"))
    ENarr_DialogueState CurrentState;

    // Current line index
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    int32 CurrentLineIndex;

    // Trigger the dialogue sequence
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerDialogue();

    // Advance to next line
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceDialogue();

    // Reset dialogue for replay
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void ResetDialogue();

    // Get current line
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueLine GetCurrentLine() const;

    // Check if dialogue is active
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsDialogueActive() const;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    float LineTimer;
    bool bAutoAdvance;
};

// ============================================================
// ANarr_DialogueManager — singleton manager for all dialogue
// ============================================================

UCLASS()
class TRANSPERSONALGAME_API ANarr_DialogueManager : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueManager();

    // All registered dialogue trees
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueTree> AllDialogueTrees;

    // Active dialogue zone
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    ENarr_DialogueZone ActiveZone;

    // Quest objective chains (tutorial)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_QuestObjective> TutorialObjectives;

    // Register a dialogue tree
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterDialogueTree(const FNarr_DialogueTree& Tree);

    // Get tree for zone
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueTree GetDialogueTreeForZone(ENarr_DialogueZone Zone) const;

    // Mark zone as triggered
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void MarkZoneTriggered(ENarr_DialogueZone Zone);

    // Check if zone has been triggered
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool HasZoneBeenTriggered(ENarr_DialogueZone Zone) const;

    // Update objective progress
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void UpdateObjectiveProgress(const FString& ObjectiveID, int32 Delta);

    // Initialize all dialogue trees with content
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Narrative")
    void InitializeDefaultDialogueTrees();

protected:
    virtual void BeginPlay() override;
};

#include "DialogueSystem.generated.h"
