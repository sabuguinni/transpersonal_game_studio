#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StampedeController.generated.h"

// ============================================================
// Stampede Controller — Agent #13 Crowd & Traffic Simulation
// Manages mass panic/stampede events for dinosaur herds.
// Triggered by: predator encounter, player proximity, loud noise.
// Up to 50 agents driven by simple flocking + terrain avoidance.
// ============================================================

UENUM(BlueprintType)
enum class ECrowd_StampedePhase : uint8
{
    Inactive        UMETA(DisplayName = "Inactive"),
    Triggered       UMETA(DisplayName = "Triggered"),
    Accelerating    UMETA(DisplayName = "Accelerating"),
    FullRun         UMETA(DisplayName = "Full Run"),
    Dispersing      UMETA(DisplayName = "Dispersing"),
    Settled         UMETA(DisplayName = "Settled")
};

UENUM(BlueprintType)
enum class ECrowd_StampedeCause : uint8
{
    PredatorSighted     UMETA(DisplayName = "Predator Sighted"),
    PlayerTooClose      UMETA(DisplayName = "Player Too Close"),
    LoudNoise           UMETA(DisplayName = "Loud Noise"),
    MemberKilled        UMETA(DisplayName = "Member Killed"),
    FireOrExplosion     UMETA(DisplayName = "Fire or Explosion")
};

USTRUCT(BlueprintType)
struct FCrowd_StampedeAgent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    float MaxSpeed = 1200.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    bool bIsLeader = false;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    int32 AgentIndex = -1;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    bool bIsAlive = true;
};

USTRUCT(BlueprintType)
struct FCrowd_StampedeEvent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    FVector TriggerLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    FVector FleeDirection = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    ECrowd_StampedeCause Cause = ECrowd_StampedeCause::PredatorSighted;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    float TriggerRadius = 3000.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    float PanicRadius = 6000.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    float EventTimestamp = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    int32 AgentsAffected = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCrowd_OnStampedeStarted,
    FVector, TriggerLocation, ECrowd_StampedeCause, Cause);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCrowd_OnStampedeEnded,
    int32, AgentsSurvived);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCrowd_OnAgentTrampled,
    int32, AgentIndex, FVector, Location);

UCLASS(ClassGroup = "Crowd", meta = (DisplayName = "Stampede Controller"))
class TRANSPERSONALGAME_API AStampedeController : public AActor
{
    GENERATED_BODY()

public:
    AStampedeController();

    // ── Configuration ─────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede|Config")
    int32 MaxAgents = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede|Config")
    float AgentSeparationRadius = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede|Config")
    float CohesionRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede|Config")
    float AlignmentRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede|Config")
    float TerrainAvoidanceRadius = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede|Config")
    float PlayerDangerRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede|Config")
    float StampedeAcceleration = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede|Config")
    float SettleTime = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede|Config")
    float TramplingDamage = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede|Config")
    float TramplingRadius = 200.0f;

    // ── State ─────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Stampede|State",
              meta = (AllowPrivateAccess = "true"))
    ECrowd_StampedePhase CurrentPhase = ECrowd_StampedePhase::Inactive;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Stampede|State",
              meta = (AllowPrivateAccess = "true"))
    FCrowd_StampedeEvent ActiveEvent;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Stampede|State",
              meta = (AllowPrivateAccess = "true"))
    TArray<FCrowd_StampedeAgent> Agents;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Stampede|State",
              meta = (AllowPrivateAccess = "true"))
    float PhaseElapsedTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Stampede|State",
              meta = (AllowPrivateAccess = "true"))
    int32 AgentsSurvivedCount = 0;

    // ── Delegates ─────────────────────────────────────────────
    UPROPERTY(BlueprintAssignable, Category = "Crowd|Stampede|Events")
    FCrowd_OnStampedeStarted OnStampedeStarted;

    UPROPERTY(BlueprintAssignable, Category = "Crowd|Stampede|Events")
    FCrowd_OnStampedeEnded OnStampedeEnded;

    UPROPERTY(BlueprintAssignable, Category = "Crowd|Stampede|Events")
    FCrowd_OnAgentTrampled OnAgentTrampled;

    // ── Public API ────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void TriggerStampede(FVector TriggerLocation, ECrowd_StampedeCause Cause,
                         FVector FleeDirection);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void AddAgent(FVector SpawnLocation, bool bIsLeader = false);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void ClearAllAgents();

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    bool IsStampedeActive() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    int32 GetActiveAgentCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    ECrowd_StampedePhase GetCurrentPhase() const { return CurrentPhase; }

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void ForceSettle();

    // ── AActor overrides ──────────────────────────────────────
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

protected:
    void UpdatePhase(float DeltaTime);
    void UpdateAgentFlocking(float DeltaTime);
    FVector ComputeFlockingForce(const FCrowd_StampedeAgent& Agent) const;
    FVector ComputeSeparation(const FCrowd_StampedeAgent& Agent) const;
    FVector ComputeCohesion(const FCrowd_StampedeAgent& Agent) const;
    FVector ComputeAlignment(const FCrowd_StampedeAgent& Agent) const;
    void CheckPlayerTrampling();
    void TransitionToPhase(ECrowd_StampedePhase NewPhase);

private:
    float TickInterval = 0.05f; // 20Hz for crowd simulation
    float TickAccumulator = 0.0f;
};
