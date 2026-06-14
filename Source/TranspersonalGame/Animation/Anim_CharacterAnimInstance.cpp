#include "Anim_CharacterAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_CharacterAnimInstance::UAnim_CharacterAnimInstance()
{
    CurrentMovementState = EAnim_MovementState::Idle;
    CurrentWeaponType = EAnim_WeaponType::None;
    CharacterArchetype = EChar_CharacterArchetype::TribalWarrior;
    ArchetypeSpeedModifier = 1.0f;
    ArchetypeAggressionModifier = 1.0f;
    ArchetypePrecisionModifier = 1.0f;
    IKInterpSpeed = 15.0f;
    FootTraceDistance = 100.0f;
}

void UAnim_CharacterAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    OwnerCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        UpdateArchetypeModifiers();
    }
}

void UAnim_CharacterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
    Super::NativeUpdateAnimation(DeltaTime);
    
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }

    LastUpdateTime = GetWorld()->GetTimeSeconds();
    
    UpdateMovementData(DeltaTime);
    UpdateMovementState();
    UpdateIKData(DeltaTime);
}

void UAnim_CharacterAnimInstance::UpdateMovementData(float DeltaTime)
{
    if (!MovementComponent)
    {
        return;
    }

    // Update basic movement data
    MovementData.Velocity = MovementComponent->Velocity;
    MovementData.Speed = MovementData.Velocity.Size();
    MovementData.bIsInAir = MovementComponent->IsFalling();
    MovementData.bIsCrouching = MovementComponent->IsCrouching();
    MovementData.Acceleration = MovementComponent->GetCurrentAcceleration().Size();

    // Calculate movement direction relative to character forward
    if (MovementData.Speed > 0.1f)
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

    // Calculate jump height for falling/jumping states
    if (MovementData.bIsInAir)
    {
        MovementData.JumpHeight = FMath::Abs(MovementData.Velocity.Z);
    }
    else
    {
        MovementData.JumpHeight = 0.0f;
    }
}

void UAnim_CharacterAnimInstance::UpdateMovementState()
{
    if (!MovementComponent)
    {
        CurrentMovementState = EAnim_MovementState::Idle;
        return;
    }

    // Determine movement state based on current conditions
    if (MovementData.bIsInAir)
    {
        if (MovementData.Velocity.Z > 0.0f)
        {
            CurrentMovementState = EAnim_MovementState::Jumping;
        }
        else
        {
            CurrentMovementState = EAnim_MovementState::Falling;
        }
    }
    else if (MovementData.bIsCrouching)
    {
        CurrentMovementState = EAnim_MovementState::Crouching;
    }
    else if (MovementData.Speed > 0.1f)
    {
        // Apply archetype speed modifiers for different movement thresholds
        float WalkThreshold = 150.0f * ArchetypeSpeedModifier;
        float RunThreshold = 400.0f * ArchetypeSpeedModifier;
        
        if (MovementData.Speed > RunThreshold)
        {
            CurrentMovementState = EAnim_MovementState::Running;
        }
        else if (MovementData.Speed > WalkThreshold)
        {
            CurrentMovementState = EAnim_MovementState::Walking;
        }
        else
        {
            CurrentMovementState = EAnim_MovementState::Idle;
        }
    }
    else
    {
        CurrentMovementState = EAnim_MovementState::Idle;
    }
}

void UAnim_CharacterAnimInstance::UpdateIKData(float DeltaTime)
{
    if (!OwnerCharacter || MovementData.bIsInAir)
    {
        // Reset IK data when in air
        IKData.LeftFootIKAlpha = FMath::FInterpTo(IKData.LeftFootIKAlpha, 0.0f, DeltaTime, IKInterpSpeed);
        IKData.RightFootIKAlpha = FMath::FInterpTo(IKData.RightFootIKAlpha, 0.0f, DeltaTime, IKInterpSpeed);
        IKData.HipOffset = FMath::FInterpTo(IKData.HipOffset, 0.0f, DeltaTime, IKInterpSpeed);
        return;
    }

    // Get foot bone locations (approximated from character bounds)
    UCapsuleComponent* CapsuleComp = OwnerCharacter->GetCapsuleComponent();
    if (!CapsuleComp)
    {
        return;
    }

    float CapsuleHalfHeight = CapsuleComp->GetScaledCapsuleHalfHeight();
    float CapsuleRadius = CapsuleComp->GetScaledCapsuleRadius();
    FVector CharacterLocation = OwnerCharacter->GetActorLocation();
    
    // Approximate foot positions
    FVector LeftFootWorldPos = CharacterLocation + OwnerCharacter->GetActorRightVector() * (-CapsuleRadius * 0.5f);
    FVector RightFootWorldPos = CharacterLocation + OwnerCharacter->GetActorRightVector() * (CapsuleRadius * 0.5f);
    
    // Perform foot traces
    FVector LeftFootIK = PerformFootTrace(LeftFootWorldPos, FootTraceDistance);
    FVector RightFootIK = PerformFootTrace(RightFootWorldPos, FootTraceDistance);
    
    // Update IK locations with interpolation
    IKData.LeftFootIKLocation = FMath::VInterpTo(IKData.LeftFootIKLocation, LeftFootIK, DeltaTime, IKInterpSpeed);
    IKData.RightFootIKLocation = FMath::VInterpTo(IKData.RightFootIKLocation, RightFootIK, DeltaTime, IKInterpSpeed);
    
    // Calculate IK alphas based on distance from original position
    IKData.LeftFootIKAlpha = CalculateFootIKAlpha(IKData.LeftFootIKLocation);
    IKData.RightFootIKAlpha = CalculateFootIKAlpha(IKData.RightFootIKLocation);
    
    // Calculate hip offset to maintain character stability
    float LeftFootOffset = IKData.LeftFootIKLocation.Z - CharacterLocation.Z;
    float RightFootOffset = IKData.RightFootIKLocation.Z - CharacterLocation.Z;
    float DesiredHipOffset = FMath::Min(LeftFootOffset, RightFootOffset);
    
    IKData.HipOffset = FMath::FInterpTo(IKData.HipOffset, DesiredHipOffset, DeltaTime, IKInterpSpeed);
    
    // Calculate foot rotations based on surface normals (simplified)
    // In a full implementation, this would use the hit normal from the trace
    IKData.LeftFootIKRotation = FMath::RInterpTo(IKData.LeftFootIKRotation, FRotator::ZeroRotator, DeltaTime, IKInterpSpeed);
    IKData.RightFootIKRotation = FMath::RInterpTo(IKData.RightFootIKRotation, FRotator::ZeroRotator, DeltaTime, IKInterpSpeed);
}

void UAnim_CharacterAnimInstance::UpdateArchetypeModifiers()
{
    // Apply archetype-specific animation modifiers
    switch (CharacterArchetype)
    {
        case EChar_CharacterArchetype::YoungHunter:
            ArchetypeSpeedModifier = 1.2f;      // Faster, more agile
            ArchetypeAggressionModifier = 0.8f; // Less aggressive
            ArchetypePrecisionModifier = 1.3f;  // More precise
            break;
            
        case EChar_CharacterArchetype::ElderShaman:
            ArchetypeSpeedModifier = 0.7f;      // Slower, more deliberate
            ArchetypeAggressionModifier = 0.5f; // Peaceful
            ArchetypePrecisionModifier = 1.5f;  // Very precise
            break;
            
        case EChar_CharacterArchetype::FemaleGatherer:
            ArchetypeSpeedModifier = 0.9f;      // Moderate speed
            ArchetypeAggressionModifier = 0.6f; // Low aggression
            ArchetypePrecisionModifier = 1.4f;  // High precision
            break;
            
        case EChar_CharacterArchetype::ChildScout:
            ArchetypeSpeedModifier = 1.4f;      // Very fast
            ArchetypeAggressionModifier = 0.3f; // Very low aggression
            ArchetypePrecisionModifier = 0.8f;  // Lower precision
            break;
            
        case EChar_CharacterArchetype::TribalWarrior:
            ArchetypeSpeedModifier = 1.0f;      // Standard speed
            ArchetypeAggressionModifier = 1.5f; // High aggression
            ArchetypePrecisionModifier = 1.1f;  // Good precision
            break;
            
        case EChar_CharacterArchetype::CraftsMaster:
            ArchetypeSpeedModifier = 0.8f;      // Slower, methodical
            ArchetypeAggressionModifier = 0.7f; // Low aggression
            ArchetypePrecisionModifier = 1.6f;  // Highest precision
            break;
            
        default:
            ArchetypeSpeedModifier = 1.0f;
            ArchetypeAggressionModifier = 1.0f;
            ArchetypePrecisionModifier = 1.0f;
            break;
    }
}

FVector UAnim_CharacterAnimInstance::PerformFootTrace(const FVector& FootLocation, float TraceDistance)
{
    if (!OwnerCharacter)
    {
        return FootLocation;
    }

    UWorld* World = OwnerCharacter->GetWorld();
    if (!World)
    {
        return FootLocation;
    }

    FVector StartLocation = FootLocation + FVector(0.0f, 0.0f, 20.0f);
    FVector EndLocation = FootLocation - FVector(0.0f, 0.0f, TraceDistance);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    QueryParams.bTraceComplex = false;
    
    bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECollisionChannel::ECC_WorldStatic,
        QueryParams
    );
    
    if (bHit)
    {
        return HitResult.Location;
    }
    
    return FootLocation;
}

float UAnim_CharacterAnimInstance::CalculateFootIKAlpha(const FVector& IKLocation)
{
    if (!OwnerCharacter)
    {
        return 0.0f;
    }
    
    FVector CharacterLocation = OwnerCharacter->GetActorLocation();
    float Distance = FMath::Abs(IKLocation.Z - CharacterLocation.Z);
    
    // Alpha based on distance - closer to ground = higher alpha
    float MaxIKDistance = 50.0f;
    float Alpha = FMath::Clamp(Distance / MaxIKDistance, 0.0f, 1.0f);
    
    return Alpha;
}