#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "../Core/AnimationSystemManager.h"
#include "PlayerAnimationController.generated.h"

UENUM(BlueprintType)
enum class EPlayerMovementState : uint8
{
    Idle,
    Walking,
    Running,
    Crouching,
    Crawling,
    Climbing,
    Jumping,
    Falling,
    Landing,
    Hiding,
    Interacting
};

UENUM(BlueprintType)
enum class EPlayerEmotionalState : uint8
{
    Calm,
    Nervous,
    Afraid,
    Panicked,
    Focused,
    Exhausted,
    Alert
};

USTRUCT(BlueprintType)
struct FPlayerAnimationData
{
    GENERATED_BODY()

    // Estados básicos
    UPROPERTY(BlueprintReadOnly)
    EPlayerMovementState MovementState;

    UPROPERTY(BlueprintReadOnly)
    EPlayerEmotionalState EmotionalState;

    // Dados de movimento para Motion Matching
    UPROPERTY(BlueprintReadOnly)
    FVector Velocity;

    UPROPERTY(BlueprintReadOnly)
    float Speed;

    UPROPERTY(BlueprintReadOnly)
    float Direction;

    UPROPERTY(BlueprintReadOnly)
    bool bIsMoving;

    UPROPERTY(BlueprintReadOnly)
    bool bIsGrounded;

    // Dados contextuais para animações procedurais
    UPROPERTY(BlueprintReadOnly)
    float StaminaLevel;

    UPROPERTY(BlueprintReadOnly)
    float FearLevel;

    UPROPERTY(BlueprintReadOnly)
    float HealthPercentage;

    // IK Data
    UPROPERTY(BlueprintReadOnly)
    bool bEnableFootIK;

    UPROPERTY(BlueprintReadOnly)
    FVector LeftFootIKOffset;

    UPROPERTY(BlueprintReadOnly)
    FVector RightFootIKOffset;

    UPROPERTY(BlueprintReadOnly)
    FRotator LeftFootIKRotation;

    UPROPERTY(BlueprintReadOnly)
    FRotator RightFootIKRotation;

    FPlayerAnimationData()
    {
        MovementState = EPlayerMovementState::Idle;
        EmotionalState = EPlayerEmotionalState::Calm;
        Velocity = FVector::ZeroVector;
        Speed = 0.0f;
        Direction = 0.0f;
        bIsMoving = false;
        bIsGrounded = true;
        StaminaLevel = 1.0f;
        FearLevel = 0.0f;
        HealthPercentage = 1.0f;
        bEnableFootIK = true;
        LeftFootIKOffset = FVector::ZeroVector;
        RightFootIKOffset = FVector::ZeroVector;
        LeftFootIKRotation = FRotator::ZeroRotator;
        RightFootIKRotation = FRotator::ZeroRotator;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPlayerAnimationController : public UAnimInstance
{
    GENERATED_BODY()

public:
    UPlayerAnimationController();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    // Referência ao personagem
    UPROPERTY(BlueprintReadOnly, Category = "Character")
    ACharacter* OwningCharacter;

    // Dados de animação atuais
    UPROPERTY(BlueprintReadOnly, Category = "Animation Data")
    FPlayerAnimationData AnimationData;

    // Motion Matching Database para diferentes estados
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TMap<EPlayerMovementState, class UPoseSearchDatabase*> MovementDatabases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TMap<EPlayerEmotionalState, class UPoseSearchDatabase*> EmotionalDatabases;

    // Configurações de IK
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float FootIKTraceDistance = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float FootIKInterpSpeed = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FName LeftFootBoneName = "foot_l";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FName RightFootBoneName = "foot_r";

    // Sistema de transições suaves
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    float StateTransitionSpeed = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    float EmotionalTransitionSpeed = 2.0f;

private:
    // Funções de atualização
    void UpdateMovementData(float DeltaTime);
    void UpdateEmotionalState(float DeltaTime);
    void UpdateFootIK(float DeltaTime);
    
    // Funções auxiliares de IK
    FVector PerformFootTrace(const FName& FootBoneName, float TraceDistance) const;
    void CalculateFootIKOffset(const FName& FootBoneName, FVector& OutOffset, FRotator& OutRotation) const;

    // Cache para otimização
    float LastUpdateTime;
    EPlayerMovementState PreviousMovementState;
    EPlayerEmotionalState PreviousEmotionalState;

public:
    // Funções Blueprint
    UFUNCTION(BlueprintCallable, Category = "Player Animation")
    void SetEmotionalState(EPlayerEmotionalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Player Animation")
    void SetFearLevel(float NewFearLevel);

    UFUNCTION(BlueprintCallable, Category = "Player Animation")
    void TriggerEmotionalResponse(EPlayerEmotionalState ResponseState, float Duration = 3.0f);

    UFUNCTION(BlueprintPure, Category = "Player Animation")
    FPlayerAnimationData GetCurrentAnimationData() const { return AnimationData; }

    UFUNCTION(BlueprintPure, Category = "Player Animation")
    bool ShouldUseEmotionalOverride() const;
};