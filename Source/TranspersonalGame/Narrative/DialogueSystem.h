
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SphereComponent.h"
#include "Sound/SoundBase.h"
#include "DialogueSystem.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// Enums & Structs — Narr_ prefix to avoid cross-agent collisions
// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ENarr_DialogueTriggerType : uint8
{
    Proximity       UMETA(DisplayName = "Proximity"),
    Interaction     UMETA(DisplayName = "Interaction (E key)"),
    QuestComplete   UMETA(DisplayName = "Quest Complete"),
    CraftingDone    UMETA(DisplayName = "Crafting Done"),
    DayNight        UMETA(DisplayName = "Day/Night Transition"),
};

UENUM(BlueprintType)
enum class ENarr_SpeakerID : uint8
{
    Elder           UMETA(DisplayName = "Elder"),
    HerdTracker     UMETA(DisplayName = "Herd Tracker"),
    YoungHunter     UMETA(DisplayName = "Young Hunter"),
    Narrator        UMETA(DisplayName = "Narrator"),
    LoreStone       UMETA(DisplayName = "Lore Stone"),
};

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FString LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_SpeakerID Speaker = ENarr_SpeakerID::Elder;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TSoftObjectPtr<USoundBase> VoiceAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    float DisplayDuration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    bool bRequiresQuestActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName RequiredQuestID = NAME_None;
};

USTRUCT(BlueprintType)
struct FNarr_DialogueNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName NodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_DialogueTriggerType TriggerType = ENarr_DialogueTriggerType::Proximity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    bool bPlayOnce = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    float CooldownSeconds = 30.0f;
};

USTRUCT(BlueprintType)
struct FNarr_LoreEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Lore")
    FName LoreID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Lore")
    FString Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Lore")
    FString BodyText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Lore")
    bool bDiscovered = false;
};

// ─────────────────────────────────────────────────────────────────────────────
// Delegates
// ─────────────────────────────────────────────────────────────────────────────

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNarr_OnDialogueStarted, FName, NodeID, ENarr_SpeakerID, Speaker);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNarr_OnDialogueEnded,    FName, NodeID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNarr_OnLoreDiscovered,   FName, LoreID);

// ─────────────────────────────────────────────────────────────────────────────
// UDialogueSystem — ActorComponent
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent), DisplayName = "Dialogue System")
class TRANSPERSONALGAME_API UDialogueSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UDialogueSystem();

    // ── Dialogue nodes configured in editor ──────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueNode> DialogueNodes;

    // ── Lore entries this actor carries ─────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Lore")
    TArray<FNarr_LoreEntry> LoreEntries;

    // ── Proximity trigger radius ─────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Trigger", meta = (ClampMin = "50.0", ClampMax = "2000.0"))
    float ProximityRadius = 300.0f;

    // ── Currently playing ────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Narrative|State")
    bool bIsDialoguePlaying = false;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|State")
    FName ActiveNodeID = NAME_None;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|State")
    int32 ActiveLineIndex = 0;

    // ── Delegates ────────────────────────────────────────────────────────────
    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FNarr_OnDialogueStarted OnDialogueStarted;

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FNarr_OnDialogueEnded OnDialogueEnded;

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FNarr_OnLoreDiscovered OnLoreDiscovered;

    // ── Public API ────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void TriggerDialogueNode(FName NodeID);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void AdvanceDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool IsNodeOnCooldown(FName NodeID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void TriggerProximityCheck(AActor* PlayerActor);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Lore")
    bool DiscoverLoreEntry(FName LoreID);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Lore")
    FNarr_LoreEntry GetLoreEntry(FName LoreID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative|Lore")
    TArray<FNarr_LoreEntry> GetAllDiscoveredLore() const;

    // ── Called by CraftingSystem delegate ────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Narrative|Quest")
    void OnItemCrafted(FName ItemID);

    // ── Pre-built Elder dialogue setup ───────────────────────────────────────
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Narrative|Setup")
    void InitializeElderDialogue();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    // Cooldown tracking: NodeID → world time when last played
    UPROPERTY()
    TMap<FName, float> NodeCooldownMap;

    // Nodes played once and done
    UPROPERTY()
    TSet<FName> PlayedOnceNodes;

    // Timer handle for auto-advancing lines
    FTimerHandle LineAdvanceTimer;

    void AdvanceToNextLine();
    const FNarr_DialogueNode* FindNode(FName NodeID) const;
};
