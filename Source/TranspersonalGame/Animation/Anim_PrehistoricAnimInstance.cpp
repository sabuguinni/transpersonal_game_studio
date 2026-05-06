#include "Anim_PrehistoricAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Animation/AnimMontage.h"

UAnim_PrehistoricAnimInstance::UAnim_PrehistoricAnimInstance()
{
    // Configurações padrão de movimento
    WalkSpeedThreshold = 150.0f;
    RunSpeedThreshold = 400.0f;
    DirectionSmoothingSpeed = 10.0f;
    
    // Inicializar variáveis de controlo
    LastDirection = 0.0f;
    DirectionChangeSpeed = 0.0f;
    bWasInAir = false;
    TimeInCurrentState = 0.0f;
    
    // Inicializar referências como null
    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
    
    // Inicializar assets como null
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
    
    // Obter referência para o personagem dono
    OwnerCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        UE_LOG(LogTemp, Log, TEXT("PrehistoricAnimInstance: Inicializado para %s"), *OwnerCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("PrehistoricAnimInstance: Falha ao obter referência do Character"));
    }
}

void UAnim_PrehistoricAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Atualizar todos os dados de animação
    UpdateMovementData(DeltaTimeX);
    UpdateMovementState();
    UpdateActionState();
    UpdateSurvivalData();
    
    // Incrementar tempo no estado atual
    TimeInCurrentState += DeltaTimeX;
}

void UAnim_PrehistoricAnimInstance::UpdateMovementData(float DeltaTime)
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Calcular velocidade
    FVector Velocity = MovementComponent->Velocity;
    AnimData.Speed = Velocity.Size();
    
    // Calcular direção de movimento
    if (AnimData.Speed > 0.1f)
    {
        FVector Forward = OwnerCharacter->GetActorForwardVector();
        FVector MovementDirection = Velocity.GetSafeNormal();
        
        float DotProduct = FVector::DotProduct(Forward, MovementDirection);
        float CrossProduct = FVector::CrossProduct(Forward, MovementDirection).Z;
        
        float TargetDirection = FMath::Atan2(CrossProduct, DotProduct) * (180.0f / PI);
        
        // Suavizar mudanças de direção
        AnimData.Direction = FMath::FInterpTo(AnimData.Direction, TargetDirection, DeltaTime, DirectionSmoothingSpeed);
        
        // Calcular velocidade de mudança de direção
        DirectionChangeSpeed = FMath::Abs(AnimData.Direction - LastDirection) / DeltaTime;
        LastDirection = AnimData.Direction;
    }
    else
    {
        AnimData.Direction = FMath::FInterpTo(AnimData.Direction, 0.0f, DeltaTime, DirectionSmoothingSpeed * 2.0f);
        DirectionChangeSpeed = 0.0f;
    }
    
    // Estado no ar
    AnimData.bIsInAir = MovementComponent->IsFalling();
    
    // Estado agachado
    AnimData.bIsCrouching = MovementComponent->IsCrouching();
}

void UAnim_PrehistoricAnimInstance::UpdateMovementState()
{
    EAnim_PrehistoricMovementState NewState = CalculateMovementState();
    
    // Se mudou de estado, resetar timer
    if (NewState != AnimData.MovementState)
    {
        TimeInCurrentState = 0.0f;
        
        // Log para debug
        UE_LOG(LogTemp, Log, TEXT("PrehistoricAnimInstance: Estado mudou para %d"), (int32)NewState);
    }
    
    AnimData.MovementState = NewState;
}

EAnim_PrehistoricMovementState UAnim_PrehistoricAnimInstance::CalculateMovementState() const
{
    if (!MovementComponent)
    {
        return EAnim_PrehistoricMovementState::Idle;
    }
    
    // Prioridade: estados especiais primeiro
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
    
    if (AnimData.bIsCrouching)
    {
        return EAnim_PrehistoricMovementState::Crouching;
    }
    
    if (MovementComponent->IsSwimming())
    {
        return EAnim_PrehistoricMovementState::Swimming;
    }
    
    // Estados de movimento baseados na velocidade
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
    // Por agora, manter estado atual
    // Esta função será expandida quando tivermos sistema de ações
}

void UAnim_PrehistoricAnimInstance::UpdateSurvivalData()
{
    // Tentar obter dados de sobrevivência do personagem
    // Por agora, usar valores padrão
    AnimData.HealthPercentage = 1.0f;
    AnimData.StaminaPercentage = 1.0f;
    AnimData.FearLevel = 0.0f;
    
    // TODO: Integrar com sistema de sobrevivência quando disponível
}

void UAnim_PrehistoricAnimInstance::PlayActionMontage(EAnim_PrehistoricActionState ActionType)
{
    UAnimMontage* MontageToPlay = GetMontageForAction(ActionType);
    
    if (MontageToPlay)
    {
        // Parar montage atual se estiver a tocar
        if (GetCurrentActiveMontage())
        {
            Montage_Stop(0.2f);
        }
        
        // Tocar novo montage
        Montage_Play(MontageToPlay, 1.0f);
        AnimData.ActionState = ActionType;
        
        UE_LOG(LogTemp, Log, TEXT("PrehistoricAnimInstance: A tocar montage para ação %d"), (int32)ActionType);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("PrehistoricAnimInstance: Montage não encontrado para ação %d"), (int32)ActionType);
    }
}

void UAnim_PrehistoricAnimInstance::StopActionMontage()
{
    if (GetCurrentActiveMontage())
    {
        Montage_Stop(0.2f);
        AnimData.ActionState = EAnim_PrehistoricActionState::None;
        
        UE_LOG(LogTemp, Log, TEXT("PrehistoricAnimInstance: Montage parado"));
    }
}

void UAnim_PrehistoricAnimInstance::SetFearLevel(float NewFearLevel)
{
    AnimData.FearLevel = FMath::Clamp(NewFearLevel, 0.0f, 1.0f);
    
    // Ajustar animações baseado no nível de medo
    if (AnimData.FearLevel > 0.7f)
    {
        // Medo alto: movimentos mais rápidos e nervosos
        DirectionSmoothingSpeed = 15.0f;
    }
    else if (AnimData.FearLevel > 0.3f)
    {
        // Medo médio: movimentos cautelosos
        DirectionSmoothingSpeed = 8.0f;
    }
    else
    {
        // Calmo: movimentos normais
        DirectionSmoothingSpeed = 10.0f;
    }
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
        case EAnim_PrehistoricActionState::Combat:
            // TODO: Adicionar montages de combate
            return nullptr;
        default:
            return nullptr;
    }
}