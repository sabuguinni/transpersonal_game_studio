#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"

UENUM(BlueprintType)
enum class ECombat_DinosaurState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Territorial UMETA(DisplayName = "Territorial")
};

UENUM(BlueprintType)
enum class ECombat_DinosaurSpecies : uint8
{
    TRex           UMETA(DisplayName = "T-Rex"),
    Velociraptor   UMETA(DisplayName = "Velociraptor"),
    Triceratops    UMETA(DisplayName = "Triceratops"),
    Brachiosaurus  UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus   UMETA(DisplayName = "Ankylosaurus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_DinosaurStats
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
    float SightRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float HearingRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float MovementSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float Aggression = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float TerritorialRadius = 1000.0f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_DinosaurBehavior : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_DinosaurBehavior();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core behavior functions
    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void SetDinosaurState(ECombat_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    ECombat_DinosaurState GetDinosaurState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void SetSpecies(ECombat_DinosaurSpecies Species);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void InitializeStats();

    // Combat functions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AttackTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool CanAttackTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TakeDamage(float Damage, AActor* Attacker);

    // Perception functions
    UFUNCTION(BlueprintCallable, Category = "Perception")
    void OnTargetSighted(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Perception")
    void OnTargetLost(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Perception")
    void OnSoundHeard(FVector SoundLocation, float SoundIntensity);

protected:
    // State properties
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    ECombat_DinosaurState CurrentState = ECombat_DinosaurState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    ECombat_DinosaurSpecies DinosaurSpecies = ECombat_DinosaurSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    FCombat_DinosaurStats Stats;

    // Target tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    FVector LastKnownTargetLocation = FVector::ZeroVector;

    // Timers
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Timers")
    float LastAttackTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timers")
    float AttackCooldown = 2.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Timers")
    float StateChangeTime = 0.0f;

    // Behavior functions
    void UpdateIdleBehavior(float DeltaTime);
    void UpdatePatrolBehavior(float DeltaTime);
    void UpdateHuntingBehavior(float DeltaTime);
    void UpdateAttackingBehavior(float DeltaTime);
    void UpdateFleeingBehavior(float DeltaTime);
    void UpdateFeedingBehavior(float DeltaTime);
    void UpdateTerritorialBehavior(float DeltaTime);

    // Utility functions
    bool IsTargetInRange(AActor* Target, float Range) const;
    float GetDistanceToTarget(AActor* Target) const;
    void MoveTowardsTarget(AActor* Target, float DeltaTime);
    void MoveAwayFromTarget(AActor* Target, float DeltaTime);
    void RotateTowardsTarget(AActor* Target, float DeltaTime);
};

UCLASS()
class TRANSPERSONALGAME_API ACombat_DinosaurAIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombat_DinosaurAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Possess(APawn* InPawn) override;

public:
    // AI Perception callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // Behavior Tree functions
    UFUNCTION(BlueprintCallable, Category = "AI")
    void StartBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void StopBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetBlackboardValue(const FName& KeyName, UObject* Value);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetBlackboardVector(const FName& KeyName, FVector Value);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetBlackboardFloat(const FName& KeyName, float Value);

protected:
    // AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionComponent* AIPerceptionComponent;

    // Perception configs
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Perception")
    class UAISightConfig* SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Perception")
    class UAIHearingConfig* HearingConfig;

    // Behavior Tree asset
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBehaviorTree* BehaviorTreeAsset;

    // Blackboard asset
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBlackboardData* BlackboardAsset;

private:
    void SetupPerception();
    void SetupBlackboard();
};

#include "Combat_DinosaurAI.generated.h"#include "Combat_DinosaurAI.generated.h"
