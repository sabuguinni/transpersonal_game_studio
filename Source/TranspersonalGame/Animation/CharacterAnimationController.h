#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Engine/Engine.h"
#include "CharacterAnimationController.generated.h"

class USkeletalMeshComponent;
class UAnimSequence;
class UBlendSpace;

UENUM(BlueprintType)
enum class EAnim_CharacterState : uint8
{
    Idle           UMETA(DisplayName = "Idle"),
    Walking        UMETA(DisplayName = "Walking"),
    Running        UMETA(DisplayName = "Running"),
    Jumping        UMETA(DisplayName = "Jumping"),
    Falling        UMETA(DisplayName = "Falling"),
    Landing        UMETA(DisplayName = "Landing"),
    Crouching      UMETA(DisplayName = "Crouching"),
    Crawling       UMETA(DisplayName = "Crawling"),
    Swimming       UMETA(DisplayName = "Swimming"),
    Climbing       UMETA(DisplayName = "Climbing"),
    Injured        UMETA(DisplayName = "Injured"),
    Exhausted      UMETA(DisplayName = "Exhausted"),
    Fearful        UMETA(DisplayName = "Fearful"),
    Dead           UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class EAnim_MovementDirection : uint8
{
    Forward        UMETA(DisplayName = "Forward"),
    Backward       UMETA(DisplayName = "Backward"),
    Left           UMETA(DisplayName = "Left"),
    Right          UMETA(DisplayName = "Right"),
    ForwardLeft    UMETA(DisplayName = "Forward Left"),
    ForwardRight   UMETA(DisplayName = "Forward Right"),
    BackwardLeft   UMETA(DisplayName = "Backward Left"),
    BackwardRight  UMETA(DisplayName = "Backward Right")
};

USTRUCT(BlueprintType)
struct FAnim_CharacterAnimationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float Speed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float Direction = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bIsInAir = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bIsCrouching = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bIsInjured = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bIsExhausted = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bIsFearful = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float HealthPercentage = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float StaminaPercentage = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float FearLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    EAnim_CharacterState CurrentState = EAnim_CharacterState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    EAnim_MovementDirection MovementDirection = EAnim_MovementDirection::Forward;
};

USTRUCT(BlueprintType)
struct FAnim_AnimationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float WalkSpeedThreshold = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float RunSpeedThreshold = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float JumpVelocityThreshold = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float FallVelocityThreshold = -200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float InjuredHealthThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float ExhaustedStaminaThreshold = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float FearfulThreshold = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float AnimationBlendSpeed = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float StateTransitionDelay = 0.1f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCharacterAnimationController : public UActorComponent
{
    GENERATED_BODY()

public:
    UCharacterAnimationController();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Animation State Management
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateAnimationState(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetCharacterState(EAnim_CharacterState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    EAnim_CharacterState GetCharacterState() const { return AnimationData.CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateMovementData(float Speed, float Direction, bool bInAir);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateSurvivalStats(float Health, float Stamina, float Fear);

    // Animation Playback
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayMontage(UAnimMontage* Montage, float PlayRate = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopMontage(UAnimMontage* Montage = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool IsPlayingMontage(UAnimMontage* Montage = nullptr) const;

    // Getters for Animation Blueprint
    UFUNCTION(BlueprintPure, Category = "Animation")
    FAnim_CharacterAnimationData GetAnimationData() const { return AnimationData; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    float GetSpeed() const { return AnimationData.Speed; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    float GetDirection() const { return AnimationData.Direction; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool IsInAir() const { return AnimationData.bIsInAir; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool IsCrouching() const { return AnimationData.bIsCrouching; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool IsInjured() const { return AnimationData.bIsInjured; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool IsExhausted() const { return AnimationData.bIsExhausted; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool IsFearful() const { return AnimationData.bIsFearful; }

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
    FAnim_CharacterAnimationData AnimationData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    FAnim_AnimationSettings Settings;

    UPROPERTY()
    USkeletalMeshComponent* SkeletalMeshComponent;

    UPROPERTY()
    UAnimInstance* AnimInstance;

    // State transition timing
    float StateTransitionTimer = 0.0f;
    EAnim_CharacterState PendingState = EAnim_CharacterState::Idle;

private:
    void InitializeComponents();
    void DetermineCharacterState();
    void UpdateMovementDirection();
    bool CanTransitionToState(EAnim_CharacterState NewState) const;
    void OnStateChanged(EAnim_CharacterState OldState, EAnim_CharacterState NewState);
};