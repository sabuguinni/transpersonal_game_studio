#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "../SharedTypes.h"
#include "Anim_BlendSpaceController.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_BlendSpaceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    float Speed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    float Direction = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    float LeanAngle = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    bool bIsMoving = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    bool bIsFalling = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    bool bIsJumping = false;

    FAnim_BlendSpaceData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        LeanAngle = 0.0f;
        bIsMoving = false;
        bIsFalling = false;
        bIsJumping = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_BlendSpaceSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space Settings")
    float SpeedSmoothingRate = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space Settings")
    float DirectionSmoothingRate = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space Settings")
    float LeanSmoothingRate = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space Settings")
    float MinMovementSpeed = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space Settings")
    float MaxLeanAngle = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space Settings")
    bool bUseAcceleration = true;

    FAnim_BlendSpaceSettings()
    {
        SpeedSmoothingRate = 10.0f;
        DirectionSmoothingRate = 15.0f;
        LeanSmoothingRate = 8.0f;
        MinMovementSpeed = 10.0f;
        MaxLeanAngle = 45.0f;
        bUseAcceleration = true;
    }
};

/**
 * Advanced Blend Space Animation Controller for TranspersonalGame
 * Handles 2D blend space calculations for locomotion animation
 * Supports speed/direction blending with smooth transitions
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_BlendSpaceController : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_BlendSpaceController();

protected:
    // Animation Blueprint Interface
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    // Blend Space Data
    UPROPERTY(BlueprintReadOnly, Category = "Blend Space", meta = (AllowPrivateAccess = "true"))
    FAnim_BlendSpaceData BlendSpaceData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space Settings", meta = (AllowPrivateAccess = "true"))
    FAnim_BlendSpaceSettings BlendSpaceSettings;

    // Character References
    UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
    ACharacter* OwnerCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
    UCharacterMovementComponent* MovementComponent;

    // Animation State Tracking
    UPROPERTY(BlueprintReadOnly, Category = "Animation State", meta = (AllowPrivateAccess = "true"))
    FVector Velocity;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State", meta = (AllowPrivateAccess = "true"))
    FVector Acceleration;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State", meta = (AllowPrivateAccess = "true"))
    float GroundSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State", meta = (AllowPrivateAccess = "true"))
    bool bShouldMove;

    // Smoothed Values
    UPROPERTY(BlueprintReadOnly, Category = "Smoothed Values", meta = (AllowPrivateAccess = "true"))
    float SmoothedSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "Smoothed Values", meta = (AllowPrivateAccess = "true"))
    float SmoothedDirection;

    UPROPERTY(BlueprintReadOnly, Category = "Smoothed Values", meta = (AllowPrivateAccess = "true"))
    float SmoothedLeanAngle;

public:
    // Blend Space Calculation Functions
    UFUNCTION(BlueprintCallable, Category = "Blend Space")
    void UpdateBlendSpaceValues(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Blend Space")
    void CalculateSpeedAndDirection();

    UFUNCTION(BlueprintCallable, Category = "Blend Space")
    void CalculateLeanAngle();

    UFUNCTION(BlueprintCallable, Category = "Blend Space")
    float GetBlendSpaceSpeed() const { return BlendSpaceData.Speed; }

    UFUNCTION(BlueprintCallable, Category = "Blend Space")
    float GetBlendSpaceDirection() const { return BlendSpaceData.Direction; }

    UFUNCTION(BlueprintCallable, Category = "Blend Space")
    float GetBlendSpaceLeanAngle() const { return BlendSpaceData.LeanAngle; }

    // State Query Functions
    UFUNCTION(BlueprintCallable, Category = "Animation State")
    bool IsMoving() const { return BlendSpaceData.bIsMoving; }

    UFUNCTION(BlueprintCallable, Category = "Animation State")
    bool IsFalling() const { return BlendSpaceData.bIsFalling; }

    UFUNCTION(BlueprintCallable, Category = "Animation State")
    bool IsJumping() const { return BlendSpaceData.bIsJumping; }

    // Configuration Functions
    UFUNCTION(BlueprintCallable, Category = "Configuration")
    void SetBlendSpaceSettings(const FAnim_BlendSpaceSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Configuration")
    FAnim_BlendSpaceSettings GetBlendSpaceSettings() const { return BlendSpaceSettings; }

    // Debug Functions
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogBlendSpaceData() const;

private:
    // Internal Calculation Helpers
    void UpdateCharacterReferences();
    void UpdateMovementData();
    void SmoothValues(float DeltaTime);
    float CalculateDirectionFromVelocity() const;
    float CalculateLeanFromAcceleration() const;
    bool ShouldCharacterMove() const;
};