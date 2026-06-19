// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Core/Physics/Core_RagdollSystem.h"
#include "Core/Physics/Core_PhysicsSystemManager.h"
#include "Core/Survival/SurvivalComponent.h"
#include "TranspersonalCharacter.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTranspersonalCharacter, Log, All);

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UCore_RagdollSystem;
class UCore_PhysicsSystemManager;

/**
 * ATranspersonalCharacter — Prehistoric Human Survivor (Player Character)
 *
 * The player is a primitive human struggling to survive in a dangerous
 * prehistoric world filled with dinosaurs. This character integrates:
 * - SurvivalComponent: health, hunger, thirst, stamina, fear (delegated)
 * - Physics: ragdoll, collision, impact response
 * - Input: WASD movement, camera, jump, sprint, sneak, interact, craft
 *
 * Design: Player is always prey, never predator. Survival is fragile.
 */
UCLASS(config=Game)
class TRANSPERSONALGAME_API ATranspersonalCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ATranspersonalCharacter();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // ── Camera ──────────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    UCameraComponent* FollowCamera;

    // ── Survival Component (delegates health/hunger/thirst/stamina/fear) ────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Survival, meta = (AllowPrivateAccess = "true"))
    USurvivalComponent* SurvivalComp;

    // ── Physics Components ───────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Physics, meta = (AllowPrivateAccess = "true"))
    UCore_RagdollSystem* RagdollComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Physics, meta = (AllowPrivateAccess = "true"))
    UCore_PhysicsSystemManager* PhysicsManager;

    // ── Input ────────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* JumpAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* LookAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* SneakAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* RunAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* InteractAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* CraftAction;

public:
    // ── Movement States ──────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
    bool bIsHiding;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
    bool bIsSneaking;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
    bool bIsRunning;

    // ── Physics States ───────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Physics)
    bool bIsRagdolled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Physics)
    bool bPhysicsEnabled;

    // ── Crafting ─────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Crafting)
    int32 CraftingLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Crafting)
    float CraftingExperience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Crafting)
    TArray<FString> KnownRecipes;

    // ── Survival Delegates (thin wrappers — real logic in SurvivalComp) ──────
    UFUNCTION(BlueprintCallable, Category = Survival)
    void ConsumeStamina(float Amount);

    UFUNCTION(BlueprintCallable, Category = Survival)
    void IncreaseFear(float Amount);

    UFUNCTION(BlueprintCallable, Category = Survival)
    void RestoreHealth(float Amount);

    UFUNCTION(BlueprintCallable, Category = Survival)
    void RestoreStamina(float Amount);

    UFUNCTION(BlueprintCallable, Category = Survival)
    void RestoreHunger(float Amount);

    UFUNCTION(BlueprintCallable, Category = Survival)
    void RestoreThirst(float Amount);

    // ── Physics Functions ────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = Physics)
    void EnableRagdoll();

    UFUNCTION(BlueprintCallable, Category = Physics)
    void DisableRagdoll();

    UFUNCTION(BlueprintCallable, Category = Physics)
    void ApplyImpact(const FVector& ImpactForce, const FVector& ImpactLocation);

    UFUNCTION(BlueprintCallable, Category = Physics)
    void SetPhysicsEnabled(bool bEnabled);

    // ── Survival Getters (delegate to SurvivalComp) ──────────────────────────
    UFUNCTION(BlueprintPure, Category = Survival)
    float GetHealthPercentage() const;

    UFUNCTION(BlueprintPure, Category = Survival)
    float GetStaminaPercentage() const;

    UFUNCTION(BlueprintPure, Category = Survival)
    float GetHungerPercentage() const;

    UFUNCTION(BlueprintPure, Category = Survival)
    float GetThirstPercentage() const;

    UFUNCTION(BlueprintPure, Category = Survival)
    float GetFearPercentage() const;

    // ── Physics Getters ───────────────────────────────────────────────────────
    UFUNCTION(BlueprintPure, Category = Physics)
    bool IsRagdolled() const { return bIsRagdolled; }

    UFUNCTION(BlueprintPure, Category = Physics)
    UCore_RagdollSystem* GetRagdollComponent() const { return RagdollComponent; }

    UFUNCTION(BlueprintPure, Category = Physics)
    UCore_PhysicsSystemManager* GetPhysicsManager() const { return PhysicsManager; }

    // ── SurvivalComp Accessor ─────────────────────────────────────────────────
    UFUNCTION(BlueprintPure, Category = Survival)
    USurvivalComponent* GetSurvivalComponent() const { return SurvivalComp; }

    // ── Camera Accessors ──────────────────────────────────────────────────────
    FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
    FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

protected:
    // Input handlers
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void StartSneaking();
    void StopSneaking();
    void StartRunning();
    void StopRunning();
    void Interact();
    void OpenCraftingMenu();

    // Internal update helpers
    void UpdateMovementSpeed();
    void UpdatePhysicsState(float DeltaTime);
    void HandleCollisionEvents();
    void ProcessRagdollTransition();
};
