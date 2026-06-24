#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorComponent.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// NPC Behavior States — prehistoric survivor AI
// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Investigate UMETA(DisplayName = "Investigate"),
    Alert       UMETA(DisplayName = "Alert"),
    Flee        UMETA(DisplayName = "Flee"),
    Hide        UMETA(DisplayName = "Hide"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ENPC_ThreatLevel : uint8
{
    None    UMETA(DisplayName = "None"),
    Low     UMETA(DisplayName = "Low"),
    Medium  UMETA(DisplayName = "Medium"),
    High    UMETA(DisplayName = "High"),
    Extreme UMETA(DisplayName = "Extreme")
};

// ─────────────────────────────────────────────────────────────────────────────
// Memory entry — what this NPC remembers about a threat
// ─────────────────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FNPC_ThreatMemory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float TimeSinceLastSeen = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    ENPC_ThreatLevel ThreatLevel = ENPC_ThreatLevel::None;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    bool bIsActive = false;
};

// ─────────────────────────────────────────────────────────────────────────────
// NPCBehaviorComponent — drives prehistoric survivor NPC logic
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ── State machine ──────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    ENPC_BehaviorState GetBehaviorState() const { return CurrentState; }

    // ── Threat perception ──────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "NPC|Perception")
    void RegisterThreat(AActor* ThreatActor, ENPC_ThreatLevel Level, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "NPC|Perception")
    void ClearThreat();

    UFUNCTION(BlueprintPure, Category = "NPC|Perception")
    bool HasActiveThreat() const { return ThreatMemory.bIsActive; }

    UFUNCTION(BlueprintPure, Category = "NPC|Perception")
    FNPC_ThreatMemory GetThreatMemory() const { return ThreatMemory; }

    // ── Patrol ────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    void SetPatrolPoints(const TArray<FVector>& Points);

    UFUNCTION(BlueprintPure, Category = "NPC|Patrol")
    FVector GetNextPatrolPoint() const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    void AdvancePatrolPoint();

    // ── Flee ──────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintPure, Category = "NPC|Flee")
    FVector ComputeFleeDirection() const;

    // ── Config ────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float AlertRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float FleeRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float MemoryDecayRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float PatrolWaitTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float InvestigateTimeout = 8.0f;

protected:
    void UpdateMemoryDecay(float DeltaTime);
    void EvaluateStateTransitions();
    void TickPatrol(float DeltaTime);
    void TickInvestigate(float DeltaTime);

private:
    UPROPERTY(VisibleAnywhere, Category = "NPC|State", meta = (AllowPrivateAccess = "true"))
    ENPC_BehaviorState CurrentState = ENPC_BehaviorState::Idle;

    UPROPERTY(VisibleAnywhere, Category = "NPC|State", meta = (AllowPrivateAccess = "true"))
    FNPC_ThreatMemory ThreatMemory;

    UPROPERTY(VisibleAnywhere, Category = "NPC|Patrol", meta = (AllowPrivateAccess = "true"))
    TArray<FVector> PatrolPoints;

    UPROPERTY(VisibleAnywhere, Category = "NPC|Patrol", meta = (AllowPrivateAccess = "true"))
    int32 CurrentPatrolIndex = 0;

    UPROPERTY(VisibleAnywhere, Category = "NPC|Patrol", meta = (AllowPrivateAccess = "true"))
    float PatrolWaitTimer = 0.0f;

    UPROPERTY(VisibleAnywhere, Category = "NPC|Investigate", meta = (AllowPrivateAccess = "true"))
    float InvestigateTimer = 0.0f;

    UPROPERTY()
    TWeakObjectPtr<AActor> TrackedThreatActor;
};
