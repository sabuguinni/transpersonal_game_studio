#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Core_CharacterController.generated.h"

TRANSPERSONALGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogCharacterController, Log, All);

/**
 * Core character controller system for WALK AROUND milestone
 * Implements third-person camera setup, WASD movement, running, jumping
 * Provides the foundation for the minimum viable playable prototype
 */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_CharacterController : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_CharacterController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === WALK AROUND MILESTONE REQUIREMENTS ===
    
    /** Setup third-person camera with boom and follow camera */
    UFUNCTION(BlueprintCallable, Category = "Character Controller")
    void SetupThirdPersonCamera(ACharacter* Character);

    /** Configure WASD movement input */
    UFUNCTION(BlueprintCallable, Category = "Character Controller")
    void SetupMovementInput(ACharacter* Character);

    /** Enable running mechanics */
    UFUNCTION(BlueprintCallable, Category = "Character Controller")
    void EnableRunning(ACharacter* Character);

    /** Enable jumping mechanics */
    UFUNCTION(BlueprintCallable, Category = "Character Controller")
    void EnableJumping(ACharacter* Character);

    /** Validate character controller setup for milestone */
    UFUNCTION(BlueprintCallable, Category = "Character Controller")
    bool ValidateControllerSetup(ACharacter* Character);

    // === CAMERA SYSTEM ===

    /** Camera boom component reference */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    class USpringArmComponent* CameraBoom;

    /** Follow camera component reference */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    class UCameraComponent* FollowCamera;

    /** Camera boom arm length */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (ClampMin = "100.0", ClampMax = "1000.0"))
    float CameraBoomLength = 300.0f;

    /** Camera boom socket offset */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    FVector CameraBoomOffset = FVector(0.0f, 0.0f, 60.0f);

    /** Use pawn control rotation for camera */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    bool bUsePawnControlRotation = true;

    // === MOVEMENT SYSTEM ===

    /** Base walking speed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "100.0", ClampMax = "1000.0"))
    float BaseWalkSpeed = 300.0f;

    /** Running speed multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "1.0", ClampMax = "5.0"))
    float RunSpeedMultiplier = 2.0f;

    /** Jump velocity */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "200.0", ClampMax = "1000.0"))
    float JumpVelocity = 600.0f;

    /** Air control factor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AirControl = 0.35f;

    /** Ground friction */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float GroundFriction = 8.0f;

    /** Braking deceleration walking */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "100.0", ClampMax = "5000.0"))
    float BrakingDecelerationWalking = 2048.0f;

    // === INPUT SYSTEM ===

    /** Is running currently active */
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsRunning = false;

    /** Movement input vector */
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FVector2D MovementInput = FVector2D::ZeroVector;

    /** Look input vector */
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FVector2D LookInput = FVector2D::ZeroVector;

    // === VALIDATION SYSTEM ===

    /** Controller validation status */
    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bControllerValid = false;

    /** Camera setup validation */
    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bCameraSetupValid = false;

    /** Movement setup validation */
    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bMovementSetupValid = false;

    /** Input setup validation */
    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bInputSetupValid = false;

    // === MILESTONE TRACKING ===

    /** Get milestone completion percentage */
    UFUNCTION(BlueprintCallable, Category = "Milestone")
    float GetMilestoneCompletion() const;

    /** Get detailed milestone status */
    UFUNCTION(BlueprintCallable, Category = "Milestone")
    FString GetMilestoneStatus() const;

    // === UTILITY FUNCTIONS ===

    /** Log controller status */
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogControllerStatus() const;

    /** Reset controller to default state */
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ResetController();

private:
    /** Internal validation helper */
    void ValidateInternalState();

    /** Setup default movement parameters */
    void SetupDefaultMovementParameters(UCharacterMovementComponent* MovementComp);

    /** Validate camera components */
    bool ValidateCameraComponents(ACharacter* Character) const;

    /** Validate movement components */
    bool ValidateMovementComponents(ACharacter* Character) const;
};