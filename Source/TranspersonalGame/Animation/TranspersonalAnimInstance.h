#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../SharedTypes.h"
#include "TranspersonalAnimInstance.generated.h"

/**
 * Core Animation Instance for Transpersonal Game characters
 * Handles Motion Matching, IK, and prehistoric character movement
 * Based on RDR2-style character animation with weight and intention
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UTranspersonalAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UTranspersonalAnimInstance();

    // Animation Blueprint interface
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

protected:
    // === CORE MOVEMENT DATA ===
    
    /** Current movement speed */
    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float Speed;
    
    /** Current movement direction relative to character facing */
    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float Direction;
    
    /** Is character in the air */
    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    bool bIsInAir;
    
    /** Is character accelerating */
    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    bool bIsAccelerating;
    
    /** Current movement state */
    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    EAnim_MovementState MovementState;

    // === MOTION MATCHING DATA ===
    
    /** Motion matching velocity for database queries */
    UPROPERTY(BlueprintReadOnly, Category = "MotionMatching", meta = (AllowPrivateAccess = "true"))
    FVector MotionMatchingVelocity;
    
    /** Desired facing direction for motion matching */
    UPROPERTY(BlueprintReadOnly, Category = "MotionMatching", meta = (AllowPrivateAccess = "true"))
    FVector DesiredFacingDirection;
    
    /** Current animation pose cost (lower = better match) */
    UPROPERTY(BlueprintReadOnly, Category = "MotionMatching", meta = (AllowPrivateAccess = "true"))
    float PoseCost;
    
    /** Motion matching search radius */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MotionMatching")
    float SearchRadius;

    // === IK SYSTEM DATA ===
    
    /** Left foot IK offset */
    UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
    FVector LeftFootIKOffset;
    
    /** Right foot IK offset */
    UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
    FVector RightFootIKOffset;
    
    /** Pelvis IK offset for terrain adaptation */
    UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
    FVector PelvisIKOffset;
    
    /** Left foot rotation for terrain adaptation */
    UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
    FRotator LeftFootIKRotation;
    
    /** Right foot rotation for terrain adaptation */
    UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
    FRotator RightFootIKRotation;

    // === CHARACTER STATE ===
    
    /** Current character action state */
    UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
    EAnim_ActionState ActionState;
    
    /** Current weapon type being held */
    UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
    EAnim_WeaponType WeaponType;
    
    /** Is character crouching */
    UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
    bool bIsCrouching;
    
    /** Character's current stamina percentage */
    UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
    float StaminaPercentage;

private:
    // === CACHED REFERENCES ===
    
    /** Reference to the character */
    UPROPERTY()
    ACharacter* Character;
    
    /** Reference to character movement component */
    UPROPERTY()
    UCharacterMovementComponent* CharacterMovement;

    // === INTERNAL METHODS ===
    
    /** Update basic movement variables */
    UFUNCTION()
    void UpdateMovementVariables();
    
    /** Update motion matching data */
    UFUNCTION()
    void UpdateMotionMatchingData();
    
    /** Update IK foot placement */
    UFUNCTION()
    void UpdateFootIK();
    
    /** Calculate foot IK offset for terrain */
    UFUNCTION()
    FVector CalculateFootIKOffset(const FName& SocketName, float TraceDistance = 50.0f);
    
    /** Calculate foot rotation for terrain */
    UFUNCTION()
    FRotator CalculateFootIKRotation(const FName& SocketName);
    
    /** Update character state variables */
    UFUNCTION()
    void UpdateCharacterState();
    
    /** Smooth interpolation for IK offsets */
    UFUNCTION()
    FVector SmoothIKOffset(const FVector& CurrentOffset, const FVector& TargetOffset, float DeltaTime, float InterpSpeed = 10.0f);

    // === IK SETTINGS ===
    
    /** Maximum distance to trace for foot IK */
    UPROPERTY(EditAnywhere, Category = "IK Settings")
    float FootIKTraceDistance;
    
    /** IK interpolation speed */
    UPROPERTY(EditAnywhere, Category = "IK Settings")
    float IKInterpSpeed;
    
    /** Minimum pelvis adjustment */
    UPROPERTY(EditAnywhere, Category = "IK Settings")
    float MinPelvisOffset;
    
    /** Maximum pelvis adjustment */
    UPROPERTY(EditAnywhere, Category = "IK Settings")
    float MaxPelvisOffset;
};