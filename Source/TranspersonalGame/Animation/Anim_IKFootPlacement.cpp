#include "Anim_IKFootPlacement.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_IKFootPlacement::UAnim_IKFootPlacement()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    TraceDistance = 50.0f;
    IKInterpSpeed = 15.0f;
    MaxFootOffset = 30.0f;
    LeftFootBoneName = FName("foot_l");
    RightFootBoneName = FName("foot_r");
}

void UAnim_IKFootPlacement::BeginPlay()
{
    Super::BeginPlay();
    
    OwnerMesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
    if (!OwnerMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("UAnim_IKFootPlacement: No SkeletalMeshComponent found on owner"));
    }
}

void UAnim_IKFootPlacement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (OwnerMesh && OwnerMesh->GetAnimInstance())
    {
        UpdateFootIK();
    }
}

void UAnim_IKFootPlacement::UpdateFootIK()
{
    if (!OwnerMesh)
        return;

    // Get foot bone locations in world space
    FVector LeftFootLocation = OwnerMesh->GetBoneLocation(LeftFootBoneName, EBoneSpaces::WorldSpace);
    FVector RightFootLocation = OwnerMesh->GetBoneLocation(RightFootBoneName, EBoneSpaces::WorldSpace);

    // Calculate IK data for each foot
    FAnim_FootIKData NewLeftFootIK = CalculateFootIK(LeftFootBoneName, LeftFootLocation);
    FAnim_FootIKData NewRightFootIK = CalculateFootIK(RightFootBoneName, RightFootLocation);

    // Interpolate to new values for smooth transitions
    float DeltaTime = GetWorld()->GetDeltaSeconds();
    
    LeftFootIK.FootOffset = FMath::VInterpTo(LeftFootIK.FootOffset, NewLeftFootIK.FootOffset, DeltaTime, IKInterpSpeed);
    LeftFootIK.FootRotation = FMath::RInterpTo(LeftFootIK.FootRotation, NewLeftFootIK.FootRotation, DeltaTime, IKInterpSpeed);
    LeftFootIK.IKAlpha = FMath::FInterpTo(LeftFootIK.IKAlpha, NewLeftFootIK.IKAlpha, DeltaTime, IKInterpSpeed);

    RightFootIK.FootOffset = FMath::VInterpTo(RightFootIK.FootOffset, NewRightFootIK.FootOffset, DeltaTime, IKInterpSpeed);
    RightFootIK.FootRotation = FMath::RInterpTo(RightFootIK.FootRotation, NewRightFootIK.FootRotation, DeltaTime, IKInterpSpeed);
    RightFootIK.IKAlpha = FMath::FInterpTo(RightFootIK.IKAlpha, NewRightFootIK.IKAlpha, DeltaTime, IKInterpSpeed);
}

FAnim_FootIKData UAnim_IKFootPlacement::CalculateFootIK(const FName& FootBoneName, const FVector& FootLocation)
{
    FAnim_FootIKData FootIKData;
    
    // Trace down from foot location to find ground
    FVector GroundLocation = TraceForGround(FootLocation);
    
    if (GroundLocation != FVector::ZeroVector)
    {
        // Calculate offset needed to place foot on ground
        float OffsetZ = GroundLocation.Z - FootLocation.Z;
        OffsetZ = FMath::Clamp(OffsetZ, -MaxFootOffset, MaxFootOffset);
        
        FootIKData.FootOffset = FVector(0.0f, 0.0f, OffsetZ);
        FootIKData.IKAlpha = 1.0f;
        
        // Calculate foot rotation based on ground normal
        FVector ImpactNormal = FVector::UpVector; // Default up vector
        FootIKData.FootRotation = CalculateFootRotation(ImpactNormal);
    }
    else
    {
        // No ground found, disable IK
        FootIKData.FootOffset = FVector::ZeroVector;
        FootIKData.FootRotation = FRotator::ZeroRotator;
        FootIKData.IKAlpha = 0.0f;
    }
    
    return FootIKData;
}

FVector UAnim_IKFootPlacement::TraceForGround(const FVector& StartLocation) const
{
    if (!GetWorld())
        return FVector::ZeroVector;

    FVector StartTrace = StartLocation + FVector(0.0f, 0.0f, 25.0f);
    FVector EndTrace = StartLocation - FVector(0.0f, 0.0f, TraceDistance);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartTrace,
        EndTrace,
        ECollisionChannel::ECC_WorldStatic,
        QueryParams
    );
    
    if (bHit)
    {
        return HitResult.Location;
    }
    
    return FVector::ZeroVector;
}

FRotator UAnim_IKFootPlacement::CalculateFootRotation(const FVector& ImpactNormal) const
{
    // Calculate rotation to align foot with ground normal
    FVector ForwardVector = GetOwner()->GetActorForwardVector();
    FVector RightVector = FVector::CrossProduct(ImpactNormal, ForwardVector).GetSafeNormal();
    ForwardVector = FVector::CrossProduct(RightVector, ImpactNormal).GetSafeNormal();
    
    return UKismetMathLibrary::MakeRotationFromAxes(ForwardVector, RightVector, ImpactNormal);
}