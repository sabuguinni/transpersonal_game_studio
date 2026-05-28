#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TranspersonalGame/TranspersonalGame.h"
#include "Anim_CharacterAnimController.generated.h"

UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Sprinting   UMETA(DisplayName = "Sprinting"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Swimming    UMETA(DisplayName = "Swimming"),
    Climbing    UMETA(DisplayName = "Climbing")
};

UENUM(BlueprintType)
enum class EAnim_CombatState : uint8
{
    None        UMETA(DisplayName = "None"),
    Ready       UMETA(DisplayName = "Ready"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Blocking    UMETA(DisplayName = "Blocking"),
    Dodging     UMETA(DisplayName = "Dodging"),
    Stunned     UMETA(DisplayName = "Stunned"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class EAnim_EmotionalState : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Alert       UMETA(DisplayName = "Alert"),
    Fearful     UMETA(DisplayName = "Fearful"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Exhausted   UMETA(DisplayName = "Exhausted"),
    Injured     UMETA(DisplayName = "Injured"),
    Focused     UMETA(DisplayName = "Focused")
};

USTRUCT(BlueprintType)
struct FAnim_CharacterAnimData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    EAnim_MovementState MovementState;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    EAnim_CombatState CombatState;

    UPROPERTY(BlueprintReadOnly, Category = "Emotion")
    EAnim_EmotionalState EmotionalState;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FVector LeftFootIKLocation;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FVector RightFootIKLocation;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FRotator LeftFootIKRotation;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FRotator RightFootIKRotation;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float PelvisOffset;

    FAnim_CharacterAnimData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        MovementState = EAnim_MovementState::Idle;
        CombatState = EAnim_CombatState::None;
        EmotionalState = EAnim_EmotionalState::Calm;
        LeftFootIKLocation = FVector::ZeroVector;
        RightFootIKLocation = FVector::ZeroVector;
        LeftFootIKRotation = FRotator::ZeroRotator;
        RightFootIKRotation = FRotator::ZeroRotator;
        PelvisOffset = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_CharacterAnimController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_CharacterAnimController();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Animation Data
    UPROPERTY(BlueprintReadOnly, Category = "Animation Data", meta = (AllowPrivateAccess = "true"))
    FAnim_CharacterAnimData AnimData;

    // Animation Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UBlendSpace* MovementBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UAnimMontage* JumpMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UAnimMontage* AttackMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UAnimMontage* BlockMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UAnimMontage* DodgeMontage;

    // IK Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableFootIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float FootIKTraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float FootIKInterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FName LeftFootSocketName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FName RightFootSocketName;

    // Animation Control Functions
    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void PlayMontage(class UAnimMontage* Montage, float PlayRate = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void StopMontage(class UAnimMontage* Montage);

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void SetMovementState(EAnim_MovementState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void SetCombatState(EAnim_CombatState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void SetEmotionalState(EAnim_EmotionalState NewState);

    // IK Functions
    UFUNCTION(BlueprintCallable, Category = "IK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "IK")
    FVector PerformFootTrace(const FName& SocketName, float TraceDistance);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Animation Data")
    FAnim_CharacterAnimData GetAnimationData() const { return AnimData; }

    UFUNCTION(BlueprintPure, Category = "Animation Data")
    EAnim_MovementState GetMovementState() const { return AnimData.MovementState; }

    UFUNCTION(BlueprintPure, Category = "Animation Data")
    EAnim_CombatState GetCombatState() const { return AnimData.CombatState; }

    UFUNCTION(BlueprintPure, Category = "Animation Data")
    EAnim_EmotionalState GetEmotionalState() const { return AnimData.EmotionalState; }

private:
    // Component References
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    UPROPERTY()
    class USkeletalMeshComponent* MeshComponent;

    UPROPERTY()
    class UAnimInstance* AnimInstance;

    // Internal Functions
    void UpdateMovementData(float DeltaTime);
    void UpdateCombatData(float DeltaTime);
    void UpdateEmotionalData(float DeltaTime);
    void CacheComponentReferences();
    bool IsValidForAnimation() const;
};