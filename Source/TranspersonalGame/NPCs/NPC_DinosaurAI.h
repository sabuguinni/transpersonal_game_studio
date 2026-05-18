#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "../SharedTypes.h"
#include "NPC_DinosaurAI.generated.h"

UENUM(BlueprintType)
enum class ENPC_DinosaurSpecies : uint8
{
    TRex           UMETA(DisplayName = "Tyrannosaurus Rex"),
    Velociraptor   UMETA(DisplayName = "Velociraptor"),
    Triceratops    UMETA(DisplayName = "Triceratops"),
    Brachiosaurus  UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus   UMETA(DisplayName = "Ankylosaurus")
};

UENUM(BlueprintType)
enum class ENPC_DinosaurState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Sleeping    UMETA(DisplayName = "Sleeping"),
    Territorial UMETA(DisplayName = "Territorial"),
    Dead        UMETA(DisplayName = "Dead")
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
    float Stamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MovementSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float DetectionRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackRange = 200.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANPC_DinosaurAI : public APawn
{
    GENERATED_BODY()

public:
    ANPC_DinosaurAI();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* DinosaurMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* CollisionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPawnSensingComponent* PawnSensing;

    // AI Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    ENPC_DinosaurSpecies Species = ENPC_DinosaurSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    ENPC_DinosaurState CurrentState = ENPC_DinosaurState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    FNPC_DinosaurStats Stats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBehaviorTree* BehaviorTreeAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float PatrolRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float StateChangeTimer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float IdleTime = 5.0f;

    // Target and Navigation
    UPROPERTY(BlueprintReadOnly, Category = "AI")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "AI")
    FVector HomeLocation;

    UPROPERTY(BlueprintReadOnly, Category = "AI")
    FVector PatrolDestination;

    // AI Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetDinosaurState(ENPC_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void StartPatrolling();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void StartHunting(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void StartFleeing(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void AttackTarget();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void TakeDamage(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "AI")
    bool IsPlayerInRange() const;

    UFUNCTION(BlueprintCallable, Category = "AI")
    float GetDistanceToPlayer() const;

    // Species-specific behavior
    UFUNCTION(BlueprintCallable, Category = "AI")
    void InitializeSpeciesStats();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void UpdateAIBehavior(float DeltaTime);

    // Perception callbacks
    UFUNCTION()
    void OnPlayerSeen(APawn* SeenPawn);

    UFUNCTION()
    void OnPlayerHeard(APawn* HeardPawn, const FVector& Location, float Volume);

private:
    // Internal AI logic
    void UpdateIdleState(float DeltaTime);
    void UpdatePatrollingState(float DeltaTime);
    void UpdateHuntingState(float DeltaTime);
    void UpdateFleeingState(float DeltaTime);
    void UpdateFeedingState(float DeltaTime);

    FVector GetRandomPatrolPoint();
    bool HasReachedDestination() const;
    void MoveTowardsTarget(const FVector& TargetLocation, float DeltaTime);

    // Species behavior modifiers
    void ApplyTRexBehavior();
    void ApplyVelociraptorBehavior();
    void ApplyTriceratopsBehavior();
    void ApplyBrachiosaurusBehavior();
    void ApplyAnkylosaurusBehavior();
};