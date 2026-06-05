#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataAsset.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace1D.h"
#include "../SharedTypes.h"
#include "Anim_MotionMatchingSystem.generated.h"

// Motion matching pose data structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionPose
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float Timestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FVector RootMotionVelocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FVector RootMotionDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TArray<FTransform> BoneTransforms;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    ECharacterMovementState MovementState;

    FAnim_MotionPose()
    {
        Timestamp = 0.0f;
        RootMotionVelocity = FVector::ZeroVector;
        RootMotionDirection = FVector::ForwardVector;
        MovementSpeed = 0.0f;
        MovementState = ECharacterMovementState::Idle;
    }
};

// Motion matching database asset
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UAnim_MotionDatabase : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Database")
    TArray<FAnim_MotionPose> MotionPoses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Database")
    TArray<UAnimSequence*> AnimationSequences;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Database")
    float PoseExtractionRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Database")
    int32 MaxSearchResults;

    UAnim_MotionDatabase()
    {
        PoseExtractionRate = 30.0f;
        MaxSearchResults = 5;
    }
};

// Motion matching search parameters
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionQuery
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Query")
    FVector DesiredVelocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Query")
    FVector DesiredDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Query")
    ECharacterMovementState DesiredState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Query")
    float SpeedWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Query")
    float DirectionWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Query")
    float PoseWeight;

    FAnim_MotionQuery()
    {
        DesiredVelocity = FVector::ZeroVector;
        DesiredDirection = FVector::ForwardVector;
        DesiredState = ECharacterMovementState::Idle;
        SpeedWeight = 1.0f;
        DirectionWeight = 1.0f;
        PoseWeight = 1.0f;
    }
};

// Motion matching component for fluid character animation
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_MotionMatchingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_MotionMatchingComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Motion matching database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    UAnim_MotionDatabase* MotionDatabase;

    // Current motion query
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    FAnim_MotionQuery CurrentQuery;

    // Best matching pose
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    FAnim_MotionPose BestMatchPose;

    // Motion matching settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float SearchRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float BlendTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bEnableMotionMatching;

    // Character reference
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    // Motion matching functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionQuery(const FVector& DesiredVelocity, const FVector& DesiredDirection);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_MotionPose FindBestMatchingPose(const FAnim_MotionQuery& Query);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculatePoseCost(const FAnim_MotionPose& Pose, const FAnim_MotionQuery& Query);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void ApplyMotionMatching(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetMotionDatabase(UAnim_MotionDatabase* NewDatabase);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    bool IsMotionMatchingEnabled() const { return bEnableMotionMatching; }

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void EnableMotionMatching(bool bEnable) { bEnableMotionMatching = bEnable; }

private:
    // Internal motion matching state
    float LastUpdateTime;
    FVector LastVelocity;
    FVector LastDirection;
    
    // Performance optimization
    int32 FramesSinceLastSearch;
    int32 SearchFrequency;
};

// Primitive character animation instance
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UAnim_PrimitiveCharacterInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_PrimitiveCharacterInstance();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTime) override;

public:
    // Character movement variables
    UPROPERTY(BlueprintReadOnly, Category = "Character Movement")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Character Movement")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Character Movement")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Character Movement")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Character Movement")
    ECharacterMovementState MovementState;

    // Survival action states
    UPROPERTY(BlueprintReadOnly, Category = "Survival Actions")
    bool bIsGathering;

    UPROPERTY(BlueprintReadOnly, Category = "Survival Actions")
    bool bIsCrafting;

    UPROPERTY(BlueprintReadOnly, Category = "Survival Actions")
    bool bIsEating;

    UPROPERTY(BlueprintReadOnly, Category = "Survival Actions")
    bool bIsDrinking;

    // Combat states
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsAttacking;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsBlocking;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsDodging;

    // Animation montages
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
    UAnimMontage* SpearThrustMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
    UAnimMontage* GatherMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
    UAnimMontage* CraftMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
    UAnimMontage* EatMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
    UAnimMontage* DrinkMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
    UAnimMontage* DodgeMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
    UAnimMontage* BlockMontage;

    // Blend spaces
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Spaces")
    UBlendSpace1D* LocomotionBlendSpace;

    // Character reference
    UPROPERTY()
    class ACharacter* Character;

    // Motion matching component reference
    UPROPERTY()
    UAnim_MotionMatchingComponent* MotionMatchingComponent;

    // Animation functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlaySurvivalAction(ESurvivalActionType ActionType);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayCombatAction(ECombatActionType ActionType);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopAllMontages();

private:
    void UpdateCharacterMovement();
    void UpdateSurvivalStates();
    void UpdateCombatStates();
};