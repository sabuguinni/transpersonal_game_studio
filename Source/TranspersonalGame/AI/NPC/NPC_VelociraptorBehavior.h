#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "NPC_VelociraptorBehavior.generated.h"

UENUM(BlueprintType)
enum class ENPC_VelociraptorState : uint8
{
    Idle,
    Patrolling,
    Hunting,
    Attacking,
    Fleeing,
    Feeding,
    Socializing
};

USTRUCT(BlueprintType)
struct FNPC_VelociraptorPack
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    TArray<AActor*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    AActor* PackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    FVector PackCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    float PackRadius;

    FNPC_VelociraptorPack()
    {
        PackMembers.Empty();
        PackLeader = nullptr;
        PackCenter = FVector::ZeroVector;
        PackRadius = 2000.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_VelociraptorBehavior : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_VelociraptorBehavior();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Behavior State Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior State")
    ENPC_VelociraptorState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior State")
    float StateTimer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior State")
    AActor* CurrentTarget;

    // Pack Behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    FNPC_VelociraptorPack PackData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    bool bIsPackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    float PackCommunicationRange;

    // Hunting Behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting")
    float HuntingRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting")
    float FleeHealthThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting")
    TArray<TSubclassOf<AActor>> PreferredPreyTypes;

    // Patrol Behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    TArray<FVector> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    int32 CurrentPatrolIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    float PatrolSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    float PatrolWaitTime;

    // Social Behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float SocialInteractionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float SocialInteractionCooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float LastSocialInteraction;

    // Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetBehaviorState(ENPC_VelociraptorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void UpdateBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void JoinPack(UNPC_VelociraptorBehavior* OtherRaptor);

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void UpdatePackBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Hunting")
    AActor* FindNearestPrey();

    UFUNCTION(BlueprintCallable, Category = "Hunting")
    void StartHunt(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Hunting")
    void ExecutePackHunt();

    UFUNCTION(BlueprintCallable, Category = "Patrol")
    void InitializePatrolRoute();

    UFUNCTION(BlueprintCallable, Category = "Patrol")
    void UpdatePatrol(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Social")
    void PerformSocialBehavior();

private:
    FTimerHandle BehaviorUpdateTimer;
    FTimerHandle PackCommunicationTimer;
    
    void OnBehaviorUpdate();
    void OnPackCommunication();
    
    float GetDistanceToTarget(AActor* Target) const;
    bool IsTargetInRange(AActor* Target, float Range) const;
    void MoveTowardsTarget(AActor* Target, float Speed);
    void MoveToLocation(const FVector& Location, float Speed);
};