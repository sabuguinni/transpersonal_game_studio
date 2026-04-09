#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Components/ActorComponent.h"
#include "JurassicBehaviorTreeSystemV43.generated.h"

UENUM(BlueprintType)
enum class EJurassicBehaviorState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Foraging UMETA(DisplayName = "Foraging"),
    Hunting UMETA(DisplayName = "Hunting"),
    Fleeing UMETA(DisplayName = "Fleeing"),
    Socializing UMETA(DisplayName = "Socializing"),
    Territorial UMETA(DisplayName = "Territorial"),
    Sleeping UMETA(DisplayName = "Sleeping"),
    Migrating UMETA(DisplayName = "Migrating"),
    Mating UMETA(DisplayName = "Mating"),
    Patrolling UMETA(DisplayName = "Patrolling")
};

UENUM(BlueprintType)
enum class EJurassicNPCArchetype : uint8
{
    AlphaHerbivore UMETA(DisplayName = "Alpha Herbivore"),
    PackHerbivore UMETA(DisplayName = "Pack Herbivore"),
    SoloCarnivore UMETA(DisplayName = "Solo Carnivore"),
    PackCarnivore UMETA(DisplayName = "Pack Carnivore"),
    ApexPredator UMETA(DisplayName = "Apex Predator"),
    Scavenger UMETA(DisplayName = "Scavenger"),
    Omnivore UMETA(DisplayName = "Omnivore"),
    Guardian UMETA(DisplayName = "Guardian")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FJurassicBehaviorContext
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Context")
    EJurassicBehaviorState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Context")
    EJurassicBehaviorState PreviousState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Context")
    float StateTransitionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Context")
    float StateDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Context")
    TArray<AActor*> PerceivedActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Context")
    AActor* PrimaryTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Context")
    FVector LastKnownPlayerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Context")
    float ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Context")
    bool bIsInPack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Context")
    AActor* PackLeader;

    FJurassicBehaviorContext()
    {
        CurrentState = EJurassicBehaviorState::Idle;
        PreviousState = EJurassicBehaviorState::Idle;
        StateTransitionTime = 0.0f;
        StateDuration = 0.0f;
        PrimaryTarget = nullptr;
        LastKnownPlayerLocation = FVector::ZeroVector;
        ThreatLevel = 0.0f;
        bIsInPack = false;
        PackLeader = nullptr;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UJurassicBehaviorTreeSystemV43 : public UActorComponent
{
    GENERATED_BODY()

public:
    UJurassicBehaviorTreeSystemV43();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core behavior tree management
    UFUNCTION(BlueprintCallable, Category = "Jurassic Behavior")
    void InitializeBehaviorTree(UBehaviorTree* InBehaviorTree, EJurassicNPCArchetype InArchetype);

    UFUNCTION(BlueprintCallable, Category = "Jurassic Behavior")
    void UpdateBehaviorContext(const FJurassicBehaviorContext& NewContext);

    UFUNCTION(BlueprintCallable, Category = "Jurassic Behavior")
    void TransitionToState(EJurassicBehaviorState NewState, float TransitionDelay = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Jurassic Behavior")
    bool CanTransitionToState(EJurassicBehaviorState TargetState) const;

    // Perception and awareness
    UFUNCTION(BlueprintCallable, Category = "Jurassic Behavior")
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION(BlueprintCallable, Category = "Jurassic Behavior")
    void SetThreatLevel(float NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Jurassic Behavior")
    float CalculateThreatLevel(AActor* ThreatActor) const;

    // Pack behavior
    UFUNCTION(BlueprintCallable, Category = "Jurassic Behavior")
    void JoinPack(AActor* Leader);

    UFUNCTION(BlueprintCallable, Category = "Jurassic Behavior")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Jurassic Behavior")
    void BroadcastPackSignal(const FString& SignalType, const FVector& Location);

    // Advanced behavior patterns
    UFUNCTION(BlueprintCallable, Category = "Jurassic Behavior")
    void ExecuteHuntingBehavior(AActor* PreyTarget);

    UFUNCTION(BlueprintCallable, Category = "Jurassic Behavior")
    void ExecuteFlockingBehavior(const TArray<AActor*>& FlockMembers);

    UFUNCTION(BlueprintCallable, Category = "Jurassic Behavior")
    void ExecuteTerritorialBehavior(const FVector& TerritoryCenter, float TerritoryRadius);

    // Blackboard management
    UFUNCTION(BlueprintCallable, Category = "Jurassic Behavior")
    void UpdateBlackboardValues();

    UFUNCTION(BlueprintCallable, Category = "Jurassic Behavior")
    void SetBlackboardValue(const FName& KeyName, UObject* Value);

    UFUNCTION(BlueprintCallable, Category = "Jurassic Behavior")
    void SetBlackboardVector(const FName& KeyName, const FVector& Value);

    UFUNCTION(BlueprintCallable, Category = "Jurassic Behavior")
    void SetBlackboardFloat(const FName& KeyName, float Value);

    UFUNCTION(BlueprintCallable, Category = "Jurassic Behavior")
    void SetBlackboardBool(const FName& KeyName, bool Value);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Jurassic Behavior")
    EJurassicBehaviorState GetCurrentState() const { return BehaviorContext.CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Jurassic Behavior")
    EJurassicNPCArchetype GetArchetype() const { return NPCArchetype; }

    UFUNCTION(BlueprintPure, Category = "Jurassic Behavior")
    float GetThreatLevel() const { return BehaviorContext.ThreatLevel; }

    UFUNCTION(BlueprintPure, Category = "Jurassic Behavior")
    bool IsInPack() const { return BehaviorContext.bIsInPack; }

    UFUNCTION(BlueprintPure, Category = "Jurassic Behavior")
    AActor* GetPackLeader() const { return BehaviorContext.PackLeader; }

    UFUNCTION(BlueprintPure, Category = "Jurassic Behavior")
    const FJurassicBehaviorContext& GetBehaviorContext() const { return BehaviorContext; }

protected:
    // Core components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAIPerceptionComponent* PerceptionComponent;

    // Behavior configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Configuration")
    UBehaviorTree* DefaultBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Configuration")
    EJurassicNPCArchetype NPCArchetype;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Configuration")
    FJurassicBehaviorContext BehaviorContext;

    // State transition rules
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State Transitions")
    TMap<EJurassicBehaviorState, TArray<EJurassicBehaviorState>> AllowedTransitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State Transitions")
    TMap<EJurassicBehaviorState, float> MinStateDurations;

    // Archetype-specific behavior trees
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype Trees")
    TMap<EJurassicNPCArchetype, UBehaviorTree*> ArchetypeBehaviorTrees;

    // Pack behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    TArray<AActor*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    float PackCommunicationRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    float PackCoordinationStrength;

    // Threat assessment
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    TMap<TSubclassOf<AActor>, float> ThreatWeights;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    float ThreatDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    float MaxThreatDistance;

private:
    // Internal state management
    float StateTimer;
    float TransitionCooldown;
    bool bIsTransitioning;

    // Helper functions
    void InitializeStateTransitions();
    void InitializeArchetypeTrees();
    void InitializeThreatWeights();
    void UpdateStateTimer(float DeltaTime);
    bool ValidateStateTransition(EJurassicBehaviorState FromState, EJurassicBehaviorState ToState) const;
    void OnStateEntered(EJurassicBehaviorState NewState);
    void OnStateExited(EJurassicBehaviorState OldState);
    void UpdatePerceptionData();
    void ProcessPackCommunication();
    void UpdateThreatAssessment(float DeltaTime);

    // Blackboard key names
    static const FName BB_CurrentState;
    static const FName BB_PrimaryTarget;
    static const FName BB_LastKnownPlayerLocation;
    static const FName BB_ThreatLevel;
    static const FName BB_PackLeader;
    static const FName BB_IsInPack;
    static const FName BB_PatrolLocation;
    static const FName BB_HomeLocation;
    static const FName BB_FleeLocation;
    static const FName BB_HuntTarget;
};