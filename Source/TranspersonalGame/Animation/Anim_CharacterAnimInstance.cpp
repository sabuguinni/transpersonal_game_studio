#include "Anim_CharacterAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_CharacterAnimInstance::UAnim_CharacterAnimInstance()
{
    // Set default values
    WalkThreshold = 50.0f;
    RunThreshold = 300.0f;
    JumpThreshold = 200.0f;
    bEnableFootIK = true;
    FootIKTraceDistance = 50.0f;
    
    CurrentMovementState = EAnim_MovementState::Idle;
    CurrentCombatState = EAnim_CombatState::None;
}

void UAnim_CharacterAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    // Get character reference
    OwnerCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        UE_LOG(LogTemp, Log, TEXT("Animation Instance initialized for character: %s"), *OwnerCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Animation Instance failed to get character reference"));
    }
}

void UAnim_CharacterAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);

    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }

    // Update all animation data
    UpdateMovementData();
    UpdateMovementState();
    
    if (bEnableFootIK)
    {
        UpdateFootIK();
    }
}

void UAnim_CharacterAnimInstance::UpdateMovementData()
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }

    // Get basic movement data
    MovementData.Velocity = MovementComponent->Velocity;
    MovementData.Speed = MovementData.Velocity.Size();
    MovementData.bIsInAir = MovementComponent->IsFalling();
    MovementData.bIsCrouching = MovementComponent->IsCrouching();

    // Calculate movement direction relative to character rotation
    if (MovementData.Speed > 0.0f)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector VelocityNormalized = MovementData.Velocity.GetSafeNormal();
        MovementData.Direction = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ForwardVector, VelocityNormalized)));
        
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

    // Calculate ground distance for landing prediction
    FVector StartLocation = OwnerCharacter->GetActorLocation();
    FVector EndLocation = StartLocation - FVector(0.0f, 0.0f, 200.0f);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, QueryParams))
    {
        MovementData.GroundDistance = HitResult.Distance;
    }
    else
    {
        MovementData.GroundDistance = 200.0f;
    }
}

void UAnim_CharacterAnimInstance::UpdateMovementState()
{
    CurrentMovementState = DetermineMovementState();
}

EAnim_MovementState UAnim_CharacterAnimInstance::DetermineMovementState() const
{
    if (!MovementComponent)
    {
        return EAnim_MovementState::Idle;
    }

    // Check if in air first
    if (MovementData.bIsInAir)
    {
        if (MovementData.Velocity.Z > JumpThreshold)
        {
            return EAnim_MovementState::Jumping;
        }
        else
        {
            return EAnim_MovementState::Falling;
        }
    }

    // Check if crouching
    if (MovementData.bIsCrouching)
    {
        return EAnim_MovementState::Crouching;
    }

    // Check movement speed
    if (MovementData.Speed < WalkThreshold)
    {
        return EAnim_MovementState::Idle;
    }
    else if (MovementData.Speed < RunThreshold)
    {
        return EAnim_MovementState::Walking;
    }
    else
    {
        return EAnim_MovementState::Running;
    }
}

void UAnim_CharacterAnimInstance::UpdateFootIK()
{
    if (!OwnerCharacter)
    {
        return;
    }

    // Update foot IK for both feet
    PerformFootTrace(TEXT("foot_l"), LeftFootOffset, LeftFootRotation);
    PerformFootTrace(TEXT("foot_r"), RightFootOffset, RightFootRotation);
}

void UAnim_CharacterAnimInstance::PerformFootTrace(const FName& SocketName, FVector& OutOffset, FRotator& OutRotation)
{
    if (!OwnerCharacter)
    {
        OutOffset = FVector::ZeroVector;
        OutRotation = FRotator::ZeroRotator;
        return;
    }

    USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
    if (!MeshComp)
    {
        OutOffset = FVector::ZeroVector;
        OutRotation = FRotator::ZeroRotator;
        return;
    }

    // Get foot socket location
    FVector SocketLocation = MeshComp->GetSocketLocation(SocketName);
    FVector StartLocation = SocketLocation + FVector(0.0f, 0.0f, 20.0f);
    FVector EndLocation = SocketLocation - FVector(0.0f, 0.0f, FootIKTraceDistance + 20.0f);

    // Perform trace
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult, 
        StartLocation, 
        EndLocation, 
        ECC_Visibility, 
        QueryParams
    );

    if (bHit)
    {
        // Calculate offset
        float TargetZ = HitResult.Location.Z;
        float CurrentZ = SocketLocation.Z;
        float Offset = TargetZ - CurrentZ;
        
        // Clamp offset to reasonable range
        Offset = FMath::Clamp(Offset, -FootIKTraceDistance, FootIKTraceDistance);
        OutOffset = FVector(0.0f, 0.0f, Offset);

        // Calculate rotation based on surface normal
        FVector SurfaceNormal = HitResult.Normal;
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector RightVector = FVector::CrossProduct(SurfaceNormal, ForwardVector).GetSafeNormal();
        ForwardVector = FVector::CrossProduct(RightVector, SurfaceNormal).GetSafeNormal();
        
        OutRotation = UKismetMathLibrary::MakeRotationFromAxes(ForwardVector, RightVector, SurfaceNormal);
    }
    else
    {
        // No ground found, reset to default
        OutOffset = FVector::ZeroVector;
        OutRotation = FRotator::ZeroRotator;
    }
}

void UAnim_CharacterAnimInstance::SetCombatState(EAnim_CombatState NewState)
{
    if (CurrentCombatState != NewState)
    {
        CurrentCombatState = NewState;
        UE_LOG(LogTemp, Log, TEXT("Combat state changed to: %d"), (int32)NewState);
    }
}