#include "Anim_CharacterAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"

UAnim_CharacterAnimInstance::UAnim_CharacterAnimInstance()
{
    CurrentMovementState = EAnim_MovementState::Idle;
    CurrentEmotionalState = EAnim_EmotionalState::Calm;
    Speed = 0.0f;
    Direction = 0.0f;
    bIsInAir = false;
    bIsCrouching = false;
    bIsAccelerating = false;
    
    FearLevel = 0.0f;
    StaminaLevel = 1.0f;
    HealthLevel = 1.0f;
    
    LeftFootIKOffset = 0.0f;
    RightFootIKOffset = 0.0f;
    LeftFootIKRotation = FRotator::ZeroRotator;
    RightFootIKRotation = FRotator::ZeroRotator;
    
    IdleToWalkBlend = 0.0f;
    WalkToRunBlend = 0.0f;
    EmotionalBlend = 0.0f;
    
    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
}

void UAnim_CharacterAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    OwnerCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
    }
}

void UAnim_CharacterAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Update basic movement properties
    FVector Velocity = MovementComponent->Velocity;
    Speed = Velocity.Size();
    Direction = UKismetMathLibrary::CalculateDirection(Velocity, OwnerCharacter->GetActorRotation()).Yaw;
    
    bIsInAir = MovementComponent->IsFalling();
    bIsCrouching = MovementComponent->IsCrouching();
    bIsAccelerating = MovementComponent->GetCurrentAcceleration().SizeSquared() > 0.0f;
    
    // Update states
    UpdateMovementState();
    UpdateEmotionalState();
    UpdateIKValues();
    UpdateBlendValues();
}

void UAnim_CharacterAnimInstance::UpdateMovementState()
{
    CurrentMovementState = CalculateMovementState();
}

void UAnim_CharacterAnimInstance::UpdateEmotionalState()
{
    CurrentEmotionalState = CalculateEmotionalState();
}

void UAnim_CharacterAnimInstance::UpdateIKValues()
{
    if (!OwnerCharacter)
    {
        return;
    }
    
    // Calculate foot IK offsets for terrain adaptation
    LeftFootIKOffset = CalculateFootIKOffset(FName("foot_l"));
    RightFootIKOffset = CalculateFootIKOffset(FName("foot_r"));
    
    LeftFootIKRotation = CalculateFootIKRotation(FName("foot_l"));
    RightFootIKRotation = CalculateFootIKRotation(FName("foot_r"));
}

void UAnim_CharacterAnimInstance::UpdateBlendValues()
{
    // Calculate blend values for smooth transitions
    if (Speed < 50.0f)
    {
        IdleToWalkBlend = FMath::Clamp(Speed / 50.0f, 0.0f, 1.0f);
    }
    else
    {
        IdleToWalkBlend = 1.0f;
    }
    
    if (Speed > 300.0f)
    {
        WalkToRunBlend = FMath::Clamp((Speed - 300.0f) / 300.0f, 0.0f, 1.0f);
    }
    else
    {
        WalkToRunBlend = 0.0f;
    }
    
    // Emotional blend based on fear and health
    EmotionalBlend = FMath::Clamp(FearLevel + (1.0f - HealthLevel), 0.0f, 1.0f);
}

EAnim_MovementState UAnim_CharacterAnimInstance::CalculateMovementState() const
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return EAnim_MovementState::Idle;
    }
    
    if (bIsInAir)
    {
        if (MovementComponent->Velocity.Z > 0)
        {
            return EAnim_MovementState::Jumping;
        }
        else
        {
            return EAnim_MovementState::Falling;
        }
    }
    
    if (bIsCrouching)
    {
        return EAnim_MovementState::Crouching;
    }
    
    if (Speed < 50.0f)
    {
        return EAnim_MovementState::Idle;
    }
    else if (Speed < 300.0f)
    {
        return EAnim_MovementState::Walking;
    }
    else
    {
        return EAnim_MovementState::Running;
    }
}

EAnim_EmotionalState UAnim_CharacterAnimInstance::CalculateEmotionalState() const
{
    if (HealthLevel < 0.3f)
    {
        return EAnim_EmotionalState::Injured;
    }
    
    if (StaminaLevel < 0.2f)
    {
        return EAnim_EmotionalState::Exhausted;
    }
    
    if (FearLevel > 0.7f)
    {
        return EAnim_EmotionalState::Fearful;
    }
    else if (FearLevel > 0.4f)
    {
        return EAnim_EmotionalState::Alert;
    }
    
    // Check for combat state (simplified - would need combat component reference)
    if (CurrentMovementState == EAnim_MovementState::Combat)
    {
        return EAnim_EmotionalState::Aggressive;
    }
    
    return EAnim_EmotionalState::Calm;
}

float UAnim_CharacterAnimInstance::CalculateFootIKOffset(const FName& SocketName) const
{
    if (!OwnerCharacter)
    {
        return 0.0f;
    }
    
    // Get foot socket location
    USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
    if (!MeshComp)
    {
        return 0.0f;
    }
    
    FVector SocketLocation = MeshComp->GetSocketLocation(SocketName);
    FVector TraceStart = SocketLocation + FVector(0, 0, 50.0f);
    FVector TraceEnd = SocketLocation - FVector(0, 0, 100.0f);
    
    // Perform line trace to ground
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
    {
        float DistanceToGround = (SocketLocation - HitResult.Location).Z;
        return FMath::Clamp(DistanceToGround, -50.0f, 50.0f);
    }
    
    return 0.0f;
}

FRotator UAnim_CharacterAnimInstance::CalculateFootIKRotation(const FName& SocketName) const
{
    if (!OwnerCharacter)
    {
        return FRotator::ZeroRotator;
    }
    
    // Get foot socket location
    USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
    if (!MeshComp)
    {
        return FRotator::ZeroRotator;
    }
    
    FVector SocketLocation = MeshComp->GetSocketLocation(SocketName);
    FVector TraceStart = SocketLocation + FVector(0, 0, 50.0f);
    FVector TraceEnd = SocketLocation - FVector(0, 0, 100.0f);
    
    // Perform line trace to ground
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
    {
        FVector GroundNormal = HitResult.Normal;
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector RightVector = OwnerCharacter->GetActorRightVector();
        
        // Calculate foot rotation to align with ground normal
        float PitchOffset = FMath::Asin(FVector::DotProduct(GroundNormal, ForwardVector)) * 180.0f / PI;
        float RollOffset = FMath::Asin(FVector::DotProduct(GroundNormal, RightVector)) * 180.0f / PI;
        
        return FRotator(PitchOffset, 0.0f, RollOffset);
    }
    
    return FRotator::ZeroRotator;
}