#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "NPC_DinosaurBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_DinosaurState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Patrolling UMETA(DisplayName = "Patrolling"),
    Hunting UMETA(DisplayName = "Hunting"),
    Fleeing UMETA(DisplayName = "Fleeing"),
    Feeding UMETA(DisplayName = "Feeding"),
    Sleeping UMETA(DisplayName = "Sleeping"),
    Territorial UMETA(DisplayName = "Territorial"),
    Mating UMETA(DisplayName = "Mating")
};

UENUM(BlueprintType)
enum class ENPC_DinosaurSpecies : uint8
{
    TRex UMETA(DisplayName = "T-Rex"),
    Raptor UMETA(DisplayName = "Raptor"),
    Triceratops UMETA(DisplayName = "Triceratops"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Stegosaurus UMETA(DisplayName = "Stegosaurus"),
    Pteranodon UMETA(DisplayName = "Pteranodon"),
    Ankylosaurus UMETA(DisplayName = "Ankylosaurus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus")
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
    float Thirst = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Stamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Fear = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Aggression = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float TerritorialRadius = 2000.0f;
};

USTRUCT(BlueprintType)
struct FNPC_DinosaurBehaviorSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float PatrolRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float DetectionRange = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float FleeRange = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float WalkSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float RunSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    int32 PackSize = 3;
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

    // Core behavior functions
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void SetDinosaurSpecies(ENPC_DinosaurSpecies NewSpecies);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void SetBehaviorState(ENPC_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    ENPC_DinosaurState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void StartPatrolling();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void StartHunting(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void StartFleeing(AActor* ThreatSource);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void TakeDamage(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    bool CanSeeTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    float GetDistanceToTarget(AActor* Target) const;

    // Pack behavior
    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void JoinPack(UNPC_DinosaurBehaviorComponent* PackLeader);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    bool IsInPack() const { return PackLeader != nullptr; }

    // Species-specific behaviors
    UFUNCTION(BlueprintCallable, Category = "Species Behavior")
    void InitializeTRexBehavior();

    UFUNCTION(BlueprintCallable, Category = "Species Behavior")
    void InitializeRaptorBehavior();

    UFUNCTION(BlueprintCallable, Category = "Species Behavior")
    void InitializeHerbivoreBehavior();

protected:
    // Core properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Setup")
    ENPC_DinosaurSpecies Species = ENPC_DinosaurSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Setup")
    ENPC_DinosaurState CurrentState = ENPC_DinosaurState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Setup")
    FNPC_DinosaurStats Stats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Setup")
    FNPC_DinosaurBehaviorSettings BehaviorSettings;

    // Behavior state
    UPROPERTY(BlueprintReadOnly, Category = "Behavior State")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Behavior State")
    FVector PatrolCenter;

    UPROPERTY(BlueprintReadOnly, Category = "Behavior State")
    FVector CurrentDestination;

    UPROPERTY(BlueprintReadOnly, Category = "Behavior State")
    float StateTimer = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Behavior State")
    float LastAttackTime = 0.0f;

    // Pack behavior
    UPROPERTY(BlueprintReadOnly, Category = "Pack Behavior")
    UNPC_DinosaurBehaviorComponent* PackLeader = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Pack Behavior")
    TArray<UNPC_DinosaurBehaviorComponent*> PackMembers;

    // Internal behavior functions
    void UpdateIdleBehavior(float DeltaTime);
    void UpdatePatrolBehavior(float DeltaTime);
    void UpdateHuntingBehavior(float DeltaTime);
    void UpdateFleeingBehavior(float DeltaTime);
    void UpdateFeedingBehavior(float DeltaTime);

    void ScanForTargets();
    void ScanForThreats();
    void UpdateStats(float DeltaTime);
    void MoveToLocation(const FVector& Location, float Speed);
    
    AActor* FindNearestPlayer() const;
    AActor* FindNearestPrey() const;
    AActor* FindNearestThreat() const;
    
    bool IsLocationReachable(const FVector& Location) const;
    FVector GetRandomPatrolPoint() const;
    
    void PlayRoarSound();
    void PlayAttackSound();
    void PlayHurtSound();
};