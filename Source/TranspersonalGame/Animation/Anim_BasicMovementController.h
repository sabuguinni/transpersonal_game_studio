#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/BlendSpace1D.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Anim_BasicMovementController.generated.h"

UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Crouching   UMETA(DisplayName = "Crouching")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MovementData
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

    FAnim_MovementData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        MovementState = EAnim_MovementState::Idle;
    }
};

/**
 * Controlador básico de animações de movimento para personagens
 * Gere transições entre idle, walk, run, jump e crouch
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_BasicMovementController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_BasicMovementController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Configuração de animações
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UBlendSpace1D* MovementBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* JumpMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* LandMontage;

    // Thresholds de velocidade
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings")
    float WalkThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings")
    float RunThreshold;

    // Dados de movimento actuais
    UPROPERTY(BlueprintReadOnly, Category = "Movement Data")
    FAnim_MovementData CurrentMovementData;

    // Referências
    UPROPERTY(BlueprintReadOnly, Category = "References")
    ACharacter* OwnerCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    UCharacterMovementComponent* MovementComponent;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    UAnimInstance* AnimInstance;

    // Funções públicas
    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void UpdateMovementData();

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void PlayJumpAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void PlayLandAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    EAnim_MovementState GetCurrentMovementState() const;

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    float GetMovementSpeed() const;

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    bool IsMoving() const;

protected:
    // Funções internas
    void InitializeReferences();
    void UpdateMovementState();
    void UpdateBlendSpaceInput();
    EAnim_MovementState CalculateMovementState() const;

private:
    // Estado anterior para detectar mudanças
    EAnim_MovementState PreviousMovementState;
    bool bWasInAir;
};