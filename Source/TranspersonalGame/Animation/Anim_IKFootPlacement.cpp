#include "Anim_IKFootPlacement.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "CollisionQueryParams.h"

UAnim_IKFootPlacement::UAnim_IKFootPlacement()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    
    // Initialize default settings
    IKSettings.TraceDistance = 50.0f;
    IKSettings.InterpSpeed = 15.0f;
    IKSettings.FootHeight = 5.0f;
    IKSettings.bEnableFootRotation = true;
    IKSettings.MaxFootRotationAngle = 45.0f;
    IKSettings.LeftFootBoneName = TEXT("foot_l");
    IKSettings.RightFootBoneName = TEXT("foot_r");
    IKSettings.LeftFootIKBoneName = TEXT("ik_foot_l");
    IKSettings.RightFootIKBoneName = TEXT("ik_foot_r");
    
    bEnableIK = true;
    IKUpdateRate = 60.0f;
    LastUpdateTime = 0.0f;
    
    // Initialize foot data
    LeftFootData = FAnim_FootIKData();
    RightFootData = FAnim_FootIKData();
    PreviousLeftFootData = FAnim_FootIKData();
    PreviousRightFootData = FAnim_FootIKData();
}

void UAnim_IKFootPlacement::BeginPlay()
{
    Super::BeginPlay();
    
    // Get component references
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        SkeletalMeshComponent = OwnerCharacter->GetMesh();
        UE_LOG(LogTemp, Log, TEXT("Foot IK Component initialized for character: %s"), 
               *OwnerCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Foot IK Component: Failed to get character reference"));
    }
}

void UAnim_IKFootPlacement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bEnableIK || !OwnerCharacter || !SkeletalMeshComponent)
    {
        return;
    }
    
    // Update rate limiting
    LastUpdateTime += DeltaTime;
    float UpdateInterval = 1.0f / IKUpdateRate;
    
    if (LastUpdateTime >= UpdateInterval)
    {
        // Update foot IK
        UpdateFootIK(LeftFootData, IKSettings.LeftFootBoneName, DeltaTime);
        UpdateFootIK(RightFootData, IKSettings.RightFootBoneName, DeltaTime);
        
        LastUpdateTime = 0.0f;
    }
    else
    {
        // Interpolate between updates
        InterpolateFootData(LeftFootData, PreviousLeftFootData, DeltaTime);
        InterpolateFootData(RightFootData, PreviousRightFootData, DeltaTime);
    }
}

void UAnim_IKFootPlacement::UpdateFootIK(FAnim_FootIKData& FootData, const FName& FootBoneName, float DeltaTime)
{
    if (!SkeletalMeshComponent)
    {
        return;
    }
    
    // Store previous data for interpolation
    if (FootBoneName == IKSettings.LeftFootBoneName)
    {
        PreviousLeftFootData = LeftFootData;
    }
    else
    {
        PreviousRightFootData = RightFootData;
    }
    
    // Get foot world location
    FVector FootWorldLocation = GetFootWorldLocation(FootBoneName);
    if (FootWorldLocation == FVector::ZeroVector)
    {
        FootData.bIsGrounded = false;
        FootData.IKAlpha = 0.0f;
        return;
    }
    
    // Trace for ground
    FVector HitLocation;
    FVector HitNormal;
    bool bHitGround = TraceForGround(FootWorldLocation, HitLocation, HitNormal);
    
    if (bHitGround)
    {
        // Calculate distance from ground
        FootData.DistanceFromGround = FootWorldLocation.Z - HitLocation.Z;
        FootData.bIsGrounded = true;
        
        // Calculate IK alpha based on distance
        float MaxIKDistance = IKSettings.TraceDistance;
        FootData.IKAlpha = FMath::Clamp(1.0f - (FMath::Abs(FootData.DistanceFromGround) / MaxIKDistance), 0.0f, 1.0f);
        
        // Calculate foot location adjustment
        FVector TargetLocation = FootWorldLocation;
        TargetLocation.Z = HitLocation.Z + IKSettings.FootHeight;
        FootData.FootLocation = FMath::VInterpTo(FootData.FootLocation, TargetLocation, DeltaTime, IKSettings.InterpSpeed);
        
        // Calculate foot rotation
        if (IKSettings.bEnableFootRotation)
        {
            FRotator TargetRotation = CalculateFootRotation(HitNormal);
            FootData.FootRotation = FMath::RInterpTo(FootData.FootRotation, TargetRotation, DeltaTime, IKSettings.InterpSpeed);
        }
        else
        {
            FootData.FootRotation = FRotator::ZeroRotator;
        }
    }
    else
    {
        // No ground hit - fade out IK
        FootData.bIsGrounded = false;
        FootData.IKAlpha = FMath::FInterpTo(FootData.IKAlpha, 0.0f, DeltaTime, IKSettings.InterpSpeed);
        FootData.DistanceFromGround = IKSettings.TraceDistance;
    }
}

FVector UAnim_IKFootPlacement::GetFootWorldLocation(const FName& FootBoneName) const
{
    if (!SkeletalMeshComponent)
    {
        return FVector::ZeroVector;
    }
    
    int32 BoneIndex = SkeletalMeshComponent->GetBoneIndex(FootBoneName);
    if (BoneIndex == INDEX_NONE)
    {
        UE_LOG(LogTemp, Warning, TEXT("Foot IK: Bone '%s' not found"), *FootBoneName.ToString());
        return FVector::ZeroVector;
    }
    
    return SkeletalMeshComponent->GetBoneLocation(FootBoneName, EBoneSpaces::WorldSpace);
}

bool UAnim_IKFootPlacement::TraceForGround(const FVector& StartLocation, FVector& HitLocation, FVector& HitNormal) const
{
    if (!GetWorld())
    {
        return false;
    }
    
    // Setup trace parameters
    FVector TraceStart = StartLocation + FVector(0.0f, 0.0f, IKSettings.TraceDistance * 0.5f);
    FVector TraceEnd = StartLocation - FVector(0.0f, 0.0f, IKSettings.TraceDistance);
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    QueryParams.bTraceComplex = true;
    
    FHitResult HitResult;
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_Visibility,
        QueryParams
    );
    
    if (bHit)
    {
        HitLocation = HitResult.Location;
        HitNormal = HitResult.Normal;
        
        // Debug drawing
        if (CVarShowDebugAnimation.GetValueOnGameThread())
        {
            DrawDebugLine(GetWorld(), TraceStart, HitLocation, FColor::Green, false, 0.1f, 0, 1.0f);
            DrawDebugSphere(GetWorld(), HitLocation, 2.0f, 8, FColor::Red, false, 0.1f);
        }
        
        return true;
    }
    
    // Debug drawing for failed trace
    if (CVarShowDebugAnimation.GetValueOnGameThread())
    {
        DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 0.1f, 0, 1.0f);
    }
    
    return false;
}

FRotator UAnim_IKFootPlacement::CalculateFootRotation(const FVector& HitNormal) const
{
    // Calculate rotation to align foot with surface normal
    FVector UpVector = FVector::UpVector;
    FVector RightVector = FVector::CrossProduct(UpVector, HitNormal).GetSafeNormal();
    FVector ForwardVector = FVector::CrossProduct(HitNormal, RightVector).GetSafeNormal();
    
    FRotator SurfaceRotation = UKismetMathLibrary::MakeRotationFromAxes(ForwardVector, RightVector, HitNormal);
    
    // Clamp rotation to max angle
    FRotator ClampedRotation = SurfaceRotation;
    ClampedRotation.Pitch = FMath::Clamp(ClampedRotation.Pitch, -IKSettings.MaxFootRotationAngle, IKSettings.MaxFootRotationAngle);
    ClampedRotation.Roll = FMath::Clamp(ClampedRotation.Roll, -IKSettings.MaxFootRotationAngle, IKSettings.MaxFootRotationAngle);
    
    return ClampedRotation;
}

void UAnim_IKFootPlacement::InterpolateFootData(FAnim_FootIKData& CurrentData, const FAnim_FootIKData& TargetData, float DeltaTime)
{
    CurrentData.FootLocation = FMath::VInterpTo(CurrentData.FootLocation, TargetData.FootLocation, DeltaTime, IKSettings.InterpSpeed);
    CurrentData.FootRotation = FMath::RInterpTo(CurrentData.FootRotation, TargetData.FootRotation, DeltaTime, IKSettings.InterpSpeed);
    CurrentData.IKAlpha = FMath::FInterpTo(CurrentData.IKAlpha, TargetData.IKAlpha, DeltaTime, IKSettings.InterpSpeed);
}

void UAnim_IKFootPlacement::GetFootIKTransforms(FTransform& LeftFootTransform, FTransform& RightFootTransform, float& LeftIKAlpha, float& RightIKAlpha) const
{
    // Left foot transform
    LeftFootTransform.SetLocation(LeftFootData.FootLocation);
    LeftFootTransform.SetRotation(LeftFootData.FootRotation.Quaternion());
    LeftFootTransform.SetScale3D(FVector::OneVector);
    LeftIKAlpha = LeftFootData.IKAlpha;
    
    // Right foot transform
    RightFootTransform.SetLocation(RightFootData.FootLocation);
    RightFootTransform.SetRotation(RightFootData.FootRotation.Quaternion());
    RightFootTransform.SetScale3D(FVector::OneVector);
    RightIKAlpha = RightFootData.IKAlpha;
}

void UAnim_IKFootPlacement::ForceUpdateIK()
{
    if (!bEnableIK || !OwnerCharacter || !SkeletalMeshComponent)
    {
        return;
    }
    
    float DeltaTime = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.016f;
    
    UpdateFootIK(LeftFootData, IKSettings.LeftFootBoneName, DeltaTime);
    UpdateFootIK(RightFootData, IKSettings.RightFootBoneName, DeltaTime);
    
    UE_LOG(LogTemp, Log, TEXT("Foot IK force updated"));
}