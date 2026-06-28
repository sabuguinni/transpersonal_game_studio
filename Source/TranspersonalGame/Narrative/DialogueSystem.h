// DialogueSystem.h
// Agent #15 — Narrative & Dialogue
// Cycle: PROD_CYCLE_AUTO_20260628_009
// Dialogue trigger, NPC conversation, and lore fragment system

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DataTable.h"
#include "DialogueSystem.generated.h"

// ─── Enums ───────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ENarr_DialogueTrigger : uint8
{
    Proximity       UMETA(DisplayName = "Proximity"),
    QuestStart      UMETA(DisplayName = "QuestStart"),
    QuestComplete   UMETA(DisplayName = "QuestComplete"),
    ItemPickup      UMETA(DisplayName = "ItemPickup"),
    CombatNear      UMETA(DisplayName = "CombatNear"),
    FirstCraft      UMETA(DisplayName = "FirstCraft"),
    DinosaурSeen    UMETA(DisplayName = "DinosaurSeen"),
    PlayerDeath     UMETA(DisplayName = "PlayerDeath"),
    NightFall       UMETA(DisplayName = "NightFall"),
    Manual          UMETA(DisplayName = "Manual")
};

UENUM(BlueprintType)
enum class ENarr_CharacterRole : uint8
{
    TribeElder      UMETA(DisplayName = "TribeElder"),
    Scout           UMETA(DisplayName = "Scout"),
    Crafter         UMETA(DisplayName = "Crafter"),
    Hunter          UMETA(DisplayName = "Hunter"),
    Survivor        UMETA(DisplayName = "Survivor"),
    Narrator        UMETA(DisplayName = "Narrator")
};

UENUM(BlueprintType)
enum class ENarr_LoreCategory : uint8
{
    DinosaurBehavior    UMETA(DisplayName = "DinosaurBehavior"),
    SurvivalTip         UMETA(DisplayName = "SurvivalTip"),
    TribeHistory        UMETA(DisplayName = "TribeHistory"),
    TerrainWarning      UMETA(DisplayName = "TerrainWarning"),
    CraftingKnowledge   UMETA(DisplayName = "CraftingKnowledge"),
    MissingPerson       UMETA(DisplayName = "MissingPerson")
};

// ─── Structs ─────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_CharacterRole SpeakerRole = ENarr_CharacterRole::Narrator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueTrigger TriggerCondition = ENarr_DialogueTrigger::Proximity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioAssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDurationSeconds = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bHasBeenPlayed = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bPlayOnce = true;
};

USTRUCT(BlueprintType)
struct FNarr_LoreFragment : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString FragmentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_LoreCategory Category = ENarr_LoreCategory::SurvivalTip;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString BodyText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bDiscovered = false;
};

USTRUCT(BlueprintType)
struct FNarr_ConversationNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString NodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FNarr_DialogueLine Line;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FString> NextNodeIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString RequiredQuestID;
};

// ─── Dialogue Trigger Volume ──────────────────────────────────────────────────

UCLASS(ClassGroup = "Narrative", meta = (DisplayName = "Narr_DialogueTriggerVolume"))
class TRANSPERSONALGAME_API ANarr_DialogueTriggerVolume : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueTriggerVolume();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
              meta = (AllowPrivateAccess = "true"))
    USphereComponent* TriggerSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
              meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* DebugMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueTrigger TriggerType = ENarr_DialogueTrigger::Proximity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float TriggerRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bTriggered = false;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerDialogue(AActor* Instigator);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void ResetTrigger();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    int32 GetUnplayedLineCount() const;

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnSphereOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                         UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                         bool bFromSweep, const FHitResult& SweepResult);

private:
    int32 CurrentLineIndex;
};

// ─── Lore Stone (carved rock with lore fragment) ──────────────────────────────

UCLASS(ClassGroup = "Narrative", meta = (DisplayName = "Narr_LoreStone"))
class TRANSPERSONALGAME_API ANarr_LoreStone : public AActor
{
    GENERATED_BODY()

public:
    ANarr_LoreStone();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
              meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* StoneMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
              meta = (AllowPrivateAccess = "true"))
    USphereComponent* InteractSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FNarr_LoreFragment LoreData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bHasBeenRead = false;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void ReadLore(AActor* Reader);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FString GetLoreText() const;

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnInteractOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                           UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                           bool bFromSweep, const FHitResult& SweepResult);
};

// ─── Dialogue Manager (singleton-style actor) ─────────────────────────────────

UCLASS(ClassGroup = "Narrative", meta = (DisplayName = "Narr_DialogueManager"))
class TRANSPERSONALGAME_API ANarr_DialogueManager : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_LoreFragment> DiscoveredLore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FString> PlayedDialogueIDs;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterLoreDiscovery(const FNarr_LoreFragment& Fragment);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool HasLoreBeenDiscovered(const FString& FragmentID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    int32 GetTotalLoreCount() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    int32 GetDiscoveredLoreCount() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void LogDialoguePlayed(const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool WasDialoguePlayed(const FString& DialogueID) const;

protected:
    virtual void BeginPlay() override;

private:
    static ANarr_DialogueManager* Instance;
};
