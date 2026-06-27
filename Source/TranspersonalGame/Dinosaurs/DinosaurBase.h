#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SharedTypes.h"
#include "DinosaurBase.generated.h"

UENUM(BlueprintType)
enum class EEng_DinosaurBehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Eating      UMETA(DisplayName = "Eating"),
    Sleeping    UMETA(DisplayName = "Sleeping"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class EEng_DinosaurSpecies : uint8
{
    TyrannosaurusRex    UMETA(DisplayName = "Tyrannosaurus Rex"),
    Velociraptor        UMETA(DisplayName = "Velociraptor"),
    Triceratops         UMETA(DisplayName = "Triceratops"),
    Brachiosaurus       UMETA(DisplayName = "Brachiosaurus"),
    Pterodactyl         UMETA(DisplayName = "Pterodactyl"),
    Ankylosaurus        UMETA(DisplayName = "Ankylosaurus"),
    Stegosaurus         UMETA(DisplayName = "Stegosaurus"),
    Spinosaurus         UMETA(DisplayName = "Spinosaurus")
};

USTRUCT(BlueprintType)
struct FEng_DinosaurStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float CurrentHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float AttackDamage = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float DetectionRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float PatrolRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float WalkSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float RunSpeed = 700.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float Hunger = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    bool bIsCarnivore = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    bool bIsPackHunter = false;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurBase : public ACharacter
{
    GENERATED_BODY()

public:
    ADinosaurBase();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // --- Species & Stats ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    EEng_DinosaurSpecies Species = EEng_DinosaurSpecies::TyrannosaurusRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    FEng_DinosaurStats Stats;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur")
    EEng_DinosaurBehaviorState BehaviorState = EEng_DinosaurBehaviorState::Idle;

    // --- Target tracking ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    FVector PatrolOrigin = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    FVector PatrolDestination = FVector::ZeroVector;

    // --- Combat ---
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void TakeDinosaurDamage(float DamageAmount, AActor* DamageInstigator);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void PerformAttack();

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Combat")
    bool IsAlive() const;

    // --- AI Behavior ---
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    void SetBehaviorState(EEng_DinosaurBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    void DetectNearbyActors();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    void UpdatePatrol(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    void ChooseNewPatrolDestination();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    void UpdateHunting(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    void UpdateFleeing(float DeltaTime);

    // --- Events (override in BP) ---
    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnDeath();
    virtual void OnDeath_Implementation();

    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnAttackHit(AActor* HitActor);
    virtual void OnAttackHit_Implementation(AActor* HitActor);

protected:
    // Internal timers
    float TimeSinceLastDetection = 0.0f;
    float TimeSinceLastAttack = 0.0f;
    float AttackCooldown = 1.5f;
    float DetectionInterval = 0.5f;
    float HungerDecayRate = 0.5f;

    void UpdateBehavior(float DeltaTime);
};
