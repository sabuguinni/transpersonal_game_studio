#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/BlendSpace.h"
#include "Animation/BlendSpace1D.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Anim_BlendSpaceController.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MovementData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsMoving;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsInAir;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsCrouching;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float LeanAngle;

    FAnim_MovementData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        Velocity = 0.0f;
        bIsMoving = false;
        bIsInAir = false;
        bIsCrouching = false;
        LeanAngle = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_BlendSpaceSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    float SpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    float DirectionSmoothness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    float VelocityInterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    bool bUseAcceleration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    float MaxWalkSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    float MaxRunSpeed;

    FAnim_BlendSpaceSettings()
    {
        SpeedThreshold = 10.0f;
        DirectionSmoothness = 5.0f;
        VelocityInterpSpeed = 8.0f;
        bUseAcceleration = true;
        MaxWalkSpeed = 200.0f;
        MaxRunSpeed = 600.0f;
    }
};

UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Sprinting   UMETA(DisplayName = "Sprinting"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Landing     UMETA(DisplayName = "Landing")
};

/**
 * Advanced Blend Space Controller for Prehistoric Character Movement
 * Manages smooth transitions between movement animations using blend spaces
 * Handles directional movement, speed variations, and terrain adaptation
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_BlendSpaceController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_BlendSpaceController();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core Blend Space Functions
    UFUNCTION(BlueprintCallable, Category = "Blend Space Controller")
    void UpdateMovementData(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Blend Space Controller")
    void SetBlendSpaceParameters();

    UFUNCTION(BlueprintCallable, Category = "Blend Space Controller")
    void UpdateMovementState();

    // Movement State Management
    UFUNCTION(BlueprintCallable, Category = "Movement State")
    void SetMovementState(EAnim_MovementState NewState);

    UFUNCTION(BlueprintPure, Category = "Movement State")
    EAnim_MovementState GetMovementState() const { return CurrentMovementState; }

    UFUNCTION(BlueprintCallable, Category = "Movement State")
    bool CanTransitionToState(EAnim_MovementState TargetState);

    // Blend Space Asset Management
    UFUNCTION(BlueprintCallable, Category = "Blend Space Assets")
    void SetLocomotionBlendSpace(UBlendSpace* NewBlendSpace);

    UFUNCTION(BlueprintCallable, Category = "Blend Space Assets")
    void SetCrouchBlendSpace(UBlendSpace* NewBlendSpace);

    UFUNCTION(BlueprintCallable, Category = "Blend Space Assets")
    void SetSprintBlendSpace(UBlendSpace1D* NewBlendSpace);

    // Getters for Animation Blueprint
    UFUNCTION(BlueprintPure, Category = "Movement Data")
    FAnim_MovementData GetMovementData() const { return MovementData; }

    UFUNCTION(BlueprintPure, Category = "Movement Data")
    float GetSpeed() const { return MovementData.Speed; }

    UFUNCTION(BlueprintPure, Category = "Movement Data")
    float GetDirection() const { return MovementData.Direction; }

    UFUNCTION(BlueprintPure, Category = "Movement Data")
    float getVelocity() const { return MovementData.Velocity; }

    UFUNCTION(BlueprintPure, Category = "Movement Data")
    bool IsMoving() const { return MovementData.bIsMoving; }

    UFUNCTION(BlueprintPure, Category = "Movement Data")
    bool IsInAir() const { return MovementData.bIsInAir; }

    UFUNCTION(BlueprintPure, Category = "Movement Data")
    bool IsCrouching() const { return MovementData.bIsCrouching; }

    // Settings
    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetBlendSpaceSettings(const FAnim_BlendSpaceSettings& NewSettings);

    UFUNCTION(BlueprintPure, Category = "Settings")
    FAnim_BlendSpaceSettings GetBlendSpaceSettings() const { return BlendSpaceSettings; }

protected:
    // Movement Data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement Data")
    FAnim_MovementData MovementData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement Data")
    EAnim_MovementState CurrentMovementState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement Data")
    EAnim_MovementState PreviousMovementState;

    // Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FAnim_BlendSpaceSettings BlendSpaceSettings;

    // Blend Space Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space Assets")
    UBlendSpace* LocomotionBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space Assets")
    UBlendSpace* CrouchBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space Assets")
    UBlendSpace1D* SprintBlendSpace;

    // Component References
    UPROPERTY()
    UCharacterMovementComponent* MovementComponent;

    UPROPERTY()
    UAnimInstance* AnimInstance;

    // Internal State
    FVector LastVelocity;
    float LastSpeed;
    float LastDirection;
    float StateTransitionTime;

private:
    // Helper Functions
    void InitializeComponent();
    void CacheComponentReferences();
    float CalculateDirection(const FVector& Velocity, const FRotator& ActorRotation);
    float CalculateSpeed(const FVector& Velocity);
    void SmoothMovementData(float DeltaTime);
    bool ShouldUpdateMovementState();
    void HandleStateTransition(EAnim_MovementState NewState);
    float GetSpeedRatio() const;
};