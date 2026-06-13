#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/PawnSensingComponent.h"
#include "Engine/Engine.h"
#include "CombatAIController.generated.h"

UENUM(BlueprintType)
enum class ECombat_AIState : uint8
{
    Idle,
    Patrolling,
    Investigating,
    Chasing,
    Attacking,
    Fleeing,
    Stunned,
    Dead
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
struct FCombat_ThreatInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    AActor* ThreatActor = nullptr;

    UPROPERTY(BlueprintReadOnly)
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly)
    float ThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float LastSeenTime = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    bool bIsVisible = false;

    FCombat_ThreatInfo()
    {
        ThreatActor = nullptr;
        LastKnownLocation = FVector::ZeroVector;
        ThreatLevel = 0.0f;
        LastSeenTime = 0.0f;
        bIsVisible = false;
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
    class UBehaviorTree* BehaviorTree;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Behavior")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Behavior")
    class UBlackboardComponent* BlackboardComponent;

    // Combat AI Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI", meta = (AllowPrivateAccess = "true"))
    ECombat_AIState CurrentState = ECombat_AIState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI", meta = (AllowPrivateAccess = "true"))
    float SightRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI", meta = (AllowPrivateAccess = "true"))
    float HearingRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI", meta = (AllowPrivateAccess = "true"))
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI", meta = (AllowPrivateAccess = "true"))
    float FleeHealthThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI", meta = (AllowPrivateAccess = "true"))
    float AggressionLevel = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI", meta = (AllowPrivateAccess = "true"))
    float PatrolRadius = 1000.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI", meta = (AllowPrivateAccess = "true"))
    TArray<FCombat_ThreatInfo> KnownThreats;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI", meta = (AllowPrivateAccess = "true"))
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI", meta = (AllowPrivateAccess = "true"))
    FVector HomeLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI", meta = (AllowPrivateAccess = "true"))
    float StateChangeTime = 0.0f;

public:
    // Perception callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Combat AI functions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetAIState(ECombat_AIState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    ECombat_AIState GetAIState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCurrentTarget(AActor* NewTarget);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    AActor* GetCurrentTarget() const { return CurrentTarget; }

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void AddThreat(AActor* ThreatActor, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RemoveThreat(AActor* ThreatActor);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    FCombat_ThreatInfo GetHighestThreat() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool CanAttackTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldFlee() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void StartCombat(AActor* Enemy);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void EndCombat();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector GetBestAttackPosition(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateThreatAssessment();

protected:
    void InitializeAIPerception();
    void InitializeBehaviorTree();
    void UpdateAIState(float DeltaTime);
    void ProcessThreats(float DeltaTime);
    float CalculateThreatLevel(AActor* Actor) const;
    bool IsActorHostile(AActor* Actor) const;
    void SetBlackboardValues();
};