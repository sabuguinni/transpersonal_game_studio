// DinosaurBase.h
// Engine Architect #02 — Cycle PROD_CYCLE_AUTO_20260620_007
// Base class for all dinosaur pawns. Inherits ACharacter for movement + capsule.
// All species (TRex, Raptor, Brachiosaurus, etc.) subclass this.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DinosaurBase.generated.h"

// Dinosaur AI state machine — global scope, unique prefix Eng_
UENUM(BlueprintType)
enum class EDinoState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Roaming     UMETA(DisplayName = "Roaming"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Dead        UMETA(DisplayName = "Dead")
};

// Delegate broadcast on dinosaur death (for quests, loot, ecology)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDinosaurDied, ADinosaurBase*, DeadDino);

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurBase : public ACharacter
{
    GENERATED_BODY()

public:
    ADinosaurBase();

    // ── Survival Stats ──────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxHealth;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Stats")
    float CurrentHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MoveSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float AttackDamage;

    // ── Detection & Combat ──────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float DetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AttackRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Behavior")
    bool bIsAggressive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Behavior")
    bool bIsPack;

    // ── Identity ────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    FString SpeciesName;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|State")
    EDinoState DinoState;

    // ── Events ──────────────────────────────────────────────────────
    UPROPERTY(BlueprintAssignable, Category = "Dinosaur|Events")
    FOnDinosaurDied OnDinosaurDied;

    // ── Interface ───────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Dinosaur")
    bool IsAlive() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur")
    void SetDinoState(EDinoState NewState);

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
        AController* EventInstigator, AActor* DamageCauser) override;

    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur")
    void OnDeath();
    virtual void OnDeath_Implementation();

private:
    // Redirect BlueprintNativeEvent to implementation
    void OnDeath() { OnDeath_Implementation(); }
};
