#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "TranspersonalGame/Characters/TranspersonalCharacter.h"
#include "Anim_CharacterAnimationInstance.generated.h"

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
enum class EAnim_SurvivalState : uint8
{
    Healthy     UMETA(DisplayName = "Healthy"),
    Tired       UMETA(DisplayName = "Tired"),
    Hungry      UMETA(DisplayName = "Hungry"),
    Thirsty     UMETA(DisplayName = "Thirsty"),
    Injured     UMETA(DisplayName = "Injured"),
    Fearful     UMETA(DisplayName = "Fearful"),
    Dying       UMETA(DisplayName = "Dying")
};

/**
 * Animation Instance para o TranspersonalCharacter
 * Gere transições de animação baseadas em movimento e stats de sobrevivência
 */
UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API UAnim_CharacterAnimationInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_CharacterAnimationInstance();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    // === MOVEMENT PROPERTIES ===
    
    /** Velocidade actual do personagem */
    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float Speed;

    /** Direcção do movimento (0-360 graus) */
    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float Direction;

    /** Se o personagem está no ar */
    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    bool bIsInAir;

    /** Se o personagem está agachado */
    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    bool bIsCrouching;

    /** Estado actual de movimento */
    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    EAnim_MovementState MovementState;

    /** Velocidade de rotação do personagem */
    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float TurnRate;

    // === SURVIVAL PROPERTIES ===
    
    /** Estado de sobrevivência actual */
    UPROPERTY(BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
    EAnim_SurvivalState SurvivalState;

    /** Nível de stamina (0.0 - 1.0) */
    UPROPERTY(BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
    float StaminaLevel;

    /** Nível de saúde (0.0 - 1.0) */
    UPROPERTY(BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
    float HealthLevel;

    /** Nível de fome (0.0 - 1.0) */
    UPROPERTY(BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
    float HungerLevel;

    /** Nível de sede (0.0 - 1.0) */
    UPROPERTY(BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
    float ThirstLevel;

    /** Nível de medo (0.0 - 1.0) */
    UPROPERTY(BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
    float FearLevel;

    // === ANIMATION BLENDING ===
    
    /** Peso do blend entre idle e walk */
    UPROPERTY(BlueprintReadOnly, Category = "Animation Blending", meta = (AllowPrivateAccess = "true"))
    float IdleWalkBlend;

    /** Peso do blend entre walk e run */
    UPROPERTY(BlueprintReadOnly, Category = "Animation Blending", meta = (AllowPrivateAccess = "true"))
    float WalkRunBlend;

    /** Multiplicador de velocidade de animação baseado em stamina */
    UPROPERTY(BlueprintReadOnly, Category = "Animation Blending", meta = (AllowPrivateAccess = "true"))
    float AnimationSpeedMultiplier;

    // === CONFIGURATION ===
    
    /** Velocidade mínima para considerar movimento */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float MinMovementSpeed = 10.0f;

    /** Velocidade que separa walk de run */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float WalkToRunThreshold = 300.0f;

    /** Suavização das transições de movimento */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float MovementSmoothingSpeed = 5.0f;

private:
    /** Referência ao personagem */
    UPROPERTY()
    ATranspersonalCharacter* TranspersonalCharacter;

    /** Referência ao movimento do personagem */
    UPROPERTY()
    UCharacterMovementComponent* CharacterMovement;

    /** Velocidade anterior para suavização */
    float PreviousSpeed;

    /** Direcção anterior para suavização */
    float PreviousDirection;

    // === PRIVATE METHODS ===
    
    /** Actualiza propriedades de movimento */
    void UpdateMovementProperties(float DeltaTime);

    /** Actualiza propriedades de sobrevivência */
    void UpdateSurvivalProperties();

    /** Actualiza pesos de blending */
    void UpdateAnimationBlending(float DeltaTime);

    /** Determina o estado de movimento actual */
    EAnim_MovementState DetermineMovementState() const;

    /** Determina o estado de sobrevivência actual */
    EAnim_SurvivalState DetermineSurvivalState() const;

    /** Calcula a direcção do movimento relativa ao personagem */
    float CalculateDirection() const;
};