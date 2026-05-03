#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "DinosaurAIController.generated.h"

UENUM(BlueprintType)
enum class ENPC_DinosaurState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Hunt        UMETA(DisplayName = "Hunt"),
    Flee        UMETA(DisplayName = "Flee"),
    Attack      UMETA(DisplayName = "Attack"),
    Dead        UMETA(DisplayName = "Dead")
};

USTRUCT(BlueprintType)
struct FNPC_DinosaurStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MovementSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float SightRange = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float PatrolRadius = 1500.0f;
};

/**
 * AI Controller base para todos os dinossauros
 * Implementa comportamento básico: patrulha, caça, fuga, ataque
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANPC_DinosaurAIController : public AAIController
{
    GENERATED_BODY()

public:
    ANPC_DinosaurAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Componentes de IA
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionComponent* AIPerceptionComponent;

    // Configuração da IA
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    class UBehaviorTree* BehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    class UBlackboardData* BlackboardAsset;

    // Stats do dinossauro
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    FNPC_DinosaurStats DinosaurStats;

    // Estado actual
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    ENPC_DinosaurState CurrentState;

    // Target actual
    UPROPERTY(BlueprintReadOnly, Category = "AI")
    AActor* CurrentTarget;

    // Posição inicial para patrulha
    UPROPERTY(BlueprintReadOnly, Category = "AI")
    FVector HomeLocation;

public:
    // Funções públicas para controlo da IA
    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetDinosaurState(ENPC_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "AI")
    ENPC_DinosaurState GetDinosaurState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "AI")
    AActor* GetTarget() const { return CurrentTarget; }

    UFUNCTION(BlueprintCallable, Category = "AI")
    void TakeDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "AI")
    bool IsAlive() const { return DinosaurStats.Health > 0.0f; }

    UFUNCTION(BlueprintCallable, Category = "AI")
    float GetHealthPercentage() const;

protected:
    // Callbacks de percepção
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // Lógica de comportamento
    void UpdateBehavior(float DeltaTime);
    void HandleIdleState();
    void HandlePatrolState();
    void HandleHuntState();
    void HandleAttackState();
    void HandleFleeState();

    // Utilitários
    bool CanSeeTarget(AActor* Target) const;
    float GetDistanceToTarget(AActor* Target) const;
    FVector GetRandomPatrolPoint() const;
};