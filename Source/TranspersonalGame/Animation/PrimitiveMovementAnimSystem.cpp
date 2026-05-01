#include "PrimitiveMovementAnimSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Engine/Engine.h"

UPrimitiveMovementAnimSystem::UPrimitiveMovementAnimSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    CurrentMovementState = EAnim_PrimitiveMovementState::Idle;
    PreviousMovementState = EAnim_PrimitiveMovementState::Idle;
    CurrentSpeed = 0.0f;
    MovementDirection = 0.0f;
    bIsInAir = false;
    bIsCrouching = false;
    TimeInCurrentState = 0.0f;

    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
    MeshComponent = nullptr;
}

void UPrimitiveMovementAnimSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeComponents();
}

void UPrimitiveMovementAnimSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }

    // Actualizar dados de movimento
    UpdateMovementData();
    
    // Actualizar estado de movimento
    UpdateMovementState();
    
    // Incrementar tempo no estado actual
    TimeInCurrentState += DeltaTime;
}

void UPrimitiveMovementAnimSystem::InitializeComponents()
{
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        MeshComponent = OwnerCharacter->GetMesh();
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
                FString::Printf(TEXT("PrimitiveMovementAnimSystem initialized for %s"), 
                *OwnerCharacter->GetName()));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("PrimitiveMovementAnimSystem: Owner is not a Character!"));
    }
}

void UPrimitiveMovementAnimSystem::UpdateMovementData()
{
    if (!MovementComponent)
    {
        return;
    }

    // Velocidade actual
    FVector Velocity = MovementComponent->Velocity;
    CurrentSpeed = Velocity.Size();
    
    // Direcção de movimento (relativa ao forward do personagem)
    if (CurrentSpeed > 1.0f && OwnerCharacter)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector NormalizedVelocity = Velocity.GetSafeNormal();
        MovementDirection = FVector::DotProduct(ForwardVector, NormalizedVelocity);
    }
    else
    {
        MovementDirection = 0.0f;
    }

    // Estado no ar
    bIsInAir = MovementComponent->IsFalling();
    
    // Estado agachado
    bIsCrouching = MovementComponent->IsCrouching();
}

void UPrimitiveMovementAnimSystem::UpdateMovementState()
{
    EAnim_PrimitiveMovementState NewState = DetermineNewState();
    
    if (NewState != CurrentMovementState)
    {
        // Verificar se a transição é válida
        if (CanTransitionToState(NewState))
        {
            PreviousMovementState = CurrentMovementState;
            CurrentMovementState = NewState;
            TimeInCurrentState = 0.0f;
            
            OnStateChanged(PreviousMovementState, CurrentMovementState);
        }
    }
}

EAnim_PrimitiveMovementState UPrimitiveMovementAnimSystem::DetermineNewState() const
{
    if (!MovementComponent)
    {
        return CurrentMovementState;
    }

    // Prioridade: Estados no ar
    if (bIsInAir)
    {
        if (MovementComponent->Velocity.Z > 0.0f)
        {
            return EAnim_PrimitiveMovementState::Jumping;
        }
        else
        {
            return EAnim_PrimitiveMovementState::Falling;
        }
    }

    // Se acabou de aterrar
    if (PreviousMovementState == EAnim_PrimitiveMovementState::Falling && !bIsInAir)
    {
        return EAnim_PrimitiveMovementState::Landing;
    }

    // Estados agachados
    if (bIsCrouching)
    {
        if (CurrentSpeed > WalkThreshold)
        {
            return EAnim_PrimitiveMovementState::Crawling;
        }
        else
        {
            return EAnim_PrimitiveMovementState::Crouching;
        }
    }

    // Estados de movimento normal
    if (CurrentSpeed < WalkThreshold)
    {
        return EAnim_PrimitiveMovementState::Idle;
    }
    else if (CurrentSpeed < RunThreshold)
    {
        return EAnim_PrimitiveMovementState::Walking;
    }
    else
    {
        return EAnim_PrimitiveMovementState::Running;
    }
}

void UPrimitiveMovementAnimSystem::OnStateChanged(EAnim_PrimitiveMovementState OldState, EAnim_PrimitiveMovementState NewState)
{
    if (GEngine)
    {
        FString OldStateName = UEnum::GetValueAsString(OldState);
        FString NewStateName = UEnum::GetValueAsString(NewState);
        
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
            FString::Printf(TEXT("Animation State: %s -> %s"), 
            *OldStateName, *NewStateName));
    }

    // Executar acções específicas para certas transições
    HandleStateTransition();
}

void UPrimitiveMovementAnimSystem::HandleStateTransition()
{
    switch (CurrentMovementState)
    {
        case EAnim_PrimitiveMovementState::Jumping:
            PlayJumpAnimation();
            break;
            
        case EAnim_PrimitiveMovementState::Landing:
            PlayLandAnimation();
            break;
            
        default:
            break;
    }
}

bool UPrimitiveMovementAnimSystem::CanTransitionToState(EAnim_PrimitiveMovementState TargetState) const
{
    // Verificar tempo mínimo no estado actual
    float MinTime = GetMinimumStateTime(CurrentMovementState);
    if (TimeInCurrentState < MinTime)
    {
        return false;
    }

    // Verificar se a transição é válida
    return IsValidTransition(CurrentMovementState, TargetState);
}

bool UPrimitiveMovementAnimSystem::IsValidTransition(EAnim_PrimitiveMovementState From, EAnim_PrimitiveMovementState To) const
{
    // Transições sempre válidas
    if (From == To)
    {
        return true;
    }

    // Transições para estados no ar sempre válidas
    if (To == EAnim_PrimitiveMovementState::Jumping || To == EAnim_PrimitiveMovementState::Falling)
    {
        return true;
    }

    // Landing só pode vir de Falling
    if (To == EAnim_PrimitiveMovementState::Landing)
    {
        return From == EAnim_PrimitiveMovementState::Falling;
    }

    // Outras transições são válidas
    return true;
}

float UPrimitiveMovementAnimSystem::GetMinimumStateTime(EAnim_PrimitiveMovementState State) const
{
    switch (State)
    {
        case EAnim_PrimitiveMovementState::Landing:
            return 0.3f; // Tempo mínimo para animação de aterragem
            
        case EAnim_PrimitiveMovementState::Jumping:
            return 0.1f; // Tempo mínimo para início do salto
            
        default:
            return 0.0f;
    }
}

void UPrimitiveMovementAnimSystem::ForceState(EAnim_PrimitiveMovementState NewState)
{
    if (NewState != CurrentMovementState)
    {
        PreviousMovementState = CurrentMovementState;
        CurrentMovementState = NewState;
        TimeInCurrentState = 0.0f;
        
        OnStateChanged(PreviousMovementState, CurrentMovementState);
    }
}

void UPrimitiveMovementAnimSystem::PlayJumpAnimation()
{
    if (AnimConfig.JumpMontage && MeshComponent)
    {
        UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();
        if (AnimInstance)
        {
            AnimInstance->Montage_Play(AnimConfig.JumpMontage);
        }
    }
}

void UPrimitiveMovementAnimSystem::PlayLandAnimation()
{
    if (AnimConfig.LandMontage && MeshComponent)
    {
        UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();
        if (AnimInstance)
        {
            AnimInstance->Montage_Play(AnimConfig.LandMontage);
        }
    }
}

float UPrimitiveMovementAnimSystem::GetBlendSpaceValue() const
{
    // Valor para BlendSpace baseado na velocidade
    if (CurrentSpeed < WalkThreshold)
    {
        return 0.0f; // Idle
    }
    else if (CurrentSpeed < RunThreshold)
    {
        return FMath::GetMappedRangeValueClamped(
            FVector2D(WalkThreshold, RunThreshold), 
            FVector2D(0.5f, 1.0f), 
            CurrentSpeed
        );
    }
    else
    {
        return 1.0f; // Run
    }
}

bool UPrimitiveMovementAnimSystem::ShouldPlayIdleAnimation() const
{
    return CurrentMovementState == EAnim_PrimitiveMovementState::Idle ||
           CurrentMovementState == EAnim_PrimitiveMovementState::Crouching;
}

bool UPrimitiveMovementAnimSystem::ShouldPlayMovementAnimation() const
{
    return CurrentMovementState == EAnim_PrimitiveMovementState::Walking ||
           CurrentMovementState == EAnim_PrimitiveMovementState::Running ||
           CurrentMovementState == EAnim_PrimitiveMovementState::Crawling;
}