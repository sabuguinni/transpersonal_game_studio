#include "Anim_PrehistoricAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimMontage.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

UAnim_PrehistoricAnimInstance::UAnim_PrehistoricAnimInstance()
{
    // Configurações padrão
    WalkSpeedThreshold = 150.0f;
    RunSpeedThreshold = 400.0f;
    DirectionSmoothingSpeed = 10.0f;
    
    // Inicializar variáveis de controlo
    LastDirection = 0.0f;
    DirectionChangeSpeed = 0.0f;
    bWasInAir = false;
    TimeInCurrentState = 0.0f;
    
    // Inicializar referências como nulas
    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
    
    // Inicializar assets como nulos
    LocomotionBlendSpace = nullptr;
    CrouchBlendSpace = nullptr;
    JumpMontage = nullptr;
    GatherMontage = nullptr;
    CraftMontage = nullptr;
    EatMontage = nullptr;
    DrinkMontage = nullptr;
}

void UAnim_PrehistoricAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Obter referências do owner
    OwnerCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        UE_LOG(LogTemp, Log, TEXT("Anim_PrehistoricAnimInstance: Inicializado para %s"), *OwnerCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Anim_PrehistoricAnimInstance: Falha ao obter OwnerCharacter"));
    }
}

void UAnim_PrehistoricAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Actualizar todos os dados de animação
    UpdateMovementData(DeltaTimeX);
    UpdateMovementState();
    UpdateActionState();
    UpdateSurvivalData();
    
    TimeInCurrentState += DeltaTimeX;
}

void UAnim_PrehistoricAnimInstance::UpdateMovementData(float DeltaTime)
{
    if (!MovementComponent)
    {
        return;
    }
    
    // Calcular velocidade
    FVector Velocity = MovementComponent->Velocity;
    AnimData.Speed = Velocity.Size();
    
    // Calcular direcção (relativa ao forward do character)
    if (AnimData.Speed > 0.1f)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector RightVector = OwnerCharacter->GetActorRightVector();
        
        FVector NormalizedVelocity = Velocity.GetSafeNormal();
        
        float ForwardDot = FVector::DotProduct(NormalizedVelocity, ForwardVector);
        float RightDot = FVector::DotProduct(NormalizedVelocity, RightVector);
        
        float TargetDirection = UKismetMathLibrary::Atan2(RightDot, ForwardDot) * 180.0f / PI;
        
        // Suavizar mudanças de direcção
        AnimData.Direction = UKismetMathLibrary::FInterpTo(AnimData.Direction, TargetDirection, DeltaTime, DirectionSmoothingSpeed);
    }
    
    // Estado de ar
    bWasInAir = AnimData.bIsInAir;
    AnimData.bIsInAir = MovementComponent->IsFalling();
    
    // Estado de crouch
    AnimData.bIsCrouching = MovementComponent->IsCrouching();
}

void UAnim_PrehistoricAnimInstance::UpdateMovementState()
{
    EAnim_PrehistoricMovementState NewState = CalculateMovementState();
    
    if (NewState != AnimData.MovementState)
    {
        AnimData.MovementState = NewState;
        TimeInCurrentState = 0.0f;
        
        // Log da mudança de estado
        UE_LOG(LogTemp, Log, TEXT("Anim_PrehistoricAnimInstance: Estado mudou para %d"), (int32)NewState);
    }
}

EAnim_PrehistoricMovementState UAnim_PrehistoricAnimInstance::CalculateMovementState() const
{
    if (!MovementComponent)
    {
        return EAnim_PrehistoricMovementState::Idle;
    }
    
    // Verificar se está no ar
    if (AnimData.bIsInAir)
    {
        if (MovementComponent->Velocity.Z > 0.0f)
        {
            return EAnim_PrehistoricMovementState::Jumping;
        }
        else
        {
            return EAnim_PrehistoricMovementState::Falling;
        }
    }
    
    // Verificar se está agachado
    if (AnimData.bIsCrouching)
    {
        return EAnim_PrehistoricMovementState::Crouching;
    }
    
    // Estados baseados na velocidade
    if (AnimData.Speed < 10.0f)
    {
        return EAnim_PrehistoricMovementState::Idle;
    }
    else if (AnimData.Speed < RunSpeedThreshold)
    {
        return EAnim_PrehistoricMovementState::Walking;
    }
    else
    {
        return EAnim_PrehistoricMovementState::Running;
    }
}

void UAnim_PrehistoricAnimInstance::UpdateActionState()
{
    // Por agora, manter o estado actual
    // Esta função será expandida quando tivermos sistemas de acção
}

void UAnim_PrehistoricAnimInstance::UpdateSurvivalData()
{
    // Valores padrão por agora
    // Estes serão conectados ao sistema de sobrevivência mais tarde
    AnimData.HealthPercentage = 1.0f;
    AnimData.StaminaPercentage = 1.0f;
    
    // Fear level baseado na velocidade (placeholder)
    if (AnimData.Speed > RunSpeedThreshold)
    {
        AnimData.FearLevel = FMath::Min(AnimData.FearLevel + 0.01f, 1.0f);
    }
    else
    {
        AnimData.FearLevel = FMath::Max(AnimData.FearLevel - 0.005f, 0.0f);
    }
}

void UAnim_PrehistoricAnimInstance::PlayActionMontage(EAnim_PrehistoricActionState ActionType)
{
    UAnimMontage* MontageToPlay = GetMontageForAction(ActionType);
    
    if (MontageToPlay)
    {
        if (!Montage_IsPlaying(MontageToPlay))
        {
            Montage_Play(MontageToPlay);
            AnimData.ActionState = ActionType;
            
            UE_LOG(LogTemp, Log, TEXT("Anim_PrehistoricAnimInstance: A reproduzir montage para acção %d"), (int32)ActionType);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Anim_PrehistoricAnimInstance: Montage não encontrado para acção %d"), (int32)ActionType);
    }
}

void UAnim_PrehistoricAnimInstance::StopActionMontage()
{
    if (IsAnyMontagePlaying())
    {
        Montage_Stop(0.2f);
        AnimData.ActionState = EAnim_PrehistoricActionState::None;
        
        UE_LOG(LogTemp, Log, TEXT("Anim_PrehistoricAnimInstance: Montage parado"));
    }
}

void UAnim_PrehistoricAnimInstance::SetFearLevel(float NewFearLevel)
{
    AnimData.FearLevel = FMath::Clamp(NewFearLevel, 0.0f, 1.0f);
}

UAnimMontage* UAnim_PrehistoricAnimInstance::GetMontageForAction(EAnim_PrehistoricActionState ActionType) const
{
    switch (ActionType)
    {
        case EAnim_PrehistoricActionState::Gathering:
            return GatherMontage;
        case EAnim_PrehistoricActionState::Crafting:
            return CraftMontage;
        case EAnim_PrehistoricActionState::Eating:
            return EatMontage;
        case EAnim_PrehistoricActionState::Drinking:
            return DrinkMontage;
        case EAnim_PrehistoricActionState::Jumping:
            return JumpMontage;
        default:
            return nullptr;
    }
}