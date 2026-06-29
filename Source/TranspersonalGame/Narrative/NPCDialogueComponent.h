// NPCDialogueComponent.h
// Agent #15 — Narrative & Dialogue Agent
// NPC Social Dynamics and Dialogue System
// Provides contextual dialogue, relationship tracking, and social interaction for NPCs

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "NPCDialogueComponent.generated.h"

// ─── Enums ────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ENarr_DialogueTrigger : uint8
{
    PlayerApproach       UMETA(DisplayName = "Player Approach"),
    PlayerHelped         UMETA(DisplayName = "Player Helped Tribe"),
    DinosaurNearby       UMETA(DisplayName = "Dinosaur Nearby"),
    HunterReturned       UMETA(DisplayName = "Hunter Returned"),
    ResourceFound        UMETA(DisplayName = "Resource Found"),
    DangerDetected       UMETA(DisplayName = "Danger Detected"),
    NightFalling         UMETA(DisplayName = "Night Falling"),
    MigrationSeen        UMETA(DisplayName = "Migration Seen"),
    PlayerFirstMeet      UMETA(DisplayName = "Player First Meeting"),
    TradeOffer           UMETA(DisplayName = "Trade Offer"),
};

UENUM(BlueprintType)
enum class ENarr_NPCRelationship : uint8
{
    Hostile      UMETA(DisplayName = "Hostile"),
    Wary         UMETA(DisplayName = "Wary"),
    Neutral      UMETA(DisplayName = "Neutral"),
    Friendly     UMETA(DisplayName = "Friendly"),
    Trusted      UMETA(DisplayName = "Trusted"),
    Allied       UMETA(DisplayName = "Allied"),
};

UENUM(BlueprintType)
enum class ENarr_NPCRole : uint8
{
    Elder        UMETA(DisplayName = "Elder"),
    Scout        UMETA(DisplayName = "Scout"),
    Hunter       UMETA(DisplayName = "Hunter"),
    Tracker      UMETA(DisplayName = "Tracker"),
    Crafter      UMETA(DisplayName = "Crafter"),
    Guard        UMETA(DisplayName = "Guard"),
    Gatherer     UMETA(DisplayName = "Gatherer"),
};

// ─── Structs ──────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueTrigger Trigger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_NPCRelationship MinRelationship;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float CooldownSeconds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioAssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bOneTimeOnly;

    FNarr_DialogueLine()
        : Trigger(ENarr_DialogueTrigger::PlayerApproach)
        , MinRelationship(ENarr_NPCRelationship::Neutral)
        , CooldownSeconds(30.0f)
        , bOneTimeOnly(false)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_NPCKnowledge
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knowledge")
    FString KnowledgeKey;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knowledge")
    FString KnowledgeValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knowledge")
    float AcquiredAtTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knowledge")
    bool bSharedWithPlayer;

    FNarr_NPCKnowledge()
        : AcquiredAtTime(0.0f)
        , bSharedWithPlayer(false)
    {}
};

// ─── Component ────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCDialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCDialogueComponent();

    // ── NPC Identity ──────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    ENarr_NPCRole NPCRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    FString TribeName;

    // ── Relationship ──────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    ENarr_NPCRelationship CurrentRelationship;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    float RelationshipScore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    float RelationshipScoreMax;

    // ── Dialogue Data ─────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DialogueTriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bCanSpeak;

    // ── NPC Knowledge ─────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knowledge")
    TArray<FNarr_NPCKnowledge> KnowledgeBase;

    // ── State ─────────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bHasMetPlayer;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float LastDialogueTime;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FString LastSpokenLine;

    // ── Public API ────────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool TryTriggerDialogue(ENarr_DialogueTrigger Trigger);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void ModifyRelationship(float Delta);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    ENarr_NPCRelationship GetRelationshipTier() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AddKnowledge(const FString& Key, const FString& Value);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FString GetKnowledge(const FString& Key) const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    TArray<FNarr_DialogueLine> GetAvailableLines(ENarr_DialogueTrigger Trigger) const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void MarkPlayerMet();

    UFUNCTION(BlueprintPure, Category = "Dialogue")
    bool CanTriggerDialogue(ENarr_DialogueTrigger Trigger) const;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    TMap<ENarr_DialogueTrigger, float> LastTriggerTimes;
    TSet<int32> SpokenOneTimeLines;

    void UpdateRelationshipTier();
    bool IsLineCooledDown(const FNarr_DialogueLine& Line, ENarr_DialogueTrigger Trigger) const;
};
