#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DinosaurBase.generated.h"

UENUM(BlueprintType)
enum class EDinosaurSpecies : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    TyrannosaurusRex UMETA(DisplayName = "Tyrannosaurus Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Stegosaurus     UMETA(DisplayName = "Stegosaurus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Pterodactyl     UMETA(DisplayName = "Pterodactyl"),
    Spinosaurus     UMETA(DisplayName = "Spinosaurus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus"),
    Diplodocus      UMETA(DisplayName = "Diplodocus")
};

UENUM(BlueprintType)
enum class EDinosaurBehavior : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Wandering   UMETA(DisplayName = "Wandering"),
    Foraging    UMETA(DisplayName = "Foraging"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Resting     UMETA(DisplayName = "Resting"),
    Territorial UMETA(DisplayName = "Territorial")
};

UENUM(BlueprintType)
enum class EDinosaurDiet : uint8
{
    Carnivore   UMETA(DisplayName = "Carnivore"),
    Herbivore   UMETA(DisplayName = "Herbivore"),
    Omnivore    UMETA(DisplayName = "Omnivore")
};

USTRUCT(BlueprintType)
struct FDinosaurStats
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
    float WalkSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float RunSpeed = 700.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float TerritoryRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float Hunger = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float HungerDecayRate = 2.0f;
};

/**
 * ADinosaurBase — Base class for all dinosaur species in the game.
 * Provides core stats, behavior state machine, and survival mechanics.
 * All specific dinosaur types (TRex, Raptor, etc.) inherit from this class.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurBase : public ACharacter
{
    GENERATED_BODY()

public:
    ADinosaurBase();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // --- Species & Identity ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    EDinosaurSpecies Species = EDinosaurSpecies::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    EDinosaurDiet Diet = EDinosaurDiet::Carnivore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    FName DinosaurName = NAME_None;

    // --- Stats ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    FDinosaurStats Stats;

    // --- Behavior State ---

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Behavior")
    EDinosaurBehavior CurrentBehavior = EDinosaurBehavior::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Behavior")
    bool bIsAggressive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Behavior")
    bool bIsPackHunter = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Behavior")
    AActor* CurrentTarget = nullptr;

    // --- Functions ---

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
        AController* EventInstigator, AActor* DamageCauser) override;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    virtual void Attack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    virtual void SetBehavior(EDinosaurBehavior NewBehavior);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    virtual void DetectThreats();

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Stats")
    bool IsAlive() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Stats")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Stats")
    void HealDinosaur(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    void OnDeath();

protected:
    virtual void UpdateBehaviorTick(float DeltaTime);
    virtual void UpdateHunger(float DeltaTime);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Components",
        meta = (AllowPrivateAccess = "true"))
    class USphereComponent* DetectionSphere = nullptr;

    float TimeSinceLastAttack = 0.0f;
    float AttackCooldown = 1.5f;
};
