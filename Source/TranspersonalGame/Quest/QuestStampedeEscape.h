// QuestStampedeEscape.h
// Agent #14 — Quest & Mission Designer
// CYCLE: PROD_CYCLE_AUTO_20260628_011
// Quest: "Stampede Warning" — player detects fleeing herd, must escape the valley floor
// Integrates with Agent #13 CrowdHerdBehavior stampede markers

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "QuestStampedeEscape.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// Enums — global scope (UHT requirement)
// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EQuest_StampedePhase : uint8
{
    Inactive        UMETA(DisplayName = "Inactive"),
    HerdDetected    UMETA(DisplayName = "HerdDetected"),     // Player enters herd proximity
    StampedeBegins  UMETA(DisplayName = "StampedeBegins"),   // Herd starts fleeing
    EscapeWindow    UMETA(DisplayName = "EscapeWindow"),      // Player must reach high ground
    PlayerEscaped   UMETA(DisplayName = "PlayerEscaped"),     // Quest success
    PlayerCrushed   UMETA(DisplayName = "PlayerCrushed")      // Quest fail — player in path
};

UENUM(BlueprintType)
enum class EQuest_StampedeObjective : uint8
{
    ReachHighGround     UMETA(DisplayName = "ReachHighGround"),
    AvoidStampedePath   UMETA(DisplayName = "AvoidStampedePath"),
    InvestigateThreat   UMETA(DisplayName = "InvestigateThreat"),
    ReturnToCamp        UMETA(DisplayName = "ReturnToCamp")
};

// ─────────────────────────────────────────────────────────────────────────────
// Structs
// ─────────────────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FQuest_StampedeObjectiveData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Stampede")
    EQuest_StampedeObjective ObjectiveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Stampede")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Stampede")
    float CompletionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Stampede")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Stampede")
    FString ObjectiveLabel;

    FQuest_StampedeObjectiveData()
        : ObjectiveType(EQuest_StampedeObjective::ReachHighGround)
        , TargetLocation(FVector::ZeroVector)
        , CompletionRadius(300.f)
        , bIsCompleted(false)
        , ObjectiveLabel(TEXT("Reach high ground"))
    {}
};

// ─────────────────────────────────────────────────────────────────────────────
// Quest Trigger Volume — placed near herd center (3000, 2000, 100)
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (Quest), meta = (DisplayName = "Quest Stampede Trigger"))
class TRANSPERSONALGAME_API AQuestStampedeTrigger : public AActor
{
    GENERATED_BODY()

public:
    AQuestStampedeTrigger();

    // Trigger sphere — player entering starts the quest
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|Stampede",
        meta = (AllowPrivateAccess = "true"))
    USphereComponent* TriggerSphere;

    // Visual marker mesh (cylinder/sphere placeholder)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|Stampede",
        meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* MarkerMesh;

    // Radius within which player activates the quest
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Stampede")
    float DetectionRadius;

    // Current phase of the stampede quest
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|Stampede")
    EQuest_StampedePhase CurrentPhase;

    // Time window (seconds) player has to escape once stampede begins
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Stampede")
    float EscapeTimeLimit;

    // Elapsed time since stampede began
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|Stampede")
    float ElapsedEscapeTime;

    // Objectives for this quest
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Stampede")
    TArray<FQuest_StampedeObjectiveData> Objectives;

    // XP reward on success
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Stampede")
    int32 XPReward;

    // Has this quest already been triggered (prevent re-trigger)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|Stampede")
    bool bQuestTriggered;

    // ── Methods ──────────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Quest|Stampede")
    void ActivateQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest|Stampede")
    void TriggerStampedePhase();

    UFUNCTION(BlueprintCallable, Category = "Quest|Stampede")
    void CompleteObjective(EQuest_StampedeObjective ObjectiveType);

    UFUNCTION(BlueprintCallable, Category = "Quest|Stampede")
    void EvaluatePlayerPosition(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest|Stampede")
    bool IsPlayerInStampedePath(FVector PlayerLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Quest|Stampede")
    void QuestSuccess();

    UFUNCTION(BlueprintCallable, Category = "Quest|Stampede")
    void QuestFail();

    UFUNCTION(BlueprintCallable, Category = "Quest|Stampede")
    EQuest_StampedePhase GetCurrentPhase() const { return CurrentPhase; }

    UFUNCTION(BlueprintCallable, Category = "Quest|Stampede")
    float GetEscapeTimeRemaining() const;

    UFUNCTION(BlueprintCallable, Category = "Quest|Stampede")
    int32 GetCompletedObjectiveCount() const;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION()
    void OnTriggerOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

private:
    // Stampede path bounding box (northeast corridor from herd center)
    FVector StampedePathStart;
    FVector StampedePathEnd;
    float StampedePathWidth;
};

// ─────────────────────────────────────────────────────────────────────────────
// Quest Escape Zone — high ground target the player must reach
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (Quest), meta = (DisplayName = "Quest Stampede Escape Zone"))
class TRANSPERSONALGAME_API AQuestStampedeEscapeZone : public AActor
{
    GENERATED_BODY()

public:
    AQuestStampedeEscapeZone();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|Stampede",
        meta = (AllowPrivateAccess = "true"))
    USphereComponent* EscapeSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|Stampede",
        meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* EscapeMarker;

    // Radius to count as "escaped"
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Stampede")
    float EscapeRadius;

    // Which trigger quest this escape zone belongs to
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Stampede")
    TSoftObjectPtr<AQuestStampedeTrigger> LinkedQuestTrigger;

    UFUNCTION(BlueprintCallable, Category = "Quest|Stampede")
    void NotifyPlayerEscaped(AActor* PlayerActor);

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnEscapeOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);
};
