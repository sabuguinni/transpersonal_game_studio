// DialogueSystem.h
// Agent #15 — Narrative & Dialogue Agent
// Prehistoric survival dialogue system — NPC conversations, quest hooks, survival lore delivery
// PROD_CYCLE_AUTO_20260622_011

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "SharedTypes.h"
#include "DialogueSystem.generated.h"

// ─── Enums ────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    PlayerInRange   UMETA(DisplayName = "PlayerInRange"),
    Active          UMETA(DisplayName = "Active"),
    Cooldown        UMETA(DisplayName = "Cooldown")
};

UENUM(BlueprintType)
enum class ENarr_NPCRole : uint8
{
    TribalElder     UMETA(DisplayName = "TribalElder"),
    Scout           UMETA(DisplayName = "Scout"),
    Craftsman       UMETA(DisplayName = "Craftsman"),
    Tracker         UMETA(DisplayName = "Tracker"),
    Survivor        UMETA(DisplayName = "Survivor")
};

// ─── Structs ──────────────────────────────────────────────────────────────────

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
    bool bTriggersQuest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName QuestID;

    FNarr_DialogueLine()
        : SpeakerName(TEXT("Unknown"))
        , LineText(TEXT(""))
        , DisplayDuration(4.0f)
        , bTriggersQuest(false)
        , QuestID(NAME_None)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_DialogueSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName SequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float CooldownSeconds;

    FNarr_DialogueSequence()
        : SequenceID(NAME_None)
        , bRepeatable(true)
        , CooldownSeconds(30.0f)
    {}
};

// ─── Main Class ───────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ANarr_DialogueNPC : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueNPC();

    // ── Components ────────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    USphereComponent* InteractionRadius;

    // ── NPC Data ──────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|NPC")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|NPC")
    ENarr_NPCRole NPCRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueSequence> DialogueSequences;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative|State")
    ENarr_DialogueState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    float InteractionRangeRadius;

    // ── Runtime State ─────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative|State")
    int32 CurrentSequenceIndex;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative|State")
    int32 CurrentLineIndex;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative|State")
    bool bPlayerInRange;

    // ── Functions ─────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void BeginDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceLine();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueLine GetCurrentLine() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool HasMoreLines() const;

    UFUNCTION(BlueprintPure, Category = "Narrative")
    FString GetNPCDisplayName() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AddDialogueLine(const FNarr_DialogueLine& NewLine, int32 SequenceIndex = 0);

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION()
    void OnPlayerEnterRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnPlayerExitRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    float CooldownTimer;
    bool bOnCooldown;

    void InitDefaultDialogue();
};
