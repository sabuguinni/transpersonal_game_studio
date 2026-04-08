#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "CombatAITypes.h"
#include "DinosaurCombatController.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;
class UAIPerceptionComponent;
class UBehaviorTree;
class UBlackboardData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCombatStateChanged, ECombatState, OldState, ECombatState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnThreatDetected, AActor*, ThreatActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnThreatLost, AActor*, ThreatActor);

UCLASS()
class TRANSPERSONALGAME_API ADinosaurCombatController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurCombatController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAIPerceptionComponent* AIPerceptionComponent;

    // Behavior Tree Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBehaviorTree* BehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBlackboardData* BlackboardAsset;

    // Combat Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FDinosaurArchetype DinosaurArchetype;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FCombatStats CurrentCombatStats;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    ECombatState CurrentCombatState;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    FCombatMemory CombatMemory;

    // Perception Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float SightRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float LoseSightRadius = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float FieldOfViewAngle = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float HearingRadius = 1500.0f;

    // Combat Timing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackCooldown = 2.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float LastAttackTime = -1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float DecisionUpdateInterval = 0.5f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float LastDecisionTime = 0.0f;

    // Current Targets and Threats
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    TArray<AActor*> KnownThreats;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    TArray<AActor*> PackMembers;

public:
    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnCombatStateChanged OnCombatStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnThreatDetected OnThreatDetected;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnThreatLost OnThreatLost;

    // Public Interface
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(ECombatState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    ECombatState GetCombatState() const { return CurrentCombatState; }

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool CanAttack() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsInAttackRange(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float GetThreatLevel(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AddThreat(AActor* ThreatActor, float ThreatValue = 50.0f);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RemoveThreat(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    AActor* GetHighestThreat() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdateCombatMemory(AActor* Actor, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    FVector GetLastKnownLocation(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteAttackPattern(EAttackPattern Pattern);

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void JoinPack(ADinosaurCombatController* PackLeader);

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack")
    bool IsInPack() const { return PackMembers.Num() > 1; }

protected:
    // AI Perception Callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Combat Decision Making
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdateCombatDecisions();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void EvaluateThreats();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SelectBestTarget();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    EAttackPattern ChooseAttackPattern();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldFlee() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldInvestigate(const FVector& Location) const;

    // Blackboard Management
    UFUNCTION(BlueprintCallable, Category = "AI")
    void UpdateBlackboard();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetBlackboardTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetBlackboardLocation(const FName& KeyName, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetBlackboardBool(const FName& KeyName, bool Value);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetBlackboardFloat(const FName& KeyName, float Value);

private:
    // Internal state management
    void InitializePerception();
    void InitializeBehaviorTree();
    void ProcessPerceptionStimulus(AActor* Actor, const FAIStimulus& Stimulus);
    float CalculateThreatLevel(AActor* Actor) const;
    void UpdateStamina(float DeltaTime);
    void DecayMemory(float DeltaTime);
};