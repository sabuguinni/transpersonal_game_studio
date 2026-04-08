#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "CombatAIController.generated.h"

UENUM(BlueprintType)
enum class EDinosaurType : uint8
{
    SmallPredator,      // Compsognathus, Velociraptor
    MediumPredator,     // Dilophosaurus, Carnotaurus  
    LargePredator,      // T-Rex, Allosaurus
    SmallHerbivore,     // Gallimimus, Parasaurolophus
    MediumHerbivore,    // Triceratops, Stegosaurus
    LargeHerbivore      // Brontosaurus, Brachiosaurus
};

UENUM(BlueprintType)
enum class ECombatState : uint8
{
    Passive,            // Comportamento normal, não em combate
    Hunting,            // Procurando presas
    Stalking,           // Seguindo presa à distância
    Attacking,          // Combate activo
    Fleeing,            // Fugindo de predador maior
    Territorial,        // Defendendo território
    Pack,               // Comportamento de matilha
    Domesticated        // Domesticado pelo jogador
};

USTRUCT(BlueprintType)
struct FCombatStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AttackDamage = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AttackCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MovementSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FleeThreshold = 30.0f; // % de vida para fugir

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanBeDomesticated = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DomesticationProgress = 0.0f;
};

/**
 * Controller de IA especializado em combate para dinossauros
 * Implementa comportamentos táticos baseados no tipo de dinossauro
 */
UCLASS()
class TRANSPERSONALGAME_API ACombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombatAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Componentes de IA
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBlackboardComponent* BlackboardComponent;

    // Configuração do dinossauro
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    EDinosaurType DinosaurType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    FCombatStats CombatStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBehaviorTree* BehaviorTree;

    // Estado de combate
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    ECombatState CurrentCombatState;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    AActor* CurrentThreat;

    // Timers
    FTimerHandle AttackCooldownTimer;
    FTimerHandle StateUpdateTimer;

public:
    // Funções de combate
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(ECombatState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AttackTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool CanAttackTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void FleeFromThreat(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void StartHunting();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void StopCombat();

    // Domesticação
    UFUNCTION(BlueprintCallable, Category = "Domestication")
    void IncreaseDomesticationProgress(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Domestication")
    bool IsDomesticated() const;

    // Getters
    UFUNCTION(BlueprintPure, Category = "Combat")
    ECombatState GetCombatState() const { return CurrentCombatState; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    EDinosaurType GetDinosaurType() const { return DinosaurType; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    FCombatStats GetCombatStats() const { return CombatStats; }

protected:
    // Callbacks de percepção
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Lógica de combate
    void UpdateCombatLogic();
    void EvaluateThreats();
    void SelectTarget();
    bool ShouldFlee(AActor* Threat) const;
    bool IsValidTarget(AActor* Actor) const;
    bool IsValidThreat(AActor* Actor) const;

    // Comportamentos específicos por tipo
    void UpdatePredatorBehavior();
    void UpdateHerbivoreBehavior();
    void UpdatePackBehavior();

    // Utilitários
    float GetDistanceToActor(AActor* Actor) const;
    bool HasLineOfSight(AActor* Actor) const;
    void SetBlackboardValues();
};