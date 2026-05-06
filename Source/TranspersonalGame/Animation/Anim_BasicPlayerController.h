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
#include "Anim_BasicPlayerController.generated.h"

// Estados básicos de movimento para o jogador
UENUM(BlueprintType)
enum class EAnim_PlayerMovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Climbing    UMETA(DisplayName = "Climbing")
};

// Estados de acção para sobrevivência
UENUM(BlueprintType)
enum class EAnim_PlayerActionState : uint8
{
    None            UMETA(DisplayName = "None"),
    Gathering       UMETA(DisplayName = "Gathering"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Combat          UMETA(DisplayName = "Combat"),
    Eating          UMETA(DisplayName = "Eating"),
    Drinking        UMETA(DisplayName = "Drinking"),
    Building        UMETA(DisplayName = "Building"),
    Sleeping        UMETA(DisplayName = "Sleeping")
};

// Dados de animação do jogador
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_PlayerAnimationData
{
    GENERATED_BODY()

    // Estados de movimento
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    EAnim_PlayerMovementState MovementState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    EAnim_PlayerActionState ActionState;

    // Dados de velocidade e direcção
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsMoving;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsInAir;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsCrouching;

    // Dados de sobrevivência que afectam animação
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float HealthPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float StaminaPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float HungerLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float FearLevel;

    // Dados de combate
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsInCombat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsBlocking;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsAttacking;

    FAnim_PlayerAnimationData()
    {
        MovementState = EAnim_PlayerMovementState::Idle;
        ActionState = EAnim_PlayerActionState::None;
        Speed = 0.0f;
        Direction = 0.0f;
        bIsMoving = false;
        bIsInAir = false;
        bIsCrouching = false;
        HealthPercentage = 1.0f;
        StaminaPercentage = 1.0f;
        HungerLevel = 0.0f;
        FearLevel = 0.0f;
        bIsInCombat = false;
        bIsBlocking = false;
        bIsAttacking = false;
    }
};

/**
 * Controlador básico de animação para o jogador
 * Gere os estados de movimento e acção do personagem principal
 * Integra com o sistema de sobrevivência para animações realistas
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_BasicPlayerController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_BasicPlayerController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === CONFIGURAÇÃO ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float WalkSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float RunSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float DirectionSmoothingSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float SpeedSmoothingSpeed;

    // === DADOS DE ANIMAÇÃO ===
    
    UPROPERTY(BlueprintReadOnly, Category = "Animation Data")
    FAnim_PlayerAnimationData AnimationData;

    // === REFERÊNCIAS ===
    
    UPROPERTY(BlueprintReadOnly, Category = "References")
    class ACharacter* OwnerCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    class UCharacterMovementComponent* MovementComponent;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    class USkeletalMeshComponent* MeshComponent;

    // === MÉTODOS PRINCIPAIS ===
    
    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void UpdateMovementState();

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void UpdateActionState(EAnim_PlayerActionState NewActionState);

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void UpdateSurvivalData(float Health, float Stamina, float Hunger, float Fear);

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void SetCombatState(bool bInCombat, bool bBlocking = false, bool bAttacking = false);

    // === GETTERS ===
    
    UFUNCTION(BlueprintPure, Category = "Animation Data")
    EAnim_PlayerMovementState GetMovementState() const { return AnimationData.MovementState; }

    UFUNCTION(BlueprintPure, Category = "Animation Data")
    EAnim_PlayerActionState GetActionState() const { return AnimationData.ActionState; }

    UFUNCTION(BlueprintPure, Category = "Animation Data")
    float GetSpeed() const { return AnimationData.Speed; }

    UFUNCTION(BlueprintPure, Category = "Animation Data")
    float GetDirection() const { return AnimationData.Direction; }

    UFUNCTION(BlueprintPure, Category = "Animation Data")
    bool IsMoving() const { return AnimationData.bIsMoving; }

    UFUNCTION(BlueprintPure, Category = "Animation Data")
    bool IsInAir() const { return AnimationData.bIsInAir; }

private:
    // === MÉTODOS INTERNOS ===
    
    void InitializeReferences();
    void CalculateMovementData();
    void DetermineMovementState();
    void SmoothAnimationValues(float DeltaTime);

    // === VARIÁVEIS INTERNAS ===
    
    float CurrentSpeed;
    float TargetSpeed;
    float CurrentDirection;
    float TargetDirection;
    
    FVector LastFrameVelocity;
    float LastFrameSpeed;
};