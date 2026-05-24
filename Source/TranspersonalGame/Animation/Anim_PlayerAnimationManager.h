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
#include "../SharedTypes.h"
#include "Anim_PlayerAnimationManager.generated.h"

UENUM(BlueprintType)
enum class EAnim_PlayerState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Crafting    UMETA(DisplayName = "Crafting"),
    Gathering   UMETA(DisplayName = "Gathering"),
    Combat      UMETA(DisplayName = "Combat"),
    Injured     UMETA(DisplayName = "Injured"),
    Exhausted   UMETA(DisplayName = "Exhausted"),
    Afraid      UMETA(DisplayName = "Afraid")
};

UENUM(BlueprintType)
enum class EAnim_MovementDirection : uint8
{
    Forward     UMETA(DisplayName = "Forward"),
    Backward    UMETA(DisplayName = "Backward"),
    Left        UMETA(DisplayName = "Left"),
    Right       UMETA(DisplayName = "Right"),
    ForwardLeft UMETA(DisplayName = "Forward Left"),
    ForwardRight UMETA(DisplayName = "Forward Right"),
    BackwardLeft UMETA(DisplayName = "Backward Left"),
    BackwardRight UMETA(DisplayName = "Backward Right")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_PlayerAnimationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bIsInAir;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bIsCrouching;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    EAnim_PlayerState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    EAnim_MovementDirection MovementDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float HealthPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float StaminaPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float FearLevel;

    FAnim_PlayerAnimationData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        CurrentState = EAnim_PlayerState::Idle;
        MovementDirection = EAnim_MovementDirection::Forward;
        HealthPercentage = 1.0f;
        StaminaPercentage = 1.0f;
        FearLevel = 0.0f;
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_PlayerAnimationManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_PlayerAnimationManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Animation data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    FAnim_PlayerAnimationData AnimationData;

    // Animation assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UBlendSpace* LocomotionBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UAnimMontage* JumpMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UAnimMontage* CraftingMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UAnimMontage* GatheringMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UAnimMontage* CombatMontage;

    // Animation control functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateAnimationData();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetPlayerState(EAnim_PlayerState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayMontage(class UAnimMontage* Montage, float PlayRate = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopMontage(class UAnimMontage* Montage);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool IsPlayingMontage(class UAnimMontage* Montage) const;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetMovementDirection(const FVector& MovementInput);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateSurvivalStats(float Health, float Stamina, float Fear);

    // Blueprint implementable events
    UFUNCTION(BlueprintImplementableEvent, Category = "Animation Events")
    void OnStateChanged(EAnim_PlayerState OldState, EAnim_PlayerState NewState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation Events")
    void OnMontageStarted(class UAnimMontage* Montage);

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation Events")
    void OnMontageEnded(class UAnimMontage* Montage);

private:
    // Cached references
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    UPROPERTY()
    class USkeletalMeshComponent* MeshComponent;

    UPROPERTY()
    class UAnimInstance* AnimInstance;

    // Internal state
    EAnim_PlayerState PreviousState;
    float StateChangeTimer;
    float MinStateChangeDuration;

    // Helper functions
    void CacheComponents();
    EAnim_PlayerState CalculatePlayerState() const;
    EAnim_MovementDirection CalculateMovementDirection(const FVector& MovementInput) const;
    void HandleStateTransition(EAnim_PlayerState NewState);
    void UpdateBlendSpaceParameters();
};