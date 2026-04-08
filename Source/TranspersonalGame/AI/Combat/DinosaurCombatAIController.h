#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/Engine.h"
#include "DinosaurCombatAIController.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;
class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;
class UAISenseConfig_Damage;

UENUM(BlueprintType)
enum class EDinosaurCombatState : uint8
{
    Idle,
    Hunting,
    Stalking,
    Attacking,
    Fleeing,
    Territorial,
    Feeding,
    Investigating
};

UENUM(BlueprintType)
enum class EDinosaurThreatLevel : uint8
{
    None,
    Low,
    Medium,
    High,
    Critical
};

USTRUCT(BlueprintType)
struct FCombatMemory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    AActor* LastKnownTarget = nullptr;

    UPROPERTY(BlueprintReadWrite)
    FVector LastKnownTargetLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite)
    float TimeSinceLastSeen = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    EDinosaurThreatLevel ThreatLevel = EDinosaurThreatLevel::None;

    UPROPERTY(BlueprintReadWrite)
    bool bHasBeenDamaged = false;

    UPROPERTY(BlueprintReadWrite)
    float LastDamageTime = 0.0f;
};

/**
 * AI Controller especializado para combate de dinossauros
 * Implementa comportamentos táticos, memória de combate e percepção avançada
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurCombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurCombatAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Behavior Tree e Blackboard
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    class UBehaviorTree* BehaviorTree;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    class UBlackboardAsset* BlackboardAsset;

    // Componentes de Percepção
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Perception")
    class UAISenseConfig_Sight* SightConfig;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Perception")
    class UAISenseConfig_Hearing* HearingConfig;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Perception")
    class UAISenseConfig_Damage* DamageConfig;

    // Configurações de Combate
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float StalkingDistance = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FleeDistance = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float MemoryDuration = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsAmbushPredator = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float TerritorialRadius = 1500.0f;

    // Estado de Combate
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    EDinosaurCombatState CurrentCombatState = EDinosaurCombatState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    FCombatMemory CombatMemory;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    TArray<AActor*> KnownThreats;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    TArray<AActor*> PackMembers;

public:
    // Funções de Combate
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(EDinosaurCombatState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    EDinosaurThreatLevel EvaluateThreatLevel(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool CanSeeTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsInAttackRange(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    FVector GetOptimalAttackPosition(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdateCombatMemory(AActor* Target, const FVector& LastSeenLocation);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ForgetTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldFlee();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AlertPackMembers(AActor* Threat);

    // Callbacks de Percepção
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

protected:
    // Funções internas
    void InitializePerception();
    void UpdateCombatLogic(float DeltaTime);
    void ProcessThreatAssessment();
    void ExecuteCombatBehavior();
    
    // Comportamentos específicos
    void ExecuteHuntingBehavior();
    void ExecuteStalkingBehavior();
    void ExecuteAttackingBehavior();
    void ExecuteFleeingBehavior();
    void ExecuteTerritorialBehavior();
    void ExecuteInvestigatingBehavior();

private:
    float LastPerceptionUpdate = 0.0f;
    float PerceptionUpdateInterval = 0.1f;
    
    bool bIsInitialized = false;
};