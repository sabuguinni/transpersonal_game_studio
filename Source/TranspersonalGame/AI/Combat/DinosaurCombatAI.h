#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "GameplayTags.h"
#include "DinosaurCombatAI.generated.h"

UENUM(BlueprintType)
enum class EDinosaurCombatState : uint8
{
    Idle,
    Hunting,
    Stalking,
    Attacking,
    Fleeing,
    Territorial,
    PackCoordination,
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
    Extreme
};

UENUM(BlueprintType)
enum class EDinosaurArchetype : uint8
{
    ApexPredator,      // T-Rex, Giganotosaurus
    PackHunter,        // Velociraptor, Deinonychus
    AmbushPredator,    // Carnotaurus, Baryonyx
    TerritorialHerbivore, // Triceratops, Ankylosaurus
    FlockHerbivore,    // Parasaurolophus, Gallimimus
    SolitaryHerbivore, // Brachiosaurus, Diplodocus
    Scavenger,         // Compsognathus, Ornithomimus
    AquaticPredator,   // Spinosaurus, Mosasaurus
    FlyingPredator     // Pteranodon, Quetzalcoatlus
};

USTRUCT(BlueprintType)
struct FCombatPersonality
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Aggression = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Curiosity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Caution = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Territoriality = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float PackLoyalty = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intelligence = 0.5f;
};

USTRUCT(BlueprintType)
struct FDinosaurMemory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    TArray<FVector> LastKnownPlayerPositions;

    UPROPERTY(BlueprintReadWrite)
    float LastPlayerSightingTime = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    FVector LastNoiseLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite)
    float LastNoiseTime = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    TArray<AActor*> KnownThreats;

    UPROPERTY(BlueprintReadWrite)
    TArray<AActor*> KnownPrey;

    UPROPERTY(BlueprintReadWrite)
    FVector TerritoryCenter = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite)
    float TerritoryRadius = 2000.0f;
};

/**
 * Advanced AI Controller for Dinosaur Combat Behavior
 * Implements tactical AI with personality-driven decision making
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

    // Core AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionComponent* AIPerceptionComponent;

    // Combat Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    EDinosaurArchetype DinosaurArchetype = EDinosaurArchetype::PackHunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FCombatPersonality Personality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    class UBehaviorTree* CombatBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    class UBlackboard* CombatBlackboard;

    // Combat State
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    EDinosaurCombatState CurrentCombatState = EDinosaurCombatState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    FDinosaurMemory Memory;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    EDinosaurThreatLevel CurrentThreatLevel = EDinosaurThreatLevel::None;

    // Pack Coordination
    UPROPERTY(BlueprintReadOnly, Category = "Pack")
    TArray<ADinosaurCombatAI*> PackMembers;

    UPROPERTY(BlueprintReadOnly, Category = "Pack")
    bool bIsPackLeader = false;

    UPROPERTY(BlueprintReadOnly, Category = "Pack")
    ADinosaurCombatAI* PackLeader = nullptr;

public:
    // Combat State Management
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(EDinosaurCombatState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    EDinosaurCombatState GetCombatState() const { return CurrentCombatState; }

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    AActor* GetCurrentTarget() const { return CurrentTarget; }

    // Threat Assessment
    UFUNCTION(BlueprintCallable, Category = "Combat")
    EDinosaurThreatLevel AssessThreatLevel(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float CalculateThreatScore(AActor* Actor);

    // Pack Coordination
    UFUNCTION(BlueprintCallable, Category = "Pack")
    void JoinPack(ADinosaurCombatAI* Leader);

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void BecomePackLeader();

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void CoordinatePackAttack(AActor* Target);

    // Memory System
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void UpdatePlayerMemory(FVector PlayerPosition);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void UpdateNoiseMemory(FVector NoiseLocation);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    FVector GetLastKnownPlayerPosition();

    UFUNCTION(BlueprintCallable, Category = "Memory")
    bool HasRecentPlayerMemory(float TimeThreshold = 30.0f);

    // Tactical Decisions
    UFUNCTION(BlueprintCallable, Category = "Tactics")
    bool ShouldAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Tactics")
    bool ShouldFlee(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Tactics")
    bool ShouldInvestigate(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Tactics")
    FVector FindFlankingPosition(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Tactics")
    FVector FindAmbushPosition(AActor* Target);

protected:
    // Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Behavior Tree Events
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    // Internal Functions
    void UpdateCombatState();
    void UpdateThreatAssessment();
    void UpdatePackCoordination();
    void ProcessPersonalityFactors();

    // Archetype-Specific Behaviors
    void ExecuteApexPredatorBehavior();
    void ExecutePackHunterBehavior();
    void ExecuteAmbushPredatorBehavior();
    void ExecuteTerritorialHerbivoreBehavior();

private:
    float LastStateUpdateTime = 0.0f;
    float StateUpdateInterval = 0.1f;
    
    FGameplayTag CombatStateTag;
    FGameplayTag ThreatLevelTag;
    FGameplayTag ArchetypeTag;
};