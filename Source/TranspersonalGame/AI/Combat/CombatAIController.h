#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "GameplayTagContainer.h"
#include "CombatAIController.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;
class UAIPerceptionComponent;
class UPawnSensingComponent;

UENUM(BlueprintType)
enum class ECombatState : uint8
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
enum class EThreatLevel : uint8
{
    None,
    Low,
    Medium,
    High,
    Critical
};

UENUM(BlueprintType)
enum class EDinosaurArchetype : uint8
{
    ApexPredator,      // T-Rex, Giganotosaurus
    PackHunter,        // Velociraptor, Deinonychus
    AmbushPredator,    // Carnotaurus, Baryonyx
    HerbivoreDefensive, // Triceratops, Ankylosaurus
    HerbivoreFlighty,  // Parasaurolophus, Gallimimus
    Scavenger          // Compsognathus, Coelophysis
};

/**
 * Advanced AI Controller for dinosaur combat behavior
 * Implements tactical combat AI with emergent behaviors
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
    virtual void OnPossess(APawn* InPawn) override;

    // Core AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAIPerceptionComponent* AIPerceptionComponent;

    // Combat State Management
    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    ECombatState CurrentCombatState;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    EThreatLevel CurrentThreatLevel;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    EDinosaurArchetype DinosaurArchetype;

    // Behavior Trees for different archetypes
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    TMap<EDinosaurArchetype, UBehaviorTree*> ArchetypeBehaviorTrees;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    UBlackboard* CombatBlackboard;

    // Combat Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FearThreshold = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float TerritorialRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float HuntingRange = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FleeDistance = 2000.0f;

    // Individual Variation System
    UPROPERTY(BlueprintReadWrite, Category = "Individual")
    float IndividualAggression;

    UPROPERTY(BlueprintReadWrite, Category = "Individual")
    float IndividualCourage;

    UPROPERTY(BlueprintReadWrite, Category = "Individual")
    float IndividualIntelligence;

    UPROPERTY(BlueprintReadWrite, Category = "Individual")
    float IndividualTerritoriality;

    // Memory and Learning
    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    TArray<FVector> KnownPlayerLocations;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    float LastPlayerSightingTime;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    int32 PlayerEncounterCount;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    bool bHasBeenHurtByPlayer;

    // Gameplay Tags for combat states
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayTags")
    FGameplayTagContainer CombatTags;

public:
    // Combat State Functions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(ECombatState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdateThreatLevel(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void InitializeIndividualTraits();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldEngageTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldFleeFromTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    FVector GetOptimalAttackPosition(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    FVector GetFleeDirection(AActor* ThreatSource);

    // Perception Callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Memory Functions
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void RememberPlayerLocation(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    FVector GetLastKnownPlayerLocation();

    UFUNCTION(BlueprintCallable, Category = "Memory")
    bool HasRecentPlayerMemory(float TimeThreshold = 30.0f);

    // Tactical Functions
    UFUNCTION(BlueprintCallable, Category = "Tactics")
    void ExecuteAmbushBehavior();

    UFUNCTION(BlueprintCallable, Category = "Tactics")
    void ExecutePackHuntingBehavior();

    UFUNCTION(BlueprintCallable, Category = "Tactics")
    void ExecuteTerritoralBehavior();

protected:
    void SetupPerception();
    void SetupBlackboardKeys();
    void UpdateCombatLogic(float DeltaTime);
    float CalculateThreatScore(AActor* Actor);
    void AdaptBehaviorBasedOnExperience();
};