#include "Anim_IKFootPlacementSystem.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"

UAnim_IKFootPlacementSystem::UAnim_IKFootPlacementSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    // Initialize bone names
    LeftFootBoneName = TEXT("foot_l");
    RightFootBoneName = TEXT("foot_r");
    PelvisBoneName = TEXT("pelvis");

    // Initialize settings
    IKSettings = FAnim_IKSettings();
    
    // Hip adjustment
    HipOffset = 0.0f;
    HipInterpSpeed = 10.0f;

    // Performance
    bEnableIK = true;
    UpdateFrequency = 30.0f; // 30 FPS for IK updates
    LastUpdateTime = 0.0f;

    // Cache
    CachedActorLocation = FVector::ZeroVector;
    CachedActorRotation = FRotator::ZeroRotator;
}

void UAnim_IKFootPlacementSystem::BeginPlay()
{
    Super::BeginPlay();

    // Get skeletal mesh component
    AActor* Owner = GetOwner();
    if (Owner)
    {
        SkeletalMeshComp = Owner->FindComponentByClass<USkeletalMeshComponent>();
        
        if (!SkeletalMeshComp)
        {
            UE_LOG(LogTemp, Warning, TEXT("IK Foot Placement: No SkeletalMeshComponent found on %s"), *Owner->GetName());
        }
    }
}

void UAnim_IKFootPlacementSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bEnableIK || !SkeletalMeshComp)
    {
        return;
    }

    // Performance throttling
    if (!ShouldUpdateIK())
    {
        return;
    }

    LastUpdateTime = GetWorld()->GetTimeSeconds();

    // Update foot IK
    UpdateFootIK(DeltaTime);

    // Update hip offset
    UpdateHipOffset(DeltaTime);
}

void UAnim_IKFootPlacementSystem::UpdateFootIK(float DeltaTime)
{
    if (!SkeletalMeshComp)
    {
        return;
    }

    // Calculate IK for both feet
    LeftFootIK = CalculateFootIK(LeftFootBoneName, DeltaTime);
    RightFootIK = CalculateFootIK(RightFootBoneName, DeltaTime);

    // Cache actor transform for performance
    AActor* Owner = GetOwner();
    if (Owner)
    {
        CachedActorLocation = Owner->GetActorLocation();
        CachedActorRotation = Owner->GetActorRotation();
    }
}

FAnim_FootIKData UAnim_IKFootPlacementSystem::CalculateFootIK(const FName& FootBoneName, float DeltaTime)
{
    FAnim_FootIKData FootData;

    if (!SkeletalMeshComp)
    {
        return FootData;
    }

    // Get foot bone world location
    FVector FootLocation = GetBoneWorldLocation(FootBoneName);
    
    if (FootLocation.IsZero())
    {
        return FootData;
    }

    // Perform trace to find ground
    float DistanceToGround = 0.0f;
    FVector GroundLocation = PerformFootTrace(FootLocation, DistanceToGround);

    // Calculate IK alpha based on distance
    float IKAlpha = 0.0f;
    if (DistanceToGround <= IKSettings.TraceDistance)
    {
        IKAlpha = FMath::Clamp(1.0f - (DistanceToGround / IKSettings.TraceDistance), 0.0f, 1.0f);
    }

    // Interpolate foot position
    FVector TargetFootLocation = GroundLocation + FVector(0, 0, IKSettings.FootOffset);
    FootData.FootLocation = FMath::VInterpTo(FootData.FootLocation, TargetFootLocation, DeltaTime, IKSettings.InterpSpeed);

    // Calculate foot rotation from surface normal
    if (IKSettings.bEnableFootRotation)
    {
        // Perform additional trace for normal calculation
        FHitResult HitResult;
        FVector TraceStart = FootLocation + FVector(0, 0, IKSettings.TraceDistance);
        FVector TraceEnd = FootLocation - FVector(0, 0, IKSettings.TraceDistance);

        TArray<AActor*> ActorsToIgnore;
        ActorsToIgnore.Add(GetOwner());

        bool bHit = UKismetSystemLibrary::LineTraceSingle(
            GetWorld(),
            TraceStart,
            TraceEnd,
            UEngineTypes::ConvertToTraceType(ECC_WorldStatic),
            false,
            ActorsToIgnore,
            EDrawDebugTrace::None,
            HitResult,
            true
        );

        if (bHit)
        {
            FRotator TargetRotation = CalculateFootRotationFromNormal(HitResult.Normal);
            FootData.FootRotation = FMath::RInterpTo(FootData.FootRotation, TargetRotation, DeltaTime, IKSettings.InterpSpeed);
        }
    }

    FootData.IKAlpha = IKAlpha;
    FootData.DistanceFromGround = DistanceToGround;

    return FootData;
}

void UAnim_IKFootPlacementSystem::UpdateHipOffset(float DeltaTime)
{
    // Calculate hip offset based on foot positions
    float LeftFootDistance = LeftFootIK.DistanceFromGround;
    float RightFootDistance = RightFootIK.DistanceFromGround;

    // Find the lowest foot
    float LowestFootDistance = FMath::Min(LeftFootDistance, RightFootDistance);

    // Calculate target hip offset
    float TargetHipOffset = 0.0f;
    if (LowestFootDistance < IKSettings.TraceDistance)
    {
        TargetHipOffset = -FMath::Clamp(LowestFootDistance - IKSettings.FootOffset, -IKSettings.MaxFootAdjustment, IKSettings.MaxFootAdjustment);
    }

    // Interpolate hip offset
    HipOffset = FMath::FInterpTo(HipOffset, TargetHipOffset, DeltaTime, HipInterpSpeed);
}

FVector UAnim_IKFootPlacementSystem::PerformFootTrace(const FVector& FootLocation, float& OutDistance)
{
    FHitResult HitResult;
    FVector TraceStart = FootLocation + FVector(0, 0, IKSettings.TraceDistance);
    FVector TraceEnd = FootLocation - FVector(0, 0, IKSettings.TraceDistance);

    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(GetOwner());

    bool bHit = UKismetSystemLibrary::LineTraceSingle(
        GetWorld(),
        TraceStart,
        TraceEnd,
        UEngineTypes::ConvertToTraceType(ECC_WorldStatic),
        false,
        ActorsToIgnore,
        EDrawDebugTrace::None,
        HitResult,
        true
    );

    if (bHit)
    {
        OutDistance = FVector::Dist(FootLocation, HitResult.Location);
        return HitResult.Location;
    }

    OutDistance = IKSettings.TraceDistance + 1.0f; // Beyond trace range
    return FootLocation;
}

void UAnim_IKFootPlacementSystem::SetIKEnabled(bool bEnabled)
{
    bEnableIK = bEnabled;

    // Reset IK data when disabled
    if (!bEnabled)
    {
        LeftFootIK = FAnim_FootIKData();
        RightFootIK = FAnim_FootIKData();
        HipOffset = 0.0f;
    }
}

FVector UAnim_IKFootPlacementSystem::GetBoneWorldLocation(const FName& BoneName) const
{
    if (!SkeletalMeshComp)
    {
        return FVector::ZeroVector;
    }

    int32 BoneIndex = SkeletalMeshComp->GetBoneIndex(BoneName);
    if (BoneIndex == INDEX_NONE)
    {
        return FVector::ZeroVector;
    }

    return SkeletalMeshComp->GetBoneLocation(BoneName, EBoneSpaces::WorldSpace);
}

FRotator UAnim_IKFootPlacementSystem::CalculateFootRotationFromNormal(const FVector& SurfaceNormal) const
{
    // Calculate rotation to align foot with surface
    FVector ForwardVector = GetOwner()->GetActorForwardVector();
    FVector RightVector = FVector::CrossProduct(SurfaceNormal, ForwardVector).GetSafeNormal();
    ForwardVector = FVector::CrossProduct(RightVector, SurfaceNormal).GetSafeNormal();

    return FRotationMatrix::MakeFromXZ(ForwardVector, SurfaceNormal).Rotator();
}

bool UAnim_IKFootPlacementSystem::ShouldUpdateIK() const
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float TimeSinceLastUpdate = CurrentTime - LastUpdateTime;
    float UpdateInterval = 1.0f / UpdateFrequency;

    return TimeSinceLastUpdate >= UpdateInterval;
}