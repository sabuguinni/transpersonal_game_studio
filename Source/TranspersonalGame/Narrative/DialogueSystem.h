#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "DialogueSystem.generated.h"

// ============================================================
// ENarr_DialogueCondition — conditions that gate dialogue lines
// ============================================================
UENUM(BlueprintType)
enum class ENarr_DialogueCondition : uint8
{
    None            UMETA(DisplayName = "None"),
    HasCampfire     UMETA(DisplayName = "Has Crafted Campfire"),
    HasSpear        UMETA(DisplayName = "Has Crafted Spear"),
    HasStoneAxe     UMETA(DisplayName = "Has Stone Axe"),
    HasLeatherWrap  UMETA(DisplayName = "Has Leather Wrap"),
    HerdStampeding  UMETA(DisplayName = "Herd Is Stampeding"),
    NearRaptorNest  UMETA(DisplayName = "Near Raptor Nest"),
    NearCampfire    UMETA(DisplayName = "Near Campfire Site"),
    FirstVisit      UMETA(DisplayName = "First Time Visiting NPC"),
    PlayerHealthLow UMETA(DisplayName = "Player Health Below 30pct"),
    NightTime       UMETA(DisplayName = "Is Night Time")
};

// ============================================================
// ENarr_NPCRole — what kind of NPC is speaking
// ============================================================
UENUM(BlueprintType)
enum class ENarr_NPCRole : uint8
{
    TribalElder   UMETA(DisplayName = "Tribal Elder"),
    ChiefHunter   UMETA(DisplayName = "Chief Hunter"),
    ScoutRunner   UMETA(DisplayName = "Scout Runner"),
    CampBuilder   UMETA(DisplayName = "Camp Builder"),
    Tracker       UMETA(DisplayName = "Tracker")
};

// ============================================================
// FNarr_DialogueLine — a single spoken line with condition
// ============================================================
USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueCondition RequiredCondition = ENarr_DialogueCondition::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bIsFirstVisitOnly = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDurationSeconds = 5.0f;
};

// ============================================================
// FNarr_NPCDialogueTree — full dialogue set for one NPC
// ============================================================
USTRUCT(BlueprintType)
struct FNarr_NPCDialogueTree
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_NPCRole Role = ENarr_NPCRole::TribalElder;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString DefaultLine;
};

// ============================================================
// UNarr_DialogueComponent — attach to any NPC actor
// ============================================================
UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_DialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FNarr_NPCDialogueTree DialogueTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float InteractionRadius = 300.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    bool bHasBeenVisited = false;

    // Returns the best matching line given current game state flags
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueLine GetBestLine(
        bool bPlayerHasCampfire,
        bool bPlayerHasSpear,
        bool bPlayerHasStoneAxe,
        bool bPlayerHasLeatherWrap,
        bool bHerdStampeding,
        bool bNearRaptorNest,
        bool bIsNight,
        float PlayerHealthPct
    );

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void MarkVisited();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsPlayerInRange(FVector PlayerLocation) const;

protected:
    virtual void BeginPlay() override;

private:
    bool ConditionMet(ENarr_DialogueCondition Cond,
        bool bHasCampfire, bool bHasSpear, bool bHasAxe, bool bHasWrap,
        bool bHerdStamping, bool bNearNest, bool bIsNight, float HealthPct) const;
};

// ============================================================
// ANarr_DialogueNPC — world NPC actor with dialogue
// ============================================================
UCLASS()
class TRANSPERSONALGAME_API ANarr_DialogueNPC : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueNPC();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative",
        meta = (AllowPrivateAccess = "true"))
    UNarr_DialogueComponent* DialogueComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative",
        meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* MeshComp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_NPCRole NPCRole = ENarr_NPCRole::TribalElder;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void InitialiseDialogueTree();

protected:
    virtual void BeginPlay() override;
};
