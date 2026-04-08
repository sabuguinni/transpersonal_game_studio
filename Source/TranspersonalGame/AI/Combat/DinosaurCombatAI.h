#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "GameplayTags.h"
#include "DinosaurCombatAI.generated.h"

class UBehaviorTree;
class UBlackboardAsset;

UENUM(BlueprintType)
enum class EDinosaurThreatLevel : uint8
{
    Passive,        // Herbívoros pacíficos
    Defensive,      // Herbívoros que se defendem
    Territorial,    // Predadores pequenos/médios
    Apex           // Predadores supremos (T-Rex, etc.)
};

UENUM(BlueprintType)
enum class EDinosaurHuntingState : uint8
{
    Idle,
    Patrolling,
    Investigating,
    Stalking,
    Hunting,
    Attacking,
    Feeding,
    Retreating
};

USTRUCT(BlueprintType)
struct FCombatPersonality
{
    GENERATED_BODY()

    // Agressividade base (0.0 = passivo, 1.0 = extremamente agressivo)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Aggressiveness = 0.5f;

    // Curiosidade (tendência a investigar sons/movimentos)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Curiosity = 0.5f;

    // Persistência na caça
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float HuntingPersistence = 0.5f;

    // Inteligência tática
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TacticalIntelligence = 0.5f;

    // Medo/cautela
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Caution = 0.5f;
};

/**
 * AI Controller especializado para combate de dinossauros
 * Implementa comportamentos táticos avançados baseados em personalidade
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurCombatAI : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurCombatAI();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Componentes principais
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UAIPerceptionComponent* PerceptionComponent;

    // Configuração de percepção
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    class UAISightConfig* SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    class UAIHearingConfig* HearingConfig;

    // Assets de AI
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Assets")
    UBehaviorTree* BehaviorTreeAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Assets")
    UBlackboardAsset* BlackboardAsset;

    // Configuração de combate
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Configuration")
    EDinosaurThreatLevel ThreatLevel = EDinosaurThreatLevel::Territorial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Configuration")
    FCombatPersonality Personality;

    // Gameplay Tags para estados e comportamentos
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Tags")
    FGameplayTag HuntingStateTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Tags")
    FGameplayTag ThreatLevelTag;

    // Estado atual de caça
    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    EDinosaurHuntingState CurrentHuntingState = EDinosaurHuntingState::Idle;

    // Alvo atual
    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    AActor* CurrentTarget = nullptr;

    // Última posição conhecida do alvo
    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    FVector LastKnownTargetLocation = FVector::ZeroVector;

    // Tempo desde que perdeu o alvo
    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    float TimeSinceTargetLost = 0.0f;

    // Configuração de distâncias
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Distances")
    float OptimalAttackDistance = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Distances")
    float MaxChaseDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Distances")
    float StalkingDistance = 800.0f;

public:
    // Funções públicas para Behavior Tree
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetHuntingState(EDinosaurHuntingState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    EDinosaurHuntingState GetHuntingState() const { return CurrentHuntingState; }

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCurrentTarget(AActor* NewTarget);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    AActor* GetCurrentTarget() const { return CurrentTarget; }

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldContinueHunting() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector GetBestAttackPosition() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool IsInOptimalAttackRange() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldRetreat() const;

    // Sistema de memória tática
    UFUNCTION(BlueprintCallable, Category = "Tactical Memory")
    void RememberDangerousLocation(FVector Location, float DangerLevel);

    UFUNCTION(BlueprintCallable, Category = "Tactical Memory")
    bool IsLocationDangerous(FVector Location) const;

protected:
    // Callbacks de percepção
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Lógica interna
    void UpdateHuntingBehavior(float DeltaTime);
    void ProcessPerceptionData();
    void UpdateBlackboardValues();
    
    // Cálculos táticos
    FVector CalculateFlankingPosition() const;
    FVector CalculateAmbushPosition() const;
    bool HasClearLineOfSight(FVector TargetLocation) const;
    float CalculateThreatLevel(AActor* Target) const;

private:
    // Memória tática - locais perigosos
    TMap<FVector, float> DangerousLocations;
    
    // Timer para limpeza de memória
    float MemoryCleanupTimer = 0.0f;
    const float MemoryCleanupInterval = 30.0f;
    
    // Configuração de personalidade baseada em threat level
    void InitializePersonalityFromThreatLevel();
    
    // Sistema de cooldowns para ações
    float LastAttackTime = 0.0f;
    float AttackCooldown = 2.0f;
};