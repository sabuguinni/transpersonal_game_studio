#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Anim_MotionMatchingComponent.generated.h"

UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Combat      UMETA(DisplayName = "Combat"),
    Gathering   UMETA(DisplayName = "Gathering"),
    Crafting    UMETA(DisplayName = "Crafting")
};

UENUM(BlueprintType)
enum class EAnim_EmotionalState : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Alert       UMETA(DisplayName = "Alert"),
    Fearful     UMETA(DisplayName = "Fearful"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Exhausted   UMETA(DisplayName = "Exhausted"),
    Injured     UMETA(DisplayName = "Injured")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    float Speed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    float Direction = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    bool bIsInAir = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    bool bIsCrouching = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    float Acceleration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    EAnim_MovementState MovementState = EAnim_MovementState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    EAnim_EmotionalState EmotionalState = EAnim_EmotionalState::Calm;
};

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
    // Motion matching parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float WalkThreshold = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float RunThreshold = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float BlendTime = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float DirectionSmoothingSpeed = 10.0f;

    // Animation assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UBlendSpace> MovementBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UAnimMontage> JumpMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UAnimMontage> CombatMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UAnimMontage> GatheringMontage;

    // Current motion data
    UPROPERTY(BlueprintReadOnly, Category = "Motion Data")
    FAnim_MotionData CurrentMotionData;

    // Component references
    UPROPERTY()
    TObjectPtr<UCharacterMovementComponent> MovementComponent;

    UPROPERTY()
    TObjectPtr<UAnimInstance> AnimInstance;

public:
    // Public interface
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionData();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetEmotionalState(EAnim_EmotionalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void PlayMontage(UAnimMontage* Montage, float PlayRate = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void TriggerJump();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void TriggerCombatAction();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void TriggerGatheringAction();

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    EAnim_MovementState GetCurrentMovementState() const { return CurrentMotionData.MovementState; }

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    EAnim_EmotionalState GetCurrentEmotionalState() const { return CurrentMotionData.EmotionalState; }

private:
    void InitializeComponents();
    void CalculateMovementState();
    void UpdateBlendSpaceParameters();
    void SmoothDirectionChange(float DeltaTime);

    float SmoothedDirection = 0.0f;
    float LastUpdateTime = 0.0f;
};