#include "Anim_SurvivalAnimBlueprint.h"
#include "Animation/AnimSequence.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_SurvivalAnimBlueprint::UAnim_SurvivalAnimBlueprint()
{
    // Inicializar dados de movimento
    MovementData.Speed = 0.0f;
    MovementData.Direction = 0.0f;
    MovementData.bIsInAir = false;
    MovementData.bIsCrouching = false;
    MovementData.MovementState = EAnim_MovementState::Idle;
    MovementData.Velocity = FVector::ZeroVector;
    MovementData.GroundDistance = 0.0f;

    // Inicializar dados de sobrevivência
    SurvivalData.HealthPercent = 1.0f;
    SurvivalData.HungerPercent = 1.0f;
    SurvivalData.ThirstPercent = 1.0f;
    SurvivalData.StaminaPercent = 1.0f;
    SurvivalData.FearLevel = 0.0f;
    SurvivalData.SurvivalState = EAnim_SurvivalState::Normal;
    SurvivalData.bIsExhausted = false;
    SurvivalData.bIsInDanger = false;

    // Configurações padrão
    WalkThreshold = 50.0f;
    RunThreshold = 300.0f;
    TransitionSpeed = 5.0f;
    
    LastUpdateTime = 0.0f;
    bIsInitialized = false;

    // Limpar referências de animação
    IdleAnimation = nullptr;
    WalkBlendSpace = nullptr;
    RunBlendSpace = nullptr;
    JumpAnimMontage = nullptr;
    CrouchAnimMontage = nullptr;
}

void UAnim_SurvivalAnimBlueprint::InitializeAnimationBlueprint(USkeletalMeshComponent* MeshComponent)
{
    if (!MeshComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("UAnim_SurvivalAnimBlueprint::InitializeAnimationBlueprint - MeshComponent is null"));
        return;
    }

    OwnerMeshComponent = MeshComponent;
    bIsInitialized = true;
    LastUpdateTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

    UE_LOG(LogTemp, Log, TEXT("UAnim_SurvivalAnimBlueprint initialized for mesh: %s"), 
           *MeshComponent->GetName());
}

void UAnim_SurvivalAnimBlueprint::UpdateMovementData(const FVector& Velocity, bool bIsInAir, bool bIsCrouching)
{
    if (!bIsInitialized)
    {
        return;
    }

    // Actualizar velocidade e dados básicos
    MovementData.Velocity = Velocity;
    MovementData.Speed = Velocity.Size();
    MovementData.bIsInAir = bIsInAir;
    MovementData.bIsCrouching = bIsCrouching;

    // Calcular direcção do movimento
    if (MovementData.Speed > 1.0f)
    {
        FVector ForwardVector = OwnerMeshComponent.IsValid() ? 
            OwnerMeshComponent->GetForwardVector() : FVector::ForwardVector;
        
        float DotProduct = FVector::DotProduct(Velocity.GetSafeNormal(), ForwardVector);
        MovementData.Direction = FMath::RadiansToDegrees(FMath::Acos(DotProduct));
        
        // Ajustar sinal baseado no produto cruzado
        FVector CrossProduct = FVector::CrossProduct(ForwardVector, Velocity.GetSafeNormal());
        if (CrossProduct.Z < 0.0f)
        {
            MovementData.Direction *= -1.0f;
        }
    }
    else
    {
        MovementData.Direction = 0.0f;
    }

    // Calcular estado de movimento
    CalculateMovementState();

    // Actualizar tempo
    LastUpdateTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
}

void UAnim_SurvivalAnimBlueprint::UpdateSurvivalData(float Health, float Hunger, float Thirst, float Stamina, float Fear)
{
    if (!bIsInitialized)
    {
        return;
    }

    // Actualizar dados de sobrevivência (clamped entre 0 e 1)
    SurvivalData.HealthPercent = FMath::Clamp(Health, 0.0f, 1.0f);
    SurvivalData.HungerPercent = FMath::Clamp(Hunger, 0.0f, 1.0f);
    SurvivalData.ThirstPercent = FMath::Clamp(Thirst, 0.0f, 1.0f);
    SurvivalData.StaminaPercent = FMath::Clamp(Stamina, 0.0f, 1.0f);
    SurvivalData.FearLevel = FMath::Clamp(Fear, 0.0f, 1.0f);

    // Calcular estados derivados
    SurvivalData.bIsExhausted = SurvivalData.StaminaPercent < 0.2f;
    SurvivalData.bIsInDanger = SurvivalData.FearLevel > 0.5f || SurvivalData.HealthPercent < 0.3f;

    // Calcular estado de sobrevivência
    CalculateSurvivalState();
}

void UAnim_SurvivalAnimBlueprint::TransitionToMovementState(EAnim_MovementState NewState)
{
    if (MovementData.MovementState != NewState)
    {
        EAnim_MovementState OldState = MovementData.MovementState;
        MovementData.MovementState = NewState;

        UE_LOG(LogTemp, Log, TEXT("Movement state transition: %d -> %d"), 
               (int32)OldState, (int32)NewState);
    }
}

void UAnim_SurvivalAnimBlueprint::TransitionToSurvivalState(EAnim_SurvivalState NewState)
{
    if (SurvivalData.SurvivalState != NewState)
    {
        EAnim_SurvivalState OldState = SurvivalData.SurvivalState;
        SurvivalData.SurvivalState = NewState;

        UE_LOG(LogTemp, Log, TEXT("Survival state transition: %d -> %d"), 
               (int32)OldState, (int32)NewState);
    }
}

void UAnim_SurvivalAnimBlueprint::SetIdleAnimation(UAnimSequence* IdleAnim)
{
    IdleAnimation = IdleAnim;
    UE_LOG(LogTemp, Log, TEXT("Idle animation set: %s"), 
           IdleAnim ? *IdleAnim->GetName() : TEXT("nullptr"));
}

void UAnim_SurvivalAnimBlueprint::SetWalkBlendSpace(UBlendSpace* WalkBlendSpace_New)
{
    WalkBlendSpace = WalkBlendSpace_New;
    UE_LOG(LogTemp, Log, TEXT("Walk blend space set: %s"), 
           WalkBlendSpace_New ? *WalkBlendSpace_New->GetName() : TEXT("nullptr"));
}

void UAnim_SurvivalAnimBlueprint::SetRunBlendSpace(UBlendSpace* RunBlendSpace_New)
{
    RunBlendSpace = RunBlendSpace_New;
    UE_LOG(LogTemp, Log, TEXT("Run blend space set: %s"), 
           RunBlendSpace_New ? *RunBlendSpace_New->GetName() : TEXT("nullptr"));
}

void UAnim_SurvivalAnimBlueprint::SetJumpMontage(UAnimMontage* JumpMontage)
{
    JumpAnimMontage = JumpMontage;
    UE_LOG(LogTemp, Log, TEXT("Jump montage set: %s"), 
           JumpMontage ? *JumpMontage->GetName() : TEXT("nullptr"));
}

void UAnim_SurvivalAnimBlueprint::CalculateMovementState()
{
    EAnim_MovementState NewState = EAnim_MovementState::Idle;

    if (MovementData.bIsInAir)
    {
        // No ar - jumping ou falling
        if (MovementData.Velocity.Z > 0.0f)
        {
            NewState = EAnim_MovementState::Jumping;
        }
        else
        {
            NewState = EAnim_MovementState::Falling;
        }
    }
    else if (MovementData.bIsCrouching)
    {
        // Agachado
        if (MovementData.Speed > 10.0f)
        {
            NewState = EAnim_MovementState::Crawling;
        }
        else
        {
            NewState = EAnim_MovementState::Crouching;
        }
    }
    else
    {
        // Movimento normal no chão
        if (MovementData.Speed < WalkThreshold)
        {
            NewState = EAnim_MovementState::Idle;
        }
        else if (MovementData.Speed < RunThreshold)
        {
            NewState = EAnim_MovementState::Walking;
        }
        else
        {
            NewState = EAnim_MovementState::Running;
        }
    }

    // Aplicar transição se necessário
    if (NewState != MovementData.MovementState)
    {
        TransitionToMovementState(NewState);
    }
}

void UAnim_SurvivalAnimBlueprint::CalculateSurvivalState()
{
    EAnim_SurvivalState NewState = EAnim_SurvivalState::Normal;

    // Prioridade: Dying > Injured > Fearful > Thirsty > Hungry > Tired > Normal
    if (SurvivalData.HealthPercent < 0.1f)
    {
        NewState = EAnim_SurvivalState::Dying;
    }
    else if (SurvivalData.HealthPercent < 0.3f)
    {
        NewState = EAnim_SurvivalState::Injured;
    }
    else if (SurvivalData.FearLevel > 0.7f)
    {
        NewState = EAnim_SurvivalState::Fearful;
    }
    else if (SurvivalData.ThirstPercent < 0.2f)
    {
        NewState = EAnim_SurvivalState::Thirsty;
    }
    else if (SurvivalData.HungerPercent < 0.2f)
    {
        NewState = EAnim_SurvivalState::Hungry;
    }
    else if (SurvivalData.StaminaPercent < 0.3f)
    {
        NewState = EAnim_SurvivalState::Tired;
    }

    // Aplicar transição se necessário
    if (NewState != SurvivalData.SurvivalState)
    {
        TransitionToSurvivalState(NewState);
    }
}

void UAnim_SurvivalAnimBlueprint::UpdateAnimationBlending(float DeltaTime)
{
    if (!bIsInitialized || !OwnerMeshComponent.IsValid())
    {
        return;
    }

    // Esta função pode ser chamada pelo Animation Blueprint para actualizar blending
    // Por agora, apenas log para debug
    UE_LOG(LogTemp, VeryVerbose, TEXT("UpdateAnimationBlending - DeltaTime: %f, Speed: %f, State: %d"), 
           DeltaTime, MovementData.Speed, (int32)MovementData.MovementState);
}