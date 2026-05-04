#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Kismet/KismetMathLibrary.h"
#include "Anim_PrehistoricAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EAnim_PrehistoricMovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"), 
    Running     UMETA(DisplayName = "Running"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Swimming    UMETA(DisplayName = "Swimming"),
    Climbing    UMETA(DisplayName = "Climbing")
};

UENUM(BlueprintType)
enum class EAnim_PrehistoricActionState : uint8
{
    None            UMETA(DisplayName = "None"),
    Gathering       UMETA(DisplayName = "Gathering"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Combat          UMETA(DisplayName = "Combat"),
    Eating          UMETA(DisplayName = "Eating"),
    Drinking        UMETA(DisplayName = "Drinking"),
    Sleeping        UMETA(DisplayName = "Sleeping"),
    Hiding          UMETA(DisplayName = "Hiding")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_PrehistoricAnimData
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
    EAnim_PrehistoricMovementState MovementState;

    UPROPERTY(BlueprintReadOnly, Category = "Action")
    EAnim_PrehistoricActionState ActionState;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float HealthPercentage;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float StaminaPercentage;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float FearLevel;

    FAnim_PrehistoricAnimData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        MovementState = EAnim_PrehistoricMovementState::Idle;
        ActionState = EAnim_PrehistoricActionState::None;
        HealthPercentage = 1.0f;
        StaminaPercentage = 1.0f;
        FearLevel = 0.0f;
    }
};

/**
 * Animation Instance para personagens pré-históricos
 * Gere estados de movimento, ações de sobrevivência e reações emocionais
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_PrehistoricAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_PrehistoricAnimInstance();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    // === DADOS DE ANIMAÇÃO ===
    UPROPERTY(BlueprintReadOnly, Category = "Animation Data", meta = (AllowPrivateAccess = "true"))
    FAnim_PrehistoricAnimData AnimData;

    // === REFERÊNCIAS ===
    UPROPERTY(BlueprintReadOnly, Category = "References", meta = (AllowPrivateAccess = "true"))
    ACharacter* OwnerCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "References", meta = (AllowPrivateAccess = "true"))
    UCharacterMovementComponent* MovementComponent;

    // === CONFIGURAÇÕES DE MOVIMENTO ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings")
    float WalkSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings")
    float RunSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings")
    float DirectionSmoothingSpeed;

    // === BLEND SPACES ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UBlendSpace* LocomotionBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UBlendSpace* CrouchBlendSpace;

    // === MONTAGES ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* JumpMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* GatherMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* CraftMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* EatMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* DrinkMontage;

public:
    // === FUNÇÕES PÚBLICAS ===
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayActionMontage(EAnim_PrehistoricActionState ActionType);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopActionMontage();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetFearLevel(float NewFearLevel);

    UFUNCTION(BlueprintPure, Category = "Animation")
    float GetCurrentSpeed() const { return AnimData.Speed; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    EAnim_PrehistoricMovementState GetMovementState() const { return AnimData.MovementState; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    EAnim_PrehistoricActionState GetActionState() const { return AnimData.ActionState; }

private:
    // === FUNÇÕES PRIVADAS ===
    void UpdateMovementData(float DeltaTime);
    void UpdateMovementState();
    void UpdateActionState();
    void UpdateSurvivalData();
    EAnim_PrehistoricMovementState CalculateMovementState() const;
    UAnimMontage* GetMontageForAction(EAnim_PrehistoricActionState ActionType) const;

    // === VARIÁVEIS DE CONTROLO ===
    float LastDirection;
    float DirectionChangeSpeed;
    bool bWasInAir;
    float TimeInCurrentState;
};