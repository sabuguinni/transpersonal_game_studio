#include "Anim_AdvancedIKSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "CollisionQueryParams.h"

UAnim_AdvancedIKSystem::UAnim_AdvancedIKSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Initialize default values
    FootTraceDistance = 100.0f;
    FootIKInterpSpeed = 15.0f;
    bEnableFootIK = true;
    HandIKWeight = 1.0f;
    bEnableHandIK = false;
    LookAtWeight = 1.0f;
    bEnableLookAtIK = false;
    
    LeftHandTarget = FVector::ZeroVector;
    RightHandTarget = FVector::ZeroVector;
    LookAtTarget = FVector::ZeroVector;
}

void UAnim_AdvancedIKSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Get owner's skeletal mesh component
    AActor* Owner = GetOwner();
    if (Owner)
    {
        OwnerMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
        if (OwnerMesh)
        {
            AnimInstance = OwnerMesh->GetAnimInstance();
        }
    }
}

void UAnim_AdvancedIKSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerMesh || !AnimInstance)
    {
        return;
    }
    
    // Update all IK systems
    if (bEnableFootIK)
    {
        UpdateFootIK(DeltaTime);
    }
    
    if (bEnableHandIK)
    {
        UpdateHandIK(DeltaTime);
    }
    
    if (bEnableLookAtIK)
    {
        UpdateLookAtIK(DeltaTime);
    }
    
    // Process custom IK chains
    for (FAnim_IKChainData& ChainData : IKChains)
    {
        if (ChainData.ChainWeight > 0.0f)
        {
            SolveIKChain(ChainData);
        }
    }
}

void UAnim_AdvancedIKSystem::UpdateFootIK(float DeltaTime)
{
    if (!OwnerMesh)
    {
        return;
    }
    
    // Update left foot IK
    CalculateFootIKData(LeftFootIK, FName("foot_l"), DeltaTime);
    
    // Update right foot IK
    CalculateFootIKData(RightFootIK, FName("foot_r"), DeltaTime);
}

void UAnim_AdvancedIKSystem::CalculateFootIKData(FAnim_FootIKData& FootData, const FName& FootBoneName, float DeltaTime)
{
    if (!OwnerMesh)
    {
        return;
    }
    
    // Get foot bone location in world space
    FVector FootBoneLocation = OwnerMesh->GetBoneLocation(FootBoneName, EBoneSpaceTransform::WorldSpace);
    
    // Trace downward from foot bone
    FVector TraceStart = FootBoneLocation + FVector(0, 0, 50.0f);
    FVector TraceEnd = FootBoneLocation - FVector(0, 0, FootTraceDistance);
    
    FVector HitLocation;
    FVector HitNormal;
    bool bHit = TraceForGround(TraceStart, HitLocation, HitNormal);
    
    if (bHit)
    {
        // Calculate distance to ground
        float DistanceToGround = FVector::Dist(FootBoneLocation, HitLocation);
        FootData.GroundDistance = DistanceToGround;
        FootData.bIsGrounded = DistanceToGround < 20.0f; // Within 20cm of ground
        
        // Calculate foot rotation to align with surface normal
        FVector ForwardVector = OwnerMesh->GetForwardVector();
        FVector RightVector = FVector::CrossProduct(HitNormal, ForwardVector).GetSafeNormal();
        ForwardVector = FVector::CrossProduct(RightVector, HitNormal).GetSafeNormal();
        
        FRotator TargetRotation = UKismetMathLibrary::MakeRotationFromAxes(ForwardVector, RightVector, HitNormal);
        
        // Interpolate foot position and rotation
        FootData.FootLocation = FMath::VInterpTo(FootData.FootLocation, HitLocation, DeltaTime, FootIKInterpSpeed);
        FootData.FootRotation = FMath::RInterpTo(FootData.FootRotation, TargetRotation, DeltaTime, FootIKInterpSpeed);
        
        // Calculate IK weight based on ground distance
        float TargetWeight = FootData.bIsGrounded ? 1.0f : FMath::Clamp(1.0f - (DistanceToGround / FootTraceDistance), 0.0f, 1.0f);
        FootData.IKWeight = FMath::FInterpTo(FootData.IKWeight, TargetWeight, DeltaTime, FootIKInterpSpeed);
    }
    else
    {
        // No ground found, reduce IK weight
        FootData.IKWeight = FMath::FInterpTo(FootData.IKWeight, 0.0f, DeltaTime, FootIKInterpSpeed);
        FootData.bIsGrounded = false;
    }
}

void UAnim_AdvancedIKSystem::UpdateHandIK(float DeltaTime)
{
    // Hand IK implementation for interaction with objects
    // This would be expanded based on specific interaction needs
}

void UAnim_AdvancedIKSystem::UpdateLookAtIK(float DeltaTime)
{
    // Look-at IK implementation for head/neck tracking
    // This would control head orientation to look at targets
}

void UAnim_AdvancedIKSystem::SolveIKChain(FAnim_IKChainData& ChainData)
{
    if (!OwnerMesh || ChainData.StartBone == NAME_None || ChainData.EndBone == NAME_None)
    {
        return;
    }
    
    switch (ChainData.SolverType)
    {
        case EAnim_IKSolverType::TwoBone:
        {
            // Two-bone IK solver implementation
            FVector StartPos = OwnerMesh->GetBoneLocation(ChainData.StartBone, EBoneSpaceTransform::WorldSpace);
            FVector MiddlePos = OwnerMesh->GetBoneLocation(ChainData.MiddleBone, EBoneSpaceTransform::WorldSpace);
            FVector EndPos = OwnerMesh->GetBoneLocation(ChainData.EndBone, EBoneSpaceTransform::WorldSpace);
            
            float UpperLength = FVector::Dist(StartPos, MiddlePos);
            float LowerLength = FVector::Dist(MiddlePos, EndPos);
            
            FVector SolvedPos = SolveTwoBoneIK(StartPos, MiddlePos, EndPos, ChainData.TargetLocation, UpperLength, LowerLength);
            
            // Apply the solved position (this would require more complex bone manipulation)
            break;
        }
        case EAnim_IKSolverType::FABRIK:
        case EAnim_IKSolverType::CCDIK:
        case EAnim_IKSolverType::Custom:
        default:
            // Other IK solver implementations would go here
            break;
    }
}

FVector UAnim_AdvancedIKSystem::SolveTwoBoneIK(const FVector& RootPos, const FVector& JointPos, const FVector& EndPos, const FVector& TargetPos, float UpperLength, float LowerLength)
{
    // Simplified two-bone IK solver
    FVector ToTarget = TargetPos - RootPos;
    float TargetDistance = ToTarget.Size();
    
    // Clamp target distance to reachable range
    float MaxReach = UpperLength + LowerLength;
    if (TargetDistance > MaxReach)
    {
        TargetDistance = MaxReach;
        ToTarget = ToTarget.GetSafeNormal() * TargetDistance;
    }
    
    // Calculate joint position using law of cosines
    float CosAngle = (UpperLength * UpperLength + TargetDistance * TargetDistance - LowerLength * LowerLength) / (2.0f * UpperLength * TargetDistance);
    CosAngle = FMath::Clamp(CosAngle, -1.0f, 1.0f);
    
    float Angle = FMath::Acos(CosAngle);
    
    FVector ToTargetNorm = ToTarget.GetSafeNormal();
    FVector PerpendicularAxis = FVector::CrossProduct(ToTargetNorm, FVector::UpVector).GetSafeNormal();
    
    FVector JointDirection = ToTargetNorm.RotateAngleAxis(FMath::RadiansToDegrees(Angle), PerpendicularAxis);
    
    return RootPos + JointDirection * UpperLength;
}

bool UAnim_AdvancedIKSystem::TraceForGround(const FVector& StartLocation, FVector& OutHitLocation, FVector& OutHitNormal)
{
    if (!GetWorld())
    {
        return false;
    }
    
    FHitResult HitResult;
    FVector EndLocation = StartLocation - FVector(0, 0, FootTraceDistance);
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECC_WorldStatic,
        QueryParams
    );
    
    if (bHit)
    {
        OutHitLocation = HitResult.Location;
        OutHitNormal = HitResult.Normal;
        return true;
    }
    
    return false;
}

void UAnim_AdvancedIKSystem::SetIKChainTarget(int32 ChainIndex, const FVector& TargetLocation, const FRotator& TargetRotation)
{
    if (IKChains.IsValidIndex(ChainIndex))
    {
        IKChains[ChainIndex].TargetLocation = TargetLocation;
        IKChains[ChainIndex].TargetRotation = TargetRotation;
    }
}

void UAnim_AdvancedIKSystem::SetFootIKEnabled(bool bEnabled)
{
    bEnableFootIK = bEnabled;
}

void UAnim_AdvancedIKSystem::SetHandIKEnabled(bool bEnabled)
{
    bEnableHandIK = bEnabled;
}

void UAnim_AdvancedIKSystem::SetLookAtIKEnabled(bool bEnabled)
{
    bEnableLookAtIK = bEnabled;
}