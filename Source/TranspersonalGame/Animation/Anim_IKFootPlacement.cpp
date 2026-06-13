#include "Anim_IKFootPlacement.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_IKFootPlacement::UAnim_IKFootPlacement()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;

    LeftFootBoneName = TEXT("foot_l");
    RightFootBoneName = TEXT("foot_r");
    
    OwnerCharacter = nullptr;
    SkeletalMeshComp = nullptr;
}

void UAnim_IKFootPlacement::BeginPlay()
{
    Super::BeginPlay();

    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        SkeletalMeshComp = OwnerCharacter->GetMesh();
    }
}

void UAnim_IKFootPlacement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (IKSettings.bEnableFootIK)
    {
        UpdateFootIK(DeltaTime);
    }
}

void UAnim_IKFootPlacement::UpdateFootIK(float DeltaTime)
{
    if (!OwnerCharacter || !SkeletalMeshComp)
    {
        return;
    }

    LeftFootIKData = CalculateFootIK(LeftFootBoneName, DeltaTime);
    RightFootIKData = CalculateFootIK(RightFootBoneName, DeltaTime);
}

FAnim_FootIKData UAnim_IKFootPlacement::CalculateFootIK(const FName& FootBoneName, float DeltaTime)
{
    FAnim_FootIKData FootData;

    if (!SkeletalMeshComp)
    {
        return FootData;
    }

    FVector FootWorldLocation = GetFootWorldLocation(FootBoneName);
    FVector TraceStart = FootWorldLocation + FVector(0, 0, IKSettings.TraceDistance * 0.5f);
    FVector TraceEnd = FootWorldLocation - FVector(0, 0, IKSettings.TraceDistance);

    FVector HitLocation;
    FVector HitNormal;
    bool bHit = PerformFootTrace(TraceStart, HitLocation, HitNormal);

    if (bHit)
    {
        float DistanceFromGround = FootWorldLocation.Z - HitLocation.Z;
        FootData.DistanceFromGround = DistanceFromGround;

        if (DistanceFromGround > IKSettings.FootOffset)
        {
            // Foot is above ground, apply IK to bring it down
            FootData.FootLocation = FVector(0, 0, -(DistanceFromGround - IKSettings.FootOffset));
            FootData.FootRotation = CalculateFootRotation(HitNormal);
            FootData.IKAlpha = FMath::FInterpTo(FootData.IKAlpha, 1.0f, DeltaTime, IKSettings.IKInterpSpeed);
        }
        else
        {
            // Foot is at or below ground level
            FootData.FootLocation = FVector::ZeroVector;
            FootData.FootRotation = FRotator::ZeroRotator;
            FootData.IKAlpha = FMath::FInterpTo(FootData.IKAlpha, 0.0f, DeltaTime, IKSettings.IKInterpSpeed);
        }
    }
    else
    {
        // No ground detected, disable IK
        FootData.FootLocation = FVector::ZeroVector;
        FootData.FootRotation = FRotator::ZeroRotator;
        FootData.IKAlpha = FMath::FInterpTo(FootData.IKAlpha, 0.0f, DeltaTime, IKSettings.IKInterpSpeed);
    }

    return FootData;
}

FVector UAnim_IKFootPlacement::GetFootWorldLocation(const FName& FootBoneName)
{
    if (!SkeletalMeshComp)
    {
        return FVector::ZeroVector;
    }

    return SkeletalMeshComp->GetBoneLocation(FootBoneName, EBoneSpaces::WorldSpace);
}

bool UAnim_IKFootPlacement::PerformFootTrace(const FVector& StartLocation, FVector& OutHitLocation, FVector& OutHitNormal)
{
    if (!GetWorld())
    {
        return false;
    }

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    QueryParams.bTraceComplex = false;

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        StartLocation - FVector(0, 0, IKSettings.TraceDistance),
        IKSettings.TraceChannel,
        QueryParams
    );

    if (bHit)
    {
        OutHitLocation = HitResult.Location;
        OutHitNormal = HitResult.Normal;
    }

    return bHit;
}

FRotator UAnim_IKFootPlacement::CalculateFootRotation(const FVector& HitNormal)
{
    FVector UpVector = FVector::UpVector;
    FVector RightVector = FVector::CrossProduct(UpVector, HitNormal).GetSafeNormal();
    FVector ForwardVector = FVector::CrossProduct(HitNormal, RightVector).GetSafeNormal();

    return UKismetMathLibrary::MakeRotationFromAxes(ForwardVector, RightVector, HitNormal);
}