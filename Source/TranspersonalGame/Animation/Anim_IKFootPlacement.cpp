#include "Anim_IKFootPlacement.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_IKFootPlacement::UAnim_IKFootPlacement()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    bFootIKEnabled = true;
    TraceDistance = 100.0f;
    IKInterpSpeed = 15.0f;
    
    // Default foot bone names for UE5 mannequin
    FootBoneNames.Add(TEXT("foot_l"));
    FootBoneNames.Add(TEXT("foot_r"));
}

void UAnim_IKFootPlacement::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize foot IK data for each bone
    for (const FName& BoneName : FootBoneNames)
    {
        FootIKDataMap.Add(BoneName, FAnim_FootIKData());
    }
}

void UAnim_IKFootPlacement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bFootIKEnabled)
    {
        return;
    }
    
    // Get the skeletal mesh component from owner
    if (AActor* Owner = GetOwner())
    {
        if (USkeletalMeshComponent* MeshComp = Owner->FindComponentByClass<USkeletalMeshComponent>())
        {
            UpdateFootIK(MeshComp);
        }
    }
}

void UAnim_IKFootPlacement::UpdateFootIK(USkeletalMeshComponent* MeshComponent)
{
    if (!MeshComponent || !MeshComponent->GetSkeletalMeshAsset())
    {
        return;
    }
    
    for (const FName& BoneName : FootBoneNames)
    {
        UpdateSingleFootIK(BoneName, MeshComponent);
    }
}

void UAnim_IKFootPlacement::UpdateSingleFootIK(const FName& BoneName, USkeletalMeshComponent* MeshComponent)
{
    if (!FootIKDataMap.Contains(BoneName))
    {
        return;
    }
    
    // Get bone transform in world space
    FTransform BoneTransform = MeshComponent->GetBoneTransform(MeshComponent->GetBoneIndex(BoneName));
    FVector BoneLocation = BoneTransform.GetLocation();
    
    // Perform trace downward from bone location
    FVector TraceStart = BoneLocation + FVector(0, 0, 50.0f);
    FVector TraceEnd = BoneLocation - FVector(0, 0, TraceDistance);
    
    bool bHit = false;
    FVector HitLocation = PerformFootTrace(TraceStart, TraceEnd, bHit);
    
    FAnim_FootIKData& FootData = FootIKDataMap[BoneName];
    
    if (bHit)
    {
        // Calculate IK offset
        float IKOffset = HitLocation.Z - BoneLocation.Z;
        
        // Interpolate to new position
        FVector TargetLocation = FVector(BoneLocation.X, BoneLocation.Y, HitLocation.Z);
        FootData.FootLocation = FMath::VInterpTo(FootData.FootLocation, TargetLocation, GetWorld()->GetDeltaSeconds(), IKInterpSpeed);
        
        // Calculate foot rotation to match surface normal
        // For now, keep rotation simple - could be enhanced with surface normal calculation
        FootData.FootRotation = FRotator::ZeroRotator;
        
        FootData.IKAlpha = FMath::FInterpTo(FootData.IKAlpha, 1.0f, GetWorld()->GetDeltaSeconds(), IKInterpSpeed);
        FootData.bIsGrounded = true;
    }
    else
    {
        // No ground found, disable IK
        FootData.IKAlpha = FMath::FInterpTo(FootData.IKAlpha, 0.0f, GetWorld()->GetDeltaSeconds(), IKInterpSpeed);
        FootData.bIsGrounded = false;
    }
}

FVector UAnim_IKFootPlacement::PerformFootTrace(const FVector& StartLocation, const FVector& EndLocation, bool& bHit)
{
    if (!GetWorld())
    {
        bHit = false;
        return FVector::ZeroVector;
    }
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    
    bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECC_Visibility,
        QueryParams
    );
    
    // Debug draw trace line
    if (CVarShowFootIKDebug.GetValueOnGameThread())
    {
        DrawDebugLine(GetWorld(), StartLocation, EndLocation, bHit ? FColor::Green : FColor::Red, false, 0.1f);
        if (bHit)
        {
            DrawDebugSphere(GetWorld(), HitResult.Location, 5.0f, 8, FColor::Yellow, false, 0.1f);
        }
    }
    
    return bHit ? HitResult.Location : EndLocation;
}

FAnim_FootIKData UAnim_IKFootPlacement::GetFootIKData(const FName& BoneName) const
{
    if (FootIKDataMap.Contains(BoneName))
    {
        return FootIKDataMap[BoneName];
    }
    
    return FAnim_FootIKData();
}

void UAnim_IKFootPlacement::SetFootIKEnabled(bool bEnabled)
{
    bFootIKEnabled = bEnabled;
    
    if (!bEnabled)
    {
        // Reset all IK data when disabled
        for (auto& Pair : FootIKDataMap)
        {
            Pair.Value.IKAlpha = 0.0f;
            Pair.Value.bIsGrounded = false;
        }
    }
}