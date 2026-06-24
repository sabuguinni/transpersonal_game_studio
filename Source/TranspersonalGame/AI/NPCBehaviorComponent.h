#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Alert       UMETA(DisplayName = "Alert"),
    Chase       UMETA(DisplayName = "Chase"),
    Attack      UMETA(DisplayName = "Attack"),
    Flee        UMETA(DisplayName = "Flee"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ENPC_DinoPersonality : uint8
{
    TerritorialPatrol   UMETA(DisplayName = "Territorial Patrol"),
    PackHunter          UMETA(DisplayName = "Pack Hunter"),
    GrazerDefensive     UMETA(DisplayName = "Grazer Defensive"),
    PassiveHerd         UMETA(DisplayName = "Passive Herd"),
    AlertFlee           UMETA(DisplayName = "Alert Flee")
};

USTRUCT(BlueprintType)
struct FNPC_BehaviorMemory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    FVector LastKnownPlayerLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float TimeSinceLastPlayerSighting = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    bool bHasSeenPlayer = false;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    bool bIsAlerted = false;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float ThreatLevel = 0.0f;
};

USTRUCT(BlueprintType)
struct FNPC_PatrolData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Patrol")
    TArray<FVector> WaypointLocations;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Patrol")
    int32 CurrentWaypointIndex = 0;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Patrol")
    float WaypointAcceptanceRadius = 200.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Patrol")
    float WaitTimeAtWaypoint = 3.0f;
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === State Machine ===
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    ENPC_BehaviorState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void OnPlayerDetected(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void OnPlayerLost();

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void OnTakeDamage(float DamageAmount);

    // === Patrol ===
    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    void SetPatrolWaypoints(const TArray<FVector>& Waypoints);

    UFUNCTION(BlueprintPure, Category = "NPC|Patrol")
    FVector GetNextWaypointLocation() const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    void AdvanceToNextWaypoint();

    // === Memory ===
    UFUNCTION(BlueprintPure, Category = "NPC|Memory")
    FNPC_BehaviorMemory GetBehaviorMemory() const { return Memory; }

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void UpdateThreatLevel(float Delta);

    // === Config ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    ENPC_DinoPersonality Personality = ENPC_DinoPersonality::TerritorialPatrol;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float DetectionRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float AttackRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float FleeHealthThreshold = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float MaxHealth = 1000.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Config", meta = (AllowPrivateAccess = "true"))
    float CurrentHealth = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    FNPC_PatrolData PatrolData;

private:
    UPROPERTY()
    ENPC_BehaviorState CurrentState = ENPC_BehaviorState::Idle;

    UPROPERTY()
    FNPC_BehaviorMemory Memory;

    void TickIdle(float DeltaTime);
    void TickPatrol(float DeltaTime);
    void TickAlert(float DeltaTime);
    void TickChase(float DeltaTime);
    void TickAttack(float DeltaTime);
    void TickFlee(float DeltaTime);

    void EvaluateStateTransitions();
    float GetDistanceToPlayer() const;

    float WaypointWaitTimer = 0.0f;
    bool bWaitingAtWaypoint = false;
};
