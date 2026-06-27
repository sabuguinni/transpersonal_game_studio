#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TranspersonalGame/SharedTypes.h"
#include "DinosaurBase.generated.h"

UENUM(BlueprintType)
enum class EEng_DinosaurSpecies : uint8
{
    TyrannosaurusRex    UMETA(DisplayName = "Tyrannosaurus Rex"),
    Velociraptor        UMETA(DisplayName = "Velociraptor"),
    Triceratops         UMETA(DisplayName = "Triceratops"),
    Brachiosaurus       UMETA(DisplayName = "Brachiosaurus"),
    Stegosaurus         UMETA(DisplayName = "Stegosaurus"),
    Pterodactyl         UMETA(DisplayName = "Pterodactyl"),
    Ankylosaurus        UMETA(DisplayName = "Ankylosaurus"),
    Spinosaurus         UMETA(DisplayName = "Spinosaurus"),
    Unknown             UMETA(DisplayName = "Unknown")
};

UENUM(BlueprintType)
enum class EEng_DinosaurBehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Sleeping    UMETA(DisplayName = "Sleeping"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class EEng_DinosaurDiet : uint8
{
    Carnivore   UMETA(DisplayName = "Carnivore"),
    Herbivore   UMETA(DisplayName = "Herbivore"),
    Omnivore    UMETA(DisplayName = "Omnivore")
};

USTRUCT(BlueprintType)
struct FEng_DinosaurStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float CurrentHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float AttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float DetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MoveSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float SprintSpeed = 700.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float Hunger = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float HungerDecayRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float TerritoryRadius = 3000.0f;
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ADinosaurBase : public ACharacter
{
    GENERATED_BODY()

public:
    ADinosaurBase();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Species identity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    EEng_DinosaurSpecies Species = EEng_DinosaurSpecies::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    EEng_DinosaurDiet Diet = EEng_DinosaurDiet::Carnivore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    FString SpeciesDisplayName = TEXT("Unknown Dinosaur");

    // Stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    FEng_DinosaurStats Stats;

    // Behavior state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Behavior")
    EEng_DinosaurBehaviorState BehaviorState = EEng_DinosaurBehaviorState::Idle;

    // Territory
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Territory")
    FVector TerritoryCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Territory")
    bool bHasTerritoryCenter = false;

    // Combat
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
        class AController* EventInstigator, AActor* DamageCauser) override;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    virtual void Attack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    bool IsAlive() const;

    // Behavior
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    void SetBehaviorState(EEng_DinosaurBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    EEng_DinosaurBehaviorState GetBehaviorState() const { return BehaviorState; }

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    virtual void OnDetectPlayer(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    virtual void OnLosePlayer();

    // Events (override in BP or subclasses)
    UFUNCTION(BlueprintImplementableEvent, Category = "Dinosaur|Events")
    void OnDeath();

    UFUNCTION(BlueprintImplementableEvent, Category = "Dinosaur|Events")
    void OnAttackStart(AActor* Target);

    UFUNCTION(BlueprintImplementableEvent, Category = "Dinosaur|Events")
    void OnStateChanged(EEng_DinosaurBehaviorState OldState, EEng_DinosaurBehaviorState NewState);

protected:
    virtual void Die();
    void UpdateHunger(float DeltaTime);

    UPROPERTY()
    AActor* CurrentTarget = nullptr;

    float TimeSinceLastAttack = 0.0f;
    float AttackCooldown = 1.5f;
};
