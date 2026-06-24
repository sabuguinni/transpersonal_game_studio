// DinosaurBase.h
// Engine Architect #02 — Transpersonal Game Studio
// Base class for ALL dinosaur pawns in the game.
// Subclass this for TRex, Raptor, Triceratops, etc.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DinosaurBase.generated.h"

// ============================================================
// ENUM — Dinosaur species identifier (global scope, Eng_ prefix)
// ============================================================
UENUM(BlueprintType)
enum class EDinoSpecies : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    TyrannosaurusRex UMETA(DisplayName = "Tyrannosaurus Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Pachycephalosaurus UMETA(DisplayName = "Pachycephalosaurus"),
    Protoceratops   UMETA(DisplayName = "Protoceratops"),
    Tsintaosaurus   UMETA(DisplayName = "Tsintaosaurus"),
};

// ============================================================
// ADinosaurBase — ACharacter subclass
// ============================================================
UCLASS(Abstract, BlueprintType, Blueprintable, ClassGroup=(Dinosaurs),
       meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ADinosaurBase : public ACharacter
{
    GENERATED_BODY()

public:
    ADinosaurBase();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
                             AController* EventInstigator, AActor* DamageCauser) override;

    // --------------------------------------------------------
    // SURVIVAL STATS
    // --------------------------------------------------------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxHealth;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Stats")
    float CurrentHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxStamina;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Stats")
    float CurrentStamina;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxHunger;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Stats")
    float CurrentHunger;

    // --------------------------------------------------------
    // MOVEMENT
    // --------------------------------------------------------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Movement")
    float WalkSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Movement")
    float RunSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Movement")
    float TurnRate;

    // --------------------------------------------------------
    // BEHAVIOUR FLAGS
    // --------------------------------------------------------
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    bool bIsAggressive;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    bool bIsFleeing;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    bool bIsDead;

    // --------------------------------------------------------
    // TERRITORY & DETECTION
    // --------------------------------------------------------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|AI")
    float TerritoryRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|AI")
    float DetectionRadius;

    // --------------------------------------------------------
    // SPECIES
    // --------------------------------------------------------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    EDinoSpecies Species;

    // --------------------------------------------------------
    // PUBLIC API
    // --------------------------------------------------------
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void SetAggressive(bool bAggressive);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void SetFleeing(bool bFleeing);

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Stats")
    bool IsAlive() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Stats")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Stats")
    float GetStaminaPercent() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Stats")
    float GetHungerPercent() const;

    // --------------------------------------------------------
    // EVENTS (override in Blueprint or subclass)
    // --------------------------------------------------------
    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnDinosaurHurt(float DamageAmount, AActor* DamageCauser);

    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnDinosaurDied();

protected:
    void Die();

private:
    FTimerHandle HungerTimerHandle;
    FTimerHandle StaminaTimerHandle;

    void DrainHunger();
    void RecoverStamina();
};
