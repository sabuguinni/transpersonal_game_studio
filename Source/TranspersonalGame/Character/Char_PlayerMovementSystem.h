#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "SharedTypes.h"
#include "Char_PlayerMovementSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_MovementConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float WalkSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float RunSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float CrouchSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float JumpVelocity = 420.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float StaminaDrainRate = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float StaminaRegenRate = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MinStaminaToRun = 20.0f;

    FChar_MovementConfig()
    {
        WalkSpeed = 300.0f;
        RunSpeed = 600.0f;
        CrouchSpeed = 150.0f;
        JumpVelocity = 420.0f;
        StaminaDrainRate = 10.0f;
        StaminaRegenRate = 15.0f;
        MinStaminaToRun = 20.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_CameraConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float SpringArmLength = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    FRotator SpringArmRotation = FRotator(-20.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float MouseSensitivity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    bool bUsePawnControlRotation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    bool bInheritPitch = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    bool bInheritYaw = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    bool bInheritRoll = false;

    FChar_CameraConfig()
    {
        SpringArmLength = 400.0f;
        SpringArmRotation = FRotator(-20.0f, 0.0f, 0.0f);
        MouseSensitivity = 1.0f;
        bUsePawnControlRotation = true;
        bInheritPitch = false;
        bInheritYaw = false;
        bInheritRoll = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_PlayerMovementSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_PlayerMovementSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Movement configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    FChar_MovementConfig MovementConfig;

    // Camera configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    FChar_CameraConfig CameraConfig;

    // Current movement state
    UPROPERTY(BlueprintReadOnly, Category = "Movement State")
    bool bIsRunning = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement State")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement State")
    float CurrentStamina = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement State")
    float MaxStamina = 100.0f;

    // Movement functions
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void SetupPlayerMovement(ACharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void SetupPlayerCamera(ACharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StartRunning();

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StopRunning();

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StartCrouching();

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StopCrouching();

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void HandleJump();

    UFUNCTION(BlueprintCallable, Category = "Movement")
    bool CanRun() const;

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void UpdateStamina(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void ApplyMovementSpeed();

    // Input handling
    UFUNCTION(BlueprintCallable, Category = "Input")
    void SetupInputBindings(class UInputComponent* PlayerInputComponent);

    // Camera functions
    UFUNCTION(BlueprintCallable, Category = "Camera")
    void LookUp(float Value);

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void Turn(float Value);

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void AdjustCameraDistance(float Delta);

private:
    // Cached references
    UPROPERTY()
    ACharacter* OwnerCharacter = nullptr;

    UPROPERTY()
    UCharacterMovementComponent* MovementComponent = nullptr;

    UPROPERTY()
    USpringArmComponent* SpringArmComponent = nullptr;

    UPROPERTY()
    UCameraComponent* CameraComponent = nullptr;

    // Internal state
    bool bWantsToRun = false;
    float LastStaminaUpdate = 0.0f;

    // Helper functions
    void InitializeComponents();
    void ValidateConfiguration();
    void UpdateMovementState(float DeltaTime);
};