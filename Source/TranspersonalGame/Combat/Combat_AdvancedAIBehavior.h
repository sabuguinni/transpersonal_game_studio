#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Components/SphereComponent.h"
#include "Combat_AdvancedAIBehavior.generated.h"

UENUM(BlueprintType)
enum class ECombat_AIBehaviorState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Patrolling      UMETA(DisplayName = "Patrolling"),
    Investigating   UMETA(DisplayName = "Investigating"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Attacking       UMETA(DisplayName = "Attacking"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    PackHunting     UMETA(DisplayName = "Pack Hunting"),
    Territorial     UMETA(DisplayName = "Territorial"),
    Wounded         UMETA(DisplayName = "Wounded")
};

UENUM(BlueprintType)
enum class ECombat_DinosaurType : uint8
{
    ApexPredator    UMETA(DisplayName = "Apex Predator"),
    PackHunter      UMETA(DisplayName = "Pack Hunter"),
    SoloPredator    UMETA(DisplayName = "Solo Predator"),
    Herbivore       UMETA(DisplayName = "Herbivore"),
    Scavenger       UMETA(DisplayName = "Scavenger")
};

USTRUCT(BlueprintType)
struct FCombat_AIBehaviorData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    ECombat_AIBehaviorState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    ECombat_DinosaurType DinosaurType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    float DetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    float AttackRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    float FleeRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    float PatrolRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    float Aggression;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    float Fear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    float Health;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    bool bIsPackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    bool bIsInPack;

    FCombat_AIBehaviorData()
    {
        CurrentState = ECombat_AIBehaviorState::Idle;
        DinosaurType = ECombat_DinosaurType::SoloPredator;
        DetectionRadius = 1500.0f;
        AttackRadius = 300.0f;
        FleeRadius = 200.0f;
        PatrolRadius = 2000.0f;
        Aggression = 0.5f;
        Fear = 0.3f;
        Health = 100.0f;
        bIsPackLeader = false;
        bIsInPack = false;
    }
};

USTRUCT(BlueprintType)
struct FCombat_PackCoordinationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Coordination")
    TArray<AActor*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Coordination")
    AActor* PackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Coordination")
    AActor* CurrentTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Coordination")
    FVector FormationCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Coordination")
    float FormationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Coordination")
    bool bIsCoordinating;

    FCombat_PackCoordinationData()
    {
        PackLeader = nullptr;
        CurrentTarget = nullptr;
        FormationCenter = FVector::ZeroVector;
        FormationRadius = 500.0f;
        bIsCoordinating = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_AdvancedAIBehavior : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_AdvancedAIBehavior();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // AI Behavior Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    FCombat_AIBehaviorData BehaviorData;

    // Pack Coordination Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Coordination")
    FCombat_PackCoordinationData PackData;

    // Patrol Points
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    TArray<FVector> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    int32 CurrentPatrolIndex;

    // Detection
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Detection")
    class USphereComponent* DetectionSphere;

    // Current Target
    UPROPERTY(BlueprintReadOnly, Category = "AI Behavior")
    AActor* CurrentTarget;

    // State Management
    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void SetBehaviorState(ECombat_AIBehaviorState NewState);

    UFUNCTION(BlueprintPure, Category = "AI Behavior")
    ECombat_AIBehaviorState GetBehaviorState() const;

    // Pack Coordination
    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    void JoinPack(AActor* Leader);

    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    void SetAsPackLeader();

    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    void CoordinatePackAttack(AActor* Target);

    // Patrol System
    UFUNCTION(BlueprintCallable, Category = "Patrol")
    void SetupPatrolRoute(const TArray<FVector>& Points);

    UFUNCTION(BlueprintCallable, Category = "Patrol")
    FVector GetNextPatrolPoint();

    // Detection Events
    UFUNCTION()
    void OnDetectionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnDetectionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    // Behavior Logic
    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void UpdateBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void ProcessIdleBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void ProcessPatrolBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void ProcessHuntingBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void ProcessAttackingBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void ProcessPackHuntingBehavior(float DeltaTime);

    // Utility Functions
    UFUNCTION(BlueprintPure, Category = "AI Behavior")
    float GetDistanceToTarget() const;

    UFUNCTION(BlueprintPure, Category = "AI Behavior")
    bool IsTargetInAttackRange() const;

    UFUNCTION(BlueprintPure, Category = "AI Behavior")
    bool IsTargetInDetectionRange() const;

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void ClearTarget();

private:
    // Internal timers
    float StateChangeTimer;
    float PatrolWaitTimer;
    float AttackCooldownTimer;
    float PackCoordinationTimer;

    // Internal flags
    bool bIsMovingToPatrolPoint;
    bool bIsWaitingAtPatrolPoint;
    bool bHasValidTarget;

    // Helper functions
    void InitializeDetectionSphere();
    void UpdatePackFormation();
    void CheckForThreats();
    void EvaluateStateTransitions();
    FVector CalculatePackFormationPosition();
};

#include "Combat_AdvancedAIBehavior.generated.h"