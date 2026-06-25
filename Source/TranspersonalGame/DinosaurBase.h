// DinosaurBase.h — Base class for all dinosaur pawns
// Engine Architect #02 — Transpersonal Game Studio
// Cycle: PROD_CYCLE_AUTO_20260625_007

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DinosaurBase.generated.h"

UENUM(BlueprintType)
enum class EDinosaurSpecies : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    TRex            UMETA(DisplayName = "T-Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus"),
    Pachycephalosaurus UMETA(DisplayName = "Pachycephalosaurus"),
    Protoceratops   UMETA(DisplayName = "Protoceratops"),
    Tsintaosaurus   UMETA(DisplayName = "Tsintaosaurus"),
};

UENUM(BlueprintType)
enum class EDinosaurBehavior : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Wandering   UMETA(DisplayName = "Wandering"),
    Grazing     UMETA(DisplayName = "Grazing"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Resting     UMETA(DisplayName = "Resting"),
    Dead        UMETA(DisplayName = "Dead"),
};

UCLASS(BlueprintType, Blueprintable, ClassGroup = "TranspersonalGame")
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

    // --- Health & Survival ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxHealth;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|Stats")
    float CurrentHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxStamina;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|Stats")
    float CurrentStamina;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|Stats")
    float HungerLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|Stats")
    float ThirstLevel;

    // --- Combat ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AttackDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float DetectionRadius;

    UPROPERTY(BlueprintReadWrite, Category = "Dinosaur|Combat")
    bool bIsAggressive;

    UPROPERTY(BlueprintReadWrite, Category = "Dinosaur|Combat")
    bool bIsAlerted;

    // --- Movement ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Movement")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Movement")
    float SprintSpeed;

    // --- Identity ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    EDinosaurSpecies DinosaurSpecies;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|Behavior")
    EDinosaurBehavior BehaviorState;

    // --- Public API ---
    UFUNCTION(BlueprintCallable, Category = "Dinosaur")
    bool IsAlive() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur")
    void SetBehaviorState(EDinosaurBehavior NewState);

protected:
    void OnDeath();
    void OnBehaviorStateChanged(EDinosaurBehavior NewState);

    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur")
    void OnDeathBP();
    virtual void OnDeathBP_Implementation();
};
