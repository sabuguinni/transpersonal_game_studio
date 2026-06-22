// DialogueSystem.h
// Agent #15 — Narrative & Dialogue Agent
// Dialogue trigger and NPC conversation system for prehistoric survival game
// Priority: P5 — Quest & Narrative

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "SharedTypes.h"
#include "DialogueSystem.generated.h"

// ============================================================
// Enums — Narr_ prefix to avoid conflicts
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
enum class ENarr_NPCRole : uint8
{
    TribalElder  UMETA(DisplayName = "Tribal Elder"),
    Hunter       UMETA(DisplayName = "Hunter"),
    Scout        UMETA(DisplayName = "Scout"),
    Crafter      UMETA(DisplayName = "Crafter"),
    Survivor     UMETA(DisplayName = "Survivor")
};

// ============================================================
// Structs — Narr_ prefix to avoid conflicts
// ============================================================

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LineText;

    // Index into audio asset array (matched by order)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    int32 AudioIndex = -1;

    // Survival stat modifier triggered by this line (e.g. fear +10)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float FearDelta = 0.0f;
};

USTRUCT(BlueprintType)
struct FNarr_DialogueTree
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_NPCRole NPCRole = ENarr_NPCRole::TribalElder;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> Lines;

    // Quest ID unlocked after completing this dialogue
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString UnlocksQuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueState State = ENarr_DialogueState::Idle;
};

// ============================================================
// ANarr_DialogueTrigger — spatial trigger that fires dialogue
// ============================================================

UCLASS(ClassGroup = "Narrative", meta = (DisplayName = "Dialogue Trigger"))
class TRANSPERSONALGAME_API ANarr_DialogueTrigger : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueTrigger();

    // Trigger volume — player enters to start dialogue
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative",
        meta = (AllowPrivateAccess = "true"))
    UBoxComponent* TriggerVolume;

    // The dialogue tree this trigger fires
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FNarr_DialogueTree DialogueTree;

    // Whether this trigger fires once or every time
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bFireOnce = true;

    // NPC role displayed in HUD
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_NPCRole NPCRole = ENarr_NPCRole::TribalElder;

    // Current state
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    ENarr_DialogueState CurrentState = ENarr_DialogueState::Idle;

    // Current line index being displayed
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    int32 CurrentLineIndex = 0;

    // Called when player enters trigger
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StartDialogue();

    // Advance to next line
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceLine();

    // End dialogue and apply quest unlock
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void EndDialogue();

    // Reset dialogue to Idle (for repeatable triggers)
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void ResetDialogue();

    // Get current line text for HUD
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FString GetCurrentLineText() const;

    // Get current speaker name for HUD
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FString GetCurrentSpeakerName() const;

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor, UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
    bool bHasFired = false;
};

// ============================================================
// UNarr_DialogueManager — subsystem managing all active dialogues
// ============================================================

UCLASS(ClassGroup = "Narrative", meta = (DisplayName = "Dialogue Manager Component"))
class TRANSPERSONALGAME_API UNarr_DialogueManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueManager();

    // All dialogue trees registered in the world
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueTree> AllDialogueTrees;

    // Currently active dialogue (null if none)
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    FString ActiveDialogueID;

    // Register a new dialogue tree
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterDialogue(const FNarr_DialogueTree& Tree);

    // Mark dialogue as completed
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void CompleteDialogue(const FString& DialogueID);

    // Check if dialogue is available
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsDialogueAvailable(const FString& DialogueID) const;

    // Get dialogue by ID
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool GetDialogueByID(const FString& DialogueID, FNarr_DialogueTree& OutTree) const;

protected:
    virtual void BeginPlay() override;
    virtual void InitializeComponent() override;
};
