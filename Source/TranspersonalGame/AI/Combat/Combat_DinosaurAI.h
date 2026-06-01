#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "SharedTypes.h"
#include "Combat_DinosaurAI.generated.h"

class UBehaviorTree;
class UBlackboardData;

UENUM(BlueprintType)
enum class ECombat_DinosaurState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Territorial UMETA(DisplayName = "Territorial"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ECombat_ThreatLevel : uint8
{
    None    UMETA(DisplayName = "None"),
    Low     UMETA(DisplayName = "Low"),
    Medium  UMETA(DisplayName = "Medium"),
    High    UMETA(DisplayName = "High"),
    Extreme UMETA(DisplayName = "Extreme")
};

USTRUCT(BlueprintType)
struct FCombat_ThreatInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    AActor* ThreatActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    ECombat_ThreatLevel ThreatLevel = ECombat_ThreatLevel::None;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    float LastSeenTime = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    float DistanceToThreat = 0.0f;

    FCombat_ThreatInfo()
    {
        ThreatActor = nullptr;
        ThreatLevel = ECombat_ThreatLevel::None;
        LastSeenTime = 0.0f;
        LastKnownLocation = FVector::ZeroVector;
        DistanceToThreat = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_DinosaurAI : public AAIController
{
    GENERATED_BODY()

public:
    ACombat_DinosaurAI();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAIPerceptionComponent* AIPerceptionComponent;

    // Behavior Trees
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    UBehaviorTree* TRexBehaviorTree;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    UBehaviorTree* VelociraptorBehaviorTree;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    UBehaviorTree* TriceratopsBehaviorTree;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    UBehaviorTree* BrachiosaurusBehaviorTree;

    // Blackboard Data
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    UBlackboardData* DinosaurBlackboard;

    // Combat State
    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    ECombat_DinosaurState CurrentState;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    TArray<FCombat_ThreatInfo> DetectedThreats;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    FVector PatrolPoint;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    float StateChangeTime;

    // Combat Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float SightRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float HearingRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float FleeRange = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float PatrolRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float FearLevel = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    bool bIsTerritorial = true;

public:
    // AI Functions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetDinosaurType(EDinosaurSpecies DinosaurType);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateThreatAssessment();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_ThreatLevel CalculateThreatLevel(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void EngageCombat(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void FleeFromThreat(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void StartPatrolling();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector GetRandomPatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool IsInAttackRange(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void PerformAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

protected:
    // Internal Functions
    void InitializeAIPerception();
    void SetupBehaviorTree();
    void UpdateBlackboard();
    void ProcessThreatList();
    AActor* FindBestTarget();
    void UpdateCombatState();

    // Perception Callbacks
    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
};