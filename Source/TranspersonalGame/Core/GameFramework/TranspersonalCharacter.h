// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Core/Physics/Core_RagdollSystem.h"
#include "Core/Physics/Core_PhysicsSystemManager.h"
#include "TranspersonalCharacter.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTranspersonalCharacter, Log, All);

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UCore_RagdollSystem;
class UCore_PhysicsSystemManager;

/**
 * Transpersonal Character - The Paleontologist Player Character
 * 
 * Represents the paleontologist who was transported back in time.
 * Features:
 * - Survival mechanics (health, stamina, hunger, thirst)
 * - Fear system (dynamic fear based on nearby threats)
 * - Physics integration (ragdoll, collision, destruction)
 * - Crafting system (tools and shelter building)
 * - Stealth mechanics (sneaking, hiding)
 * 
 * Design Philosophy:
 * - Player is always the prey, never the predator
 * - Survival is fragile and requires constant attention
 * - Fear should be a constant companion
 * - Realistic physics simulation for immersion
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

    // Camera Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    UCameraComponent* FollowCamera;
    
    // Physics Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Physics, meta = (AllowPrivateAccess = "true"))
    UCore_RagdollSystem* RagdollComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Physics, meta = (AllowPrivateAccess = "true"))
    UCore_PhysicsSystemManager* PhysicsManager;
    
    // Input Mapping Context
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputMappingContext* DefaultMappingContext;

    // Input Actions
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
    // Survival Stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Health;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float MaxHealth;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Stamina;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float MaxStamina;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Hunger;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float MaxHunger;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Thirst;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float MaxThirst;
    
    // Fear System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fear")
    float FearLevel;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fear")
    float MaxFearLevel;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fear")
    bool bNearbyThreat;
    
    // Movement States
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsHiding;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsSneaking;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsRunning;
    
    // Physics States
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bIsRagdolled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bPhysicsEnabled;
    
    // Crafting System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    int32 CraftingLevel;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    float CraftingExperience;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FString> KnownRecipes;

    // Survival Functions
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ConsumeStamina(float Amount);
    
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void IncreaseFear(float Amount);
    
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void RestoreHealth(float Amount);
    
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void RestoreStamina(float Amount);
    
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void RestoreHunger(float Amount);
    
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void RestoreThirst(float Amount);

    // Physics Functions
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void EnableRagdoll();

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void DisableRagdoll();

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void ApplyImpact(const FVector& ImpactForce, const FVector& ImpactLocation);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void SetPhysicsEnabled(bool bEnabled);

    // Getters for UI
    UFUNCTION(BlueprintPure, Category = "Survival")
    float GetHealthPercentage() const { return Health / MaxHealth; }
    
    UFUNCTION(BlueprintPure, Category = "Survival")
    float GetStaminaPercentage() const { return Stamina / MaxStamina; }
    
    UFUNCTION(BlueprintPure, Category = "Survival")
    float GetHungerPercentage() const { return Hunger / MaxHunger; }
    
    UFUNCTION(BlueprintPure, Category = "Survival")
    float GetThirstPercentage() const { return Thirst / MaxThirst; }
    
    UFUNCTION(BlueprintPure, Category = "Fear")
    float GetFearPercentage() const { return FearLevel / MaxFearLevel; }

    // Physics Getters
    UFUNCTION(BlueprintPure, Category = "Physics")
    bool IsRagdolled() const { return bIsRagdolled; }

    UFUNCTION(BlueprintPure, Category = "Physics")
    UCore_RagdollSystem* GetRagdollComponent() const { return RagdollComponent; }

    UFUNCTION(BlueprintPure, Category = "Physics")
    UCore_PhysicsSystemManager* GetPhysicsManager() const { return PhysicsManager; }

protected:
    // Input Functions
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void StartSneaking();
    void StopSneaking();
    void StartRunning();
    void StopRunning();
    void Interact();
    void OpenCraftingMenu();
    
    // Survival Mechanics
    void UpdateSurvivalStats(float DeltaTime);
    void UpdateFearSystem(float DeltaTime);
    void CheckForThreats();

    // Physics Mechanics
    void UpdatePhysicsState(float DeltaTime);
    void HandleCollisionEvents();
    void ProcessRagdollTransition();

public:
    // Accessors
    FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
    FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};