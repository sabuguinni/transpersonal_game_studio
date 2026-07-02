// TRexBehavior.h
// Agent #11 — NPC Behavior Agent
// T-Rex AI behavior component header

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TRexBehavior.generated.h"

// T-Rex AI states — 6 states covering full behavioral lifecycle
UENUM(BlueprintType)
enum class ENPC_TRexState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Investigate UMETA(DisplayName = "Investigate"),
    Chase       UMETA(DisplayName = "Chase"),
    Attack      UMETA(DisplayName = "Attack"),
    Roar        UMETA(DisplayName = "Roar")
};

// T-Rex behavior configuration — all tunable in editor
USTRUCT(BlueprintType)
struct FNPC_TRexConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Patrol")
    float PatrolRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Detection")
    float ChaseRange = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Movement")
    float PatrolSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Movement")
    float ChaseSpeed = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float AttackDamage = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Roar")
    float RoarCooldown = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Roar")
    float RoarRadius = 2000.0f;
};

// T-Rex behavior component — attach to any Pawn to give it T-Rex AI
UCLASS(ClassGroup = (AI), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UTRexBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTRexBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Config")
    FNPC_TRexConfig Config;

    // Current state — readable from Blueprint
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TRex|State", meta = (AllowPrivateAccess = "true"))
    ENPC_TRexState CurrentState;

    // Home location (set at BeginPlay)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TRex|State")
    FVector HomeLocation;

    // Last known player location (shared on roar)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TRex|State")
    FVector LastKnownPlayerLocation;

    // State query functions
    UFUNCTION(BlueprintCallable, Category = "TRex|State")
    ENPC_TRexState GetCurrentState() const;

    UFUNCTION(BlueprintCallable, Category = "TRex|State")
    FVector GetLastKnownPlayerLocation() const;

    // Manual state transition (callable from Blueprint)
    UFUNCTION(BlueprintCallable, Category = "TRex|State")
    void TransitionToState(ENPC_TRexState NewState);

private:
    void UpdateBehavior(float DeltaTime);
    void UpdateIdle(float DeltaTime, APawn* Player);
    void UpdatePatrol(float DeltaTime, APawn* Player);
    void UpdateInvestigate(float DeltaTime, APawn* Player);
    void UpdateChase(float DeltaTime, APawn* Player);
    void UpdateAttack(float DeltaTime, APawn* Player);
    void UpdateRoar(float DeltaTime, APawn* Player);

    APawn* DetectPlayer() const;
    float GetDistanceToPlayer(APawn* Player) const;
    void SelectNewPatrolWaypoint();
    void PerformAttack(APawn* Target);
    void BroadcastRoarAlert();

    FVector CurrentPatrolWaypoint;
    float TimeSinceLastRoar;
    float StateTimer;
    bool bIsInitialized;
};
