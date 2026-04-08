#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "TranspersonalPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UPhysicsSystemManager;
class URagdollSystemComponent;

/**
 * Transpersonal Player Controller
 * Handles input for survival gameplay in prehistoric world
 * Manages physics interactions and environmental controls
 */
UCLASS()
class TRANSPERSONALGAME_API ATranspersonalPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    ATranspersonalPlayerController();

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;
    virtual void Tick(float DeltaTime) override;

    /** Input Mapping Context */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputMappingContext> DefaultMappingContext;

    /** Movement Input Action */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> MoveAction;

    /** Look Input Action */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> LookAction;

    /** Jump Input Action */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> JumpAction;

    /** Interact Input Action */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> InteractAction;

    /** Crouch Input Action */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> CrouchAction;

    /** Run Input Action */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> RunAction;

    /** Physics Debug Input Action */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> PhysicsDebugAction;

    /** Input handlers */
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void Jump();
    void Interact();
    void StartCrouch();
    void StopCrouch();
    void StartRun();
    void StopRun();
    void TogglePhysicsDebug();

public:
    /** Physics interaction functions */
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void ApplyPhysicsImpulse(AActor* Target, const FVector& Impulse, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void ActivateRagdoll(AActor* Target, const FVector& ImpulseLocation = FVector::ZeroVector, float ImpulseStrength = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void DeactivateRagdoll(AActor* Target);

    /** Environmental interaction */
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void InteractWithEnvironment();

    UFUNCTION(BlueprintCallable, Category = "Environment")
    AActor* GetInteractableActor() const;

    /** Survival mechanics */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void StartCrafting();

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void StartBuilding();

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void StartHunting();

    /** Physics debug visualization */
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ShowPhysicsDebugInfo(bool bShow);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ShowCollisionDebug(bool bShow);

protected:
    /** Interaction range for environmental objects */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float InteractionRange = 300.0f;

    /** Physics debug enabled state */
    UPROPERTY(BlueprintReadOnly, Category = "Debug")
    bool bPhysicsDebugEnabled = false;

    /** Collision debug enabled state */
    UPROPERTY(BlueprintReadOnly, Category = "Debug")
    bool bCollisionDebugEnabled = false;

    /** Reference to physics system manager */
    UPROPERTY()
    TObjectPtr<UPhysicsSystemManager> PhysicsManager;

    /** Current interactable actor */
    UPROPERTY(BlueprintReadOnly, Category = "Interaction")
    TObjectPtr<AActor> CurrentInteractable;

private:
    /** Update interaction detection */
    void UpdateInteractionDetection();

    /** Perform line trace for interaction */
    AActor* TraceForInteractable() const;

    /** Handle physics interaction with traced object */
    void HandlePhysicsInteraction(AActor* HitActor, const FVector& HitLocation);

    /** Timer for interaction updates */
    FTimerHandle InteractionUpdateTimer;
};