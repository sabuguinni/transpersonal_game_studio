#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Components/PawnNoiseEmitterComponent.h"
#include "GameplayTags.h"
#include "CombatAIController.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;
class UAIPerceptionComponent;
class UBehaviorTree;
class UBlackboard;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCombatStateChanged, class ADinosaurPawn*, Dinosaur, ECombatState, NewState);

UENUM(BlueprintType)
enum class ECombatState : uint8
{
    Passive,
    Investigating,
    Hunting,
    Attacking,
    Fleeing,
    Territorial,
    Feeding
};

UENUM(BlueprintType)
enum class EThreatLevel : uint8
{
    None,
    Low,
    Medium,
    High,
    Extreme
};

UENUM(BlueprintType)
enum class ECombatArchetype : uint8
{
    ApexPredator,      // T-Rex, Spinosaurus
    PackHunter,        // Velociraptor, Utahraptor
    AmbushPredator,    // Carnotaurus, Baryonyx
    TerritorialHerbivore, // Triceratops, Ankylosaurus
    FlockHerbivore,    // Parasaurolophus, Gallimimus
    SolitaryHerbivore  // Brachiosaurus, Diplodocus
};

/**
 * Advanced AI Controller for dinosaur combat behavior
 * Implements tactical AI with dynamic threat assessment and adaptive behavior
 */
UCLASS(BlueprintType, Blueprintable)
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
    UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAIPerceptionComponent* AIPerceptionComponent;

    // Behavior Trees for different archetypes
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Behavior Trees")
    TMap<ECombatArchetype, UBehaviorTree*> ArchetypeBehaviorTrees;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Behavior Trees")
    UBlackboard* CombatBlackboard;

public:
    // Combat State Management
    UPROPERTY(BlueprintAssignable)
    FOnCombatStateChanged OnCombatStateChanged;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    ECombatState CurrentCombatState;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    ECombatArchetype CombatArchetype;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    EThreatLevel CurrentThreatLevel;

    // Combat Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Detection")
    float SightRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Detection")
    float HearingRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Detection")
    float SightAngle = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Behavior")
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Behavior")
    float TerritorialRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Behavior")
    float FleeHealthThreshold = 0.3f;

    // Tactical Combat Features
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Tactics")
    bool bUsesAmbushTactics = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Tactics")
    bool bUsesPackCoordination = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Tactics")
    bool bCanCallForHelp = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Tactics")
    float AmbushWaitTime = 5.0f;

    // Memory and Learning
    UPROPERTY(BlueprintReadOnly, Category = "Combat|Memory")
    TArray<AActor*> KnownThreats;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Memory")
    TArray<FVector> PatrolPoints;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Memory")
    AActor* LastKnownPlayerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Memory")
    float MemoryDuration = 30.0f;

public:
    // Core Combat Functions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(ECombatState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatArchetype(ECombatArchetype NewArchetype);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    EThreatLevel CalculateThreatLevel(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdateThreatAssessment();

    // Tactical Functions
    UFUNCTION(BlueprintCallable, Category = "Combat|Tactics")
    FVector FindAmbushPosition(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat|Tactics")
    FVector FindFlankingPosition(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat|Tactics")
    void CallForBackup(float Radius = 1500.0f);

    UFUNCTION(BlueprintCallable, Category = "Combat|Tactics")
    bool ShouldRetreat();

    // Pack Coordination
    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    TArray<ACombatAIController*> GetNearbyPackMembers(float Radius = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void CoordinatePackAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void AssignPackRoles();

    // Environmental Awareness
    UFUNCTION(BlueprintCallable, Category = "Combat|Environment")
    bool CanUseEnvironmentalAdvantage(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat|Environment")
    FVector FindHighGroundPosition(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat|Environment")
    bool IsInTerritory(FVector Location);

protected:
    // Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Internal State Management
    void UpdateBlackboardValues();
    void ProcessThreatMemory();
    void UpdatePatrolBehavior();

    // Combat Timers
    FTimerHandle ThreatAssessmentTimer;
    FTimerHandle MemoryUpdateTimer;
    FTimerHandle StateUpdateTimer;

    // Gameplay Tags for Combat States
    UPROPERTY(EditDefaultsOnly, Category = "Combat|Tags")
    FGameplayTag CombatStateTag;

    UPROPERTY(EditDefaultsOnly, Category = "Combat|Tags")
    FGameplayTag ThreatLevelTag;

    UPROPERTY(EditDefaultsOnly, Category = "Combat|Tags")
    FGameplayTag ArchetypeTag;
};