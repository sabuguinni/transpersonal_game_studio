#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Anim_CharacterAnimationSystem.generated.h"

UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Swimming    UMETA(DisplayName = "Swimming")
};

UENUM(BlueprintType)
enum class EAnim_ActionState : uint8
{
    None        UMETA(DisplayName = "None"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Gathering   UMETA(DisplayName = "Gathering"),
    Crafting    UMETA(DisplayName = "Crafting"),
    Interacting UMETA(DisplayName = "Interacting"),
    Climbing    UMETA(DisplayName = "Climbing")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MovementData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Speed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Direction = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsMoving = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsInAir = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsCrouching = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    EAnim_MovementState MovementState = EAnim_MovementState::Idle;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_ActionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
    EAnim_ActionState CurrentAction = EAnim_ActionState::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
    float ActionProgress = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
    bool bIsPerformingAction = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
    FVector ActionTargetLocation = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MontageConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    TSoftObjectPtr<UAnimMontage> IdleMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    TSoftObjectPtr<UAnimMontage> WalkMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    TSoftObjectPtr<UAnimMontage> RunMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    TSoftObjectPtr<UAnimMontage> JumpMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    TSoftObjectPtr<UAnimMontage> AttackMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    TSoftObjectPtr<UAnimMontage> GatherMontage;
};

/**
 * Character Animation System Component
 * Manages character animations, state transitions, and montage playback
 * Integrates with UE5 Motion Matching and provides smooth animation blending
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_CharacterAnimationSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_CharacterAnimationSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Animation State Management
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateMovementState(const FAnim_MovementData& MovementData);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetActionState(EAnim_ActionState NewActionState, float Duration = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayMontage(UAnimMontage* Montage, float PlayRate = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopMontage(float BlendOutTime = 0.25f);

    // Motion Matching Integration
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void EnableMotionMatching(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetMotionMatchingDatabase(class UMotionMatchingComponent* Database);

    // IK System Integration
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void EnableFootIK(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateFootIKTargets();

    // Animation Events
    UFUNCTION(BlueprintImplementableEvent, Category = "Animation")
    void OnAnimationStateChanged(EAnim_MovementState OldState, EAnim_MovementState NewState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation")
    void OnActionStarted(EAnim_ActionState ActionState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation")
    void OnActionCompleted(EAnim_ActionState ActionState);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Animation")
    FAnim_MovementData GetMovementData() const { return CurrentMovementData; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    FAnim_ActionData GetActionData() const { return CurrentActionData; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool IsPlayingMontage() const;

protected:
    // Component References
    UPROPERTY()
    USkeletalMeshComponent* SkeletalMeshComponent;

    UPROPERTY()
    UCharacterMovementComponent* MovementComponent;

    UPROPERTY()
    UAnimInstance* AnimInstance;

    // Animation Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    FAnim_MovementData CurrentMovementData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    FAnim_ActionData CurrentActionData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    FAnim_MontageConfig MontageConfiguration;

    // Animation Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings", meta = (AllowPrivateAccess = "true"))
    float WalkSpeedThreshold = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings", meta = (AllowPrivateAccess = "true"))
    float RunSpeedThreshold = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings", meta = (AllowPrivateAccess = "true"))
    float BlendTime = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings", meta = (AllowPrivateAccess = "true"))
    bool bUseMotionMatching = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings", meta = (AllowPrivateAccess = "true"))
    bool bUseFootIK = true;

    // Motion Matching
    UPROPERTY()
    class UMotionMatchingComponent* MotionMatchingComponent;

    // IK System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings", meta = (AllowPrivateAccess = "true"))
    float FootIKTraceDistance = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings", meta = (AllowPrivateAccess = "true"))
    FName LeftFootBoneName = TEXT("foot_l");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings", meta = (AllowPrivateAccess = "true"))
    FName RightFootBoneName = TEXT("foot_r");

private:
    // Internal State
    EAnim_MovementState PreviousMovementState;
    EAnim_ActionState PreviousActionState;
    float ActionTimer = 0.0f;
    float ActionDuration = 0.0f;

    // Internal Methods
    void InitializeComponents();
    void UpdateAnimationBlueprint();
    EAnim_MovementState DetermineMovementState() const;
    void HandleStateTransition(EAnim_MovementState NewState);
    void UpdateActionProgress(float DeltaTime);
    void PerformFootIKTrace(const FName& BoneName, FVector& OutIKTarget, FRotator& OutIKRotation);
};