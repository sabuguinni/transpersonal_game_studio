#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NarrativeDialogueSystem.generated.h"

// ============================================================
// Narrative & Dialogue System — Agent #15
// Stampede Quest dialogue triggers, Elder NPC lines,
// urgency callbacks bound to QuestStampedeManager timer.
// ============================================================

UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Inactive        UMETA(DisplayName = "Inactive"),
    Playing         UMETA(DisplayName = "Playing"),
    Completed       UMETA(DisplayName = "Completed"),
    Interrupted     UMETA(DisplayName = "Interrupted")
};

UENUM(BlueprintType)
enum class ENarr_DialogueTriggerType : uint8
{
    QuestStart      UMETA(DisplayName = "Quest Start — Elder Warning"),
    UrgencyMid      UMETA(DisplayName = "Urgency Mid — Hurry"),
    UrgencyFinal    UMETA(DisplayName = "Urgency Final — Almost There"),
    Victory         UMETA(DisplayName = "Victory — Survived"),
    Failure         UMETA(DisplayName = "Failure — Caught in Stampede")
};

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueTriggerType TriggerType;

    FNarr_DialogueLine()
        : SpeakerName(TEXT("Elder"))
        , LineText(TEXT(""))
        , DisplayDuration(4.0f)
        , TriggerType(ENarr_DialogueTriggerType::QuestStart)
    {}
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_DialogueTriggerActor : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueTriggerActor();

    // ---- Configuration ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Config")
    ENarr_DialogueTriggerType TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Config")
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Config")
    float ActivationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Config")
    bool bOneShot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Config")
    float QuestTimerThreshold;

    // ---- State ----

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|State")
    ENarr_DialogueState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|State")
    bool bHasTriggered;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|State")
    int32 CurrentLineIndex;

    // ---- Interface ----

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceLine();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void InterruptDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void ResetTrigger();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsPlayerInRange() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueLine GetCurrentLine() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsDialogueActive() const;

    // ---- Events (override in Blueprint) ----

    UFUNCTION(BlueprintImplementableEvent, Category = "Narrative")
    void OnDialogueStarted(const FNarr_DialogueLine& FirstLine);

    UFUNCTION(BlueprintImplementableEvent, Category = "Narrative")
    void OnLineAdvanced(const FNarr_DialogueLine& NewLine);

    UFUNCTION(BlueprintImplementableEvent, Category = "Narrative")
    void OnDialogueCompleted();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    FTimerHandle LineAdvanceTimer;
    void AdvanceLineInternal();
    void PopulateDefaultLines();
};

// ============================================================
// Narrative Manager — singleton-style actor that coordinates
// all dialogue triggers for the Stampede Quest arc.
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_StampedeNarrativeManager : public AActor
{
    GENERATED_BODY()

public:
    ANarr_StampedeNarrativeManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Manager")
    TArray<ANarr_DialogueTriggerActor*> RegisteredTriggers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Manager")
    float UrgencyMidThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Manager")
    float UrgencyFinalThreshold;

    UFUNCTION(BlueprintCallable, Category = "Narrative|Manager")
    void OnQuestStarted();

    UFUNCTION(BlueprintCallable, Category = "Narrative|Manager")
    void OnQuestTimerUpdated(float NormalizedTimeRemaining);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Manager")
    void OnQuestSucceeded();

    UFUNCTION(BlueprintCallable, Category = "Narrative|Manager")
    void OnQuestFailed();

    UFUNCTION(BlueprintCallable, Category = "Narrative|Manager")
    void RegisterTrigger(ANarr_DialogueTriggerActor* Trigger);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Manager")
    void FireTriggerByType(ENarr_DialogueTriggerType Type);

protected:
    virtual void BeginPlay() override;

private:
    bool bMidUrgencyFired;
    bool bFinalUrgencyFired;
};
