#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DialogueSystem.generated.h"

// ============================================================
// Narrative & Dialogue Agent #15 — DialogueSystem
// Prehistoric survival dialogue: NPC conversations, quest-giving
// dialogue lines, and environmental narrative triggers.
// ============================================================

UENUM(BlueprintType)
enum class ENarr_DialogueSpeaker : uint8
{
    Player          UMETA(DisplayName = "Player"),
    TribalLeader    UMETA(DisplayName = "Tribal Leader"),
    Scout           UMETA(DisplayName = "Scout"),
    Elder           UMETA(DisplayName = "Elder"),
    Narrator        UMETA(DisplayName = "Narrator"),
};

UENUM(BlueprintType)
enum class ENarr_DialogueTriggerType : uint8
{
    Proximity       UMETA(DisplayName = "Proximity"),
    QuestStart      UMETA(DisplayName = "Quest Start"),
    QuestComplete   UMETA(DisplayName = "Quest Complete"),
    FirstSighting   UMETA(DisplayName = "First Sighting"),
    PlayerDeath     UMETA(DisplayName = "Player Death"),
};

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueSpeaker Speaker = ENarr_DialogueSpeaker::Narrator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LineText;

    // Duration in seconds before auto-advancing (0 = wait for input)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration = 4.0f;

    // Optional: tag that links this line to a quest objective
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName LinkedQuestTag = NAME_None;
};

USTRUCT(BlueprintType)
struct FNarr_DialogueSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName SequenceID = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueTriggerType TriggerType = ENarr_DialogueTriggerType::Proximity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> Lines;

    // Once played, do not replay
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bPlayOnce = true;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    bool bHasPlayed = false;
};

// ============================================================
// UDialogueSystem — ActorComponent attached to NPCs or triggers
// ============================================================
UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDialogueSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UDialogueSystem();

    virtual void BeginPlay() override;

    // All dialogue sequences this actor can deliver
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueSequence> DialogueSequences;

    // Proximity radius for auto-trigger (cm)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    float ProximityTriggerRadius = 300.0f;

    // Currently active sequence index (-1 = none)
    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue")
    int32 ActiveSequenceIndex = -1;

    // Currently active line index within sequence
    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue")
    int32 ActiveLineIndex = -1;

    // Is a dialogue currently playing?
    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue")
    bool bDialogueActive = false;

    // Trigger a dialogue sequence by ID
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool TriggerDialogueByID(FName SequenceID);

    // Advance to next line (called by UI or timer)
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool AdvanceDialogue();

    // End current dialogue immediately
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void EndDialogue();

    // Get the current line being displayed
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool GetCurrentLine(FNarr_DialogueLine& OutLine) const;

    // Check proximity to player and auto-trigger if within radius
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void CheckProximityTrigger(AActor* PlayerActor);

    // Initialize built-in dialogue sequences (called in BeginPlay)
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void InitializeDefaultSequences();

    // Mark a sequence as played
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void MarkSequencePlayed(FName SequenceID);

    // Has a sequence already played?
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool HasSequencePlayed(FName SequenceID) const;

private:
    // Find sequence index by ID
    int32 FindSequenceIndex(FName SequenceID) const;

    // Timer handle for auto-advance
    FTimerHandle AutoAdvanceTimerHandle;

    // Called by timer to auto-advance
    void OnAutoAdvanceTimer();
};
