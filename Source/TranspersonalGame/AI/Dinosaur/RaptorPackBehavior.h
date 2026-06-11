#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "SharedTypes.h"
#include "RaptorPackBehavior.generated.h"

class ARaptorPackBehavior;
class UBehaviorTree;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FRaptorPackMember
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Pack")
    ARaptorPackBehavior* RaptorController;

    UPROPERTY(BlueprintReadWrite, Category = "Pack")
    FVector RelativePosition;

    UPROPERTY(BlueprintReadWrite, Category = "Pack")
    ERaptorRole PackRole;

    UPROPERTY(BlueprintReadWrite, Category = "Pack")
    bool bIsAlive;

    FRaptorPackMember()
    {
        RaptorController = nullptr;
        RelativePosition = FVector::ZeroVector;
        PackRole = ERaptorRole::Follower;
        bIsAlive = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ARaptorPackBehavior : public AAIController
{
    GENERATED_BODY()

public:
    ARaptorPackBehavior();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;

    // Behavior Tree
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    UBehaviorTree* BehaviorTree;

    // AI Perception
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAIPerceptionComponent* AIPerceptionComponent;

    // Pack Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float PackRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float HuntRange = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    int32 MaxPackSize = 3;

    // Pack Management
    UPROPERTY(BlueprintReadOnly, Category = "Pack")
    TArray<FRaptorPackMember> PackMembers;

    UPROPERTY(BlueprintReadOnly, Category = "Pack")
    ARaptorPackBehavior* PackLeader;

    UPROPERTY(BlueprintReadOnly, Category = "Pack")
    ERaptorRole MyPackRole;

    UPROPERTY(BlueprintReadOnly, Category = "Pack")
    bool bIsPackLeader;

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "State")
    ENPCBehaviorState CurrentBehaviorState;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    APawn* TargetPrey;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FVector PackCenterLocation;

    // Blackboard Keys
    UPROPERTY(EditDefaultsOnly, Category = "AI")
    FName TargetActorKey = TEXT("TargetActor");

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    FName PackLeaderKey = TEXT("PackLeader");

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    FName PackRoleKey = TEXT("PackRole");

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    FName BehaviorStateKey = TEXT("BehaviorState");

public:
    // Pack Management Functions
    UFUNCTION(BlueprintCallable, Category = "Pack")
    void JoinPack(ARaptorPackBehavior* Leader);

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void BecomePackLeader();

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void AddPackMember(ARaptorPackBehavior* NewMember, ERaptorRole Role);

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void RemovePackMember(ARaptorPackBehavior* Member);

    UFUNCTION(BlueprintCallable, Category = "Pack")
    FVector GetFormationPosition() const;

    // Hunting Behavior
    UFUNCTION(BlueprintCallable, Category = "Hunting")
    void StartHunting(APawn* Target);

    UFUNCTION(BlueprintCallable, Category = "Hunting")
    void CoordinateAttack(APawn* Target);

    UFUNCTION(BlueprintCallable, Category = "Hunting")
    void FlankTarget(APawn* Target, bool bLeftFlank);

    UFUNCTION(BlueprintCallable, Category = "Hunting")
    void ReturnToPatrol();

    // Communication
    UFUNCTION(BlueprintCallable, Category = "Communication")
    void BroadcastToPackMembers(ENPCBehaviorState NewState, APawn* Target = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Communication")
    void ReceivePackCommand(ENPCBehaviorState Command, APawn* Target = nullptr);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Pack")
    int32 GetAlivePackMemberCount() const;

    UFUNCTION(BlueprintCallable, Category = "Pack")
    float GetDistanceToPackLeader() const;

    UFUNCTION(BlueprintCallable, Category = "Pack")
    bool IsTargetInHuntRange(APawn* Target) const;

protected:
    // Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // Internal Functions
    void UpdatePackBehavior();
    void UpdateFormationPosition();
    void FindNearbyRaptors();
    void SetBlackboardBehaviorState(ENPCBehaviorState NewState);
    void InitializeBlackboard();
    FVector CalculateFlankingPosition(APawn* Target, bool bLeftFlank) const;
};