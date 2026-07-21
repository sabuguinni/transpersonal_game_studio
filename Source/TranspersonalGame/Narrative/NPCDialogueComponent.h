#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCDialogueComponent.generated.h"

// ============================================================
// ENUMS — Global scope (UE5 UHT requirement)
// Prefix: ENarr_ to avoid name collisions across agents
// ============================================================

UENUM(BlueprintType)
enum class ENarr_DialogueTrigger : uint8
{
    PlayerApproach          UMETA(DisplayName = "Player Approach"),
    DinosaurNearby          UMETA(DisplayName = "Dinosaur Nearby"),
    MigrationSeen           UMETA(DisplayName = "Migration Seen"),
    PlayerCombatVictory     UMETA(DisplayName = "Player Combat Victory"),
    PlayerNearDeath         UMETA(DisplayName = "Player Near Death"),
    ResourceDiscovered      UMETA(DisplayName = "Resource Discovered"),
    Nightfall               UMETA(DisplayName = "Nightfall"),
    StormApproaching        UMETA(DisplayName = "Storm Approaching"),
    TribeUnderAttack        UMETA(DisplayName = "Tribe Under Attack"),
    QuestCompleted          UMETA(DisplayName = "Quest Completed")
};

UENUM(BlueprintType)
enum class ENarr_NPCRelationship : uint8
{
    Hostile     UMETA(DisplayName = "Hostile"),
    Wary        UMETA(DisplayName = "Wary"),
    Neutral     UMETA(DisplayName = "Neutral"),
    Friendly    UMETA(DisplayName = "Friendly"),
    Trusted     UMETA(DisplayName = "Trusted"),
    Allied      UMETA(DisplayName = "Allied")
};

UENUM(BlueprintType)
enum class ENarr_NPCRole : uint8
{
    Elder       UMETA(DisplayName = "Elder"),
    Scout       UMETA(DisplayName = "Scout"),
    Hunter      UMETA(DisplayName = "Hunter"),
    Tracker     UMETA(DisplayName = "Tracker"),
    Crafter     UMETA(DisplayName = "Crafter"),
    Guard       UMETA(DisplayName = "Guard"),
    Survivor    UMETA(DisplayName = "Survivor")
};

// ============================================================
// STRUCTS — Global scope
// ============================================================

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueTrigger Trigger = ENarr_DialogueTrigger::PlayerApproach;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_NPCRelationship MinRelationshipRequired = ENarr_NPCRelationship::Neutral;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bIsContextual = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float AudioCueDelay = 0.0f;
};

// ============================================================
// DELEGATES
// ============================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogueTriggered, FNarr_DialogueLine, Line, ENarr_NPCRelationship, Relationship);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRelationshipChanged, ENarr_NPCRelationship, OldRelationship, ENarr_NPCRelationship, NewRelationship);

// ============================================================
// COMPONENT CLASS
// ============================================================

UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCDialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCDialogueComponent();

    // ---- Identity ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    FText NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    ENarr_NPCRole NPCRole;

    // ---- Relationship ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC Relationship")
    ENarr_NPCRelationship CurrentRelationship;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC Relationship")
    int32 TrustPoints;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC Relationship")
    int32 InteractionCount;

    // ---- Dialogue Config ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DialogueTriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DialogueCooldownSeconds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueLine> DialogueLines;

    // ---- State ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
    bool bIsInDialogue;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
    bool bHasGreeted;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
    ENarr_DialogueTrigger LastTrigger;

    // ---- Delegates ----
    UPROPERTY(BlueprintAssignable, Category = "Dialogue|Events")
    FOnDialogueTriggered OnDialogueTriggered;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue|Events")
    FOnRelationshipChanged OnRelationshipChanged;

    // ---- Public API ----
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void TriggerDialogue(ENarr_DialogueTrigger Trigger);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void ModifyRelationship(int32 TrustDelta);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void NotifyDinosaurNearby(FName DinosaurSpecies, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void NotifyPlayerCombatVictory();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void NotifyPlayerNearDeath();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void NotifyMigrationEvent();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void NotifyNightfall();

    UFUNCTION(BlueprintPure, Category = "Dialogue")
    ENarr_NPCRelationship GetCurrentRelationship() const { return CurrentRelationship; }

    UFUNCTION(BlueprintPure, Category = "Dialogue")
    bool IsInDialogue() const { return bIsInDialogue; }

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    FTimerHandle CooldownTimerHandle;
    bool bCooldownActive;

    void InitialiseDialogueLines();
    void StartDialogueCooldown();
    void OnCooldownExpired();
    FNarr_DialogueLine GetDialogueLineForTrigger(ENarr_DialogueTrigger Trigger);
    void AddDialogueLine(ENarr_DialogueTrigger Trigger, ENarr_NPCRelationship MinRelationship, const FString& Text, bool bContextual);
};
