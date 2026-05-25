#include "Anim_SurvivalAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UAnim_SurvivalAnimInstance::UAnim_SurvivalAnimInstance()
{
    MovementState = EAnim_MovementState::Idle;
    ActionState = EAnim_ActionState::None;
    
    WalkSpeedThreshold = 100.0f;
    RunSpeedThreshold = 300.0f;
    SprintSpeedThreshold = 500.0f;
    FootstepInterval = 0.5f;
    
    MovementBlendAlpha = 0.0f;
    ActionBlendAlpha = 0.0f;
    FootIKAlpha = 1.0f;
    
    LeftFootOffset = FVector::ZeroVector;
    RightFootOffset = FVector::ZeroVector;
}

void UAnim_SurvivalAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    OwnerCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        UE_LOG(LogTemp, Log, TEXT("SurvivalAnimInstance: Initialized for character %s"), *OwnerCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("SurvivalAnimInstance: Failed to get owner character"));
    }
}

void UAnim_SurvivalAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    UpdateMotionData(DeltaTimeX);
    UpdateMovementState();
    UpdateActionState();
    UpdateFootIK();
    
    LastUpdateTime += DeltaTimeX;
}

void UAnim_SurvivalAnimInstance::UpdateMotionData(float DeltaTime)
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Update basic motion data
    MotionData.Velocity = MovementComponent->Velocity;
    MotionData.Speed = MotionData.Velocity.Size();
    MotionData.Direction = CalculateDirectionAngle();
    MotionData.bIsInAir = MovementComponent->IsFalling();
    MotionData.bIsCrouching = MovementComponent->IsCrouching();
    
    // Calculate acceleration
    if (DeltaTime > 0.0f)
    {
        MotionData.Acceleration = (MotionData.Velocity - LastVelocity) / DeltaTime;
    }
    
    // Update footstep timing
    if (MotionData.Speed > WalkSpeedThreshold && !MotionData.bIsInAir)
    {
        MotionData.TimeSinceLastFootstep += DeltaTime;
        if (MotionData.TimeSinceLastFootstep >= FootstepInterval)
        {
            MotionData.TimeSinceLastFootstep = 0.0f;
            // Trigger footstep event here if needed
        }
    }
    else
    {
        MotionData.TimeSinceLastFootstep = 0.0f;
    }
    
    LastVelocity = MotionData.Velocity;
}

void UAnim_SurvivalAnimInstance::UpdateMovementState()
{
    EAnim_MovementState NewState = CalculateMovementState();
    
    if (NewState != MovementState)
    {
        MovementState = NewState;
        
        // Update blend alpha based on state
        switch (MovementState)
        {
            case EAnim_MovementState::Idle:
                MovementBlendAlpha = 0.0f;
                break;
            case EAnim_MovementState::Walking:
                MovementBlendAlpha = 0.25f;
                break;
            case EAnim_MovementState::Running:
                MovementBlendAlpha = 0.5f;
                break;
            case EAnim_MovementState::Sprinting:
                MovementBlendAlpha = 1.0f;
                break;
            case EAnim_MovementState::Crouching:
                MovementBlendAlpha = 0.3f;
                break;
            default:
                MovementBlendAlpha = 0.0f;
                break;
        }
    }
}

void UAnim_SurvivalAnimInstance::UpdateActionState()
{
    // Action state updates would be driven by gameplay systems
    // For now, maintain current state
    ActionBlendAlpha = (ActionState != EAnim_ActionState::None) ? 1.0f : 0.0f;
}

void UAnim_SurvivalAnimInstance::UpdateFootIK()
{
    if (!OwnerCharacter || MotionData.bIsInAir)
    {
        LeftFootOffset = FVector::ZeroVector;
        RightFootOffset = FVector::ZeroVector;
        return;
    }
    
    // Perform IK traces for both feet
    PerformFootIKTrace(TEXT("foot_l"), LeftFootOffset);
    PerformFootIKTrace(TEXT("foot_r"), RightFootOffset);
}

EAnim_MovementState UAnim_SurvivalAnimInstance::CalculateMovementState() const
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return EAnim_MovementState::Idle;
    }
    
    if (MotionData.bIsInAir)
    {
        if (MotionData.Velocity.Z > 0.0f)
        {
            return EAnim_MovementState::Jumping;
        }
        else
        {
            return EAnim_MovementState::Falling;
        }
    }
    
    if (MotionData.bIsCrouching)
    {
        return EAnim_MovementState::Crouching;
    }
    
    if (MotionData.Speed < WalkSpeedThreshold)
    {
        return EAnim_MovementState::Idle;
    }
    else if (MotionData.Speed < RunSpeedThreshold)
    {
        return EAnim_MovementState::Walking;
    }
    else if (MotionData.Speed < SprintSpeedThreshold)
    {
        return EAnim_MovementState::Running;
    }
    else
    {
        return EAnim_MovementState::Sprinting;
    }
}

float UAnim_SurvivalAnimInstance::CalculateDirectionAngle() const
{
    if (!OwnerCharacter || MotionData.Speed < 1.0f)
    {
        return 0.0f;
    }
    
    FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
    FVector VelocityNormalized = MotionData.Velocity.GetSafeNormal();
    
    float DotProduct = FVector::DotProduct(ForwardVector, VelocityNormalized);
    float CrossProduct = FVector::CrossProduct(ForwardVector, VelocityNormalized).Z;
    
    return FMath::Atan2(CrossProduct, DotProduct) * (180.0f / PI);
}

void UAnim_SurvivalAnimInstance::PerformFootIKTrace(const FName& SocketName, FVector& OutOffset)
{
    if (!OwnerCharacter)
    {
        OutOffset = FVector::ZeroVector;
        return;
    }
    
    USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
    if (!MeshComp)
    {
        OutOffset = FVector::ZeroVector;
        return;
    }
    
    FVector SocketLocation = MeshComp->GetSocketLocation(SocketName);
    FVector TraceStart = SocketLocation + FVector(0, 0, 50.0f);
    FVector TraceEnd = SocketLocation - FVector(0, 0, 100.0f);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_WorldStatic,
        QueryParams
    );
    
    if (bHit)
    {
        float IKOffset = (HitResult.Location.Z - SocketLocation.Z) + 5.0f; // Small offset to avoid clipping
        OutOffset = FVector(0, 0, FMath::Clamp(IKOffset, -50.0f, 50.0f));
    }
    else
    {
        OutOffset = FVector::ZeroVector;
    }
}