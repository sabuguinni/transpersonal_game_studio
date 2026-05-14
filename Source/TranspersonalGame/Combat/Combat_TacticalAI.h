#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "SharedTypes.h"
#include "Combat_TacticalAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_TacticalState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Hunt        UMETA(DisplayName = "Hunt"),
    Engage      UMETA(DisplayName = "Engage"),
    Flank       UMETA(DisplayName = "Flank"),
    Retreat     UMETA(DisplayName = "Retreat"),
    Regroup     UMETA(DisplayName = "Regroup")
};

UENUM(BlueprintType)
enum class ECombat_Formation : uint8
{
    None        UMETA(DisplayName = "None"),
    Line        UMETA(DisplayName = "Line"),
    Circle      UMETA(DisplayName = "Circle"),
    Pincer      UMETA(DisplayName = "Pincer"),
    Ambush      UMETA(DisplayName = "Ambush")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticalData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    ECombat_TacticalState CurrentState = ECombat_TacticalState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    ECombat_Formation PreferredFormation = ECombat_Formation::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    float CautiousDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    float AttackRange = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    bool bIsPackLeader = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    TArray<AActor*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    FVector LastKnownTargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    float LastTargetSightTime = 0.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_TacticalAI : public AAIController
{
    GENERATED_BODY()

public:
    ACombat_TacticalAI();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    UAISenseConfig_Sight* SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    UAISenseConfig_Hearing* HearingConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    FCombat_TacticalData TacticalData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    class UBehaviorTree* BehaviorTree;

public:
    // Tactical decision making
    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void UpdateTacticalState();

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void SetTacticalState(ECombat_TacticalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    ECombat_TacticalState GetTacticalState() const { return TacticalData.CurrentState; }

    // Pack coordination
    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void JoinPack(ACombat_TacticalAI* PackLeader);

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void SetPackFormation(ECombat_Formation Formation);

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    FVector CalculateFormationPosition(int32 PackIndex);

    // Target management
    UFUNCTION(BlueprintCallable, Category = "Target")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Target")
    AActor* GetCurrentTarget() const { return TacticalData.CurrentTarget; }

    UFUNCTION(BlueprintCallable, Category = "Target")
    bool HasValidTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Target")
    float GetDistanceToTarget() const;

    // Combat decisions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldEngageTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldRetreat() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldFlank() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    FVector CalculateFlankingPosition() const;

    // Perception callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

protected:
    // Internal tactical logic
    void ProcessTacticalDecisions(float DeltaTime);
    void UpdatePackCoordination();
    void ExecuteCurrentTactic();
    
    // State-specific behaviors
    void ExecuteIdleBehavior();
    void ExecutePatrolBehavior();
    void ExecuteHuntBehavior();
    void ExecuteEngageBehavior();
    void ExecuteFlankBehavior();
    void ExecuteRetreatBehavior();
    void ExecuteRegroupBehavior();

    // Utility functions
    bool IsInCombatRange() const;
    bool CanSeeTarget() const;
    FVector GetSafeRetreatPosition() const;
    void UpdateBlackboardValues();

private:
    float StateChangeTimer = 0.0f;
    float LastDecisionTime = 0.0f;
    const float DecisionInterval = 0.5f; // Make decisions every 0.5 seconds
};