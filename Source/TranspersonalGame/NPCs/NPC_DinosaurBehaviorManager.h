#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "SharedTypes.h"
#include "NPC_DinosaurBehaviorManager.generated.h"

UENUM(BlueprintType)
enum class ENPC_DinosaurState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Sleeping    UMETA(DisplayName = "Sleeping"),
    Aggressive  UMETA(DisplayName = "Aggressive")
};

UENUM(BlueprintType)
enum class ENPC_DinosaurSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus")
};

USTRUCT(BlueprintType)
struct FNPC_DinosaurStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float Hunger = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float Aggression = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float Fear = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float MovementSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float DetectionRange = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float AttackRange = 500.0f;
};

USTRUCT(BlueprintType)
struct FNPC_DinosaurMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector LastPlayerLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float LastPlayerSeen = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector HomeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FVector> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 CurrentPatrolIndex = 0;
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_DinosaurBehaviorManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_DinosaurBehaviorManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core behavior properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    ENPC_DinosaurSpecies Species = ENPC_DinosaurSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    ENPC_DinosaurState CurrentState = ENPC_DinosaurState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    FNPC_DinosaurStats DinosaurStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    FNPC_DinosaurMemory DinosaurMemory;

    // Behavior configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Config")
    float StateUpdateInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Config")
    float HungerDecayRate = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Config")
    float MemoryDuration = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Config")
    bool bIsPackHunter = false;

    // Player tracking
    UPROPERTY(BlueprintReadOnly, Category = "Player Tracking")
    class APawn* PlayerPawn;

    UPROPERTY(BlueprintReadOnly, Category = "Player Tracking")
    float DistanceToPlayer = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Player Tracking")
    bool bPlayerInSight = false;

    // Animation integration
    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    class UAnimInstance* AnimationInstance;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    class UAnim_BlendSpaceController* BlendSpaceController;

    // Behavior functions
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void SetDinosaurState(ENPC_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void UpdateBehaviorLogic();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void CheckPlayerProximity();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void UpdatePatrolBehavior();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void UpdateHuntingBehavior();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void UpdateIdleBehavior();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void InitializePatrolPoints();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    FVector GetNextPatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void UpdateMemory();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    bool IsPlayerRemembered() const;

    // Species-specific behavior
    UFUNCTION(BlueprintCallable, Category = "Species Behavior")
    void ConfigureForSpecies(ENPC_DinosaurSpecies NewSpecies);

    UFUNCTION(BlueprintCallable, Category = "Species Behavior")
    void UpdateTRexBehavior();

    UFUNCTION(BlueprintCallable, Category = "Species Behavior")
    void UpdateVelociraptorBehavior();

    UFUNCTION(BlueprintCallable, Category = "Species Behavior")
    void UpdateHerbivoreBehavior();

    // Animation integration
    UFUNCTION(BlueprintCallable, Category = "Animation Integration")
    void UpdateAnimationState();

    UFUNCTION(BlueprintCallable, Category = "Animation Integration")
    void SetAnimationSpeed(float Speed);

    UFUNCTION(BlueprintCallable, Category = "Animation Integration")
    void TriggerAnimationEvent(const FString& EventName);

private:
    FTimerHandle BehaviorUpdateTimer;
    FTimerHandle HungerUpdateTimer;
    float LastBehaviorUpdate = 0.0f;
    
    void InitializeSpeciesDefaults();
    void UpdateHungerSystem(float DeltaTime);
    bool LineOfSightToPlayer() const;
    FVector CalculateFleeDirection() const;
};