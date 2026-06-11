#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataAsset.h"
#include "Animation/AnimInstance.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Anim_MotionMatchingSystem.generated.h"

UENUM(BlueprintType)
enum class EAnim_LocomotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walk        UMETA(DisplayName = "Walk"),
    Run         UMETA(DisplayName = "Run"),
    Sprint      UMETA(DisplayName = "Sprint"),
    Jump        UMETA(DisplayName = "Jump"),
    Fall        UMETA(DisplayName = "Fall"),
    Crouch      UMETA(DisplayName = "Crouch"),
    Prone       UMETA(DisplayName = "Prone")
};

UENUM(BlueprintType)
enum class EAnim_ActionState : uint8
{
    None            UMETA(DisplayName = "None"),
    AttackMelee     UMETA(DisplayName = "Attack Melee"),
    AttackRanged    UMETA(DisplayName = "Attack Ranged"),
    Block           UMETA(DisplayName = "Block"),
    Dodge           UMETA(DisplayName = "Dodge"),
    Interact        UMETA(DisplayName = "Interact"),
    Craft           UMETA(DisplayName = "Craft"),
    Gather          UMETA(DisplayName = "Gather"),
    Death           UMETA(DisplayName = "Death")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionMatchingData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FVector Acceleration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bIsInAir;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bIsCrouching;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float GroundDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    EAnim_LocomotionState LocomotionState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    EAnim_ActionState ActionState;

    FAnim_MotionMatchingData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        Velocity = FVector::ZeroVector;
        Acceleration = FVector::ZeroVector;
        bIsInAir = false;
        bIsCrouching = false;
        GroundDistance = 0.0f;
        LocomotionState = EAnim_LocomotionState::Idle;
        ActionState = EAnim_ActionState::None;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_IKFootData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FVector FootLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FRotator FootRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float IKWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float GroundDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    bool bValidHit;

    FAnim_IKFootData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKWeight = 0.0f;
        GroundDistance = 0.0f;
        bValidHit = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_MotionMatchingDatabase : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TArray<class UAnimSequence*> AnimationSequences;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TArray<FAnim_MotionMatchingData> MotionData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Spaces")
    class UBlendSpace* LocomotionBlendSpace2D;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montages")
    TMap<EAnim_ActionState, class UAnimMontage*> ActionMontages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float BlendTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float SearchRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    int32 MaxCandidates;

    UAnim_MotionMatchingDatabase()
    {
        BlendTime = 0.2f;
        SearchRadius = 100.0f;
        MaxCandidates = 5;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_MotionMatchingSystem : public UObject
{
    GENERATED_BODY()

public:
    UAnim_MotionMatchingSystem();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_MotionMatchingData CalculateMotionData(class ACharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    class UAnimSequence* FindBestMatchingAnimation(const FAnim_MotionMatchingData& CurrentData, UAnim_MotionMatchingDatabase* Database);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    EAnim_LocomotionState DetermineLocomotionState(float Speed, bool bIsInAir, bool bIsCrouching);

    UFUNCTION(BlueprintCallable, Category = "IK")
    FAnim_IKFootData CalculateFootIK(class ACharacter* Character, FName SocketName, float TraceDistance = 50.0f);

    UFUNCTION(BlueprintCallable, Category = "IK")
    void UpdateFootIK(class USkeletalMeshComponent* MeshComponent, const FAnim_IKFootData& LeftFoot, const FAnim_IKFootData& RightFoot);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool PlayActionMontage(class UAnimInstance* AnimInstance, EAnim_ActionState ActionState, UAnim_MotionMatchingDatabase* Database);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateBlendSpaceParameters(class UAnimInstance* AnimInstance, const FAnim_MotionMatchingData& MotionData);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MotionMatchingUpdateRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float IKUpdateRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float MaxIKDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float IKInterpSpeed;

private:
    float CalculateAnimationScore(const FAnim_MotionMatchingData& Current, const FAnim_MotionMatchingData& Candidate);
    FVector GetCharacterVelocity(class ACharacter* Character);
    FVector GetCharacterAcceleration(class ACharacter* Character);
    bool PerformGroundTrace(class ACharacter* Character, FVector StartLocation, float TraceDistance, FHitResult& OutHit);
};