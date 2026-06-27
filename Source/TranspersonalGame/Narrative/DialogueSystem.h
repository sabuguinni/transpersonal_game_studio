// DialogueSystem.h
// Agent #15 — Narrative & Dialogue Agent
// PROD_CYCLE_AUTO_20260627_008
// NPC dialogue trigger and line management system

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "DialogueSystem.generated.h"

// ── Enums (global scope, Narr_ prefix) ──────────────────────────────────────

UENUM(BlueprintType)
enum class ENarr_NPCRole : uint8
{
    TribeElder      UMETA(DisplayName = "Tribe Elder"),
    HuntLeader      UMETA(DisplayName = "Hunt Leader"),
    Scout           UMETA(DisplayName = "Scout"),
    Warrior         UMETA(DisplayName = "Warrior"),
    Crafter         UMETA(DisplayName = "Crafter"),
    Survivor        UMETA(DisplayName = "Survivor")
};

UENUM(BlueprintType)
enum class ENarr_DialogueTriggerState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    PlayerNearby    UMETA(DisplayName = "Player Nearby"),
    Playing         UMETA(DisplayName = "Playing"),
    Cooldown        UMETA(DisplayName = "Cooldown")
};

// ── Structs (global scope) ───────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioAssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DisplayDuration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_NPCRole SpeakerRole = ENarr_NPCRole::Scout;

    FNarr_DialogueLine() {}
};

USTRUCT(BlueprintType)
struct FNarr_DialogueSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bPlayOnce = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float CooldownSeconds = 60.0f;

    FNarr_DialogueSequence() {}
};

// ── ANarr_DialogueTrigger ────────────────────────────────────────────────────

UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ANarr_DialogueTrigger : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueTrigger();

    // Trigger volume — player enters to start dialogue
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue",
              meta = (AllowPrivateAccess = "true"))
    UBoxComponent* TriggerVolume;

    // NPC identity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_NPCRole NPCRole;

    // Dialogue content
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FNarr_DialogueSequence DialogueSequence;

    // Interaction radius (cm)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue",
              meta = (ClampMin = "100.0", ClampMax = "2000.0"))
    float TriggerRadius = 400.0f;

    // Current state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
    ENarr_DialogueTriggerState CurrentState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
    int32 CurrentLineIndex;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
    bool bHasBeenTriggered;

    // Blueprint events
    UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
    void OnDialogueStarted(const FNarr_DialogueLine& FirstLine);

    UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
    void OnDialogueLineChanged(const FNarr_DialogueLine& NewLine, int32 LineIndex);

    UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
    void OnDialogueEnded();

    // Callable functions
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StartDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AdvanceLine();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsPlayerInRange() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueLine GetCurrentLine() const;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION()
    void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp,
                               AActor* OtherActor,
                               UPrimitiveComponent* OtherComp,
                               int32 OtherBodyIndex,
                               bool bFromSweep,
                               const FHitResult& SweepResult);

    UFUNCTION()
    void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComp,
                             AActor* OtherActor,
                             UPrimitiveComponent* OtherComp,
                             int32 OtherBodyIndex);

private:
    float CooldownTimer;
    float LineTimer;
    bool bPlayerInsideTrigger;
};
