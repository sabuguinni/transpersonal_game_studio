#include "Anim_TerrainIKSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UAnim_TerrainIKSystem::UAnim_TerrainIKSystem()
{
    OwningSkeletalMesh = nullptr;
    OwningAnimInstance = nullptr;
    
    TraceDistance = 100.0f;
    FootOffset = 5.0f;
    InterpSpeed = 10.0f;
    bEnableDebugDraw = false;
    
    LeftFootBoneName = TEXT("foot_l");
    RightFootBoneName = TEXT("foot_r");
    PelvisBoneName = TEXT("pelvis");
    
    TraceChannel = ECC_Visibility;
    
    IKData = FAnim_TerrainIKData();
}

void UAnim_TerrainIKSystem::InitializeIK(USkeletalMeshComponent* SkeletalMesh, UAnimInstance* AnimInstance)
{
    OwningSkeletalMesh = SkeletalMesh;
    OwningAnimInstance = AnimInstance;
    
    if (OwningSkeletalMesh)
    {
        // Initialize IK data with current bone positions
        IKData.LeftFoot.FootLocation = GetBoneWorldLocation(LeftFootBoneName);
        IKData.RightFoot.FootLocation = GetBoneWorldLocation(RightFootBoneName);
        IKData.bIKEnabled = true;
    }
}

void UAnim_TerrainIKSystem::UpdateTerrainIK(float DeltaTime)
{
    if (!OwningSkeletalMesh || !OwningAnimInstance || !IKData.bIKEnabled)
    {
        return;
    }
    
    // Update foot IK for both feet
    UpdateFootIK(LeftFootBoneName, IKData.LeftFoot, DeltaTime);
    UpdateFootIK(RightFootBoneName, IKData.RightFoot, DeltaTime);
    
    // Update pelvis offset based on foot positions
    UpdatePelvisOffset(DeltaTime);
}

void UAnim_TerrainIKSystem::UpdateFootIK(const FName& FootBoneName, FAnim_IKFootData& FootData, float DeltaTime)
{
    if (!OwningSkeletalMesh)
    {
        return;
    }
    
    // Get current foot bone world location
    FVector FootBoneLocation = GetBoneWorldLocation(FootBoneName);
    
    // Perform trace to find ground
    if (PerformFootTrace(FootBoneLocation, FootData))
    {
        // Interpolate to new position
        FootData.FootLocation = FMath::VInterpTo(FootData.FootLocation, 
                                               FootData.FootLocation, 
                                               DeltaTime, 
                                               InterpSpeed);
        
        // Update IK alpha based on distance from ground
        float MaxIKDistance = TraceDistance * 0.5f;
        FootData.IKAlpha = FMath::Clamp(1.0f - (FootData.DistanceFromGround / MaxIKDistance), 0.0f, 1.0f);
    }
    else
    {
        // No ground found, disable IK for this foot
        FootData.IKAlpha = FMath::FInterpTo(FootData.IKAlpha, 0.0f, DeltaTime, InterpSpeed);
        FootData.bIsGrounded = false;
    }
}

void UAnim_TerrainIKSystem::UpdatePelvisOffset(float DeltaTime)
{
    if (!IKData.LeftFoot.bIsGrounded && !IKData.RightFoot.bIsGrounded)
    {
        // No feet on ground, reset pelvis offset
        IKData.PelvisOffset = FMath::FInterpTo(IKData.PelvisOffset, 0.0f, DeltaTime, InterpSpeed);
        return;
    }
    
    // Calculate desired pelvis offset based on foot positions
    float LeftFootOffset = IKData.LeftFoot.bIsGrounded ? IKData.LeftFoot.DistanceFromGround : 0.0f;
    float RightFootOffset = IKData.RightFoot.bIsGrounded ? IKData.RightFoot.DistanceFromGround : 0.0f;
    
    // Use the smaller offset to keep both feet on ground
    float TargetPelvisOffset = FMath::Min(LeftFootOffset, RightFootOffset);
    TargetPelvisOffset = FMath::Clamp(TargetPelvisOffset, -TraceDistance * 0.5f, TraceDistance * 0.5f);
    
    // Interpolate pelvis offset
    IKData.PelvisOffset = FMath::FInterpTo(IKData.PelvisOffset, TargetPelvisOffset, DeltaTime, InterpSpeed);
}

bool UAnim_TerrainIKSystem::PerformFootTrace(const FVector& FootLocation, FAnim_IKFootData& OutFootData)
{
    if (!OwningSkeletalMesh)
    {
        return false;
    }
    
    UWorld* World = OwningSkeletalMesh->GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Setup trace parameters
    FVector TraceStart = FootLocation + FVector(0.0f, 0.0f, TraceDistance * 0.5f);
    FVector TraceEnd = FootLocation - FVector(0.0f, 0.0f, TraceDistance);
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwningSkeletalMesh->GetOwner());
    QueryParams.bTraceComplex = true;
    
    FHitResult HitResult;
    bool bHit = World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, TraceChannel, QueryParams);
    
    if (bHit)
    {
        // Calculate foot position with offset
        FVector GroundLocation = HitResult.ImpactPoint + FVector(0.0f, 0.0f, FootOffset);
        
        // Update foot data
        OutFootData.FootLocation = GroundLocation;
        OutFootData.FootRotation = CalculateFootRotation(HitResult.ImpactNormal);
        OutFootData.DistanceFromGround = FVector::Dist(FootLocation, GroundLocation);
        OutFootData.bIsGrounded = true;
        
        // Debug drawing
        if (bEnableDebugDraw)
        {
            DrawDebugInfo(OutFootData, TraceStart, TraceEnd);
        }
        
        return true;
    }
    else
    {
        OutFootData.bIsGrounded = false;
        return false;
    }
}

FVector UAnim_TerrainIKSystem::GetBoneWorldLocation(const FName& BoneName) const
{
    if (!OwningSkeletalMesh)
    {
        return FVector::ZeroVector;
    }
    
    int32 BoneIndex = OwningSkeletalMesh->GetBoneIndex(BoneName);
    if (BoneIndex == INDEX_NONE)
    {
        return FVector::ZeroVector;
    }
    
    return OwningSkeletalMesh->GetBoneLocation(BoneName);
}

FRotator UAnim_TerrainIKSystem::CalculateFootRotation(const FVector& ImpactNormal) const
{
    // Calculate foot rotation to align with ground normal
    FVector UpVector = ImpactNormal;
    FVector ForwardVector = FVector::ForwardVector;
    
    // Make sure forward vector is perpendicular to up vector
    ForwardVector = ForwardVector - (FVector::DotProduct(ForwardVector, UpVector) * UpVector);
    ForwardVector.Normalize();
    
    FVector RightVector = FVector::CrossProduct(UpVector, ForwardVector);
    
    return UKismetMathLibrary::MakeRotationFromAxes(ForwardVector, RightVector, UpVector);
}

void UAnim_TerrainIKSystem::DrawDebugInfo(const FAnim_IKFootData& FootData, const FVector& TraceStart, const FVector& TraceEnd) const
{
    if (!OwningSkeletalMesh)
    {
        return;
    }
    
    UWorld* World = OwningSkeletalMesh->GetWorld();
    if (!World)
    {
        return;
    }
    
    // Draw trace line
    FColor LineColor = FootData.bIsGrounded ? FColor::Green : FColor::Red;
    DrawDebugLine(World, TraceStart, TraceEnd, LineColor, false, 0.1f, 0, 1.0f);
    
    // Draw foot position
    if (FootData.bIsGrounded)
    {
        DrawDebugSphere(World, FootData.FootLocation, 5.0f, 8, FColor::Blue, false, 0.1f);
    }
}

void UAnim_TerrainIKSystem::SetIKEnabled(bool bEnabled)
{
    IKData.bIKEnabled = bEnabled;
}

FAnim_TerrainIKData UAnim_TerrainIKSystem::GetIKData() const
{
    return IKData;
}

void UAnim_TerrainIKSystem::SetFootBoneName(const FName& LeftFootBone, const FName& RightFootBone)
{
    LeftFootBoneName = LeftFootBone;
    RightFootBoneName = RightFootBone;
}