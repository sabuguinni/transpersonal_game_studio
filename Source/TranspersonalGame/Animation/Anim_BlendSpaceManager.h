#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/BlendSpace1D.h"
#include "Engine/BlendSpace.h"
#include "Animation/AnimSequence.h"
#include "Components/ActorComponent.h"
#include "Anim_BlendSpaceManager.generated.h"

UENUM(BlueprintType)
enum class EAnim_LocomotionState : uint8
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

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_BlendSpaceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UBlendSpace1D* LocomotionBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UBlendSpace* DirectionalBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float BlendSpaceInput;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    FVector2D DirectionalInput;

    FAnim_BlendSpaceData()
    {
        LocomotionBlendSpace = nullptr;
        DirectionalBlendSpace = nullptr;
        BlendSpaceInput = 0.0f;
        DirectionalInput = FVector2D::ZeroVector;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_SequenceSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimSequence* IdleAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimSequence* WalkAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimSequence* RunAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimSequence* JumpStartAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimSequence* JumpLoopAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimSequence* JumpEndAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimSequence* CrouchIdleAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimSequence* CrouchWalkAnimation;

    FAnim_SequenceSet()
    {
        IdleAnimation = nullptr;
        WalkAnimation = nullptr;
        RunAnimation = nullptr;
        JumpStartAnimation = nullptr;
        JumpLoopAnimation = nullptr;
        JumpEndAnimation = nullptr;
        CrouchIdleAnimation = nullptr;
        CrouchWalkAnimation = nullptr;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_BlendSpaceManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_BlendSpaceManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Blend space data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|BlendSpaces")
    FAnim_BlendSpaceData BlendSpaceData;

    // Animation sequences
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Sequences")
    FAnim_SequenceSet AnimationSequences;

    // Current locomotion state
    UPROPERTY(BlueprintReadOnly, Category = "Animation|State")
    EAnim_LocomotionState CurrentLocomotionState;

    // Speed thresholds for blend space
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Settings")
    float WalkSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Settings")
    float RunSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Settings")
    float SprintSpeedThreshold;

    // Blend space update functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateBlendSpaceInput(float Speed, FVector2D Direction);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetLocomotionState(EAnim_LocomotionState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    EAnim_LocomotionState GetLocomotionStateFromSpeed(float Speed, bool bIsCrouching, bool bIsInAir);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void CreateRuntimeBlendSpace();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool ValidateAnimationAssets();

private:
    // Internal blend space calculation
    void CalculateBlendSpaceValues(float Speed, FVector2D Direction);
    
    // State transition logic
    bool CanTransitionToState(EAnim_LocomotionState FromState, EAnim_LocomotionState ToState);
};