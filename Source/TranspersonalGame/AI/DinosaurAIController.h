#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "../Core/SharedTypes.h"
#include "DinosaurAIController.generated.h"

// Forward declarations
class UBehaviorTree;
class APawn;

UENUM(BlueprintType)
enum class ENPC_DinosaurSpecies : uint8
{
    TRex = 0        UMETA(DisplayName = "T-Rex"),
    Raptor = 1      UMETA(DisplayName = "Raptor"),
    Triceratops = 2 UMETA(DisplayName = "Triceratops"),
    Brachiosaurus = 3 UMETA(DisplayName = "Brachiosaurus"),
    Stegosaurus = 4 UMETA(DisplayName = "Stegosaurus")
};

UENUM(BlueprintType)
enum class ENPC_DinosaurState : uint8
{
    Idle = 0        UMETA(DisplayName = "Idle"),
    Patrolling = 1  UMETA(DisplayName = "Patrolling"),
    Hunting = 2     UMETA(DisplayName = "Hunting"),
    Feeding = 3     UMETA(DisplayName = "Feeding"),
    Fleeing = 4     UMETA(DisplayName = "Fleeing"),
    Fighting = 5    UMETA(DisplayName = "Fighting"),
    Sleeping = 6    UMETA(DisplayName = "Sleeping"),
    Territorial = 7 UMETA(DisplayName = "Territorial")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_DinosaurStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Damage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Speed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float SightRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float HearingRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float TerritoryRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AggressionLevel = 0.5f;

    FNPC_DinosaurStats()
    {
        Health = 100.0f;
        MaxHealth = 100.0f;
        Damage = 25.0f;
        Speed = 400.0f;
        SightRange = 2000.0f;
        HearingRange = 1500.0f;
        TerritoryRadius = 1000.0f;
        AggressionLevel = 0.5f;
    }
};

/**
 * AI Controller for all dinosaur species in the prehistoric world.
 * Handles behavior trees, perception, and species-specific AI logic.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurAIController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Possess(APawn* InPawn) override;

    // AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionComponent* PerceptionComponent;

    // Behavior Trees for different species
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    class UBehaviorTree* TRexBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    class UBehaviorTree* RaptorBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    class UBehaviorTree* HerbivoreBehaviorTree;

    // Dinosaur Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    ENPC_DinosaurSpecies DinosaurSpecies = ENPC_DinosaurSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    ENPC_DinosaurState CurrentState = ENPC_DinosaurState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    FNPC_DinosaurStats DinosaurStats;

    // Territory and Patrol
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    FVector TerritoryCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    TArray<FVector> PatrolPoints;

    UPROPERTY(BlueprintReadOnly, Category = "Territory")
    int32 CurrentPatrolIndex = 0;

    // Targets and Threats
    UPROPERTY(BlueprintReadOnly, Category = "AI")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "AI")
    AActor* CurrentThreat;

    UPROPERTY(BlueprintReadOnly, Category = "AI")
    float LastPlayerSightTime = 0.0f;

public:
    // AI State Management
    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetDinosaurState(ENPC_DinosaurState NewState);

    UFUNCTION(BlueprintPure, Category = "AI")
    ENPC_DinosaurState GetDinosaurState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintPure, Category = "AI")
    AActor* GetCurrentTarget() const { return CurrentTarget; }

    // Territory Management
    UFUNCTION(BlueprintCallable, Category = "Territory")
    void SetTerritoryCenter(FVector NewCenter);

    UFUNCTION(BlueprintCallable, Category = "Territory")
    void GeneratePatrolPoints(int32 NumPoints = 4);

    UFUNCTION(BlueprintPure, Category = "Territory")
    FVector GetNextPatrolPoint();

    UFUNCTION(BlueprintPure, Category = "Territory")
    bool IsInTerritory(FVector Location) const;

    // Combat and Behavior
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AttackTarget();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void StartHunting(AActor* Prey);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void StartFleeing(AActor* Threat);

    UFUNCTION(BlueprintPure, Category = "Behavior")
    bool ShouldAttackPlayer() const;

    UFUNCTION(BlueprintPure, Category = "Behavior")
    bool ShouldFleeFromPlayer() const;

    // Species-specific behavior
    UFUNCTION(BlueprintCallable, Category = "Species")
    void ConfigureForSpecies(ENPC_DinosaurSpecies Species);

protected:
    // Perception callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, struct FAIStimulus Stimulus);

    // Internal methods
    void SetupPerception();
    void StartBehaviorTree();
    void UpdateBlackboard();

private:
    float LastStateChangeTime = 0.0f;
    float StateChangeCooldown = 2.0f;
};