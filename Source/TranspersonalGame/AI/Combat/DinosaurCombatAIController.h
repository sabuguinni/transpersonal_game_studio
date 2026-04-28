#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
// DISABLED: #include "Perception/PerceptionComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/Engine.h"
#include "Perception/AIPerceptionComponent.h"
#include "DinosaurCombatAIController.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;
class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;
class UAISenseConfig_Damage;

UENUM(BlueprintType)
enum class EAI_DinosaurCombatState : uint8
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
enum class EAI_DinosaurThreatLevel : uint8
{
    None,
    Low,
    Medium,
    High,
    Critical
};

USTRUCT(BlueprintType)
struct FAI_CombatMemory_461
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    AActor* LastKnownTarget = nullptr;

    UPROPERTY(BlueprintReadWrite)
    FVector LastKnownTargetLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite)
    float TimeSinceLastSeen = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    EAI_DinosaurThreatLevel ThreatLevel = EAI_DinosaurThreatLevel::None;

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
    virtual void Tick(float DeltaTime);

    // Behavior Tree e Blackboard
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    class UBehaviorTree* BehaviorTree;

// [UHT-FIX]     UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    class UBlackboardData* BlackboardAsset;

    // Componentes de Percepção
    // SHADOWED: UPROPERTY class UAIPerceptionComponent* PerceptionComponent;

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
    EAI_DinosaurCombatState CurrentCombatState = EAI_DinosaurCombatState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    FAI_CombatMemory_461 CombatMemory;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    TArray<AActor*> KnownThreats;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    TArray<AActor*> PackMembers;

public:
    // Funções de Combate
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(EAI_DinosaurCombatState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    EAI_DinosaurThreatLevel EvaluateThreatLevel(AActor* Target);

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