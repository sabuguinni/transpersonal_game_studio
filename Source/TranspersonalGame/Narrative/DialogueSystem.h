#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "DialogueSystem.generated.h"

// Dialogue line entry — one NPC utterance
USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString SpeakerID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LineText;

    // Delay before next line (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float PauseDuration = 2.5f;

    // Optional: play animation tag on speaker
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName AnimationTag = NAME_None;
};

// A full dialogue sequence (multiple lines, one conversation)
USTRUCT(BlueprintType)
struct FNarr_DialogueSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName SequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> Lines;

    // Can this sequence replay after first trigger?
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bRepeatable = false;

    // Has this sequence already played?
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    bool bHasPlayed = false;
};

// NPC speaker types (survival-focused, no spiritual roles)
UENUM(BlueprintType)
enum class ENarr_NPCSpeakerType : uint8
{
    Tracker        UMETA(DisplayName = "Tracker"),
    Elder          UMETA(DisplayName = "Elder"),
    Scout          UMETA(DisplayName = "Scout"),
    Hunter         UMETA(DisplayName = "Hunter"),
    Craftsman      UMETA(DisplayName = "Craftsman"),
    TribalLeader   UMETA(DisplayName = "Tribal Leader"),
    Unknown        UMETA(DisplayName = "Unknown")
};

/**
 * ANarr_DialogueTrigger
 * Placed in the world at story beat locations.
 * When the player enters the sphere radius, the dialogue sequence fires.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_DialogueTrigger : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueTrigger();

    // Trigger radius — player must enter to activate
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative",
        meta = (AllowPrivateAccess = "true"))
    USphereComponent* TriggerSphere;

    // The dialogue sequence this trigger plays
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FNarr_DialogueSequence DialogueSequence;

    // Which NPC speaks this dialogue
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_NPCSpeakerType SpeakerType = ENarr_NPCSpeakerType::Tracker;

    // Trigger radius in cm
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float TriggerRadius = 300.0f;

    // Called when player enters the trigger sphere
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnPlayerEnterTrigger(AActor* OverlappingActor);

    // Manually fire the dialogue (for testing or quest-driven activation)
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Narrative")
    void FireDialogue();

    // Reset the trigger so it can play again
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void ResetTrigger();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    bool bTriggered = false;
};

/**
 * UNarr_DialogueManager
 * Subsystem-style manager that tracks all active dialogue triggers in the level.
 * Registered NPCs report to this manager.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_DialogueManager : public UObject
{
    GENERATED_BODY()

public:
    UNarr_DialogueManager();

    // Register a dialogue trigger
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterTrigger(ANarr_DialogueTrigger* Trigger);

    // Unregister a trigger (e.g., when actor is destroyed)
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void UnregisterTrigger(ANarr_DialogueTrigger* Trigger);

    // Get all registered triggers
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<ANarr_DialogueTrigger*> GetAllTriggers() const;

    // Check if a sequence has already played (by ID)
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool HasSequencePlayed(FName SequenceID) const;

    // Mark a sequence as played
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void MarkSequencePlayed(FName SequenceID);

    // Reset all played sequences (e.g., new game)
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void ResetAllSequences();

private:
    UPROPERTY()
    TArray<ANarr_DialogueTrigger*> RegisteredTriggers;

    TSet<FName> PlayedSequenceIDs;
};
