#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Investigate UMETA(DisplayName = "Investigate"),
    Chase       UMETA(DisplayName = "Chase"),
    Attack      UMETA(DisplayName = "Attack"),
    Flee        UMETA(DisplayName = "Flee"),
    Dead        UMETA(DisplayName = "Dead")
};

USTRUCT(BlueprintType)
struct FNPC_PatrolData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    TArray<FVector> WaypointLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    int32 CurrentWaypointIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    float WaypointAcceptanceRadius = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    float WaitTimeAtWaypoint = 2.0f;
};

USTRUCT(BlueprintType)
struct FNPC_ThreatMemory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    AActor* ThreatActor = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    float ThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    float TimeLastSeen = 0.0f;
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // --- State ---
    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    ENPC_BehaviorState CurrentState = ENPC_BehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float DetectionRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float AttackRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float FleeHealthThreshold = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float PatrolSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float ChaseSpeed = 700.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    FNPC_PatrolData PatrolData;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    FNPC_ThreatMemory ActiveThreat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float CurrentHealth = 1.0f;

    // --- Functions ---
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void RegisterThreat(AActor* ThreatActor, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void ClearThreat();

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    bool IsPlayerInDetectionRange() const;

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    bool IsPlayerInAttackRange() const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    FVector GetNextPatrolWaypoint();

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void AdvancePatrolWaypoint();

private:
    void UpdateBehaviorLogic(float DeltaTime);
    void ScanForThreats();
    float GetDistanceToPlayer() const;

    UPROPERTY()
    AActor* CachedPlayerActor = nullptr;

    float StateTimer = 0.0f;
    float ScanInterval = 0.5f;
    float ScanTimer = 0.0f;
};
