#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimBlueprint.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../SharedTypes.h"
#include "Anim_SurvivalAnimBlueprint.generated.h"

UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Walking UMETA(DisplayName = "Walking"),
    Running UMETA(DisplayName = "Running"),
    Jumping UMETA(DisplayName = "Jumping"),
    Falling UMETA(DisplayName = "Falling"),
    Crouching UMETA(DisplayName = "Crouching"),
    Crawling UMETA(DisplayName = "Crawling")
};

UENUM(BlueprintType)
enum class EAnim_SurvivalState : uint8
{
    Normal UMETA(DisplayName = "Normal"),
    Tired UMETA(DisplayName = "Tired"),
    Hungry UMETA(DisplayName = "Hungry"),
    Thirsty UMETA(DisplayName = "Thirsty"),
    Injured UMETA(DisplayName = "Injured"),
    Fearful UMETA(DisplayName = "Fearful"),
    Dying UMETA(DisplayName = "Dying")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MovementData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    EAnim_MovementState MovementState = EAnim_MovementState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float GroundDistance = 0.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_SurvivalData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float HealthPercent = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float HungerPercent = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float ThirstPercent = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float StaminaPercent = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float FearLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    EAnim_SurvivalState SurvivalState = EAnim_SurvivalState::Normal;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsExhausted = false;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsInDanger = false;
};

/**
 * Animation Blueprint controller para personagens de sobrevivência pré-histórica
 * Gere transições entre estados de movimento e sobrevivência
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_SurvivalAnimBlueprint : public UAnimBlueprint
{
    GENERATED_BODY()

public:
    UAnim_SurvivalAnimBlueprint();

    // Configuração do Animation Blueprint
    UFUNCTION(BlueprintCallable, Category = "Animation Setup")
    void InitializeAnimationBlueprint(USkeletalMeshComponent* MeshComponent);

    // Actualização de dados de movimento
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void UpdateMovementData(const FVector& Velocity, bool bIsInAir, bool bIsCrouching);

    // Actualização de dados de sobrevivência
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void UpdateSurvivalData(float Health, float Hunger, float Thirst, float Stamina, float Fear);

    // Transições de estado
    UFUNCTION(BlueprintCallable, Category = "State")
    void TransitionToMovementState(EAnim_MovementState NewState);

    UFUNCTION(BlueprintCallable, Category = "State")
    void TransitionToSurvivalState(EAnim_SurvivalState NewState);

    // Getters para Blueprint
    UFUNCTION(BlueprintPure, Category = "Movement")
    FAnim_MovementData GetMovementData() const { return MovementData; }

    UFUNCTION(BlueprintPure, Category = "Survival")
    FAnim_SurvivalData GetSurvivalData() const { return SurvivalData; }

    UFUNCTION(BlueprintPure, Category = "State")
    EAnim_MovementState GetCurrentMovementState() const { return MovementData.MovementState; }

    UFUNCTION(BlueprintPure, Category = "State")
    EAnim_SurvivalState GetCurrentSurvivalState() const { return SurvivalData.SurvivalState; }

    // Configuração de animações
    UFUNCTION(BlueprintCallable, Category = "Animation Config")
    void SetIdleAnimation(class UAnimSequence* IdleAnim);

    UFUNCTION(BlueprintCallable, Category = "Animation Config")
    void SetWalkBlendSpace(class UBlendSpace* WalkBlendSpace);

    UFUNCTION(BlueprintCallable, Category = "Animation Config")
    void SetRunBlendSpace(class UBlendSpace* RunBlendSpace);

    UFUNCTION(BlueprintCallable, Category = "Animation Config")
    void SetJumpMontage(class UAnimMontage* JumpMontage);

protected:
    // Dados de movimento
    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    FAnim_MovementData MovementData;

    // Dados de sobrevivência
    UPROPERTY(BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
    FAnim_SurvivalData SurvivalData;

    // Referências de animação
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Assets")
    class UAnimSequence* IdleAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Assets")
    class UBlendSpace* WalkBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Assets")
    class UBlendSpace* RunBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Assets")
    class UAnimMontage* JumpAnimMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Assets")
    class UAnimMontage* CrouchAnimMontage;

    // Configurações de transição
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Config")
    float WalkThreshold = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Config")
    float RunThreshold = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Config")
    float TransitionSpeed = 5.0f;

    // Estado interno
    UPROPERTY(BlueprintReadOnly, Category = "Internal")
    float LastUpdateTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Internal")
    bool bIsInitialized = false;

private:
    // Métodos internos
    void CalculateMovementState();
    void CalculateSurvivalState();
    void UpdateAnimationBlending(float DeltaTime);
    
    // Referência ao componente mesh
    UPROPERTY()
    TWeakObjectPtr<USkeletalMeshComponent> OwnerMeshComponent;
};