#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Anim_CharacterMovementController.generated.h"

/**
 * Controlador de animações de movimento para o TranspersonalCharacter
 * Gere transições entre idle, walk, run, jump, crouch baseado no estado do movimento
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_CharacterMovementController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_CharacterMovementController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Estados de movimento
    UENUM(BlueprintType)
    enum class EAnim_MovementState : uint8
    {
        Idle        UMETA(DisplayName = "Idle"),
        Walking     UMETA(DisplayName = "Walking"),
        Running     UMETA(DisplayName = "Running"),
        Jumping     UMETA(DisplayName = "Jumping"),
        Falling     UMETA(DisplayName = "Falling"),
        Crouching   UMETA(DisplayName = "Crouching"),
        CrouchWalk  UMETA(DisplayName = "Crouch Walking")
    };

    // Estado actual do movimento
    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    EAnim_MovementState CurrentMovementState;

    // Velocidades para transições
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float WalkSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float RunSpeedThreshold;

    // Referências de animação
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UAnimSequence* IdleAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UAnimSequence* WalkAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UAnimSequence* RunAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UAnimMontage* JumpMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UAnimSequence* CrouchIdleAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UAnimSequence* CrouchWalkAnimation;

    // Componentes cached
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    UPROPERTY()
    class USkeletalMeshComponent* MeshComponent;

    UPROPERTY()
    class UAnimInstance* AnimInstance;

    // Métodos públicos
    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void UpdateMovementState();

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void PlayJumpAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void SetCrouchState(bool bIsCrouching);

    UFUNCTION(BlueprintPure, Category = "Animation State")
    float GetCurrentSpeed() const;

    UFUNCTION(BlueprintPure, Category = "Animation State")
    bool IsInAir() const;

    UFUNCTION(BlueprintPure, Category = "Animation State")
    bool IsCrouching() const;

    UFUNCTION(BlueprintPure, Category = "Animation State")
    FVector GetVelocity() const;

private:
    // Estado anterior para detectar mudanças
    EAnim_MovementState PreviousMovementState;

    // Timer para evitar mudanças muito rápidas
    float StateChangeTimer;

    UPROPERTY(EditAnywhere, Category = "Animation Settings", meta = (AllowPrivateAccess = "true"))
    float StateChangeDelay;

    // Métodos privados
    void CacheComponents();
    void UpdateAnimationState();
    EAnim_MovementState CalculateMovementState();
    void OnMovementStateChanged(EAnim_MovementState NewState);
};