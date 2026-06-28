// DialogueSystemManager.h
// Agent #15 — Narrative & Dialogue Agent
// CYCLE: PROD_CYCLE_AUTO_20260628_001
// Manages NPC dialogue triggers, voice line playback, and narrative event dispatch.
// Follows SharedTypes.h conventions. All types prefixed Narr_.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Sound/SoundBase.h"
#include "DialogueSystemManager.generated.h"

// ─── Enums ────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ENarr_DialogueSpeaker : uint8
{
    None            UMETA(DisplayName = "None"),
    ScoutWarrior    UMETA(DisplayName = "Scout Warrior"),
    ElderHunter     UMETA(DisplayName = "Elder Hunter"),
    TribalNarrator  UMETA(DisplayName = "Tribal Narrator"),
    RaidScout       UMETA(DisplayName = "Raid Scout"),
    TribalLeader    UMETA(DisplayName = "Tribal Leader"),
    ChiefHunter     UMETA(DisplayName = "Chief Hunter"),
};

UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Playing     UMETA(DisplayName = "Playing"),
    Cooldown    UMETA(DisplayName = "Cooldown"),
    Completed   UMETA(DisplayName = "Completed"),
};

// ─── Structs ──────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_DialogueSpeaker Speaker = ENarr_DialogueSpeaker::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FString LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    USoundBase* VoiceAsset = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    float PlaybackDuration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    bool bOneShot = true;
};

USTRUCT(BlueprintType)
struct FNarr_DialogueSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName SequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    float CooldownSeconds = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    bool bRequiresQuestActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName RequiredQuestID;
};

// ─── Dialogue Trigger Actor ────────────────────────────────────────────────────

UCLASS(ClassGroup = "Narrative", meta = (DisplayName = "Narr Dialogue Trigger"))
class TRANSPERSONALGAME_API ANarr_DialogueTrigger : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueTrigger();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative|Components",
        meta = (AllowPrivateAccess = "true"))
    UBoxComponent* TriggerVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FNarr_DialogueSequence DialogueSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_DialogueSpeaker AssignedSpeaker = ENarr_DialogueSpeaker::None;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|State")
    ENarr_DialogueState CurrentState = ENarr_DialogueState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Debug")
    bool bDebugDraw = true;

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void TriggerDialogue(AActor* PlayerActor);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void ResetDialogue();

    UFUNCTION(BlueprintPure, Category = "Narrative|Dialogue")
    bool CanTrigger() const;

    UFUNCTION(BlueprintPure, Category = "Narrative|Dialogue")
    FString GetSpeakerName() const;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION()
    void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

private:
    float CooldownTimer = 0.0f;
    int32 CurrentLineIndex = 0;
    float LineTimer = 0.0f;
};

// ─── Dialogue System Manager ───────────────────────────────────────────────────

UCLASS(ClassGroup = "Narrative", meta = (DisplayName = "Narr Dialogue System Manager"))
class TRANSPERSONALGAME_API ANarr_DialogueSystemManager : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueSystemManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Registry")
    TArray<ANarr_DialogueTrigger*> RegisteredTriggers;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|State")
    int32 TotalDialoguesPlayed = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|State")
    ANarr_DialogueTrigger* ActiveTrigger = nullptr;

    UFUNCTION(BlueprintCallable, Category = "Narrative|Manager")
    void RegisterTrigger(ANarr_DialogueTrigger* Trigger);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Manager")
    void UnregisterTrigger(ANarr_DialogueTrigger* Trigger);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Manager")
    void NotifyDialogueStarted(ANarr_DialogueTrigger* Trigger);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Manager")
    void NotifyDialogueCompleted(ANarr_DialogueTrigger* Trigger);

    UFUNCTION(BlueprintPure, Category = "Narrative|Manager")
    bool IsAnyDialoguePlaying() const;

    UFUNCTION(BlueprintPure, Category = "Narrative|Manager")
    int32 GetActiveTriggerCount() const;

protected:
    virtual void BeginPlay() override;
};
