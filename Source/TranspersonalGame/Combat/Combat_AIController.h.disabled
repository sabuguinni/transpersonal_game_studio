#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "SharedTypes.h"
#include "Combat_AIController.generated.h"

class UCombat_TacticalAI;
class UCombat_DamageSystem;

UENUM(BlueprintType)
enum class ECombat_AIState : uint8
{
    Idle,
    Patrol,
    Investigate,
    Combat,
    Retreat,
    Regroup,
    Ambush,
    Stalking
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
struct FCombat_AIMemory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FVector LastKnownPlayerLocation;

    UPROPERTY(BlueprintReadOnly)
    float TimeSinceLastSeen;

    UPROPERTY(BlueprintReadOnly)
    ECombat_ThreatLevel ThreatLevel;

    UPROPERTY(BlueprintReadOnly)
    bool bPlayerDetected;

    UPROPERTY(BlueprintReadOnly)
    float CombatReadiness;

    FCombat_AIMemory()
    {
        LastKnownPlayerLocation = FVector::ZeroVector;
        TimeSinceLastSeen = 0.0f;
        ThreatLevel = ECombat_ThreatLevel::None;
        bPlayerDetected = false;
        CombatReadiness = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_AIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombat_AIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // AI Perception
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    UAISenseConfig_Sight* SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    UAISenseConfig_Hearing* HearingConfig;

    // Behavior Tree
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    UBehaviorTree* BehaviorTree;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    // Combat AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    UCombat_TacticalAI* TacticalAI;

    // AI State
    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    ECombat_AIState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    FCombat_AIMemory AIMemory;

    // Combat Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float SightRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float LoseSightRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float FieldOfView;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float HearingRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float CombatRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float RetreatHealthThreshold;

    // Tactical Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    bool bUsePackTactics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    bool bCanAmbush;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    bool bTerritorialDefender;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    float AggressionLevel;

public:
    // AI State Management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetAIState(ECombat_AIState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    ECombat_AIState GetAIState() const { return CurrentState; }

    // Perception Callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Combat Actions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void InitiateCombat(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteRetreat();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateThreatAssessment();

    // Pack Coordination
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CoordinateWithPack();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RequestPackAssistance();

    // Tactical Decision Making
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldEngageTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector CalculateOptimalAttackPosition(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool CanExecuteAmbush();

    // Memory Management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateAIMemory(AActor* Target, bool bVisible);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    FCombat_AIMemory GetAIMemory() const { return AIMemory; }

    // Blackboard Integration
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateBlackboard();

protected:
    void InitializePerception();
    void InitializeBehaviorTree();
    void ProcessPerceptionUpdate(AActor* Actor, bool bVisible);
    void HandleCombatStateTransition();
    void UpdateCombatReadiness(float DeltaTime);
};