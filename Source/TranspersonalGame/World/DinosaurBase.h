#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SharedTypes.h"
#include "DinosaurBase.generated.h"

// -----------------------------------------------------------------------
// Enums — must be at global scope (UE5 RULE 1)
// -----------------------------------------------------------------------

UENUM(BlueprintType)
enum class EEng_DinoState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Wandering   UMETA(DisplayName = "Wandering"),
    Foraging    UMETA(DisplayName = "Foraging"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Dead        UMETA(DisplayName = "Dead"),
};

UENUM(BlueprintType)
enum class EEng_DinoSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus"),
    Generic         UMETA(DisplayName = "Generic"),
};

UENUM(BlueprintType)
enum class EEng_DinoDiet : uint8
{
    Carnivore   UMETA(DisplayName = "Carnivore"),
    Herbivore   UMETA(DisplayName = "Herbivore"),
    Omnivore    UMETA(DisplayName = "Omnivore"),
};

// -----------------------------------------------------------------------
// Structs — must be at global scope (UE5 RULE 1)
// -----------------------------------------------------------------------

USTRUCT(BlueprintType)
struct FEng_DinoStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxHealth = 500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float CurrentHealth = 500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float AttackDamage = 80.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float AttackRange = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float AttackCooldown = 2.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float DetectionRadius = 1500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float WalkSpeed = 300.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float RunSpeed = 700.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float TurnRate = 120.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float Mass = 1000.f;
};

// -----------------------------------------------------------------------
// ADinosaurBase — base class for all dinosaur pawns
// -----------------------------------------------------------------------

UCLASS(BlueprintType, Blueprintable, ClassGroup = "TranspersonalGame|Dinosaurs")
class TRANSPERSONALGAME_API ADinosaurBase : public ACharacter
{
    GENERATED_BODY()

public:
    ADinosaurBase();

    // ---- Core UE5 overrides ----
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
                             AController* EventInstigator, AActor* DamageCauser) override;

    // ---- Species & diet ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    EEng_DinoSpecies Species = EEng_DinoSpecies::Generic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    EEng_DinoDiet Diet = EEng_DinoDiet::Carnivore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    FName DinoName = NAME_None;

    // ---- Stats ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    FEng_DinoStats Stats;

    // ---- Behavior state ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    EEng_DinoState BehaviorState = EEng_DinoState::Idle;

    // ---- Target ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    AActor* CurrentTarget = nullptr;

    // ---- Blueprint-callable functions ----
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    void SetBehaviorState(EEng_DinoState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    bool CanAttack() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void PerformAttack();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    void UpdateBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    AActor* FindNearestThreat() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    AActor* FindNearestPrey() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void Die();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Stats")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Stats")
    bool IsAlive() const;

    // ---- Blueprint events (implementable in BP subclasses) ----
    UFUNCTION(BlueprintImplementableEvent, Category = "Dinosaur|Events")
    void OnDinoAttack();

    UFUNCTION(BlueprintImplementableEvent, Category = "Dinosaur|Events")
    void OnDinoDeath();

    UFUNCTION(BlueprintImplementableEvent, Category = "Dinosaur|Events")
    void OnDinoStateChanged(EEng_DinoState OldState, EEng_DinoState NewState);

protected:
    // ---- Internal helpers ----
    void HandleIdleState(float DeltaTime);
    void HandleWanderingState(float DeltaTime);
    void HandleHuntingState(float DeltaTime);
    void HandleFleeingState(float DeltaTime);
    void HandleAttackingState(float DeltaTime);

    bool IsTargetInRange(AActor* Target, float Range) const;
    void MoveTowardTarget(AActor* Target, float DeltaTime);

    // ---- Internal state ----
    float TimeSinceLastAttack = 0.f;
    float WanderTimer = 0.f;
    float WanderInterval = 5.f;
    FVector WanderDestination = FVector::ZeroVector;
    bool bIsDead = false;
};
