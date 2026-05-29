#include "Anim_CharacterAnimBlueprint.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimMontage.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_CharacterAnimBlueprint::UAnim_CharacterAnimBlueprint()
{
    // Set default animation thresholds
    WalkSpeedThreshold = 100.0f;
    RunSpeedThreshold = 400.0f;
    IdleSpeedThreshold = 10.0f;
    
    // Initialize state tracking
    PreviousMovementState = EAnim_MovementState::Idle;
    StateChangeTimer = 0.0f;
    bWasInAir = false;
    
    // Initialize references
    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
    JumpMontage = nullptr;
    LandMontage = nullptr;
}

void UAnim_CharacterAnimBlueprint::NativeInitializeAnimation()
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
        UE_LOG(LogTemp, Warning, TEXT("Animation Blueprint failed to get character reference"));
    }
}

void UAnim_CharacterAnimBlueprint::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Update movement data
    MovementData.Velocity = OwnerCharacter->GetVelocity();
    MovementData.GroundSpeed = MovementData.Velocity.Size2D();
    MovementData.Speed = MovementData.GroundSpeed;
    MovementData.bIsInAir = MovementComponent->IsFalling();
    MovementData.bIsCrouching = MovementComponent->IsCrouching();
    
    // Calculate movement direction relative to character rotation
    if (MovementData.GroundSpeed > IdleSpeedThreshold)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector VelocityNormalized = MovementData.Velocity.GetSafeNormal2D();
        float DotProduct = FVector::DotProduct(ForwardVector, VelocityNormalized);
        MovementData.Direction = UKismetMathLibrary::DegAcos(DotProduct);
        
        // Determine if moving left or right
        FVector RightVector = OwnerCharacter->GetActorRightVector();
        float RightDot = FVector::DotProduct(RightVector, VelocityNormalized);
        if (RightDot < 0.0f)
        {
            MovementData.Direction *= -1.0f;
        }
    }
    else
    {
        MovementData.Direction = 0.0f;
    }
    
    // Update movement state
    UpdateMovementState();
    
    // Track state changes for animation blending
    if (MovementData.MovementState != PreviousMovementState)
    {
        StateChangeTimer = 0.0f;
        PreviousMovementState = MovementData.MovementState;
    }
    else
    {
        StateChangeTimer += DeltaTimeX;
    }
    
    // Handle landing detection
    if (bWasInAir && !MovementData.bIsInAir)
    {
        PlayLandMontage();
    }
    bWasInAir = MovementData.bIsInAir;
}

void UAnim_CharacterAnimBlueprint::UpdateMovementState()
{
    if (MovementData.bIsInAir)
    {
        if (MovementData.Velocity.Z > 0.0f)
        {
            MovementData.MovementState = EAnim_MovementState::Jumping;
        }
        else
        {
            MovementData.MovementState = EAnim_MovementState::Falling;
        }
    }
    else if (MovementData.bIsCrouching)
    {
        MovementData.MovementState = EAnim_MovementState::Crouching;
    }
    else if (MovementData.GroundSpeed <= IdleSpeedThreshold)
    {
        MovementData.MovementState = EAnim_MovementState::Idle;
    }
    else if (MovementData.GroundSpeed <= WalkSpeedThreshold)
    {
        MovementData.MovementState = EAnim_MovementState::Walking;
    }
    else
    {
        MovementData.MovementState = EAnim_MovementState::Running;
    }
}

bool UAnim_CharacterAnimBlueprint::ShouldPlayIdleAnimation() const
{
    return MovementData.MovementState == EAnim_MovementState::Idle;
}

bool UAnim_CharacterAnimBlueprint::ShouldPlayWalkAnimation() const
{
    return MovementData.MovementState == EAnim_MovementState::Walking;
}

bool UAnim_CharacterAnimBlueprint::ShouldPlayRunAnimation() const
{
    return MovementData.MovementState == EAnim_MovementState::Running;
}

bool UAnim_CharacterAnimBlueprint::ShouldPlayJumpAnimation() const
{
    return MovementData.MovementState == EAnim_MovementState::Jumping || 
           MovementData.MovementState == EAnim_MovementState::Falling;
}

float UAnim_CharacterAnimBlueprint::GetMovementDirection() const
{
    return MovementData.Direction;
}

float UAnim_CharacterAnimBlueprint::GetMovementSpeed() const
{
    return MovementData.Speed;
}

void UAnim_CharacterAnimBlueprint::PlayJumpMontage()
{
    if (JumpMontage && OwnerCharacter)
    {
        UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
        if (AnimInstance && !AnimInstance->IsAnyMontagePlaying())
        {
            AnimInstance->Montage_Play(JumpMontage, 1.0f);
            UE_LOG(LogTemp, Log, TEXT("Playing jump montage"));
        }
    }
}

void UAnim_CharacterAnimBlueprint::PlayLandMontage()
{
    if (LandMontage && OwnerCharacter)
    {
        UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
        if (AnimInstance)
        {
            AnimInstance->Montage_Play(LandMontage, 1.0f);
            UE_LOG(LogTemp, Log, TEXT("Playing land montage"));
        }
    }
}