#include "Anim_LocomotionBlueprint.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_LocomotionBlueprint::UAnim_LocomotionBlueprint()
{
    // Initialize default values
    WalkSpeedThreshold = 100.0f;
    RunSpeedThreshold = 300.0f;
    MovementInterpolationSpeed = 10.0f;
    
    PreviousState = EAnim_LocomotionState::Idle;
    StateChangeTime = 0.0f;
    bWasInAir = false;
    
    // Initialize movement data
    MovementData.Speed = 0.0f;
    MovementData.Direction = 0.0f;
    MovementData.bIsInAir = false;
    MovementData.bIsCrouching = false;
    MovementData.JumpHeight = 0.0f;
    MovementData.CurrentState = EAnim_LocomotionState::Idle;
}

void UAnim_LocomotionBlueprint::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Get character reference
    OwnerCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        UE_LOG(LogTemp, Log, TEXT("Animation Blueprint initialized for character: %s"), 
               *OwnerCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Animation Blueprint: Failed to get character reference"));
    }
}

void UAnim_LocomotionBlueprint::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Update movement data
    UpdateMovementData();
    
    // Update locomotion state
    UpdateLocomotionState();
    
    // Track state change time
    StateChangeTime += DeltaTimeX;
}

void UAnim_LocomotionBlueprint::UpdateMovementData()
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Get velocity and calculate speed
    FVector Velocity = OwnerCharacter->GetVelocity();
    MovementData.Speed = Velocity.Size2D();
    
    // Calculate movement direction relative to character rotation
    if (MovementData.Speed > 0.1f)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector VelocityNormalized = Velocity.GetSafeNormal2D();
        MovementData.Direction = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ForwardVector, VelocityNormalized)));
        
        // Check if moving backwards
        FVector RightVector = OwnerCharacter->GetActorRightVector();
        if (FVector::DotProduct(RightVector, VelocityNormalized) < 0.0f)
        {
            MovementData.Direction *= -1.0f;
        }
    }
    else
    {
        MovementData.Direction = 0.0f;
    }
    
    // Update air state
    MovementData.bIsInAir = MovementComponent->IsFalling();
    MovementData.JumpHeight = Velocity.Z;
    
    // Update crouching state
    MovementData.bIsCrouching = MovementComponent->IsCrouching();
}

void UAnim_LocomotionBlueprint::UpdateLocomotionState()
{
    EAnim_LocomotionState DesiredState = CalculateDesiredState();
    
    if (DesiredState != MovementData.CurrentState)
    {
        HandleStateTransition(DesiredState);
    }
}

EAnim_LocomotionState UAnim_LocomotionBlueprint::CalculateDesiredState() const
{
    // Check if in air first
    if (MovementData.bIsInAir)
    {
        if (MovementData.JumpHeight > 0.0f)
        {
            return EAnim_LocomotionState::Jumping;
        }
        else
        {
            return EAnim_LocomotionState::Falling;
        }
    }
    
    // Check if just landed
    if (bWasInAir && !MovementData.bIsInAir)
    {
        return EAnim_LocomotionState::Landing;
    }
    
    // Check crouching
    if (MovementData.bIsCrouching)
    {
        return EAnim_LocomotionState::Crouching;
    }
    
    // Check movement speed
    if (MovementData.Speed < 10.0f)
    {
        return EAnim_LocomotionState::Idle;
    }
    else if (MovementData.Speed < RunSpeedThreshold)
    {
        return EAnim_LocomotionState::Walking;
    }
    else
    {
        return EAnim_LocomotionState::Running;
    }
}

void UAnim_LocomotionBlueprint::HandleStateTransition(EAnim_LocomotionState NewState)
{
    PreviousState = MovementData.CurrentState;
    MovementData.CurrentState = NewState;
    StateChangeTime = 0.0f;
    
    // Handle special state transitions
    switch (NewState)
    {
        case EAnim_LocomotionState::Jumping:
            PlayJumpAnimation();
            break;
            
        case EAnim_LocomotionState::Landing:
            PlayLandingAnimation();
            break;
            
        default:
            break;
    }
    
    // Update air state tracking
    bWasInAir = MovementData.bIsInAir;
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Animation state changed from %d to %d"), 
           (int32)PreviousState, (int32)NewState);
}

void UAnim_LocomotionBlueprint::PlayJumpAnimation()
{
    if (JumpMontage && OwnerCharacter)
    {
        USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
        if (MeshComp && MeshComp->GetAnimInstance())
        {
            MeshComp->GetAnimInstance()->Montage_Play(JumpMontage);
            UE_LOG(LogTemp, Log, TEXT("Playing jump animation"));
        }
    }
}

void UAnim_LocomotionBlueprint::PlayLandingAnimation()
{
    if (LandingMontage && OwnerCharacter)
    {
        USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
        if (MeshComp && MeshComp->GetAnimInstance())
        {
            MeshComp->GetAnimInstance()->Montage_Play(LandingMontage);
            UE_LOG(LogTemp, Log, TEXT("Playing landing animation"));
        }
    }
}