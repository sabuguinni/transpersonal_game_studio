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

class UCombat_TacticalBehaviorComponent;
class UCombat_PackCoordinationComponent;

UENUM(BlueprintType)
enum class ECombat_AIState : uint8
{
    Idle,
    Patrolling,
    Investigating,
    Hunting,
    Attacking,
    Flanking,
    Retreating,
    Coordinating
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
struct FCombat_TacticalTarget
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    AActor* TargetActor = nullptr;

    UPROPERTY(BlueprintReadWrite)
    FVector LastKnownPosition = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite)
    float ThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    float LastSeenTime = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    bool bIsVisible = false;

    FCombat_TacticalTarget()
    {
        TargetActor = nullptr;
        LastKnownPosition = FVector::ZeroVector;
        ThreatLevel = 0.0f;
        LastSeenTime = 0.0f;
        bIsVisible = false;
    }
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

    // AI Perception
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    UAISightConfig* SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    UAIHearingConfig* HearingConfig;

    // Behavior Tree
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    UBehaviorTree* BehaviorTree;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Behavior")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Behavior")
    UBlackboardComponent* BlackboardComponent;

    // Tactical Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tactical AI")
    UCombat_TacticalBehaviorComponent* TacticalBehaviorComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tactical AI")
    UCombat_PackCoordinationComponent* PackCoordinationComponent;

    // AI State
    UPROPERTY(BlueprintReadOnly, Category = "AI State")
    ECombat_AIState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "AI State")
    FCombat_TacticalTarget PrimaryTarget;

    UPROPERTY(BlueprintReadOnly, Category = "AI State")
    TArray<FCombat_TacticalTarget> KnownTargets;

    // AI Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    float SightRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    float LoseSightRadius = 1800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    float FieldOfView = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    float HearingRange = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    float AggressionLevel = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    float IntelligenceLevel = 0.8f;

public:
    // AI State Management
    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void SetAIState(ECombat_AIState NewState);

    UFUNCTION(BlueprintPure, Category = "Tactical AI")
    ECombat_AIState GetAIState() const { return CurrentState; }

    // Target Management
    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void SetPrimaryTarget(AActor* NewTarget);

    UFUNCTION(BlueprintPure, Category = "Tactical AI")
    AActor* GetPrimaryTarget() const { return PrimaryTarget.TargetActor; }

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void AddKnownTarget(AActor* Target, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void RemoveKnownTarget(AActor* Target);

    // Tactical Queries
    UFUNCTION(BlueprintPure, Category = "Tactical AI")
    bool CanSeeTarget(AActor* Target) const;

    UFUNCTION(BlueprintPure, Category = "Tactical AI")
    float GetDistanceToTarget(AActor* Target) const;

    UFUNCTION(BlueprintPure, Category = "Tactical AI")
    ECombat_ThreatLevel EvaluateThreatLevel(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    FVector GetTacticalPosition(AActor* Target, float Distance) const;

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    bool ShouldRetreat() const;

protected:
    // Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Tactical Decision Making
    void UpdateTacticalState(float DeltaTime);
    void EvaluateThreats();
    void UpdateTargetInformation();
    void MakeTacticalDecision();

    // Blackboard Keys
    static const FName BB_TargetActor;
    static const FName BB_TargetLocation;
    static const FName BB_AIState;
    static const FName BB_ThreatLevel;
    static const FName BB_ShouldRetreat;
    static const FName BB_FlankingPosition;
    static const FName BB_PackLeader;
    static const FName BB_PackMembers;

private:
    float LastTacticalUpdate = 0.0f;
    float TacticalUpdateInterval = 0.2f;
    float MemoryDecayRate = 0.1f;
};