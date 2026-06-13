#include "Anim_IKFootPlacement.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_IKFootPlacement::UAnim_IKFootPlacement()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Default settings
    bEnableIK = true;
    TraceDistance = 50.0f;
    IKInterpSpeed = 15.0f;
    FootHeight = 5.0f;
    
    // Default bone names
    LeftFootBoneName = TEXT("foot_l");
    RightFootBoneName = TEXT("foot_r");
    LeftFootIKBoneName = TEXT("ik_foot_l");
    RightFootIKBoneName = TEXT("ik_foot_r");
    
    HipOffset = 0.0f;
    OwnerMesh = nullptr;
}

void UAnim_IKFootPlacement::BeginPlay()
{
    Super::BeginPlay();
    
    // Get skeletal mesh component
    AActor* Owner = GetOwner();
    if (Owner)
    {
        OwnerMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
        if (!OwnerMesh)
        {
            // Try to get from character
            if (ACharacter* Character = Cast<ACharacter>(Owner))
            {
                OwnerMesh = Character->GetMesh();
            }
        }
    }
}

void UAnim_IKFootPlacement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bEnableIK && OwnerMesh)
    {
        UpdateFootIK(DeltaTime);
    }
}

void UAnim_IKFootPlacement::UpdateFootIK(float DeltaTime)
{
    if (!OwnerMesh)
        return;
    
    // Calculate IK for both feet
    FAnim_FootIKData NewLeftFootIK = CalculateFootIK(LeftFootBoneName, LeftFootIKBoneName);
    FAnim_FootIKData NewRightFootIK = CalculateFootIK(RightFootBoneName, RightFootIKBoneName);
    
    // Interpolate to smooth values
    LeftFootIK.FootLocation = FMath::VInterpTo(LeftFootIK.FootLocation, NewLeftFootIK.FootLocation, DeltaTime, IKInterpSpeed);
    LeftFootIK.FootRotation = FMath::RInterpTo(LeftFootIK.FootRotation, NewLeftFootIK.FootRotation, DeltaTime, IKInterpSpeed);
    LeftFootIK.IKAlpha = FMath::FInterpTo(LeftFootIK.IKAlpha, NewLeftFootIK.IKAlpha, DeltaTime, IKInterpSpeed);
    LeftFootIK.FootOffset = FMath::FInterpTo(LeftFootIK.FootOffset, NewLeftFootIK.FootOffset, DeltaTime, IKInterpSpeed);
    
    RightFootIK.FootLocation = FMath::VInterpTo(RightFootIK.FootLocation, NewRightFootIK.FootLocation, DeltaTime, IKInterpSpeed);
    RightFootIK.FootRotation = FMath::RInterpTo(RightFootIK.FootRotation, NewRightFootIK.FootRotation, DeltaTime, IKInterpSpeed);
    RightFootIK.IKAlpha = FMath::FInterpTo(RightFootIK.IKAlpha, NewRightFootIK.IKAlpha, DeltaTime, IKInterpSpeed);
    RightFootIK.FootOffset = FMath::FInterpTo(RightFootIK.FootOffset, NewRightFootIK.FootOffset, DeltaTime, IKInterpSpeed);
    
    // Calculate hip offset
    float NewHipOffset = CalculateHipOffset();
    HipOffset = FMath::FInterpTo(HipOffset, NewHipOffset, DeltaTime, IKInterpSpeed);
}

FAnim_FootIKData UAnim_IKFootPlacement::CalculateFootIK(FName FootBoneName, FName IKBoneName)
{
    FAnim_FootIKData FootData;
    
    if (!OwnerMesh)
        return FootData;
    
    // Get foot bone location in world space
    FVector FootLocation = OwnerMesh->GetBoneLocation(FootBoneName, EBoneSpaces::WorldSpace);
    
    // Perform trace downward
    FVector TraceStart = FootLocation + FVector(0, 0, 20.0f);
    FVector TraceEnd = FootLocation - FVector(0, 0, TraceDistance);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_Visibility,
        QueryParams
    );
    
    if (bHit)
    {
        // Calculate foot placement
        FootData.FootLocation = HitResult.ImpactPoint + FVector(0, 0, FootHeight);
        FootData.FootRotation = CalculateFootRotation(HitResult.ImpactPoint, HitResult.ImpactNormal);
        FootData.IKAlpha = 1.0f;
        
        // Calculate offset from original position
        float OriginalZ = OwnerMesh->GetBoneLocation(FootBoneName, EBoneSpaces::WorldSpace).Z;
        FootData.FootOffset = FootData.FootLocation.Z - OriginalZ;
    }
    else
    {
        // No ground found, disable IK
        FootData.IKAlpha = 0.0f;
        FootData.FootOffset = 0.0f;
    }
    
    return FootData;
}

float UAnim_IKFootPlacement::CalculateHipOffset()
{
    // Use the lowest foot as reference for hip adjustment
    float LeftOffset = LeftFootIK.FootOffset;
    float RightOffset = RightFootIK.FootOffset;
    
    // Take the lower offset to keep both feet on ground
    float HipAdjustment = FMath::Min(LeftOffset, RightOffset);
    
    // Only adjust downward to prevent floating
    return FMath::Min(HipAdjustment, 0.0f);
}

FVector UAnim_IKFootPlacement::PerformFootTrace(const FVector& FootLocation)
{
    FVector TraceStart = FootLocation + FVector(0, 0, 20.0f);
    FVector TraceEnd = FootLocation - FVector(0, 0, TraceDistance);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_Visibility,
        QueryParams
    );
    
    if (bHit)
    {
        return HitResult.ImpactPoint;
    }
    
    return FootLocation;
}

FRotator UAnim_IKFootPlacement::CalculateFootRotation(const FVector& ImpactPoint, const FVector& ImpactNormal)
{
    // Calculate rotation to align foot with ground normal
    FVector ForwardVector = GetOwner()->GetActorForwardVector();
    FVector RightVector = FVector::CrossProduct(ImpactNormal, ForwardVector).GetSafeNormal();
    ForwardVector = FVector::CrossProduct(RightVector, ImpactNormal).GetSafeNormal();
    
    return UKismetMathLibrary::MakeRotationFromAxes(ForwardVector, RightVector, ImpactNormal);
}

void UAnim_IKFootPlacement::SetIKEnabled(bool bEnabled)
{
    bEnableIK = bEnabled;
    
    if (!bEnabled)
    {
        // Reset IK data
        LeftFootIK = FAnim_FootIKData();
        RightFootIK = FAnim_FootIKData();
        HipOffset = 0.0f;
    }
}