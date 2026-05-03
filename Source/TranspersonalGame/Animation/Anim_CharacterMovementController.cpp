#include "Anim_CharacterMovementController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimMontage.h"
#include "Engine/Engine.h"

UAnim_CharacterMovementController::UAnim_CharacterMovementController()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Valores padrão
    WalkSpeedThreshold = 100.0f;
    RunSpeedThreshold = 400.0f;
    StateChangeDelay = 0.1f;
    
    // Estado inicial
    CurrentMovementState = EAnim_MovementState::Idle;
    PreviousMovementState = EAnim_MovementState::Idle;
    StateChangeTimer = 0.0f;
    
    // Componentes serão cached no BeginPlay
    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
    MeshComponent = nullptr;
    AnimInstance = nullptr;
    
    // Assets de animação (serão definidos no Blueprint ou via código)
    IdleAnimation = nullptr;
    WalkAnimation = nullptr;
    RunAnimation = nullptr;
    JumpMontage = nullptr;
    CrouchIdleAnimation = nullptr;
    CrouchWalkAnimation = nullptr;
}

void UAnim_CharacterMovementController::BeginPlay()
{
    Super::BeginPlay();
    
    CacheComponents();
    
    if (OwnerCharacter)
    {
        UE_LOG(LogTemp, Log, TEXT("Anim_CharacterMovementController: Initialized for character %s"), *OwnerCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Anim_CharacterMovementController: Failed to find owner character"));
    }
}

void UAnim_CharacterMovementController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Actualizar timer de mudança de estado
    StateChangeTimer += DeltaTime;
    
    // Actualizar estado de movimento
    UpdateMovementState();
}

void UAnim_CharacterMovementController::CacheComponents()
{
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        MeshComponent = OwnerCharacter->GetMesh();
        
        if (MeshComponent)
        {
            AnimInstance = MeshComponent->GetAnimInstance();
        }
    }
}

void UAnim_CharacterMovementController::UpdateMovementState()
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Calcular novo estado baseado nas condições actuais
    EAnim_MovementState NewState = CalculateMovementState();
    
    // Só mudar se passou tempo suficiente desde a última mudança
    if (NewState != CurrentMovementState && StateChangeTimer >= StateChangeDelay)
    {
        PreviousMovementState = CurrentMovementState;
        CurrentMovementState = NewState;
        StateChangeTimer = 0.0f;
        
        OnMovementStateChanged(NewState);
    }
}

UAnim_CharacterMovementController::EAnim_MovementState UAnim_CharacterMovementController::CalculateMovementState()
{
    if (!MovementComponent)
    {
        return EAnim_MovementState::Idle;
    }
    
    // Verificar se está no ar
    if (MovementComponent->IsFalling())
    {
        // Distinguir entre jump e fall baseado na velocidade vertical
        if (MovementComponent->Velocity.Z > 0.0f)
        {
            return EAnim_MovementState::Jumping;
        }
        else
        {
            return EAnim_MovementState::Falling;
        }
    }
    
    // Verificar se está agachado
    bool bIsCrouching = MovementComponent->IsCrouching();
    
    // Calcular velocidade horizontal
    FVector HorizontalVelocity = MovementComponent->Velocity;
    HorizontalVelocity.Z = 0.0f;
    float Speed = HorizontalVelocity.Size();
    
    // Determinar estado baseado na velocidade e postura
    if (Speed < WalkSpeedThreshold)
    {
        return bIsCrouching ? EAnim_MovementState::Crouching : EAnim_MovementState::Idle;
    }
    else if (Speed < RunSpeedThreshold)
    {
        return bIsCrouching ? EAnim_MovementState::CrouchWalk : EAnim_MovementState::Walking;
    }
    else
    {
        // Não pode correr agachado
        return bIsCrouching ? EAnim_MovementState::CrouchWalk : EAnim_MovementState::Running;
    }
}

void UAnim_CharacterMovementController::OnMovementStateChanged(EAnim_MovementState NewState)
{
    UE_LOG(LogTemp, Log, TEXT("Movement state changed to: %d"), (int32)NewState);
    
    // Aqui seria onde activaríamos as animações específicas
    // Por agora apenas fazemos log para debug
    UpdateAnimationState();
}

void UAnim_CharacterMovementController::UpdateAnimationState()
{
    if (!AnimInstance)
    {
        return;
    }
    
    // Esta função seria expandida para aplicar as animações correctas
    // baseadas no CurrentMovementState
    
    switch (CurrentMovementState)
    {
        case EAnim_MovementState::Idle:
            // Aplicar animação idle
            break;
            
        case EAnim_MovementState::Walking:
            // Aplicar animação walk
            break;
            
        case EAnim_MovementState::Running:
            // Aplicar animação run
            break;
            
        case EAnim_MovementState::Jumping:
            // Trigger jump montage
            PlayJumpAnimation();
            break;
            
        case EAnim_MovementState::Falling:
            // Aplicar animação falling
            break;
            
        case EAnim_MovementState::Crouching:
            // Aplicar animação crouch idle
            break;
            
        case EAnim_MovementState::CrouchWalk:
            // Aplicar animação crouch walk
            break;
    }
}

void UAnim_CharacterMovementController::PlayJumpAnimation()
{
    if (JumpMontage && AnimInstance)
    {
        AnimInstance->Montage_Play(JumpMontage);
        UE_LOG(LogTemp, Log, TEXT("Playing jump animation"));
    }
}

void UAnim_CharacterMovementController::SetCrouchState(bool bIsCrouching)
{
    if (OwnerCharacter)
    {
        if (bIsCrouching)
        {
            OwnerCharacter->Crouch();
        }
        else
        {
            OwnerCharacter->UnCrouch();
        }
    }
}

float UAnim_CharacterMovementController::GetCurrentSpeed() const
{
    if (MovementComponent)
    {
        FVector HorizontalVelocity = MovementComponent->Velocity;
        HorizontalVelocity.Z = 0.0f;
        return HorizontalVelocity.Size();
    }
    return 0.0f;
}

bool UAnim_CharacterMovementController::IsInAir() const
{
    return MovementComponent ? MovementComponent->IsFalling() : false;
}

bool UAnim_CharacterMovementController::IsCrouching() const
{
    return MovementComponent ? MovementComponent->IsCrouching() : false;
}

FVector UAnim_CharacterMovementController::GetVelocity() const
{
    return MovementComponent ? MovementComponent->Velocity : FVector::ZeroVector;
}