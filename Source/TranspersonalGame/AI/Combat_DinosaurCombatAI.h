#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Engine/Engine.h"
#include "Combat_DinosaurCombatAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_DinosaurType : uint8
{
    Carnivore_Apex     UMETA(DisplayName = "Apex Predator (T-Rex)"),
    Carnivore_Pack     UMETA(DisplayName = "Pack Hunter (Velociraptor)"),
    Herbivore_Large    UMETA(DisplayName = "Large Herbivore (Triceratops)"),
    Herbivore_Herd     UMETA(DisplayName = "Herd Animal (Parasaurolophus)"),
    Scavenger          UMETA(DisplayName = "Scavenger (Compsognathus)")
};

UENUM(BlueprintType)
enum class ECombat_AIState : uint8
{
    Idle               UMETA(DisplayName = "Idle/Roaming"),
    Hunting            UMETA(DisplayName = "Hunting Prey"),
    Attacking          UMETA(DisplayName = "Active Combat"),
    Defending          UMETA(DisplayName = "Defensive Stance"),
    Fleeing            UMETA(DisplayName = "Fleeing from Threat"),
    Patrolling         UMETA(DisplayName = "Territory Patrol"),
    PackCoordination   UMETA(DisplayName = "Pack Coordination")
};

USTRUCT(BlueprintType)
struct FCombat_ThreatAssessment
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    AActor* ThreatActor;

    UPROPERTY(BlueprintReadOnly)
    float ThreatLevel;

    UPROPERTY(BlueprintReadOnly)
    float Distance;

    UPROPERTY(BlueprintReadOnly)
    FVector LastKnownLocation;

    UPROPERTY(BlueprintReadOnly)
    float TimeSinceLastSeen;

    FCombat_ThreatAssessment()
    {
        ThreatActor = nullptr;
        ThreatLevel = 0.0f;
        Distance = 0.0f;
        LastKnownLocation = FVector::ZeroVector;
        TimeSinceLastSeen = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FCombat_PackCoordination
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    TArray<AActor*> PackMembers;

    UPROPERTY(BlueprintReadOnly)
    AActor* PackLeader;

    UPROPERTY(BlueprintReadOnly)
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly)
    FVector FlankingPositionLeft;

    UPROPERTY(BlueprintReadOnly)
    FVector FlankingPositionRight;

    UPROPERTY(BlueprintReadOnly)
    bool bIsCoordinatedAttack;

    FCombat_PackCoordination()
    {
        PackLeader = nullptr;
        CurrentTarget = nullptr;
        FlankingPositionLeft = FVector::ZeroVector;
        FlankingPositionRight = FVector::ZeroVector;
        bIsCoordinatedAttack = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_DinosaurCombatAI : public AAIController
{
    GENERATED_BODY()

public:
    ACombat_DinosaurCombatAI();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;

    // Core AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    UAIPerceptionComponent* AIPerceptionComponent;

    // Dinosaur Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Config")
    ECombat_DinosaurType DinosaurType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Config")
    class UBehaviorTree* BehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Config")
    float SightRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Config")
    float HearingRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Config")
    float TerritoryRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Config")
    float AggressionLevel;

    // Combat State
    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    ECombat_AIState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    TArray<FCombat_ThreatAssessment> KnownThreats;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    FCombat_PackCoordination PackData;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    FVector HomeLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    float LastAttackTime;

public:
    // Combat AI Interface
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetDinosaurType(ECombat_DinosaurType NewType);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateThreatAssessment();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void InitializePackBehavior(const TArray<AActor*>& PackMembers, AActor* Leader);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatState(ECombat_AIState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool CanAttackTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector CalculateFlankingPosition(AActor* Target, bool bLeftFlank) const;

    // Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

protected:
    // Internal Combat Logic
    void UpdateCombatBehavior(float DeltaTime);
    void ProcessThreatLevel(AActor* Actor, float Distance);
    void UpdatePackCoordination();
    void HandleTerritorialBehavior();
    void ExecuteFleeingBehavior();
    
    // Blackboard Key Names
    static const FName TargetActorKey;
    static const FName CurrentStateKey;
    static const FName HomeLocationKey;
    static const FName ThreatLevelKey;
    static const FName PackLeaderKey;
    static const FName IsInPackKey;

private:
    float StateUpdateTimer;
    float ThreatUpdateInterval;
    float PackCoordinationTimer;
};