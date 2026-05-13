#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Anim_CharacterAnimationController.generated.h"

UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Walking UMETA(DisplayName = "Walking"),
    Running UMETA(DisplayName = "Running"),
    Jumping UMETA(DisplayName = "Jumping"),
    Falling UMETA(DisplayName = "Falling"),
    Crouching UMETA(DisplayName = "Crouching"),
    Swimming UMETA(DisplayName = "Swimming"),
    Climbing UMETA(DisplayName = "Climbing"),
    Combat UMETA(DisplayName = "Combat")
};

UENUM(BlueprintType)
enum class EAnim_EmotionalState : uint8
{
    Calm UMETA(DisplayName = "Calm"),
    Alert UMETA(DisplayName = "Alert"),
    Fearful UMETA(DisplayName = "Fearful"),
    Aggressive UMETA(DisplayName = "Aggressive"),
    Exhausted UMETA(DisplayName = "Exhausted"),
    Injured UMETA(DisplayName = "Injured"),
    Confident UMETA(DisplayName = "Confident")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MovementData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Movement")
    float Speed;

    UPROPERTY(BlueprintReadWrite, Category = "Movement")
    float Direction;

    UPROPERTY(BlueprintReadWrite, Category = "Movement")
    FVector Velocity;

    UPROPERTY(BlueprintReadWrite, Category = "Movement")
    bool bIsMoving;

    UPROPERTY(BlueprintReadWrite, Category = "Movement")
    bool bIsInAir;

    UPROPERTY(BlueprintReadWrite, Category = "Movement")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadWrite, Category = "Movement")
    float MovementInputMagnitude;

    FAnim_MovementData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        Velocity = FVector::ZeroVector;
        bIsMoving = false;
        bIsInAir = false;
        bIsCrouching = false;
        MovementInputMagnitude = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_AnimationSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimSequence* IdleAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UBlendSpace* MovementBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimMontage* JumpMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimMontage* LandMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimMontage* CrouchMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    TArray<class UAnimMontage*> CombatMontages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    TArray<class UAnimMontage*> EmotionalMontages;

    FAnim_AnimationSet()
    {
        IdleAnimation = nullptr;
        MovementBlendSpace = nullptr;
        JumpMontage = nullptr;
        LandMontage = nullptr;
        CrouchMontage = nullptr;
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_CharacterAnimationController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_CharacterAnimationController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core animation control
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateMovementData();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetMovementState(EAnim_MovementState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetEmotionalState(EAnim_EmotionalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayMontage(UAnimMontage* Montage, float PlayRate = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopMontage(UAnimMontage* Montage);

    // Animation blending
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void BlendToMovementState(EAnim_MovementState TargetState, float BlendTime = 0.3f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void BlendToEmotionalState(EAnim_EmotionalState TargetState, float BlendTime = 0.5f);

    // IK and procedural animation
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void EnableFootIK(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void EnableLookAtIK(bool bEnable, AActor* TargetActor = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateFootIK();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateLookAtIK();

    // Animation events
    UFUNCTION(BlueprintImplementableEvent, Category = "Animation")
    void OnMovementStateChanged(EAnim_MovementState OldState, EAnim_MovementState NewState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation")
    void OnEmotionalStateChanged(EAnim_EmotionalState OldState, EAnim_EmotionalState NewState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation")
    void OnMontageStarted(UAnimMontage* Montage);

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation")
    void OnMontageEnded(UAnimMontage* Montage);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Animation")
    EAnim_MovementState GetCurrentMovementState() const { return CurrentMovementState; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    EAnim_EmotionalState GetCurrentEmotionalState() const { return CurrentEmotionalState; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    FAnim_MovementData GetMovementData() const { return MovementData; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool IsPlayingMontage() const;

protected:
    // Character references
    UPROPERTY(BlueprintReadOnly, Category = "References")
    class ACharacter* OwnerCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    class UCharacterMovementComponent* MovementComponent;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    class USkeletalMeshComponent* MeshComponent;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    class UAnimInstance* AnimInstance;

    // Animation data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    FAnim_AnimationSet AnimationSet;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    FAnim_MovementData MovementData;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    EAnim_MovementState CurrentMovementState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    EAnim_MovementState PreviousMovementState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    EAnim_EmotionalState CurrentEmotionalState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    EAnim_EmotionalState PreviousEmotionalState;

    // IK settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    bool bEnableFootIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    bool bEnableLookAtIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float FootIKTraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FName LeftFootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FName RightFootBoneName;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    AActor* LookAtTarget;

    // Animation blending
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blending")
    float DefaultBlendTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blending")
    float MovementStateBlendTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blending")
    float EmotionalStateBlendTime;

private:
    // Internal state tracking
    float LastMovementUpdateTime;
    float StateChangeTimer;
    bool bIsBlending;

    // Helper functions
    void InitializeReferences();
    void UpdateAnimationState();
    EAnim_MovementState CalculateMovementState();
    void PerformFootIKTrace(const FName& BoneName, float& OutOffset);
    FVector CalculateLookAtDirection();
};