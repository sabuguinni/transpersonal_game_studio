#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "NPC_DinosaurAI.generated.h"

UENUM(BlueprintType)
enum class ENPC_DinosaurSpecies : uint8
{
    TRex           UMETA(DisplayName = "T-Rex"),
    Raptor         UMETA(DisplayName = "Velociraptor"),
    Triceratops    UMETA(DisplayName = "Triceratops"),
    Brachiosaurus  UMETA(DisplayName = "Brachiosaurus"),
    Stegosaurus    UMETA(DisplayName = "Stegosaurus")
};

UENUM(BlueprintType)
enum class ENPC_DinosaurState : uint8
{
    Idle           UMETA(DisplayName = "Idle"),
    Patrolling     UMETA(DisplayName = "Patrolling"),
    Hunting        UMETA(DisplayName = "Hunting"),
    Fleeing        UMETA(DisplayName = "Fleeing"),
    Feeding        UMETA(DisplayName = "Feeding"),
    Sleeping       UMETA(DisplayName = "Sleeping"),
    Alert          UMETA(DisplayName = "Alert"),
    Attacking      UMETA(DisplayName = "Attacking")
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
    float Damage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Speed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float DetectionRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Hunger = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Fear = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Aggression = 30.0f;
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API ANPC_DinosaurAI : public APawn
{
    GENERATED_BODY()

public:
    ANPC_DinosaurAI();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* CollisionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPawnSensingComponent* PawnSensingComponent;

    // Dinosaur Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    ENPC_DinosaurSpecies Species = ENPC_DinosaurSpecies::Raptor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    ENPC_DinosaurState CurrentState = ENPC_DinosaurState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    FNPC_DinosaurStats Stats;

    // AI Behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float PatrolRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    TArray<FVector> PatrolPoints;

    UPROPERTY(BlueprintReadOnly, Category = "AI")
    int32 CurrentPatrolIndex = 0;

    UPROPERTY(BlueprintReadOnly, Category = "AI")
    AActor* TargetActor = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "AI")
    FVector HomeLocation;

    // Timers
    UPROPERTY(BlueprintReadOnly, Category = "AI")
    float StateTimer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float IdleTime = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float AlertTime = 3.0f;

public:
    // AI Functions
    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetState(ENPC_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void StartPatrolling();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void StartHunting(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void StartFleeing();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void Attack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "AI")
    bool CanSeeTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "AI")
    float GetDistanceToTarget(AActor* Target);

    // Behavior Functions
    UFUNCTION(BlueprintImplementableEvent, Category = "Behavior")
    void OnStateChanged(ENPC_DinosaurState NewState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Behavior")
    void OnTargetDetected(AActor* Target);

    UFUNCTION(BlueprintImplementableEvent, Category = "Behavior")
    void OnTargetLost();

    UFUNCTION(BlueprintImplementableEvent, Category = "Behavior")
    void OnAttackTarget(AActor* Target);

protected:
    // Internal AI Logic
    void UpdateAI(float DeltaTime);
    void UpdateStats(float DeltaTime);
    void HandleIdleState(float DeltaTime);
    void HandlePatrollingState(float DeltaTime);
    void HandleHuntingState(float DeltaTime);
    void HandleFleeingState(float DeltaTime);
    void HandleAlertState(float DeltaTime);
    void HandleAttackingState(float DeltaTime);

    // Utility Functions
    FVector GetNextPatrolPoint();
    void GeneratePatrolPoints();
    bool IsPlayerNearby();
    void MoveToLocation(FVector Location);

    // Sensing Callbacks
    UFUNCTION()
    void OnSeePawn(APawn* Pawn);

    UFUNCTION()
    void OnHearNoise(APawn* NoiseInstigator, const FVector& Location, float Volume);
};