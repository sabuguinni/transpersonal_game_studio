#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SharedTypes.h"
#include "DinosaurBase.generated.h"

UENUM(BlueprintType)
enum class EEng_DinosaurSpecies : uint8
{
    TRex            UMETA(DisplayName = "Tyrannosaurus Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus"),
    Pachycephalosaurus UMETA(DisplayName = "Pachycephalosaurus"),
    Unknown         UMETA(DisplayName = "Unknown")
};

UENUM(BlueprintType)
enum class EEng_DinosaurBehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Roaming     UMETA(DisplayName = "Roaming"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Resting     UMETA(DisplayName = "Resting"),
    Aggressive  UMETA(DisplayName = "Aggressive")
};

USTRUCT(BlueprintType)
struct FEng_DinosaurStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxHealth = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float CurrentHealth = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float AttackDamage = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float DetectionRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float AttackRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MoveSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float Mass = 8000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    bool bIsCarnivore = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    bool bIsPackHunter = false;
};

/**
 * ADinosaurBase — Base class for all dinosaur actors in the prehistoric survival game.
 * Inherits from ACharacter to leverage movement, collision, and animation systems.
 * All species-specific dinosaur classes must inherit from this.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurBase : public ACharacter
{
    GENERATED_BODY()

public:
    ADinosaurBase();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ---- Species & Identity ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    EEng_DinosaurSpecies Species = EEng_DinosaurSpecies::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    FName DinosaurID;

    // ---- Combat & Survival Stats ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    FEng_DinosaurStats Stats;

    // ---- Behavior State ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Behavior")
    EEng_DinosaurBehaviorState BehaviorState = EEng_DinosaurBehaviorState::Idle;

    // ---- Territory ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Territory")
    FVector TerritoryCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Territory")
    float TerritoryRadius = 5000.0f;

    // ---- Functions ----
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    virtual float TakeDinosaurDamage(float DamageAmount, AActor* DamageCauser);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    virtual void Attack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    virtual void SetBehaviorState(EEng_DinosaurBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    bool IsAlive() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Identity")
    EEng_DinosaurSpecies GetSpecies() const { return Species; }

protected:
    virtual void OnDeath();
    virtual void UpdateBehavior(float DeltaTime);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Components",
        meta = (AllowPrivateAccess = "true"))
    float BehaviorUpdateTimer = 0.0f;

    static constexpr float BehaviorUpdateInterval = 0.5f;
};
