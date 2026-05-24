#include "Anim_BasicMovementController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/BlendSpace1D.h"
#include "Animation/AnimMontage.h"
#include "Engine/Engine.h"

UAnim_BasicMovementController::UAnim_BasicMovementController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Configurações padrão
    WalkThreshold = 100.0f;
    RunThreshold = 300.0f;
    
    // Estado inicial
    PreviousMovementState = EAnim_MovementState::Idle;
    bWasInAir = false;
    
    // Inicializar dados de movimento
    CurrentMovementData = FAnim_MovementData();
}

void UAnim_BasicMovementController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeReferences();
}

void UAnim_BasicMovementController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (OwnerCharacter && MovementComponent)
    {
        UpdateMovementData();
        UpdateMovementState();
        UpdateBlendSpaceInput();
    }
}

void UAnim_BasicMovementController::InitializeReferences()
{
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        
        USkeletalMeshComponent* SkeletalMesh = OwnerCharacter->GetMesh();
        if (SkeletalMesh)
        {
            AnimInstance = SkeletalMesh->GetAnimInstance();
        }
    }
    
    if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("UAnim_BasicMovementController: Owner is not a Character"));
    }
    
    if (!MovementComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("UAnim_BasicMovementController: No CharacterMovementComponent found"));
    }
    
    if (!AnimInstance)
    {
        UE_LOG(LogTemp, Warning, TEXT("UAnim_BasicMovementController: No AnimInstance found"));
    }
}

void UAnim_BasicMovementController::UpdateMovementData()
{
    if (!MovementComponent || !OwnerCharacter)
    {
        return;
    }
    
    // Actualizar velocidade
    FVector Velocity = MovementComponent->Velocity;
    CurrentMovementData.Speed = Velocity.Size2D();
    
    // Actualizar direcção (relativa ao forward do personagem)
    if (CurrentMovementData.Speed > 0.1f)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector VelocityNormalized = Velocity.GetSafeNormal2D();
        CurrentMovementData.Direction = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ForwardVector, VelocityNormalized)));
        
        // Determinar se é esquerda ou direita
        FVector RightVector = OwnerCharacter->GetActorRightVector();
        float RightDot = FVector::DotProduct(RightVector, VelocityNormalized);
        if (RightDot < 0.0f)
        {
            CurrentMovementData.Direction *= -1.0f;
        }
    }
    else
    {
        CurrentMovementData.Direction = 0.0f;
    }
    
    // Actualizar estado no ar
    CurrentMovementData.bIsInAir = MovementComponent->IsFalling();
    
    // Actualizar estado agachado
    CurrentMovementData.bIsCrouching = MovementComponent->IsCrouching();
}

void UAnim_BasicMovementController::UpdateMovementState()
{
    EAnim_MovementState NewState = CalculateMovementState();
    
    // Detectar mudanças de estado
    if (NewState != PreviousMovementState)
    {
        // Transições especiais
        if (PreviousMovementState != EAnim_MovementState::Jumping && NewState == EAnim_MovementState::Jumping)
        {
            PlayJumpAnimation();
        }
        else if (bWasInAir && !CurrentMovementData.bIsInAir)
        {
            PlayLandAnimation();
        }
        
        PreviousMovementState = CurrentMovementData.MovementState;
    }
    
    CurrentMovementData.MovementState = NewState;
    bWasInAir = CurrentMovementData.bIsInAir;
}

EAnim_MovementState UAnim_BasicMovementController::CalculateMovementState() const
{
    if (!MovementComponent)
    {
        return EAnim_MovementState::Idle;
    }
    
    // Verificar se está no ar
    if (CurrentMovementData.bIsInAir)
    {
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
    if (CurrentMovementData.bIsCrouching)
    {
        return EAnim_MovementState::Crouching;
    }
    
    // Verificar velocidade para determinar idle/walk/run
    if (CurrentMovementData.Speed < 1.0f)
    {
        return EAnim_MovementState::Idle;
    }
    else if (CurrentMovementData.Speed < RunThreshold)
    {
        return EAnim_MovementState::Walking;
    }
    else
    {
        return EAnim_MovementState::Running;
    }
}

void UAnim_BasicMovementController::UpdateBlendSpaceInput()
{
    if (!AnimInstance || !MovementBlendSpace)
    {
        return;
    }
    
    // Actualizar Blend Space com velocidade normalizada
    float BlendSpaceValue = 0.0f;
    
    if (CurrentMovementData.MovementState == EAnim_MovementState::Walking)
    {
        BlendSpaceValue = FMath::GetMappedRangeValueClamped(
            FVector2D(0.0f, RunThreshold), 
            FVector2D(0.0f, 1.0f), 
            CurrentMovementData.Speed
        );
    }
    else if (CurrentMovementData.MovementState == EAnim_MovementState::Running)
    {
        BlendSpaceValue = FMath::GetMappedRangeValueClamped(
            FVector2D(RunThreshold, MovementComponent->MaxWalkSpeed), 
            FVector2D(1.0f, 2.0f), 
            CurrentMovementData.Speed
        );
    }
    
    // Nota: A actualização do Blend Space seria feita no Animation Blueprint
    // Este valor pode ser exposto via Blueprint para uso no AnimBP
}

void UAnim_BasicMovementController::PlayJumpAnimation()
{
    if (AnimInstance && JumpMontage)
    {
        AnimInstance->Montage_Play(JumpMontage);
        UE_LOG(LogTemp, Log, TEXT("Playing Jump Animation"));
    }
}

void UAnim_BasicMovementController::PlayLandAnimation()
{
    if (AnimInstance && LandMontage)
    {
        AnimInstance->Montage_Play(LandMontage);
        UE_LOG(LogTemp, Log, TEXT("Playing Land Animation"));
    }
}

EAnim_MovementState UAnim_BasicMovementController::GetCurrentMovementState() const
{
    return CurrentMovementData.MovementState;
}

float UAnim_BasicMovementController::GetMovementSpeed() const
{
    return CurrentMovementData.Speed;
}

bool UAnim_BasicMovementController::IsMoving() const
{
    return CurrentMovementData.Speed > 1.0f;
}