// DinosaurBase.h
// Core Systems Programmer #03 — Transpersonal Game Studio
// Base class for all dinosaur species. Inherits ACharacter for movement + physics.
// Provides health, territory, behavior state, and AI perception registration.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AIPerceptionComponent.h"
#include "DinosaurBase.generated.h"

// ─── Behavior States ─────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EDinosaurBehavior : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Dead        UMETA(DisplayName = "Dead")
};

// ─── Species Type ─────────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EDinosaurSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Raptor          UMETA(DisplayName = "Raptor"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Pterodactyl     UMETA(DisplayName = "Pterodactyl"),
    Unknown         UMETA(DisplayName = "Unknown")
};

// ─── DinosaurBase ─────────────────────────────────────────────────────────────
UCLASS(Abstract, BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurBase : public ACharacter
{
    GENERATED_BODY()

public:
    ADinosaurBase();

    // ── Health ──────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Health")
    float MaxHealth;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|Health")
    float CurrentHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Health")
    float FleeHealthThreshold;  // % of MaxHealth below which dinosaur flees

    // ── Species & Behavior ──────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    EDinosaurSpecies Species;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|Behavior")
    EDinosaurBehavior CurrentBehavior;

    // ── Territory ────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Territory")
    FVector HomeLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Territory")
    float TerritoryRadius;

    // ── Combat ───────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AttackDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AttackCooldown;

    // ── AI Perception ────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI",
              meta = (AllowPrivateAccess = "true"))
    UAIPerceptionComponent* AIPerception;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI",
              meta = (AllowPrivateAccess = "true"))
    UAISenseConfig_Sight* SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI",
              meta = (AllowPrivateAccess = "true"))
    UAISenseConfig_Hearing* HearingConfig;

    // ── Overrides ────────────────────────────────────────────────────────────
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
                             AController* EventInstigator, AActor* DamageCauser) override;

    // ── Blueprint Events ─────────────────────────────────────────────────────
    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnDeath();
    virtual void OnDeath_Implementation();

    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnTakeDamage(float DamageAmount, AActor* DamageCauser);
    virtual void OnTakeDamage_Implementation(float DamageAmount, AActor* DamageCauser);

    // ── Utility ──────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    void SetBehaviorState(EDinosaurBehavior NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    bool IsAlive() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Territory")
    bool IsWithinTerritory() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Health")
    float GetHealthPercent() const;

protected:
    void UpdateBehaviorState(float DeltaTime);
    void RegisterAIPerception();

private:
    float LastAttackTime;
};
