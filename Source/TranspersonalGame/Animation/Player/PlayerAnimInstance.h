#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "Engine/DataTable.h"
#include "PlayerAnimInstance.generated.h"

USTRUCT(BlueprintType)
struct FMotionMatchingState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float Speed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float Direction = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bIsMoving = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bIsCrouching = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bIsInAir = false;
};

USTRUCT(BlueprintType)
struct FFootIKData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FVector LeftFootOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FVector RightFootOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FRotator LeftFootRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FRotator RightFootRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float PelvisOffset = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float IKAlpha = 1.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPlayerAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UPlayerAnimInstance();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    // Character Reference
    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class ACharacter* OwningCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class UCharacterMovementComponent* CharacterMovement;

    // Motion Matching Variables
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    FMotionMatchingState MotionState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    UPoseSearchDatabase* CurrentDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    UPoseSearchDatabase* LocomotionDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    UPoseSearchDatabase* CautiousDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    UPoseSearchDatabase* PanicDatabase;

    // Fear System Variables
    UPROPERTY(BlueprintReadOnly, Category = "Fear System")
    float CurrentFearLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fear System")
    float FearDecayRate = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fear System")
    float CautiousThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fear System")
    float PanicThreshold = 0.8f;

    // Foot IK Variables
    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FFootIKData FootIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float FootIKTraceDistance = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float FootIKInterpSpeed = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    bool bEnableFootIK = true;

    // Paleontologist-specific animations
    UPROPERTY(BlueprintReadOnly, Category = "Character Traits")
    bool bIsExaminingGround = false;

    UPROPERTY(BlueprintReadOnly, Category = "Character Traits")
    bool bIsLookingAround = false;

    UPROPERTY(BlueprintReadOnly, Category = "Character Traits")
    float CautiousLookFrequency = 2.0f;

private:
    // Internal update functions
    void UpdateMotionMatchingState(float DeltaTime);
    void UpdateFearSystem(float DeltaTime);
    void UpdateFootIK(float DeltaTime);
    void UpdateCharacterTraits(float DeltaTime);

    // Foot IK helper functions
    FVector PerformFootTrace(const FVector& FootLocation, const FName& SocketName);
    void CalculateFootIKOffset(const FVector& FootLocation, const FName& SocketName, 
                              FVector& OutOffset, FRotator& OutRotation);

    // Fear detection
    void DetectNearbyThreats();

    // Timers
    float LastLookAroundTime = 0.0f;
    float TimeSinceLastMovement = 0.0f;

public:
    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Fear System")
    void AddFear(float FearAmount);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetMotionMatchingDatabase(UPoseSearchDatabase* NewDatabase);

    UFUNCTION(BlueprintCallable, Category = "Character Traits")
    void TriggerExamineGround();

    UFUNCTION(BlueprintPure, Category = "Animation State")
    bool ShouldUseCautiousAnimations() const;

    UFUNCTION(BlueprintPure, Category = "Animation State")
    bool ShouldUsePanicAnimations() const;
};