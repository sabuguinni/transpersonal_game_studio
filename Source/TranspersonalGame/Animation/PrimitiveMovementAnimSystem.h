#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../SharedTypes.h"
#include "PrimitiveMovementAnimSystem.generated.h"

// Enum para estados de movimento primitivo
UENUM(BlueprintType)
enum class EAnim_PrimitiveMovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Landing     UMETA(DisplayName = "Landing"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Crawling    UMETA(DisplayName = "Crawling")
};

// Struct para configuração de animações
USTRUCT(BlueprintType)
struct FAnim_MovementAnimConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimSequence* IdleAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UBlendSpace* WalkRunBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimMontage* JumpMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimSequence* FallAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimMontage* LandMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimSequence* CrouchIdleAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimSequence* CrawlAnimation;

    FAnim_MovementAnimConfig()
    {
        IdleAnimation = nullptr;
        WalkRunBlendSpace = nullptr;
        JumpMontage = nullptr;
        FallAnimation = nullptr;
        LandMontage = nullptr;
        CrouchIdleAnimation = nullptr;
        CrawlAnimation = nullptr;
    }
};

/**
 * Sistema de animação para movimento primitivo de personagens
 * Gere transições entre estados de movimento básicos
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPrimitiveMovementAnimSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UPrimitiveMovementAnimSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Estado actual de movimento
    UPROPERTY(BlueprintReadOnly, Category = "Movement Animation")
    EAnim_PrimitiveMovementState CurrentMovementState;

    // Estado anterior para transições
    UPROPERTY(BlueprintReadOnly, Category = "Movement Animation")
    EAnim_PrimitiveMovementState PreviousMovementState;

    // Configuração de animações
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Config")
    FAnim_MovementAnimConfig AnimConfig;

    // Velocidade actual do personagem
    UPROPERTY(BlueprintReadOnly, Category = "Movement Animation")
    float CurrentSpeed;

    // Direcção de movimento
    UPROPERTY(BlueprintReadOnly, Category = "Movement Animation")
    float MovementDirection;

    // Se está no ar
    UPROPERTY(BlueprintReadOnly, Category = "Movement Animation")
    bool bIsInAir;

    // Se está agachado
    UPROPERTY(BlueprintReadOnly, Category = "Movement Animation")
    bool bIsCrouching;

    // Tempo no estado actual
    UPROPERTY(BlueprintReadOnly, Category = "Movement Animation")
    float TimeInCurrentState;

    // Limites de velocidade para transições
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Thresholds")
    float WalkThreshold = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Thresholds")
    float RunThreshold = 300.0f;

    // Métodos públicos
    UFUNCTION(BlueprintCallable, Category = "Movement Animation")
    void UpdateMovementState();

    UFUNCTION(BlueprintCallable, Category = "Movement Animation")
    void ForceState(EAnim_PrimitiveMovementState NewState);

    UFUNCTION(BlueprintCallable, Category = "Movement Animation")
    bool CanTransitionToState(EAnim_PrimitiveMovementState TargetState) const;

    UFUNCTION(BlueprintCallable, Category = "Movement Animation")
    void PlayJumpAnimation();

    UFUNCTION(BlueprintCallable, Category = "Movement Animation")
    void PlayLandAnimation();

    UFUNCTION(BlueprintPure, Category = "Movement Animation")
    float GetBlendSpaceValue() const;

    UFUNCTION(BlueprintPure, Category = "Movement Animation")
    bool ShouldPlayIdleAnimation() const;

    UFUNCTION(BlueprintPure, Category = "Movement Animation")
    bool ShouldPlayMovementAnimation() const;

protected:
    // Referências de componentes
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    UPROPERTY()
    class USkeletalMeshComponent* MeshComponent;

    // Métodos internos
    void InitializeComponents();
    void UpdateMovementData();
    EAnim_PrimitiveMovementState DetermineNewState() const;
    void OnStateChanged(EAnim_PrimitiveMovementState OldState, EAnim_PrimitiveMovementState NewState);
    void HandleStateTransition();

    // Validação de transições
    bool IsValidTransition(EAnim_PrimitiveMovementState From, EAnim_PrimitiveMovementState To) const;
    
    // Tempo mínimo em cada estado
    float GetMinimumStateTime(EAnim_PrimitiveMovementState State) const;
};