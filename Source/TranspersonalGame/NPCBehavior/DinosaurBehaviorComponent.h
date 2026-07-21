#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Perception/PawnSensingComponent.h"
#include "../SharedTypes.h"
#include "DinosaurBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_DinosaurState : uint8
{
    Idle,
    Patrolling,
    Hunting,
    Fleeing,
    Feeding,
    Sleeping,
    Territorial,
    Migrating
};

UENUM(BlueprintType)
enum class ENPC_DinosaurSpecies : uint8
{
    TRex,
    Raptor,
    Triceratops,
    Brachiosaurus,
    Stegosaurus,
    Parasaurolophus,
    Carnotaurus,
    Ankylosaurus
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
    float Hunger = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Stamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Fear = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Aggression = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float TerritorialRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float DetectionRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MovementSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackDamage = 25.0f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_DinosaurBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_DinosaurBehaviorComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Estado do dinossauro
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_DinosaurState CurrentState = ENPC_DinosaurState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_DinosaurSpecies Species = ENPC_DinosaurSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FNPC_DinosaurStats Stats;

    // Behavior Tree
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBehaviorTree* BehaviorTree;

    // Referências de AI
    UPROPERTY(BlueprintReadOnly, Category = "AI")
    AAIController* AIController;

    UPROPERTY(BlueprintReadOnly, Category = "AI")
    UBlackboardComponent* BlackboardComponent;

    UPROPERTY(BlueprintReadOnly, Category = "AI")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    // Componente de percepção
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UPawnSensingComponent* PawnSensingComponent;

    // Alvos e memória
    UPROPERTY(BlueprintReadOnly, Category = "Behavior")
    APawn* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Behavior")
    FVector HomeLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Behavior")
    FVector PatrolTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Behavior")
    float LastPlayerSightTime = 0.0f;

    // Funções de comportamento
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetDinosaurState(ENPC_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void InitializeDinosaurBehavior();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void StartPatrolling();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void StartHunting(APawn* Target);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void StartFleeing();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void ReturnToIdle();

    // Callbacks de percepção
    UFUNCTION()
    void OnPlayerSeen(APawn* SeenPawn);

    UFUNCTION()
    void OnPlayerHeard(APawn* HeardPawn, const FVector& Location, float Volume);

    // Funções de utilidade
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    float GetDistanceToPlayer() const;

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    bool IsPlayerInTerritory() const;

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void UpdateStats(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void TakeDamage(float Damage);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void RestoreHealth(float Amount);

private:
    // Timers internos
    float StateTimer = 0.0f;
    float PatrolTimer = 0.0f;
    float HuntTimer = 0.0f;
    float IdleTimer = 0.0f;

    // Configuração por espécie
    void ConfigureSpeciesStats();
    void SetupBehaviorTree();
    void UpdateBlackboard();
};