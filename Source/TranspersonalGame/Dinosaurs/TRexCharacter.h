// TRexCharacter.h
// Core Systems Programmer #03 — PROD_CYCLE_AUTO_20260624_001
// Tyrannosaurus Rex — apex predator subclass of ADinosaurBase.
// Sets species stats in BeginPlay; exposes Blueprint-overridable roar event.

#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "TRexCharacter.generated.h"

/**
 * ATRexCharacter
 * Concrete Tyrannosaurus Rex actor.
 * Inherits all survival stats, behavior state machine, and AI perception
 * from ADinosaurBase. Overrides BeginPlay to lock species to TyrannosaurusRex
 * so ApplySpeciesStats() sets correct values (health=1500, speed=600, aggression=0.9).
 *
 * Blueprint subclass: BP_TRex (recommended for mesh/animation assignment).
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = "Dinosaurs",
       meta = (DisplayName = "T-Rex Character"))
class TRANSPERSONALGAME_API ATRexCharacter : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ATRexCharacter();

protected:
    virtual void BeginPlay() override;

public:
    // ── Roar ──────────────────────────────────────────────────────────────────
    /** Trigger the T-Rex roar — plays sound, applies fear radius to nearby pawns. */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "TRex|Behavior")
    void PerformRoar();
    virtual void PerformRoar_Implementation();

    // ── Fear radius ───────────────────────────────────────────────────────────
    /** Radius (cm) in which the roar applies fear to other actors. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Behavior",
              meta = (ClampMin = "100.0", ClampMax = "5000.0"))
    float RoarFearRadius = 2000.0f;

    /** Fear intensity applied to pawns caught in the roar radius (0-1). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Behavior",
              meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float RoarFearIntensity = 0.85f;

    // ── Stomp ─────────────────────────────────────────────────────────────────
    /** Stomp attack — deals area damage and applies knockback. */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "TRex|Combat")
    void PerformStomp();
    virtual void PerformStomp_Implementation();

    /** Stomp damage radius (cm). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat",
              meta = (ClampMin = "50.0", ClampMax = "1000.0"))
    float StompRadius = 400.0f;

    /** Stomp base damage. Multiplied by AttackDamage from base class. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat",
              meta = (ClampMin = "0.0", ClampMax = "500.0"))
    float StompDamageMultiplier = 1.5f;

    // ── Bite ──────────────────────────────────────────────────────────────────
    /** Bite attack — single-target high damage. */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "TRex|Combat")
    void PerformBite();
    virtual void PerformBite_Implementation();

    /** Bite range (cm). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat",
              meta = (ClampMin = "50.0", ClampMax = "500.0"))
    float BiteRange = 250.0f;

    // ── Territory ─────────────────────────────────────────────────────────────
    /** Whether this T-Rex is currently defending a territory. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Territory")
    bool bIsDefendingTerritory = false;

    /** Territory center (world space). Set automatically on BeginPlay from spawn location. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TRex|Territory")
    FVector TerritoryCenter = FVector::ZeroVector;

    /** Territory radius (cm). T-Rex will return to this area when chasing prey too far. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Territory",
              meta = (ClampMin = "500.0", ClampMax = "50000.0"))
    float TerritoryRadius = 15000.0f;
};
