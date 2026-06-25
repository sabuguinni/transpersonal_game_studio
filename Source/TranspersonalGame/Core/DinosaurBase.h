// DinosaurBase.h — Base class for all dinosaur pawns
// Engine Architect #02 — PROD_CYCLE_AUTO_20260625_002
// All dinosaur species inherit from this class.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "DinosaurBase.generated.h"

// ============================================================
// ENUMS — must be at global scope (UE5 rule)
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

UENUM(BlueprintType)
enum class EDinoBehaviourState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Wandering   UMETA(DisplayName = "Wandering"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Resting     UMETA(DisplayName = "Resting"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Dead        UMETA(DisplayName = "Dead"),
};

// ============================================================
// ADinosaurBase
// ============================================================

UCLASS(Abstract, BlueprintType, Blueprintable, ClassGroup = (Dinosaurs))
class TRANSPERSONALGAME_API ADinosaurBase : public ACharacter
{
    GENERATED_BODY()

public:
    ADinosaurBase();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
        AController* EventInstigator, AActor* DamageCauser) override;

    // ---- Survival Stats ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float Health;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float Hunger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxHunger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float Stamina;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxStamina;

    // ---- Species & Behaviour ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    EDinoSpecies DinoSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Behaviour")
    EDinoBehaviourState CurrentBehaviourState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Behaviour")
    float AggressionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Behaviour")
    float TerritoryRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Behaviour")
    bool bIsAggressive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Behaviour")
    bool bIsPredator;

    // ---- Movement ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Movement")
    float WalkSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Movement")
    float RunSpeed;

    // ---- Internal ----
    FTimerHandle MetabolismTimerHandle;

    void TickMetabolism();
    void UpdateBehaviourState(float DeltaTime);
    void OnDeath();

    // Blueprint events
    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnDeathBP();
    virtual void OnDeathBP_Implementation();

    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnBehaviourStateChanged(EDinoBehaviourState NewState);
    virtual void OnBehaviourStateChanged_Implementation(EDinoBehaviourState NewState);

public:
    // ---- Public API ----
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behaviour")
    void SetBehaviourState(EDinoBehaviourState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behaviour")
    bool IsPlayerInAggressionRadius() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Stats")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Stats")
    float GetHungerPercent() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Stats")
    float GetStaminaPercent() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Identity")
    EDinoSpecies GetDinoSpecies() const { return DinoSpecies; }

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Behaviour")
    EDinoBehaviourState GetBehaviourState() const { return CurrentBehaviourState; }
};
