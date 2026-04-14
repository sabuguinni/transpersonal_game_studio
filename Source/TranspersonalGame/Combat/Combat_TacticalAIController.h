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

UENUM(BlueprintType)
enum class ECombat_TacticalState : uint8
{
    Patrol,
    Investigate,
    Engage,
    Flank,
    Retreat,
    Regroup,
    Ambush,
    Coordinate
};

UENUM(BlueprintType)
enum class ECombat_ThreatLevel : uint8
{
    None,
    Low,
    Medium,
    High,
    Critical
};

USTRUCT(BlueprintType)
struct FCombat_TacticalData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    ECombat_TacticalState CurrentState = ECombat_TacticalState::Patrol;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    ECombat_ThreatLevel ThreatLevel = ECombat_ThreatLevel::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    FVector LastKnownPlayerLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    FVector PreferredCoverLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    float EngagementRange = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    float FlankingRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    bool bCanUseRangedAttacks = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    bool bPrefersMeleeRange = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    float ReactionTime = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    int32 TeamID = 0;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_TacticalAIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombat_TacticalAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI Behavior")
    class UBehaviorTree* BehaviorTree;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI Behavior")
    class UBlackboardAsset* BlackboardAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    FCombat_TacticalData TacticalData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Perception")
    float SightRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Perception")
    float LoseSightRadius = 1800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Perception")
    float PeripheralVisionAngleDegrees = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Perception")
    float HearingRange = 800.0f;

public:
    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void SetTacticalState(ECombat_TacticalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    ECombat_TacticalState GetTacticalState() const;

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void UpdateThreatLevel(ECombat_ThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    bool CanSeeTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    FVector FindBestCoverLocation(const FVector& ThreatLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    FVector CalculateFlankingPosition(const FVector& TargetLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    bool ShouldRetreat() const;

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void CoordinateWithTeam();

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    float GetDistanceToTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    bool IsInOptimalRange() const;

protected:
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    void InitializePerception();
    void UpdateTacticalDecisions();
    void ExecuteTacticalManeuver();

private:
    UPROPERTY()
    AActor* CurrentTarget;

    UPROPERTY()
    TArray<AActor*> KnownEnemies;

    UPROPERTY()
    TArray<AActor*> TeamMembers;

    float LastDecisionTime;
    float DecisionCooldown;
};