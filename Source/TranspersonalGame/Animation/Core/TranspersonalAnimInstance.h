#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "TranspersonalAnimationTypes.h"
#include "Components/ActorComponent.h"
#include "TranspersonalAnimInstance.generated.h"

class UCharacterMovementComponent;
class UTranspersonalEmotionalStateComponent;

/**
 * Animation Instance principal do jogo
 * Implementa Motion Matching com contexto emocional e IK de pés
 * 
 * Filosofia: Cada movimento conta uma história. O protagonista não é um herói —
 * é um cientista perdido no tempo, sempre em estado de alerta.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UTranspersonalAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UTranspersonalAnimInstance();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;
    virtual void NativeThreadSafeUpdateAnimation(float DeltaTimeX) override;

    // === MOTION MATCHING CORE ===
    
    /** Database de poses para Motion Matching */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UPoseSearchDatabase* CurrentPoseDatabase;

    /** Configuração atual de Motion Matching */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FMotionMatchingConfig MotionMatchingConfig;

    /** Peso do blend atual entre animações */
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    float CurrentBlendWeight;

    // === DADOS DE MOVIMENTO ===
    
    /** Velocidade atual do personagem */
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FVector Velocity;

    /** Velocidade 2D (sem Z) */
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FVector2D Velocity2D;

    /** Velocidade escalar */
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Speed;

    /** Direção do movimento normalizada */
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FVector MovementDirection;

    /** Ângulo de rotação do movimento */
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float MovementAngle;

    /** Se o personagem está no chão */
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsOnGround;

    /** Se o personagem está a mover-se */
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsMoving;

    // === ESTADO EMOCIONAL ===
    
    /** Estado emocional atual */
    UPROPERTY(BlueprintReadOnly, Category = "Emotional State")
    EEmotionalState CurrentEmotionalState;

    /** Intensidade do medo (0-1) */
    UPROPERTY(BlueprintReadOnly, Category = "Emotional State")
    float FearIntensity;

    /** Nível de fadiga (0-1) */
    UPROPERTY(BlueprintReadOnly, Category = "Emotional State")
    float FatigueLevel;

    /** Modificador de tremor por nervosismo */
    UPROPERTY(BlueprintReadOnly, Category = "Emotional State")
    float NervousnessTremor;

    // === IK DE PÉS ===
    
    /** Configuração do pé esquerdo */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FFootIKConfig LeftFootConfig;

    /** Configuração do pé direito */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FFootIKConfig RightFootConfig;

    /** Offset atual do pé esquerdo */
    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    float LeftFootOffset;

    /** Offset atual do pé direito */
    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    float RightFootOffset;

    /** Rotação do pé esquerdo para alinhar com o terreno */
    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FRotator LeftFootRotation;

    /** Rotação do pé direito para alinhar com o terreno */
    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FRotator RightFootRotation;

    /** Offset da pelvis para manter equilíbrio */
    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    float PelvisOffset;

    // === PERFIL DE PERSONAGEM ===
    
    /** Perfil de animação único deste personagem */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Profile")
    FCharacterAnimationProfile AnimationProfile;

    // === COMPONENTES REFERENCIADOS ===
    
    /** Referência ao componente de movimento */
    UPROPERTY(BlueprintReadOnly, Category = "References")
    UCharacterMovementComponent* MovementComponent;

    /** Referência ao componente de estado emocional */
    UPROPERTY(BlueprintReadOnly, Category = "References")
    UTranspersonalEmotionalStateComponent* EmotionalStateComponent;

    /** Referência ao pawn owner */
    UPROPERTY(BlueprintReadOnly, Category = "References")
    APawn* OwnerPawn;

private:
    // === MÉTODOS INTERNOS ===
    
    /** Atualiza dados de movimento */
    void UpdateMovementData(float DeltaTime);
    
    /** Atualiza estado emocional */
    void UpdateEmotionalState(float DeltaTime);
    
    /** Atualiza IK de pés */
    void UpdateFootIK(float DeltaTime);
    
    /** Calcula offset de um pé específico */
    float CalculateFootOffset(const FFootIKConfig& FootConfig, FRotator& OutFootRotation);
    
    /** Aplica modificadores do perfil de personagem */
    void ApplyCharacterProfileModifiers();
    
    /** Seleciona database de poses baseado no contexto */
    void SelectPoseDatabase();

    // === DADOS INTERNOS ===
    
    /** Último estado emocional para detectar mudanças */
    EEmotionalState LastEmotionalState;
    
    /** Timer para mudanças de estado */
    float StateChangeTimer;
    
    /** Valores interpolados para suavizar transições */
    float SmoothedFearIntensity;
    float SmoothedFatigueLevel;

public:
    // === MÉTODOS PÚBLICOS BLUEPRINT ===
    
    /** Força mudança de estado emocional */
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetEmotionalState(EEmotionalState NewState, float Intensity = 1.0f);
    
    /** Obtém o modificador atual de velocidade baseado no estado */
    UFUNCTION(BlueprintPure, Category = "Animation")
    float GetCurrentSpeedModifier() const;
    
    /** Obtém o modificador atual de postura */
    UFUNCTION(BlueprintPure, Category = "Animation")
    float GetCurrentPostureModifier() const;
    
    /** Verifica se deve aplicar tremor de nervosismo */
    UFUNCTION(BlueprintPure, Category = "Animation")
    bool ShouldApplyNervousnessTremor() const;
    
    /** Obtém a intensidade do tremor atual */
    UFUNCTION(BlueprintPure, Category = "Animation")
    float GetTremorIntensity() const;
};