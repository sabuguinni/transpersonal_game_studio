#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "../SharedTypes.h"
#include "Combat_TacticalAIController.generated.h"

class ACharacter;
class UBehaviorTree;
class AActor;

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_TacticalAIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombat_TacticalAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // AI Perception System
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    UAISightConfig* SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    UAIHearingConfig* HearingConfig;

    // Behavior Tree and Blackboard
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    UBehaviorTree* BehaviorTree;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Behavior")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Behavior")
    UBlackboardComponent* BlackboardComponent;

    // Dinosaur Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Config")
    EEng_DinosaurSpecies DinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Config")
    EEng_DinosaurDiet Diet;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Config")
    EEng_DinosaurBehavior CurrentBehavior;

    // Combat Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float AggressionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (ClampMin = "100.0", ClampMax = "5000.0"))
    float TerritoryRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (ClampMin = "100.0", ClampMax = "3000.0"))
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (ClampMin = "100.0", ClampMax = "2000.0"))
    float FleeRange;

    // Tactical AI State
    UPROPERTY(BlueprintReadOnly, Category = "Tactical State")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Tactical State")
    FVector HomeLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Tactical State")
    TArray<FVector> PatrolWaypoints;

    UPROPERTY(BlueprintReadOnly, Category = "Tactical State")
    int32 CurrentWaypointIndex;

    UPROPERTY(BlueprintReadOnly, Category = "Tactical State")
    bool bIsAlerted;

    UPROPERTY(BlueprintReadOnly, Category = "Tactical State")
    bool bIsHunting;

    UPROPERTY(BlueprintReadOnly, Category = "Tactical State")
    float LastPlayerSightTime;

    // Pack Coordination (for Raptors)
    UPROPERTY(BlueprintReadOnly, Category = "Pack Behavior")
    TArray<ACombat_TacticalAIController*> PackMembers;

    UPROPERTY(BlueprintReadOnly, Category = "Pack Behavior")
    bool bIsPackLeader;

    UPROPERTY(BlueprintReadOnly, Category = "Pack Behavior")
    FVector PackTargetLocation;

public:
    // AI Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void SetBehaviorState(EEng_DinosaurBehavior NewBehavior);

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void AlertNearbyDinosaurs(AActor* ThreatTarget);

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void StartHunting(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void StartFleeing(AActor* ThreatSource);

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void ReturnToPatrol();

    // Tactical Functions
    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    FVector GetBestAttackPosition(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    FVector GetFlankingPosition(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    bool ShouldEngageTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    bool ShouldFleeFromTarget(AActor* Target);

    // Pack Coordination Functions
    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void JoinPack(ACombat_TacticalAIController* PackLeader);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void CoordinatePackAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    FVector GetPackFormationPosition();

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "AI Utility")
    float GetDistanceToTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "AI Utility")
    bool IsTargetInTerritory(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "AI Utility")
    void SetupPatrolWaypoints();

protected:
    // AI Perception Callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Internal AI Logic
    void UpdateAIBehavior(float DeltaTime);
    void ProcessCarnivoreAI(float DeltaTime);
    void ProcessHerbivoreAI(float DeltaTime);
    void UpdatePackCoordination(float DeltaTime);
    void CheckTerritorialBehavior();

    // Combat Logic
    bool CanAttackTarget(AActor* Target);
    void ExecuteAttackSequence();
    void ExecuteFleeSequence();
    void ExecuteAmbushSequence();

private:
    float AlertCooldownTime;
    float LastAlertTime;
    float PatrolSpeed;
    float HuntSpeed;
    float FleeSpeed;
};