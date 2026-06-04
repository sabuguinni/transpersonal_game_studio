#include "Anim_CharacterAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UAnim_CharacterAnimInstance::UAnim_CharacterAnimInstance()
{
    CurrentMovementState = EAnim_MovementState::Idle;
    Speed = 0.0f;
    Direction = 0.0f;
    bIsInAir = false;
    bIsCrouching = false;
    bIsSwimming = false;
    IdleToWalkBlend = 0.0f;
    WalkToRunBlend = 0.0f;
    LeftFootIKOffset = 0.0f;
    RightFootIKOffset = 0.0f;
    LeftFootIKRotation = FRotator::ZeroRotator;
    RightFootIKRotation = FRotator::ZeroRotator;
    OwningCharacter = nullptr;
    MovementComponent = nullptr;
}

void UAnim_CharacterAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    // Cache character and movement component references
    OwningCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwningCharacter)
    {
        MovementComponent = OwningCharacter->GetCharacterMovement();
    }
}

void UAnim_CharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwningCharacter || !MovementComponent)
    {
        return;
    }

    // Update basic movement variables
    FVector Velocity = OwningCharacter->GetVelocity();
    Speed = Velocity.Size2D();
    Direction = UKismetMathLibrary::CalculateDirection(Velocity, OwningCharacter->GetActorRotation()).Yaw;

    // Update movement state flags
    bIsInAir = MovementComponent->IsFalling();
    bIsCrouching = MovementComponent->IsCrouching();
    bIsSwimming = MovementComponent->IsSwimming();

    // Update movement state
    UpdateMovementState();

    // Update animation blending values
    IdleToWalkBlend = FMath::Clamp(Speed / 150.0f, 0.0f, 1.0f); // Walk threshold at 150 units/s
    WalkToRunBlend = FMath::Clamp((Speed - 150.0f) / 300.0f, 0.0f, 1.0f); // Run threshold at 450 units/s

    // Update IK system
    UpdateIKValues();
}

void UAnim_CharacterAnimInstance::UpdateMovementState()
{
    if (bIsSwimming)
    {
        CurrentMovementState = EAnim_MovementState::Swimming;
    }
    else if (bIsInAir)
    {
        if (MovementComponent->Velocity.Z > 0.0f)
        {
            CurrentMovementState = EAnim_MovementState::Jumping;
        }
        else
        {
            CurrentMovementState = EAnim_MovementState::Falling;
        }
    }
    else if (bIsCrouching)
    {
        CurrentMovementState = EAnim_MovementState::Crouching;
    }
    else if (Speed > 450.0f)
    {
        CurrentMovementState = EAnim_MovementState::Running;
    }
    else if (Speed > 10.0f)
    {
        CurrentMovementState = EAnim_MovementState::Walking;
    }
    else
    {
        CurrentMovementState = EAnim_MovementState::Idle;
    }
}

void UAnim_CharacterAnimInstance::UpdateIKValues()
{
    if (!OwningCharacter || bIsInAir || bIsSwimming)
    {
        // Reset IK when not grounded
        LeftFootIKOffset = FMath::FInterpTo(LeftFootIKOffset, 0.0f, GetWorld()->GetDeltaSeconds(), IKInterpSpeed);
        RightFootIKOffset = FMath::FInterpTo(RightFootIKOffset, 0.0f, GetWorld()->GetDeltaSeconds(), IKInterpSpeed);
        LeftFootIKRotation = FMath::RInterpTo(LeftFootIKRotation, FRotator::ZeroRotator, GetWorld()->GetDeltaSeconds(), IKInterpSpeed);
        RightFootIKRotation = FMath::RInterpTo(RightFootIKRotation, FRotator::ZeroRotator, GetWorld()->GetDeltaSeconds(), IKInterpSpeed);
        return;
    }

    USkeletalMeshComponent* MeshComp = OwningCharacter->GetMesh();
    if (!MeshComp)
    {
        return;
    }

    // Get foot bone locations
    FVector LeftFootLocation = MeshComp->GetSocketLocation(FName("foot_l"));
    FVector RightFootLocation = MeshComp->GetSocketLocation(FName("foot_r"));

    // Perform IK traces for both feet
    float LeftIKOffset, RightIKOffset;
    FRotator LeftIKRot, RightIKRot;

    PerformFootIKTrace(LeftFootLocation, LeftIKOffset, LeftIKRot, true);
    PerformFootIKTrace(RightFootLocation, RightIKOffset, RightIKRot, false);

    // Interpolate IK values for smooth transitions
    float DeltaTime = GetWorld()->GetDeltaSeconds();
    LeftFootIKOffset = FMath::FInterpTo(LeftFootIKOffset, LeftIKOffset, DeltaTime, IKInterpSpeed);
    RightFootIKOffset = FMath::FInterpTo(RightFootIKOffset, RightIKOffset, DeltaTime, IKInterpSpeed);
    LeftFootIKRotation = FMath::RInterpTo(LeftFootIKRotation, LeftIKRot, DeltaTime, IKInterpSpeed);
    RightFootIKRotation = FMath::RInterpTo(RightFootIKRotation, RightIKRot, DeltaTime, IKInterpSpeed);
}

void UAnim_CharacterAnimInstance::PerformFootIKTrace(FVector FootLocation, float& IKOffset, FRotator& IKRotation, bool bIsLeftFoot)
{
    if (!OwningCharacter)
    {
        IKOffset = 0.0f;
        IKRotation = FRotator::ZeroRotator;
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        IKOffset = 0.0f;
        IKRotation = FRotator::ZeroRotator;
        return;
    }

    // Convert foot location to world space
    FVector WorldFootLocation = OwningCharacter->GetMesh()->GetComponentTransform().TransformPosition(FootLocation);
    
    // Trace parameters
    FVector TraceStart = WorldFootLocation + FVector(0.0f, 0.0f, IKTraceDistance);
    FVector TraceEnd = WorldFootLocation - FVector(0.0f, 0.0f, IKTraceDistance);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwningCharacter);

    // Perform line trace
    bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_Visibility,
        QueryParams
    );

    if (bHit)
    {
        // Calculate IK offset
        float DistanceFromGround = (WorldFootLocation - HitResult.Location).Z;
        IKOffset = -DistanceFromGround;

        // Calculate foot rotation based on surface normal
        FVector SurfaceNormal = HitResult.Normal;
        FVector ForwardVector = OwningCharacter->GetActorForwardVector();
        FVector RightVector = OwningCharacter->GetActorRightVector();

        // Calculate pitch and roll for foot alignment
        float Pitch = FMath::Atan2(FVector::DotProduct(SurfaceNormal, ForwardVector), SurfaceNormal.Z);
        float Roll = FMath::Atan2(FVector::DotProduct(SurfaceNormal, RightVector), SurfaceNormal.Z);

        // Apply foot-specific adjustments
        if (bIsLeftFoot)
        {
            Roll = -Roll; // Invert roll for left foot
        }

        IKRotation = FRotator(FMath::RadiansToDegrees(Pitch), 0.0f, FMath::RadiansToDegrees(Roll));
    }
    else
    {
        IKOffset = 0.0f;
        IKRotation = FRotator::ZeroRotator;
    }
}