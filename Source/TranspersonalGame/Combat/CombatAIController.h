#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "../SharedTypes.h"
#include "CombatAIController.generated.h"

class UBehaviorTree;
class UBlackboardData;
class ACombat_EnemyPawn;

UENUM(BlueprintType)
enum class ECombat_AIState : uint8
{
    Patrol,
    Alert,
    Hunting,
    Combat,
    Flanking,
    Retreating,
    Dead
};

UENUM(BlueprintType)
enum class ECombat_TacticalRole : uint8
{
    Alpha,      // Pack leader
    Beta,       // Flanker
    Gamma,      // Support
    Solo        // Independent hunter
};

USTRUCT(BlueprintType)
struct FCombat_TacticalData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    ECombat_TacticalRole Role = ECombat_TacticalRole::Solo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    float FlankingDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    float RetreatHealthThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    bool bCanCallForHelp = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    int32 MaxPackSize = 3;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombatAIController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // AI State Management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetAIState(ECombat_AIState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    ECombat_AIState GetAIState() const { return CurrentAIState; }

    // Target Management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetTarget(APawn* NewTarget);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    APawn* GetCurrentTarget() const { return CurrentTarget; }

    // Tactical Behavior
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteFlankingManeuver();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CallForBackup();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldRetreat() const;

    // Pack Coordination
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void JoinPack(ACombatAIController* PackLeader);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void LeavePack();

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    bool IsInPack() const { return PackMembers.Num() > 1 || PackLeader != nullptr; }

protected:
    // AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionComponent* AIPerceptionComponent;

    // AI Configuration
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    class UBehaviorTree* BehaviorTree;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    class UBlackboardData* BlackboardAsset;

    // Tactical Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FCombat_TacticalData TacticalData;

    // Current State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    ECombat_AIState CurrentAIState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    APawn* CurrentTarget;

    // Pack System
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack")
    ACombatAIController* PackLeader;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack")
    TArray<ACombatAIController*> PackMembers;

    // Timers
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    float LastTargetUpdateTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    float StateChangeTime;

private:
    // Internal Methods
    void InitializeAI();
    void UpdatePerception();
    void UpdateTacticalBehavior(float DeltaTime);
    void ProcessPackCoordination();
    
    // Perception Callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // Blackboard Keys
    static const FName TargetActorKey;
    static const FName AIStateKey;
    static const FName TacticalRoleKey;
    static const FName PackLeaderKey;
};