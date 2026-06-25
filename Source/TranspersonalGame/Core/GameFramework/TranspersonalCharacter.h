// TranspersonalCharacter.h
// Core Systems Programmer #03 — Cycle PROD_CYCLE_AUTO_20260625_004
// Prehistoric survival game — ACharacter subclass with SurvivalComponent integration

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Core/Survival/SurvivalComponent.h"
#include "TranspersonalCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

/**
 * ATranspersonalCharacter
 * Player-controlled primitive human character for the prehistoric survival game.
 * Extends ACharacter for built-in movement, gravity, and CharacterMovementComponent.
 * Integrates USurvivalComponent for hunger/thirst/stamina/fear/health tracking.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ATranspersonalCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ATranspersonalCharacter();

protected:
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
    virtual void Tick(float DeltaTime) override;

    // ─── Survival Component ───────────────────────────────────────────────────

    /** Survival stats: hunger, thirst, stamina, fear, health */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival",
              meta = (AllowPrivateAccess = "true"))
    USurvivalComponent* SurvivalComp;

    // ─── Camera ───────────────────────────────────────────────────────────────

    /** Spring arm for third-person camera distance */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera",
              meta = (AllowPrivateAccess = "true"))
    USpringArmComponent* CameraBoom;

    /** Follow camera */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera",
              meta = (AllowPrivateAccess = "true"))
    UCameraComponent* FollowCamera;

    // ─── Enhanced Input ───────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input",
              meta = (AllowPrivateAccess = "true"))
    UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input",
              meta = (AllowPrivateAccess = "true"))
    UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input",
              meta = (AllowPrivateAccess = "true"))
    UInputAction* LookAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input",
              meta = (AllowPrivateAccess = "true"))
    UInputAction* JumpAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input",
              meta = (AllowPrivateAccess = "true"))
    UInputAction* SprintAction;

    // ─── Movement ─────────────────────────────────────────────────────────────

    /** Walk speed (cm/s) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float WalkSpeed = 400.f;

    /** Sprint speed (cm/s) — costs stamina */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float SprintSpeed = 750.f;

    /** Whether the player is currently sprinting */
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsSprinting = false;

    // ─── Combat / Damage ──────────────────────────────────────────────────────

    /** Base melee attack damage with primitive weapons */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float MeleeAttackDamage = 15.f;

    /** Radius for melee hit detection (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float MeleeAttackRadius = 120.f;

    // ─── Input Handlers ───────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Input")
    void Move(const FInputActionValue& Value);

    UFUNCTION(BlueprintCallable, Category = "Input")
    void Look(const FInputActionValue& Value);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StartSprint();

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StopSprint();

    // ─── Survival Accessors ───────────────────────────────────────────────────

    /** Returns current health (0-100) */
    UFUNCTION(BlueprintPure, Category = "Survival")
    float GetHealth() const;

    /** Returns current hunger (0-100; 0 = starving) */
    UFUNCTION(BlueprintPure, Category = "Survival")
    float GetHunger() const;

    /** Returns current thirst (0-100; 0 = dehydrated) */
    UFUNCTION(BlueprintPure, Category = "Survival")
    float GetThirst() const;

    /** Returns current stamina (0-100) */
    UFUNCTION(BlueprintPure, Category = "Survival")
    float GetStamina() const;

    /** Returns current fear level (0-100; 100 = panic) */
    UFUNCTION(BlueprintPure, Category = "Survival")
    float GetFear() const;

    /** Apply damage — routes through SurvivalComp health */
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
                             class AController* EventInstigator, AActor* DamageCauser) override;

private:
    /** Handle death when health reaches zero */
    void Die();

    bool bIsDead = false;
};
