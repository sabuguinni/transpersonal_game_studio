#include "ProceduralIKSystem.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UProceduralIKSystem::UProceduralIKSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    
    // Default IK settings
    bEnableFootIK = true;
    bEnableHandIK = false;
    bEnableLookAt = false;
    bEnableSpineAdjustment = true;
    
    // Foot IK parameters
    FootTraceDistance = 100.0f;
    FootTraceUpOffset = 50.0f;
    FootIKInterpSpeed = 15.0f;
    MaxFootAdjustment = 50.0f;
    
    // Hand IK parameters
    HandIKInterpSpeed = 10.0f;
    MaxHandReach = 150.0f;
    
    // Look at parameters
    LookAtInterpSpeed = 5.0f;
    MaxLookAtAngle = 90.0f;
    
    // Spine adjustment parameters
    SpineAdjustmentInterpSpeed = 8.0f;
    MaxSpineAdjustment = 30.0f;
    
    // Debug settings
    bShowDebugTraces = false;
    
    // Initialize offset vectors
    LeftFootIKOffset = FVector::ZeroVector;
    RightFootIKOffset = FVector::ZeroVector;
    LeftHandIKOffset = FVector::ZeroVector;
    RightHandIKOffset = FVector::ZeroVector;
    SpineAdjustmentOffset = FVector::ZeroVector;
    LookAtRotation = FRotator::ZeroRotator;
    
    // Bone names (these should match the skeleton)
    LeftFootBoneName = "foot_l";
    RightFootBoneName = "foot_r";
    LeftHandBoneName = "hand_l";
    RightHandBoneName = "hand_r";
    SpineBoneName = "spine_01";
    HeadBoneName = "head";
}

void UProceduralIKSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache owner character
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        SkeletalMeshComponent = OwnerCharacter->GetMesh();
    }
    
    // Initialize IK targets
    InitializeIKTargets();
}

void UProceduralIKSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerCharacter || !SkeletalMeshComponent)
    {
        return;
    }
    
    // Update foot IK
    if (bEnableFootIK)
    {
        UpdateFootIK(DeltaTime);
    }
    
    // Update hand IK
    if (bEnableHandIK)
    {
        UpdateHandIK(DeltaTime);
    }
    
    // Update look at
    if (bEnableLookAt && LookAtTarget)
    {
        UpdateLookAt(DeltaTime);
    }
    
    // Update spine adjustment
    if (bEnableSpineAdjustment)
    {
        UpdateSpineAdjustment(DeltaTime);
    }
}

void UProceduralIKSystem::InitializeIKTargets()
{
    if (!SkeletalMeshComponent)
    {
        return;
    }
    
    // Initialize foot IK targets based on current bone positions
    LeftFootIKTarget = GetBoneWorldLocation(LeftFootBoneName);
    RightFootIKTarget = GetBoneWorldLocation(RightFootBoneName);
    
    // Initialize hand IK targets
    LeftHandIKTarget = GetBoneWorldLocation(LeftHandBoneName);
    RightHandIKTarget = GetBoneWorldLocation(RightHandBoneName);
}

void UProceduralIKSystem::UpdateFootIK(float DeltaTime)
{
    if (!SkeletalMeshComponent)
    {
        return;
    }
    
    // Update left foot IK
    FVector LeftFootTarget = CalculateFootIKTarget(LeftFootBoneName, DeltaTime);
    LeftFootIKOffset = FMath::VInterpTo(LeftFootIKOffset, LeftFootTarget, DeltaTime, FootIKInterpSpeed);
    
    // Update right foot IK
    FVector RightFootTarget = CalculateFootIKTarget(RightFootBoneName, DeltaTime);
    RightFootIKOffset = FMath::VInterpTo(RightFootIKOffset, RightFootTarget, DeltaTime, FootIKInterpSpeed);
    
    // Clamp foot adjustments to maximum values
    LeftFootIKOffset = ClampVector(LeftFootIKOffset, MaxFootAdjustment);
    RightFootIKOffset = ClampVector(RightFootIKOffset, MaxFootAdjustment);
}

FVector UProceduralIKSystem::CalculateFootIKTarget(const FString& FootBoneName, float DeltaTime)
{
    if (!OwnerCharacter || !SkeletalMeshComponent)
    {
        return FVector::ZeroVector;
    }
    
    // Get foot bone world location
    FVector FootLocation = GetBoneWorldLocation(FootBoneName);
    
    // Setup trace parameters
    FVector TraceStart = FootLocation + FVector(0, 0, FootTraceUpOffset);
    FVector TraceEnd = FootLocation - FVector(0, 0, FootTraceDistance);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    QueryParams.bTraceComplex = false;
    
    // Perform line trace
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );
    
    // Draw debug trace if enabled
    if (bShowDebugTraces)
    {
        FColor TraceColor = bHit ? FColor::Green : FColor::Red;
        DrawDebugLine(GetWorld(), TraceStart, TraceEnd, TraceColor, false, 0.1f, 0, 1.0f);
        
        if (bHit)
        {
            DrawDebugSphere(GetWorld(), HitResult.Location, 5.0f, 12, FColor::Yellow, false, 0.1f);
        }
    }
    
    if (bHit)
    {
        // Calculate the offset needed to place foot on ground
        float GroundZ = HitResult.Location.Z;
        float FootZ = FootLocation.Z;
        float ZOffset = GroundZ - FootZ;
        
        // Apply surface normal adjustment for slopes
        FVector SurfaceNormal = HitResult.Normal;
        FVector AdjustmentVector = FVector(0, 0, ZOffset);
        
        // Adjust for surface angle
        if (!SurfaceNormal.IsNearlyEqual(FVector::UpVector))
        {
            FVector ProjectedNormal = FVector::VectorPlaneProject(SurfaceNormal, FVector::UpVector);
            float SlopeAngle = FMath::Acos(FVector::DotProduct(SurfaceNormal, FVector::UpVector));
            
            // Apply additional offset based on slope
            AdjustmentVector += ProjectedNormal * FMath::Sin(SlopeAngle) * 10.0f;
        }
        
        return AdjustmentVector;
    }
    
    // No ground found, return to neutral position
    return FVector::ZeroVector;
}

void UProceduralIKSystem::UpdateHandIK(float DeltaTime)
{
    if (!SkeletalMeshComponent)
    {
        return;
    }
    
    // Update left hand IK if target is set
    if (LeftHandIKTargetActor)
    {
        FVector TargetLocation = LeftHandIKTargetActor->GetActorLocation();
        FVector CurrentHandLocation = GetBoneWorldLocation(LeftHandBoneName);
        FVector DesiredOffset = TargetLocation - CurrentHandLocation;
        
        // Check if target is within reach
        if (DesiredOffset.Size() <= MaxHandReach)
        {
            LeftHandIKOffset = FMath::VInterpTo(LeftHandIKOffset, DesiredOffset, DeltaTime, HandIKInterpSpeed);
        }
    }
    else
    {
        // Return to neutral position
        LeftHandIKOffset = FMath::VInterpTo(LeftHandIKOffset, FVector::ZeroVector, DeltaTime, HandIKInterpSpeed);
    }
    
    // Update right hand IK if target is set
    if (RightHandIKTargetActor)
    {
        FVector TargetLocation = RightHandIKTargetActor->GetActorLocation();
        FVector CurrentHandLocation = GetBoneWorldLocation(RightHandBoneName);
        FVector DesiredOffset = TargetLocation - CurrentHandLocation;
        
        // Check if target is within reach
        if (DesiredOffset.Size() <= MaxHandReach)
        {
            RightHandIKOffset = FMath::VInterpTo(RightHandIKOffset, DesiredOffset, DeltaTime, HandIKInterpSpeed);
        }
    }
    else
    {
        // Return to neutral position
        RightHandIKOffset = FMath::VInterpTo(RightHandIKOffset, FVector::ZeroVector, DeltaTime, HandIKInterpSpeed);
    }
}

void UProceduralIKSystem::UpdateLookAt(float DeltaTime)
{
    if (!LookAtTarget || !SkeletalMeshComponent)
    {
        return;
    }
    
    // Calculate look at rotation
    FVector HeadLocation = GetBoneWorldLocation(HeadBoneName);
    FVector TargetLocation = LookAtTarget->GetActorLocation();
    FVector LookDirection = (TargetLocation - HeadLocation).GetSafeNormal();
    
    // Convert to rotation
    FRotator DesiredRotation = UKismetMathLibrary::FindLookAtRotation(HeadLocation, TargetLocation);
    FRotator CurrentRotation = SkeletalMeshComponent->GetBoneQuaternion(HeadBoneName, EBoneSpaces::WorldSpace).Rotator();
    
    // Clamp rotation to maximum angle
    FRotator DeltaRotation = DesiredRotation - CurrentRotation;
    DeltaRotation.Pitch = FMath::ClampAngle(DeltaRotation.Pitch, -MaxLookAtAngle, MaxLookAtAngle);
    DeltaRotation.Yaw = FMath::ClampAngle(DeltaRotation.Yaw, -MaxLookAtAngle, MaxLookAtAngle);
    
    // Interpolate to target rotation
    LookAtRotation = FMath::RInterpTo(LookAtRotation, DeltaRotation, DeltaTime, LookAtInterpSpeed);
}

void UProceduralIKSystem::UpdateSpineAdjustment(float DeltaTime)
{
    if (!SkeletalMeshComponent)
    {
        return;
    }
    
    // Calculate spine adjustment based on foot IK offsets
    float LeftFootZ = LeftFootIKOffset.Z;
    float RightFootZ = RightFootIKOffset.Z;
    
    // Average the foot offsets to determine spine adjustment
    float AverageFootOffset = (LeftFootZ + RightFootZ) * 0.5f;
    
    // Apply spine adjustment to keep character balanced
    FVector DesiredSpineAdjustment = FVector(0, 0, -AverageFootOffset * 0.5f);
    
    // Clamp spine adjustment
    DesiredSpineAdjustment = ClampVector(DesiredSpineAdjustment, MaxSpineAdjustment);
    
    // Interpolate to target adjustment
    SpineAdjustmentOffset = FMath::VInterpTo(SpineAdjustmentOffset, DesiredSpineAdjustment, DeltaTime, SpineAdjustmentInterpSpeed);
}

FVector UProceduralIKSystem::GetBoneWorldLocation(const FString& BoneName) const
{
    if (!SkeletalMeshComponent)
    {
        return FVector::ZeroVector;
    }
    
    return SkeletalMeshComponent->GetBoneLocation(*BoneName, EBoneSpaces::WorldSpace);
}

FVector UProceduralIKSystem::ClampVector(const FVector& Vector, float MaxMagnitude) const
{
    if (Vector.SizeSquared() > MaxMagnitude * MaxMagnitude)
    {
        return Vector.GetSafeNormal() * MaxMagnitude;
    }
    return Vector;
}

void UProceduralIKSystem::SetLeftHandIKTarget(AActor* Target)
{
    LeftHandIKTargetActor = Target;
}

void UProceduralIKSystem::SetRightHandIKTarget(AActor* Target)
{
    RightHandIKTargetActor = Target;
}

void UProceduralIKSystem::SetLookAtTarget(AActor* Target)
{
    LookAtTarget = Target;
}

void UProceduralIKSystem::EnableFootIK(bool bEnable)
{
    bEnableFootIK = bEnable;
    
    if (!bEnable)
    {
        // Reset foot IK offsets
        LeftFootIKOffset = FVector::ZeroVector;
        RightFootIKOffset = FVector::ZeroVector;
    }
}

void UProceduralIKSystem::EnableHandIK(bool bEnable)
{
    bEnableHandIK = bEnable;
    
    if (!bEnable)
    {
        // Reset hand IK offsets
        LeftHandIKOffset = FVector::ZeroVector;
        RightHandIKOffset = FVector::ZeroVector;
    }
}

void UProceduralIKSystem::EnableLookAt(bool bEnable)
{
    bEnableLookAt = bEnable;
    
    if (!bEnable)
    {
        // Reset look at rotation
        LookAtRotation = FRotator::ZeroRotator;
    }
}

void UProceduralIKSystem::SetFootIKInterpSpeed(float Speed)
{
    FootIKInterpSpeed = FMath::Max(Speed, 0.1f);
}

void UProceduralIKSystem::SetHandIKInterpSpeed(float Speed)
{
    HandIKInterpSpeed = FMath::Max(Speed, 0.1f);
}

void UProceduralIKSystem::SetLookAtInterpSpeed(float Speed)
{
    LookAtInterpSpeed = FMath::Max(Speed, 0.1f);
}