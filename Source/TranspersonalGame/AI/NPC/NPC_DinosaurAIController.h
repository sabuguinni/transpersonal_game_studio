#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "Engine/Engine.h"
#include "TranspersonalGame/Shared/SharedTypes.h"
#include "NPC_DinosaurAIController.generated.h"

class UBehaviorTree;
class UBlackboardData;
class APawn;
class AActor;

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANPC_DinosaurAIController : public AAIController
{
    GENERATED_BODY()

public:
    ANPC_DinosaurAIController();

protected:
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    // Behavior Tree and Blackboard
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
    UBehaviorTree* BehaviorTree;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
    UBlackboardData* BlackboardAsset;

    // AI Perception
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
    UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
    UAISightConfig* SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
    UAIHearingConfig* HearingConfig;

    // Dinosaur Behavior Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    ENPC_DinosaurSpecies DinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    ENPC_DinosaurState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    float PatrolRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    float ChaseDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    float AttackDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    float SightRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    float HearingRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    float AggressionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    bool bIsPackAnimal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    bool bIsNocturnal;

    // Target and Threat Management
    UPROPERTY(BlueprintReadOnly, Category = "AI")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "AI")
    FVector HomeLocation;

    UPROPERTY(BlueprintReadOnly, Category = "AI")
    FVector LastKnownPlayerLocation;

    UPROPERTY(BlueprintReadOnly, Category = "AI")
    float LastPlayerSightTime;

    // Pack Behavior
    UPROPERTY(BlueprintReadOnly, Category = "Pack Behavior")
    TArray<ANPC_DinosaurAIController*> PackMembers;

    UPROPERTY(BlueprintReadOnly, Category = "Pack Behavior")
    ANPC_DinosaurAIController* PackLeader;

    UPROPERTY(BlueprintReadOnly, Category = "Pack Behavior")
    bool bIsPackLeader;

public:
    // AI Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetDinosaurSpecies(ENPC_DinosaurSpecies NewSpecies);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetDinosaurState(ENPC_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void ClearTarget();

    UFUNCTION(BlueprintCallable, Category = "AI")
    bool HasValidTarget() const;

    UFUNCTION(BlueprintCallable, Category = "AI")
    float GetDistanceToTarget() const;

    UFUNCTION(BlueprintCallable, Category = "AI")
    bool IsPlayerInSight() const;

    UFUNCTION(BlueprintCallable, Category = "AI")
    bool IsPlayerInAttackRange() const;

    UFUNCTION(BlueprintCallable, Category = "AI")
    void StartPatrolling();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void StartChasing(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void StartAttacking();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void ReturnToHome();

    // Pack Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void JoinPack(ANPC_DinosaurAIController* Leader);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void BecomePackLeader();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void AddPackMember(ANPC_DinosaurAIController* Member);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void RemovePackMember(ANPC_DinosaurAIController* Member);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void AlertPack(AActor* Threat);

protected:
    // AI Perception Callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Internal Helper Functions
    void InitializeAIPerception();
    void ConfigureBehaviorTree();
    void UpdateBlackboardValues();
    void SetupSpeciesDefaults();
    FVector GetRandomPatrolPoint();
    bool IsLocationSafe(const FVector& Location) const;
    void HandleStateTransition(ENPC_DinosaurState NewState);
};