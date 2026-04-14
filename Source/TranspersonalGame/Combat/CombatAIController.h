#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "../SharedTypes.h"
#include "CombatAIController.generated.h"

UENUM(BlueprintType)
enum class ECombat_AIState : uint8
{
    Idle,
    Patrol,
    Investigating,
    Combat,
    Flanking,
    Retreating,
    Regrouping,
    Ambushing
};

UENUM(BlueprintType)
enum class ECombat_TacticalRole : uint8
{
    Aggressive,
    Defensive,
    Support,
    Flanker,
    Leader,
    Scout
};

USTRUCT(BlueprintType)
struct FCombat_TacticalData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECombat_TacticalRole Role = ECombat_TacticalRole::Aggressive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AggressionLevel = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CautiousLevel = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FlankingPreference = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GroupCoordination = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RetreatThreshold = 0.2f;

    FCombat_TacticalData()
    {
        Role = ECombat_TacticalRole::Aggressive;
        AggressionLevel = 0.7f;
        CautiousLevel = 0.3f;
        FlankingPreference = 0.5f;
        GroupCoordination = 0.8f;
        RetreatThreshold = 0.2f;
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

    // AI Perception
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Perception")
    class UAISenseConfig_Sight* SightConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Perception")
    class UAISenseConfig_Hearing* HearingConfig;

    // Behavior Tree
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    class UBehaviorTree* BehaviorTree;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Behavior")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Behavior")
    UBlackboardComponent* BlackboardComponent;

    // Combat State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    ECombat_AIState CurrentState = ECombat_AIState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    FCombat_TacticalData TacticalData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float CombatRange = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float FlankingRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float RetreatRange = 500.0f;

    // Target Management
    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    TArray<AActor*> KnownEnemies;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    TArray<AActor*> AlliedUnits;

    // Tactical Positioning
    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    FVector LastKnownTargetLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    FVector PreferredCombatPosition;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    FVector FlankingPosition;

public:
    // AI State Management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetAIState(ECombat_AIState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    ECombat_AIState GetAIState() const { return CurrentState; }

    // Target Management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    AActor* GetCurrentTarget() const { return CurrentTarget; }

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void AddKnownEnemy(AActor* Enemy);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RemoveKnownEnemy(AActor* Enemy);

    // Tactical Functions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector CalculateFlankingPosition(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector FindCoverPosition(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldRetreat();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool CanSeeTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    float GetDistanceToTarget(AActor* Target);

    // Group Coordination
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CoordinateWithAllies();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RequestBackup();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ShareTargetInformation(AActor* Target);

protected:
    // Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Internal Functions
    void UpdateCombatState(float DeltaTime);
    void UpdateTacticalPositioning();
    void ProcessThreatAssessment();
    void ExecuteCombatBehavior();
    void HandleGroupCoordination();

    // Blackboard Keys
    static const FName BB_CurrentTarget;
    static const FName BB_LastKnownLocation;
    static const FName BB_AIState;
    static const FName BB_CombatPosition;
    static const FName BB_FlankingPosition;
    static const FName BB_ShouldRetreat;
    static const FName BB_AlliesInRange;
};