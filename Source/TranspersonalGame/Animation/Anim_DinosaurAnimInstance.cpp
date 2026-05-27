#include "Anim_DinosaurAnimInstance.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_DinosaurAnimInstance::UAnim_DinosaurAnimInstance()
{
    // Initialize animation blending values
    IdleToWalkBlend = 0.0f;
    WalkToRunBlend = 0.0f;
    AttackIntensity = 0.0f;
    FeedingBlend = 0.0f;

    // Initialize IK values
    LeftFootIKOffset = FVector::ZeroVector;
    RightFootIKOffset = FVector::ZeroVector;
    LeftFootIKRotation = FRotator::ZeroRotator;
    RightFootIKRotation = FRotator::ZeroRotator;
    HeadLookAtAlpha = 0.0f;
    HeadLookAtTarget = FVector::ZeroVector;

    // Initialize species parameters
    TailSwayIntensity = 1.0f;
    NeckFlexibility = 1.0f;
    BodyMassScale = 1.0f;

    // Initialize timing
    LastUpdateTime = 0.0f;
    AnimationDeltaTime = 0.0f;

    // Initialize cached references
    OwningPawn = nullptr;
    MovementComponent = nullptr;
    MeshComponent = nullptr;
}

void UAnim_DinosaurAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    // Cache pawn and component references
    OwningPawn = TryGetPawnOwner();
    if (OwningPawn)
    {
        if (ACharacter* Character = Cast<ACharacter>(OwningPawn))
        {
            MovementComponent = Character->GetCharacterMovement();
        }
        MeshComponent = OwningPawn->FindComponentByClass<USkeletalMeshComponent>();
    }

    // Initialize behavior data based on pawn type
    if (OwningPawn)
    {
        FString PawnName = OwningPawn->GetName();
        
        // Determine species from pawn name or class
        if (PawnName.Contains("TRex") || PawnName.Contains("Tyrannosaurus"))
        {
            BehaviorData.Species = EAnim_DinosaurSpecies::TRex;
            BodyMassScale = 2.0f;
            TailSwayIntensity = 0.8f;
            NeckFlexibility = 0.6f;
        }
        else if (PawnName.Contains("Velociraptor"))
        {
            BehaviorData.Species = EAnim_DinosaurSpecies::Velociraptor;
            BodyMassScale = 0.4f;
            TailSwayIntensity = 1.5f;
            NeckFlexibility = 1.2f;
        }
        else if (PawnName.Contains("Triceratops"))
        {
            BehaviorData.Species = EAnim_DinosaurSpecies::Triceratops;
            BodyMassScale = 1.8f;
            TailSwayIntensity = 0.5f;
            NeckFlexibility = 0.4f;
        }
        else if (PawnName.Contains("Brachiosaurus"))
        {
            BehaviorData.Species = EAnim_DinosaurSpecies::Brachiosaurus;
            BodyMassScale = 3.0f;
            TailSwayIntensity = 0.3f;
            NeckFlexibility = 1.8f;
        }
    }

    UE_LOG(LogAnimation, Log, TEXT("DinosaurAnimInstance initialized for species: %d"), (int32)BehaviorData.Species);
}

void UAnim_DinosaurAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);

    AnimationDeltaTime = DeltaTimeX;
    
    if (!OwningPawn)
    {
        return;
    }

    // Update all animation data
    UpdateMotionData(DeltaTimeX);
    UpdateBehaviorData(DeltaTimeX);
    UpdateFootIK(DeltaTimeX);
    UpdateHeadTracking(DeltaTimeX);
    UpdateSpeciesParameters(DeltaTimeX);

    LastUpdateTime = GetWorld()->GetTimeSeconds();
}

void UAnim_DinosaurAnimInstance::UpdateMotionData(float DeltaTime)
{
    if (!OwningPawn || !MovementComponent)
    {
        return;
    }

    // Get velocity and speed
    MotionData.Velocity = MovementComponent->Velocity;
    MotionData.Speed = MotionData.Velocity.Size();
    
    // Calculate movement direction relative to actor forward
    if (MotionData.Speed > 1.0f)
    {
        FVector ForwardVector = OwningPawn->GetActorForwardVector();
        FVector VelocityNormalized = MotionData.Velocity.GetSafeNormal();
        MotionData.Direction = FVector::DotProduct(ForwardVector, VelocityNormalized);
        MotionData.bIsMoving = true;
    }
    else
    {
        MotionData.Direction = 0.0f;
        MotionData.bIsMoving = false;
    }

    // Check if grounded
    MotionData.bIsGrounded = MovementComponent->IsMovingOnGround();
    
    // Calculate ground distance for landing detection
    if (!MotionData.bIsGrounded)
    {
        FVector StartLocation = OwningPawn->GetActorLocation();
        FVector EndLocation = StartLocation - FVector(0, 0, 1000.0f);
        
        FHitResult HitResult;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(OwningPawn);
        
        if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_WorldStatic, QueryParams))
        {
            MotionData.GroundDistance = HitResult.Distance;
        }
        else
        {
            MotionData.GroundDistance = 1000.0f;
        }
    }
    else
    {
        MotionData.GroundDistance = 0.0f;
    }

    // Update animation blending based on speed
    float SpeedThresholds[3] = { 50.0f, 200.0f, 400.0f }; // Idle, Walk, Run thresholds
    
    if (MotionData.Speed < SpeedThresholds[0])
    {
        IdleToWalkBlend = FMath::FInterpTo(IdleToWalkBlend, 0.0f, DeltaTime, 5.0f);
    }
    else if (MotionData.Speed < SpeedThresholds[1])
    {
        IdleToWalkBlend = FMath::FInterpTo(IdleToWalkBlend, 1.0f, DeltaTime, 5.0f);
        WalkToRunBlend = FMath::FInterpTo(WalkToRunBlend, 0.0f, DeltaTime, 5.0f);
    }
    else
    {
        IdleToWalkBlend = 1.0f;
        float RunBlendAlpha = FMath::Clamp((MotionData.Speed - SpeedThresholds[1]) / (SpeedThresholds[2] - SpeedThresholds[1]), 0.0f, 1.0f);
        WalkToRunBlend = FMath::FInterpTo(WalkToRunBlend, RunBlendAlpha, DeltaTime, 3.0f);
    }
}

void UAnim_DinosaurAnimInstance::UpdateBehaviorData(float DeltaTime)
{
    if (!OwningPawn)
    {
        return;
    }

    // Simple state machine based on motion and external factors
    EAnim_DinosaurState NewState = BehaviorData.CurrentState;

    if (MotionData.bIsMoving)
    {
        if (MotionData.Speed > 300.0f)
        {
            NewState = EAnim_DinosaurState::Running;
        }
        else if (MotionData.Speed > 50.0f)
        {
            NewState = EAnim_DinosaurState::Walking;
        }
        else
        {
            NewState = EAnim_DinosaurState::Roaming;
        }
    }
    else
    {
        NewState = EAnim_DinosaurState::Idle;
    }

    // Smooth state transitions
    if (NewState != BehaviorData.CurrentState)
    {
        BehaviorData.CurrentState = NewState;
    }

    // Update behavior parameters
    BehaviorData.AggressionLevel = FMath::FInterpTo(BehaviorData.AggressionLevel, 
        BehaviorData.bHasTarget ? 1.0f : 0.0f, DeltaTime, 2.0f);

    // Update feeding blend based on state
    if (BehaviorData.CurrentState == EAnim_DinosaurState::Feeding)
    {
        FeedingBlend = FMath::FInterpTo(FeedingBlend, 1.0f, DeltaTime, 3.0f);
    }
    else
    {
        FeedingBlend = FMath::FInterpTo(FeedingBlend, 0.0f, DeltaTime, 5.0f);
    }

    // Update attack intensity
    if (BehaviorData.CurrentState == EAnim_DinosaurState::Attacking)
    {
        AttackIntensity = FMath::FInterpTo(AttackIntensity, 1.0f, DeltaTime, 8.0f);
    }
    else
    {
        AttackIntensity = FMath::FInterpTo(AttackIntensity, 0.0f, DeltaTime, 4.0f);
    }
}

void UAnim_DinosaurAnimInstance::UpdateFootIK(float DeltaTime)
{
    if (!OwningPawn || !MeshComponent || !MotionData.bIsGrounded)
    {
        // Reset IK when not grounded
        LeftFootIKOffset = FVector::ZeroVector;
        RightFootIKOffset = FVector::ZeroVector;
        LeftFootIKRotation = FRotator::ZeroRotator;
        RightFootIKRotation = FRotator::ZeroRotator;
        return;
    }

    // Trace for left foot
    FVector LeftFootTrace = TraceFootIK(FName("foot_l"), 100.0f);
    if (!LeftFootTrace.IsZero())
    {
        LeftFootIKOffset = FMath::VInterpTo(LeftFootIKOffset, LeftFootTrace, DeltaTime, 10.0f);
    }

    // Trace for right foot
    FVector RightFootTrace = TraceFootIK(FName("foot_r"), 100.0f);
    if (!RightFootTrace.IsZero())
    {
        RightFootIKOffset = FMath::VInterpTo(RightFootIKOffset, RightFootTrace, DeltaTime, 10.0f);
    }

    // Calculate foot rotations based on ground normal
    // This would require more complex ground normal calculation in a real implementation
    LeftFootIKRotation = FMath::RInterpTo(LeftFootIKRotation, FRotator::ZeroRotator, DeltaTime, 8.0f);
    RightFootIKRotation = FMath::RInterpTo(RightFootIKRotation, FRotator::ZeroRotator, DeltaTime, 8.0f);
}

void UAnim_DinosaurAnimInstance::UpdateHeadTracking(float DeltaTime)
{
    if (!OwningPawn)
    {
        return;
    }

    // Simple head tracking - look at player if nearby
    if (GetWorld())
    {
        APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
        if (PlayerPawn)
        {
            float DistanceToPlayer = FVector::Dist(OwningPawn->GetActorLocation(), PlayerPawn->GetActorLocation());
            
            if (DistanceToPlayer < 2000.0f && DistanceToPlayer > 100.0f)
            {
                HeadLookAtTarget = PlayerPawn->GetActorLocation();
                HeadLookAtAlpha = FMath::FInterpTo(HeadLookAtAlpha, 1.0f, DeltaTime, 2.0f);
            }
            else
            {
                HeadLookAtAlpha = FMath::FInterpTo(HeadLookAtAlpha, 0.0f, DeltaTime, 3.0f);
            }
        }
    }
}

void UAnim_DinosaurAnimInstance::UpdateSpeciesParameters(float DeltaTime)
{
    // Species-specific animation adjustments
    switch (BehaviorData.Species)
    {
        case EAnim_DinosaurSpecies::TRex:
            // Heavy, powerful movements
            TailSwayIntensity = FMath::FInterpTo(TailSwayIntensity, 0.8f * MotionData.Speed / 400.0f, DeltaTime, 2.0f);
            break;
            
        case EAnim_DinosaurSpecies::Velociraptor:
            // Quick, agile movements
            TailSwayIntensity = FMath::FInterpTo(TailSwayIntensity, 1.5f * MotionData.Speed / 400.0f, DeltaTime, 5.0f);
            break;
            
        case EAnim_DinosaurSpecies::Brachiosaurus:
            // Slow, graceful movements
            TailSwayIntensity = FMath::FInterpTo(TailSwayIntensity, 0.3f * MotionData.Speed / 200.0f, DeltaTime, 1.0f);
            break;
            
        default:
            TailSwayIntensity = FMath::FInterpTo(TailSwayIntensity, 1.0f * MotionData.Speed / 300.0f, DeltaTime, 3.0f);
            break;
    }

    // Clamp values
    TailSwayIntensity = FMath::Clamp(TailSwayIntensity, 0.0f, 2.0f);
}

FVector UAnim_DinosaurAnimInstance::TraceFootIK(const FName& SocketName, float TraceDistance)
{
    if (!MeshComponent || !OwningPawn)
    {
        return FVector::ZeroVector;
    }

    // Get socket location
    FVector SocketLocation = MeshComponent->GetSocketLocation(SocketName);
    if (SocketLocation.IsZero())
    {
        return FVector::ZeroVector;
    }

    // Trace downward from socket
    FVector StartLocation = SocketLocation;
    FVector EndLocation = StartLocation - FVector(0, 0, TraceDistance);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwningPawn);
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_WorldStatic, QueryParams))
    {
        // Calculate offset needed to place foot on ground
        float GroundHeight = HitResult.ImpactPoint.Z;
        float SocketHeight = SocketLocation.Z;
        float OffsetZ = GroundHeight - SocketHeight;
        
        return FVector(0, 0, OffsetZ);
    }
    
    return FVector::ZeroVector;
}

FRotator UAnim_DinosaurAnimInstance::CalculateFootRotation(const FVector& ImpactNormal)
{
    // Calculate rotation to align foot with ground normal
    FVector UpVector = FVector::UpVector;
    FVector RightVector = FVector::CrossProduct(UpVector, ImpactNormal).GetSafeNormal();
    FVector ForwardVector = FVector::CrossProduct(ImpactNormal, RightVector).GetSafeNormal();
    
    return UKismetMathLibrary::MakeRotationFromAxes(ForwardVector, RightVector, ImpactNormal);
}

FVector UAnim_DinosaurAnimInstance::CalculateHeadLookAt(const FVector& TargetLocation)
{
    if (!OwningPawn)
    {
        return FVector::ZeroVector;
    }

    FVector HeadLocation = OwningPawn->GetActorLocation() + FVector(0, 0, 100); // Approximate head height
    FVector LookDirection = (TargetLocation - HeadLocation).GetSafeNormal();
    
    return LookDirection;
}