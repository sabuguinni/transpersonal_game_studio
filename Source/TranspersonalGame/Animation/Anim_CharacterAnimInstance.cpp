#include "Anim_CharacterAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"

UAnim_CharacterAnimInstance::UAnim_CharacterAnimInstance()
{
    OwnerCharacter = nullptr;
    CharacterMovement = nullptr;
    
    GroundSpeed = 0.0f;
    MovementDirection = 0.0f;
    bShouldMove = false;
    bIsFalling = false;
    bIsJumping = false;
    
    CombatState = EAnim_CombatState::Peaceful;
    bIsAiming = false;
    bIsAttacking = false;
    
    IKTraceDistance = 50.0f;
    IKInterpSpeed = 15.0f;
    LeftFootSocketName = FName("foot_l");
    RightFootSocketName = FName("foot_r");
    
    WalkSpeedThreshold = 150.0f;
    RunSpeedThreshold = 400.0f;
}

void UAnim_CharacterAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
    if (OwnerCharacter)
    {
        CharacterMovement = OwnerCharacter->GetCharacterMovement();
    }
}

void UAnim_CharacterAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwnerCharacter || !CharacterMovement)
    {
        return;
    }
    
    UpdateMovementValues();
    UpdateCombatState();
    UpdateFootIK();
}

void UAnim_CharacterAnimInstance::UpdateMovementValues()
{
    if (!OwnerCharacter || !CharacterMovement)
    {
        return;
    }
    
    // Get velocity and speed
    FVector Velocity = CharacterMovement->Velocity;
    GroundSpeed = Velocity.Size2D();
    bShouldMove = GroundSpeed > 3.0f && !CharacterMovement->GetCurrentAcceleration().IsZero();
    
    // Calculate movement direction relative to character rotation
    if (bShouldMove)
    {
        FVector Forward = OwnerCharacter->GetActorForwardVector();
        FVector VelocityNormalized = Velocity.GetSafeNormal2D();
        MovementDirection = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Forward, VelocityNormalized)));
        
        // Determine if moving left or right
        FVector Right = OwnerCharacter->GetActorRightVector();
        if (FVector::DotProduct(Right, VelocityNormalized) < 0.0f)
        {
            MovementDirection *= -1.0f;
        }
    }
    else
    {
        MovementDirection = 0.0f;
    }
    
    // Update movement state
    bIsFalling = CharacterMovement->IsFalling();
    bIsJumping = bIsFalling && Velocity.Z > 0.0f;
    
    // Determine movement state
    if (bIsFalling)
    {
        MovementData.MovementState = bIsJumping ? EAnim_MovementState::Jumping : EAnim_MovementState::Falling;
    }
    else if (CharacterMovement->IsCrouching())
    {
        MovementData.MovementState = EAnim_MovementState::Crouching;
    }
    else if (CharacterMovement->IsSwimming())
    {
        MovementData.MovementState = EAnim_MovementState::Swimming;
    }
    else if (bShouldMove)
    {
        if (GroundSpeed >= RunSpeedThreshold)
        {
            MovementData.MovementState = EAnim_MovementState::Running;
        }
        else if (GroundSpeed >= WalkSpeedThreshold)
        {
            MovementData.MovementState = EAnim_MovementState::Walking;
        }
        else
        {
            MovementData.MovementState = EAnim_MovementState::Idle;
        }
    }
    else
    {
        MovementData.MovementState = EAnim_MovementState::Idle;
    }
    
    // Update movement data struct
    MovementData.Speed = GroundSpeed;
    MovementData.Direction = MovementDirection;
    MovementData.bIsInAir = bIsFalling;
    MovementData.bIsCrouching = CharacterMovement->IsCrouching();
}

void UAnim_CharacterAnimInstance::UpdateCombatState()
{
    // This would be updated based on game state, weapons equipped, etc.
    // For now, keep it simple - peaceful when not moving fast, alert when running
    if (GroundSpeed > RunSpeedThreshold)
    {
        CombatState = EAnim_CombatState::Alert;
    }
    else
    {
        CombatState = EAnim_CombatState::Peaceful;
    }
}

void UAnim_CharacterAnimInstance::UpdateFootIK()
{
    if (!OwnerCharacter || bIsFalling)
    {
        // Reset IK when falling or no character
        IKData.LeftFootIKAlpha = FMath::FInterpTo(IKData.LeftFootIKAlpha, 0.0f, GetDeltaSeconds(), IKInterpSpeed);
        IKData.RightFootIKAlpha = FMath::FInterpTo(IKData.RightFootIKAlpha, 0.0f, GetDeltaSeconds(), IKInterpSpeed);
        return;
    }
    
    // Calculate IK for both feet
    CalculateFootIK(LeftFootSocketName, IKData.LeftFootIKLocation, IKData.LeftFootIKRotation, IKData.LeftFootIKAlpha);
    CalculateFootIK(RightFootSocketName, IKData.RightFootIKLocation, IKData.RightFootIKRotation, IKData.RightFootIKAlpha);
}

FVector UAnim_CharacterAnimInstance::PerformFootTrace(const FName& SocketName, float TraceDistance)
{
    if (!OwnerCharacter)
    {
        return FVector::ZeroVector;
    }
    
    USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
    if (!MeshComp)
    {
        return FVector::ZeroVector;
    }
    
    // Get socket location in world space
    FVector SocketLocation = MeshComp->GetSocketLocation(SocketName);
    
    // Trace down from socket location
    FVector TraceStart = SocketLocation;
    FVector TraceEnd = TraceStart - FVector(0.0f, 0.0f, TraceDistance);
    
    FHitResult HitResult;
    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(OwnerCharacter);
    
    bool bHit = UKismetSystemLibrary::LineTraceSingle(
        GetWorld(),
        TraceStart,
        TraceEnd,
        UEngineTypes::ConvertToTraceType(ECC_Visibility),
        false,
        ActorsToIgnore,
        EDrawDebugTrace::None,
        HitResult,
        true
    );
    
    if (bHit)
    {
        return HitResult.Location;
    }
    
    return TraceEnd;
}

void UAnim_CharacterAnimInstance::CalculateFootIK(const FName& SocketName, FVector& OutLocation, FRotator& OutRotation, float& OutAlpha)
{
    if (!OwnerCharacter)
    {
        OutAlpha = 0.0f;
        return;
    }
    
    // Perform trace to find ground
    FVector GroundLocation = PerformFootTrace(SocketName, IKTraceDistance);
    
    USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
    if (!MeshComp)
    {
        OutAlpha = 0.0f;
        return;
    }
    
    FVector SocketLocation = MeshComp->GetSocketLocation(SocketName);
    
    // Calculate offset needed
    float DistanceToGround = SocketLocation.Z - GroundLocation.Z;
    
    // Only apply IK if the foot needs to be adjusted
    if (FMath::Abs(DistanceToGround) > 5.0f) // 5cm threshold
    {
        OutLocation = FVector(0.0f, 0.0f, -DistanceToGround);
        
        // Calculate rotation based on ground normal (simplified)
        OutRotation = FRotator::ZeroRotator;
        
        // Interpolate alpha
        float TargetAlpha = FMath::Clamp(FMath::Abs(DistanceToGround) / 20.0f, 0.0f, 1.0f);
        OutAlpha = FMath::FInterpTo(OutAlpha, TargetAlpha, GetDeltaSeconds(), IKInterpSpeed);
    }
    else
    {
        OutLocation = FVector::ZeroVector;
        OutRotation = FRotator::ZeroRotator;
        OutAlpha = FMath::FInterpTo(OutAlpha, 0.0f, GetDeltaSeconds(), IKInterpSpeed);
    }
}

void UAnim_CharacterAnimInstance::SetCombatState(EAnim_CombatState NewState)
{
    CombatState = NewState;
}

void UAnim_CharacterAnimInstance::TriggerAttackAnimation()
{
    bIsAttacking = true;
    // This would trigger an attack montage
    // For now, just set a flag that can be used in the AnimBP
}

void UAnim_CharacterAnimInstance::TriggerJumpAnimation()
{
    bIsJumping = true;
    // This would trigger a jump montage
    // The flag will be reset when landing is detected
}