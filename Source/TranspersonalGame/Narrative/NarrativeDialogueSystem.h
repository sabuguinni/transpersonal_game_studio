#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "SharedTypes.h"
#include "NarrativeDialogueSystem.generated.h"

// ============================================================
// Narrative & Dialogue System — Agent #15
// Prehistoric survival dialogue trigger and zone system.
// Drives story beats via proximity triggers in the world.
// ============================================================

/** Tone of a dialogue line — affects subtitle colour and audio cue */
UENUM(BlueprintType)
enum class ENarr_DialogueTone : uint8
{
    Neutral     UMETA(DisplayName = "Neutral"),
    Warning     UMETA(DisplayName = "Warning"),
    Discovery   UMETA(DisplayName = "Discovery"),
    Danger      UMETA(DisplayName = "Danger"),
    Calm        UMETA(DisplayName = "Calm"),
};

/** A single line of in-world dialogue */
USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FText SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FText LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueTone Tone = ENarr_DialogueTone::Neutral;

    /** Seconds to display this line before advancing */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration = 4.0f;
};

/** A complete dialogue sequence (multiple lines) */
USTRUCT(BlueprintType)
struct FNarr_DialogueSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName SequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> Lines;

    /** Can this sequence repeat if the player re-enters the zone? */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bRepeatable = false;
};

/**
 * ANarr_DialogueTriggerActor
 * Placed in the world. When the player enters the box volume,
 * it fires the assigned dialogue sequence via the NarrativeManager.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Narrative Dialogue Trigger"))
class TRANSPERSONALGAME_API ANarr_DialogueTriggerActor : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueTriggerActor();

    /** The dialogue sequence to play when triggered */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FNarr_DialogueSequence DialogueSequence;

    /** Trigger volume radius (box half-extent in cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FVector TriggerExtent = FVector(300.f, 300.f, 200.f);

    /** Has this trigger already fired? */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative")
    bool bHasFired = false;

    /** Activate the dialogue sequence manually (e.g. from Blueprint) */
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void ActivateDialogue();

    /** Reset so the trigger can fire again */
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void ResetTrigger();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
              meta = (AllowPrivateAccess = "true"))
    UBoxComponent* TriggerBox;

    UFUNCTION()
    void OnPlayerEntered(UPrimitiveComponent* OverlappedComp,
                         AActor* OtherActor,
                         UPrimitiveComponent* OtherComp,
                         int32 OtherBodyIndex,
                         bool bFromSweep,
                         const FHitResult& SweepResult);
};
