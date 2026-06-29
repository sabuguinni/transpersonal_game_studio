// DinosaurBase.h
// Engine Architect #02 — Cycle AUTO_20260629_004
// Base class for all dinosaur pawns. Inherits ACharacter for movement + capsule.
// All species (TRex, Raptor, Brachiosaurus, etc.) derive from this.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "DinosaurBase.generated.h"

// Forward declarations
class UAIPerceptionStimuliSourceComponent;
class UBehaviorTree;

// ============================================================
// ENUMS — must be at global scope (UHT rule)
// ============================================================

UENUM(BlueprintType)
enum class EDinosaurBehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Roaming     UMETA(DisplayName = "Roaming"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class EDinosaurSizeClass : uint8
{
    Small       UMETA(DisplayName = "Small"),    // Compies, small raptors
    Medium      UMETA(DisplayName = "Medium"),   // Raptors, Dilophosaurus
    Large       UMETA(DisplayName = "Large"),    // Allosaurus, Carnotaurus
    Massive     UMETA(DisplayName = "Massive")   // T-Rex, Brachiosaurus, Spinosaurus
};

// ============================================================
// DELEGATE
// ============================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDinosaurDied, ADinosaurBase*, Dinosaur);

// ============================================================
// MAIN CLASS
// ============================================================

UCLASS(BlueprintType, Blueprintable, ClassGroup = (Dinosaurs))
class TRANSPERSONALGAME_API ADinosaurBase : public ACharacter
{
    GENERATED_BODY()

public:
    ADinosaurBase();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
        AController* EventInstigator, AActor* DamageCauser) override;

    // --------------------------------------------------------
    // SPECIES IDENTITY
    // --------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    FName SpeciesName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    EDinosaurSizeClass SizeClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    bool bIsCarnivore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    bool bIsPackHunter;

    // --------------------------------------------------------
    // HEALTH & COMBAT
    // --------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat", Replicated)
    float MaxHealth;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Combat", Replicated)
    float CurrentHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AttackDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AttackRadius;

    // --------------------------------------------------------
    // AI / BEHAVIOR
    // --------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|AI")
    float DetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|AI")
    float TerritoryRadius;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI", Replicated)
    EDinosaurBehaviorState CurrentBehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|AI")
    UBehaviorTree* BehaviorTree;

    // --------------------------------------------------------
    // COMPONENTS
    // --------------------------------------------------------

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Components",
        meta = (AllowPrivateAccess = "true"))
    UAIPerceptionStimuliSourceComponent* PerceptionStimuliSource;

    // --------------------------------------------------------
    // DELEGATES
    // --------------------------------------------------------

    UPROPERTY(BlueprintAssignable, Category = "Dinosaur|Events")
    FOnDinosaurDied OnDinosaurDied;

    // --------------------------------------------------------
    // PUBLIC API
    // --------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    void SetBehaviorState(EDinosaurBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    bool IsAlive() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    float GetHealthPercent() const;

protected:
    // Called when behavior state changes — override per species
    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Behavior")
    void OnBehaviorStateChanged(EDinosaurBehaviorState NewState);
    virtual void OnBehaviorStateChanged_Implementation(EDinosaurBehaviorState NewState);

    // Periodic behavior tick (2s interval)
    UFUNCTION()
    void UpdateBehaviorState();

    // Death handler
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void Die();

private:
    FTimerHandle BehaviorUpdateTimer;
};
