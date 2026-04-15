#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "../SharedTypes.h"
#include "CombatAIController.generated.h"

class UBehaviorTree;
class UBlackboardData;
class ACombat_EnemyPawn;

UENUM(BlueprintType)
enum class ECombat_AIState : uint8
{
    Idle,
    Patrolling,
    Investigating,
    Hunting,
    Engaging,
    Flanking,
    Retreating,
    Coordinating,
    Ambushing
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
struct TRANSPERSONALGAME_API FCombat_TacticalData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PreferredEngagementRange = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinimumEngagementRange = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FlankingDistance = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RetreatHealthThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanCoordinate = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanAmbush = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CoordinationRadius = 1500.0f;

    FCombat_TacticalData()
    {
        PreferredEngagementRange = 500.0f;
        MinimumEngagementRange = 100.0f;
        FlankingDistance = 800.0f;
        RetreatHealthThreshold = 0.3f;
        bCanCoordinate = true;
        bCanAmbush = false;
        CoordinationRadius = 1500.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombatAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;

    // AI Perception
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    class UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    class UAISightConfig* SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    class UAIHearingConfig* HearingConfig;

    // Behavior Tree
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI Behavior")
    UBehaviorTree* BehaviorTree;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI Behavior")
    UBlackboardData* BlackboardAsset;

    // Combat AI State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    ECombat_AIState CurrentAIState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    ECombat_ThreatLevel CurrentThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    FCombat_TacticalData TacticalData;

    // Target Management
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    AActor* CurrentTarget;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    FVector LastKnownTargetLocation;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    float TimeSinceLastTargetSeen;

    // Pack Coordination
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack AI")
    TArray<ACombatAIController*> PackMembers;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack AI")
    bool bIsPackLeader;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack AI")
    ACombatAIController* PackLeader;

public:
    // AI State Management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetAIState(ECombat_AIState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    ECombat_AIState GetAIState() const { return CurrentAIState; }

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetThreatLevel(ECombat_ThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    ECombat_ThreatLevel GetThreatLevel() const { return CurrentThreatLevel; }

    // Target Management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    AActor* GetCurrentTarget() const { return CurrentTarget; }

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateLastKnownTargetLocation(const FVector& Location);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    FVector GetLastKnownTargetLocation() const { return LastKnownTargetLocation; }

    // Tactical Decision Making
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldEngageTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldFlankTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldRetreat() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector GetOptimalAttackPosition() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector GetFlankingPosition() const;

    // Pack Coordination
    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void RegisterPackMember(ACombatAIController* NewMember);

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void RemovePackMember(ACombatAIController* Member);

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void SetAsPackLeader();

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void CoordinatePackAttack();

    UFUNCTION(BlueprintPure, Category = "Pack AI")
    bool IsInPack() const { return PackMembers.Num() > 0 || PackLeader != nullptr; }

protected:
    // AI Perception Callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Internal State Management
    void UpdateAIState(float DeltaTime);
    void UpdateThreatAssessment();
    void UpdatePackCoordination();

    // Tactical Analysis
    float CalculateTargetThreat(AActor* Target) const;
    bool IsPositionSafe(const FVector& Position) const;
    bool HasClearLineOfSight(const FVector& TargetLocation) const;

    // Pack Communication
    void BroadcastToPackMembers(const FString& Message);
    void ReceivePackCommand(const FString& Command, ACombatAIController* Sender);
};