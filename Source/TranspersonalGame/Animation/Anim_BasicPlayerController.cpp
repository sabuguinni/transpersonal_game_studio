#include "Anim_BasicPlayerController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"

UAnim_BasicPlayerController::UAnim_BasicPlayerController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    // Configurações padrão
    WalkSpeedThreshold = 150.0f;
    RunSpeedThreshold = 400.0f;
    DirectionSmoothingSpeed = 10.0f;
    SpeedSmoothingSpeed = 5.0f;

    // Inicializar variáveis
    CurrentSpeed = 0.0f;
    TargetSpeed = 0.0f;
    CurrentDirection = 0.0f;
    TargetDirection = 0.0f;
    LastFrameSpeed = 0.0f;
    LastFrameVelocity = FVector::ZeroVector;

    // Inicializar dados de animação
    AnimationData = FAnim_PlayerAnimationData();
}

void UAnim_BasicPlayerController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeReferences();
    
    UE_LOG(LogTemp, Log, TEXT("Anim_BasicPlayerController: BeginPlay completed"));
}

void UAnim_BasicPlayerController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }

    // Actualizar dados de movimento
    CalculateMovementData();
    
    // Determinar estado de movimento
    DetermineMovementState();
    
    // Suavizar valores de animação
    SmoothAnimationValues(DeltaTime);
}

void UAnim_BasicPlayerController::InitializeReferences()
{
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        MeshComponent = OwnerCharacter->GetMesh();
        
        UE_LOG(LogTemp, Log, TEXT("Anim_BasicPlayerController: References initialized successfully"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Anim_BasicPlayerController: Failed to get owner character"));
    }
}

void UAnim_BasicPlayerController::CalculateMovementData()
{
    if (!MovementComponent)
    {
        return;
    }

    // Obter velocidade actual
    FVector CurrentVelocity = MovementComponent->Velocity;
    float Speed = CurrentVelocity.Size();
    
    // Actualizar dados básicos
    TargetSpeed = Speed;
    AnimationData.bIsMoving = Speed > 1.0f;
    AnimationData.bIsInAir = MovementComponent->IsFalling();
    AnimationData.bIsCrouching = MovementComponent->IsCrouching();

    // Calcular direcção de movimento
    if (AnimationData.bIsMoving && OwnerCharacter)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector VelocityDirection = CurrentVelocity.GetSafeNormal();
        
        // Calcular ângulo entre forward e velocidade
        float DotProduct = FVector::DotProduct(ForwardVector, VelocityDirection);
        float CrossProduct = FVector::CrossProduct(ForwardVector, VelocityDirection).Z;
        
        TargetDirection = FMath::Atan2(CrossProduct, DotProduct) * (180.0f / PI);
    }
    else
    {
        TargetDirection = 0.0f;
    }

    // Guardar para próximo frame
    LastFrameVelocity = CurrentVelocity;
    LastFrameSpeed = Speed;
}

void UAnim_BasicPlayerController::DetermineMovementState()
{
    if (AnimationData.bIsInAir)
    {
        // Verificar se está a saltar ou a cair
        if (MovementComponent->Velocity.Z > 0.0f)
        {
            AnimationData.MovementState = EAnim_PlayerMovementState::Jumping;
        }
        else
        {
            AnimationData.MovementState = EAnim_PlayerMovementState::Falling;
        }
    }
    else if (AnimationData.bIsCrouching)
    {
        AnimationData.MovementState = EAnim_PlayerMovementState::Crouching;
    }
    else if (AnimationData.bIsMoving)
    {
        if (TargetSpeed >= RunSpeedThreshold)
        {
            AnimationData.MovementState = EAnim_PlayerMovementState::Running;
        }
        else if (TargetSpeed >= WalkSpeedThreshold)
        {
            AnimationData.MovementState = EAnim_PlayerMovementState::Walking;
        }
        else
        {
            AnimationData.MovementState = EAnim_PlayerMovementState::Idle;
        }
    }
    else
    {
        AnimationData.MovementState = EAnim_PlayerMovementState::Idle;
    }
}

void UAnim_BasicPlayerController::SmoothAnimationValues(float DeltaTime)
{
    // Suavizar velocidade
    CurrentSpeed = FMath::FInterpTo(CurrentSpeed, TargetSpeed, DeltaTime, SpeedSmoothingSpeed);
    AnimationData.Speed = CurrentSpeed;

    // Suavizar direcção
    CurrentDirection = FMath::FInterpTo(CurrentDirection, TargetDirection, DeltaTime, DirectionSmoothingSpeed);
    AnimationData.Direction = CurrentDirection;
}

void UAnim_BasicPlayerController::UpdateMovementState()
{
    CalculateMovementData();
    DetermineMovementState();
}

void UAnim_BasicPlayerController::UpdateActionState(EAnim_PlayerActionState NewActionState)
{
    AnimationData.ActionState = NewActionState;
    
    UE_LOG(LogTemp, Log, TEXT("Anim_BasicPlayerController: Action state changed to %d"), (int32)NewActionState);
}

void UAnim_BasicPlayerController::UpdateSurvivalData(float Health, float Stamina, float Hunger, float Fear)
{
    AnimationData.HealthPercentage = FMath::Clamp(Health, 0.0f, 1.0f);
    AnimationData.StaminaPercentage = FMath::Clamp(Stamina, 0.0f, 1.0f);
    AnimationData.HungerLevel = FMath::Clamp(Hunger, 0.0f, 1.0f);
    AnimationData.FearLevel = FMath::Clamp(Fear, 0.0f, 1.0f);
}

void UAnim_BasicPlayerController::SetCombatState(bool bInCombat, bool bBlocking, bool bAttacking)
{
    AnimationData.bIsInCombat = bInCombat;
    AnimationData.bIsBlocking = bBlocking;
    AnimationData.bIsAttacking = bAttacking;
    
    if (bInCombat)
    {
        UE_LOG(LogTemp, Log, TEXT("Anim_BasicPlayerController: Entered combat state"));
    }
}