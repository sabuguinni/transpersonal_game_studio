#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "AIController.h"
#include "../SharedTypes.h"
#include "Combat_DinosaurAI.generated.h"

// Forward declarations
class UCombat_CombatManager;
class UNPC_BehaviorTreeManager;

UENUM(BlueprintType)
enum class ECombat_DinosaurSpecies : uint8
{
    TRex           UMETA(DisplayName = "T-Rex"),
    Raptor         UMETA(DisplayName = "Raptor"),
    Triceratops    UMETA(DisplayName = "Triceratops"),
    Brachiosaurus  UMETA(DisplayName = "Brachiosaurus"),
    Stegosaurus    UMETA(DisplayName = "Stegosaurus")
};

UENUM(BlueprintType)
enum class ECombat_DinosaurState : uint8
{
    Idle           UMETA(DisplayName = "Idle"),
    Patrolling     UMETA(DisplayName = "Patrolling"),
    Investigating  UMETA(DisplayName = "Investigating"),
    Hunting        UMETA(DisplayName = "Hunting"),
    Attacking      UMETA(DisplayName = "Attacking"),
    Fleeing        UMETA(DisplayName = "Fleeing"),
    Feeding        UMETA(DisplayName = "Feeding"),
    Socializing    UMETA(DisplayName = "Socializing")
};

USTRUCT(BlueprintType)
struct FCombat_DinosaurStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float AttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float SightRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float HearingRange = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float MovementSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float Aggressiveness = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float PackLoyalty = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float TerritorialRadius = 2000.0f;

    FCombat_DinosaurStats()
    {
        Health = 100.0f;
        MaxHealth = 100.0f;
        AttackDamage = 25.0f;
        AttackRange = 300.0f;
        SightRange = 1500.0f;
        HearingRange = 800.0f;
        MovementSpeed = 400.0f;
        Aggressiveness = 0.5f;
        PackLoyalty = 0.7f;
        TerritorialRadius = 2000.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_DinosaurAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_DinosaurAI();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core AI Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur AI")
    ECombat_DinosaurSpecies Species = ECombat_DinosaurSpecies::Raptor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur AI")
    ECombat_DinosaurState CurrentState = ECombat_DinosaurState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur AI")
    FCombat_DinosaurStats Stats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur AI")
    bool bIsPackLeader = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur AI")
    int32 PackID = -1;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur AI")
    TArray<AActor*> PackMembers;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur AI")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur AI")
    FVector HomeLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur AI")
    float LastAttackTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur AI")
    float StateChangeTime = 0.0f;

    // AI Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void SetDinosaurState(ECombat_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void AttackTarget();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void FleeFromThreat(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void JoinPack(int32 NewPackID, bool bAsLeader = false);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    bool IsInAttackRange(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    bool CanSeeTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    float GetDistanceToTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void TakeDamage(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void InitializeForSpecies(ECombat_DinosaurSpecies NewSpecies);

    // Pack Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void IssuePackCommand(ECombat_DinosaurState CommandState, AActor* CommandTarget = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void ReceivePackCommand(ECombat_DinosaurState CommandState, AActor* CommandTarget);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    FVector GetPackFormationPosition() const;

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void UpdatePackMembers();

private:
    // Internal AI Logic
    void UpdateAIBehavior(float DeltaTime);
    void HandleIdleState(float DeltaTime);
    void HandlePatrollingState(float DeltaTime);
    void HandleInvestigatingState(float DeltaTime);
    void HandleHuntingState(float DeltaTime);
    void HandleAttackingState(float DeltaTime);
    void HandleFleeingState(float DeltaTime);
    void HandleFeedingState(float DeltaTime);
    void HandleSocializingState(float DeltaTime);

    void ScanForThreats();
    void ScanForPrey();
    void UpdateMovement(float DeltaTime);
    void ProcessCombatLogic(float DeltaTime);

    // Species-specific behavior modifiers
    void ApplySpeciesModifiers();
    FCombat_DinosaurStats GetDefaultStatsForSpecies(ECombat_DinosaurSpecies InSpecies) const;

    // Internal state
    FVector PatrolTarget;
    float NextPatrolTime = 0.0f;
    float ThreatScanTimer = 0.0f;
    float PackUpdateTimer = 0.0f;
    bool bHasValidPatrolTarget = false;

    // Component references
    UCombat_CombatManager* CombatManager;
    UNPC_BehaviorTreeManager* BehaviorManager;
};