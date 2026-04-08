#include "PlayerAnimationController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UPlayerAnimationController::UPlayerAnimationController()
{
    // Configurações padrão
    FootIKTraceDistance = 50.0f;
    FootIKInterpSpeed = 15.0f;
    StateTransitionSpeed = 5.0f;
    EmotionalTransitionSpeed = 2.0f;
    LastUpdateTime = 0.0f;
    PreviousMovementState = EPlayerMovementState::Idle;
    PreviousEmotionalState = EPlayerEmotionalState::Calm;
}

void UPlayerAnimationController::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Obter referência ao character
    OwningCharacter = Cast<ACharacter>(GetOwningActor());
    
    if (OwningCharacter)
    {
        UE_LOG(LogTemp, Log, TEXT("Player Animation Controller initialized for: %s"), *OwningCharacter->GetName());
    }
}

void UPlayerAnimationController::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwningCharacter)
    {
        return;
    }

    // Atualizar dados de animação
    UpdateMovementData(DeltaTimeX);
    UpdateEmotionalState(DeltaTimeX);
    UpdateFootIK(DeltaTimeX);
    
    LastUpdateTime += DeltaTimeX;
}

void UPlayerAnimationController::UpdateMovementData(float DeltaTime)
{
    if (!OwningCharacter || !OwningCharacter->GetCharacterMovement())
    {
        return;
    }

    UCharacterMovementComponent* MovementComp = OwningCharacter->GetCharacterMovement();
    
    // Atualizar dados básicos de movimento
    AnimationData.Velocity = MovementComp->Velocity;
    AnimationData.Speed = AnimationData.Velocity.Size();
    AnimationData.bIsMoving = AnimationData.Speed > 3.0f;
    AnimationData.bIsGrounded = MovementComp->IsMovingOnGround();
    
    // Calcular direção relativa ao character
    if (AnimationData.bIsMoving)
    {
        FVector ForwardVector = OwningCharacter->GetActorForwardVector();
        FVector VelocityNormalized = AnimationData.Velocity.GetSafeNormal();
        AnimationData.Direction = FMath::Atan2(
            FVector::DotProduct(OwningCharacter->GetActorRightVector(), VelocityNormalized),
            FVector::DotProduct(ForwardVector, VelocityNormalized)
        ) * (180.0f / PI);
    }

    // Determinar estado de movimento
    EPlayerMovementState NewMovementState = EPlayerMovementState::Idle;
    
    if (!AnimationData.bIsGrounded)
    {
        if (AnimationData.Velocity.Z > 0)
        {
            NewMovementState = EPlayerMovementState::Jumping;
        }
        else
        {
            NewMovementState = EPlayerMovementState::Falling;
        }
    }
    else if (MovementComp->IsCrouching())
    {
        if (AnimationData.bIsMoving)
        {
            NewMovementState = EPlayerMovementState::Crouching;
        }
        else
        {
            NewMovementState = EPlayerMovementState::Crouching;
        }
    }
    else if (AnimationData.bIsMoving)
    {
        // Determinar se está andando ou correndo baseado na velocidade e stamina
        float WalkThreshold = 200.0f;
        float RunThreshold = 400.0f;
        
        if (AnimationData.Speed < WalkThreshold)
        {
            NewMovementState = EPlayerMovementState::Walking;
        }
        else if (AnimationData.Speed < RunThreshold || AnimationData.StaminaLevel < 0.3f)
        {
            NewMovementState = EPlayerMovementState::Walking;
        }
        else
        {
            NewMovementState = EPlayerMovementState::Running;
        }
    }

    // Aplicar transição suave de estado
    if (NewMovementState != PreviousMovementState)
    {
        AnimationData.MovementState = NewMovementState;
        PreviousMovementState = NewMovementState;
        
        UE_LOG(LogTemp, Log, TEXT("Player movement state changed to: %d"), (int32)NewMovementState);
    }
}

void UPlayerAnimationController::UpdateEmotionalState(float DeltaTime)
{
    // Atualizar estado emocional baseado no contexto do jogo
    // Isto será expandido quando integrarmos com sistemas de gameplay
    
    EPlayerEmotionalState NewEmotionalState = AnimationData.EmotionalState;
    
    // Lógica básica de estado emocional baseado em medo e stamina
    if (AnimationData.FearLevel > 0.8f)
    {
        NewEmotionalState = EPlayerEmotionalState::Panicked;
    }
    else if (AnimationData.FearLevel > 0.5f)
    {
        NewEmotionalState = EPlayerEmotionalState::Afraid;
    }
    else if (AnimationData.FearLevel > 0.2f)
    {
        NewEmotionalState = EPlayerEmotionalState::Nervous;
    }
    else if (AnimationData.StaminaLevel < 0.2f)
    {
        NewEmotionalState = EPlayerEmotionalState::Exhausted;
    }
    else
    {
        NewEmotionalState = EPlayerEmotionalState::Calm;
    }

    // Aplicar transição suave
    if (NewEmotionalState != PreviousEmotionalState)
    {
        AnimationData.EmotionalState = NewEmotionalState;
        PreviousEmotionalState = NewEmotionalState;
        
        UE_LOG(LogTemp, Log, TEXT("Player emotional state changed to: %d"), (int32)NewEmotionalState);
    }
}

void UPlayerAnimationController::UpdateFootIK(float DeltaTime)
{
    if (!AnimationData.bEnableFootIK || !OwningCharacter)
    {
        return;
    }

    // Calcular IK para ambos os pés
    FVector LeftFootOffset;
    FRotator LeftFootRotation;
    CalculateFootIKOffset(LeftFootBoneName, LeftFootOffset, LeftFootRotation);
    
    FVector RightFootOffset;
    FRotator RightFootRotation;
    CalculateFootIKOffset(RightFootBoneName, RightFootOffset, RightFootRotation);

    // Interpolar suavemente para evitar jitter
    AnimationData.LeftFootIKOffset = FMath::VInterpTo(
        AnimationData.LeftFootIKOffset, 
        LeftFootOffset, 
        DeltaTime, 
        FootIKInterpSpeed
    );
    
    AnimationData.RightFootIKOffset = FMath::VInterpTo(
        AnimationData.RightFootIKOffset, 
        RightFootOffset, 
        DeltaTime, 
        FootIKInterpSpeed
    );

    AnimationData.LeftFootIKRotation = FMath::RInterpTo(
        AnimationData.LeftFootIKRotation, 
        LeftFootRotation, 
        DeltaTime, 
        FootIKInterpSpeed
    );
    
    AnimationData.RightFootIKRotation = FMath::RInterpTo(
        AnimationData.RightFootIKRotation, 
        RightFootRotation, 
        DeltaTime, 
        FootIKInterpSpeed
    );
}

void UPlayerAnimationController::CalculateFootIKOffset(const FName& FootBoneName, FVector& OutOffset, FRotator& OutRotation) const
{
    if (!OwningCharacter)
    {
        OutOffset = FVector::ZeroVector;
        OutRotation = FRotator::ZeroRotator;
        return;
    }

    // Obter posição do osso do pé
    FVector FootLocation = GetOwningComponent()->GetBoneLocation(FootBoneName, EBoneSpaces::WorldSpace);
    
    // Fazer trace para baixo para encontrar o chão
    FVector TraceStart = FootLocation;
    FVector TraceEnd = FootLocation - FVector(0, 0, FootIKTraceDistance);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwningCharacter);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_WorldStatic,
        QueryParams
    );

    if (bHit)
    {
        // Calcular offset necessário
        float DistanceToGround = (TraceStart - HitResult.Location).Z;
        OutOffset = FVector(0, 0, DistanceToGround - FootIKTraceDistance);
        
        // Calcular rotação baseada na normal da superfície
        FVector UpVector = FVector::UpVector;
        FVector ForwardVector = OwningCharacter->GetActorForwardVector();
        FVector RightVector = FVector::CrossProduct(HitResult.Normal, ForwardVector).GetSafeNormal();
        ForwardVector = FVector::CrossProduct(RightVector, HitResult.Normal).GetSafeNormal();
        
        OutRotation = UKismetMathLibrary::MakeRotationFromAxes(ForwardVector, RightVector, HitResult.Normal);
    }
    else
    {
        OutOffset = FVector::ZeroVector;
        OutRotation = FRotator::ZeroRotator;
    }
}

void UPlayerAnimationController::SetEmotionalState(EPlayerEmotionalState NewState)
{
    AnimationData.EmotionalState = NewState;
    UE_LOG(LogTemp, Log, TEXT("Player emotional state manually set to: %d"), (int32)NewState);
}

void UPlayerAnimationController::SetFearLevel(float NewFearLevel)
{
    AnimationData.FearLevel = FMath::Clamp(NewFearLevel, 0.0f, 1.0f);
}

void UPlayerAnimationController::TriggerEmotionalResponse(EPlayerEmotionalState ResponseState, float Duration)
{
    // Esta função pode ser usada para trigger respostas emocionais temporárias
    // Por exemplo, quando o jogador vê um dinossauro perigoso
    SetEmotionalState(ResponseState);
    
    // Aqui poderíamos implementar um timer para retornar ao estado anterior
    UE_LOG(LogTemp, Log, TEXT("Triggered emotional response: %d for %f seconds"), (int32)ResponseState, Duration);
}

bool UPlayerAnimationController::ShouldUseEmotionalOverride() const
{
    // Determinar se devemos usar override emocional nas animações
    return AnimationData.FearLevel > 0.3f || 
           AnimationData.EmotionalState == EPlayerEmotionalState::Panicked ||
           AnimationData.EmotionalState == EPlayerEmotionalState::Afraid;
}