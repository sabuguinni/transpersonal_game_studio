#include "Anim_IKFootPlacement.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_IKFootPlacement::UAnim_IKFootPlacement()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;

    // Initialize default values
    TraceDistance = 50.0f;
    IKInterpSpeed = 15.0f;
    FootOffset = 5.0f;
    bEnableFootIK = true;
    bEnableDebugDraw = false;

    // Initialize bone names
    LeftFootBoneName = "foot_l";
    RightFootBoneName = "foot_r";
    PelvisBoneName = "pelvis";

    // Initialize internal state
    PelvisOffset = 0.0f;
    TargetPelvisOffset = 0.0f;

    // Initialize IK data
    LeftFootIK = FAnim_FootIKData();
    RightFootIK = FAnim_FootIKData();
}

void UAnim_IKFootPlacement::BeginPlay()
{
    Super::BeginPlay();
    
    CacheReferences();
}

void UAnim_IKFootPlacement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bEnableFootIK && OwnerCharacter && SkeletalMeshComponent)
    {
        UpdateFootIK();
        
        if (bEnableDebugDraw)
        {
            DrawDebugInfo();
        }
    }
}

void UAnim_IKFootPlacement::CacheReferences()
{
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        SkeletalMeshComponent = OwnerCharacter->GetMesh();
    }
}

void UAnim_IKFootPlacement::UpdateFootIK()
{
    if (!OwnerCharacter || !SkeletalMeshComponent)
    {
        return;
    }

    float DeltaTime = GetWorld()->GetDeltaSeconds();

    // Calculate IK for both feet
    LeftFootIK = CalculateFootIK(LeftFootBoneName, DeltaTime);
    RightFootIK = CalculateFootIK(RightFootBoneName, DeltaTime);

    // Update pelvis offset based on foot positions
    UpdatePelvisOffset(LeftFootIK.JointTargetOffset, RightFootIK.JointTargetOffset, DeltaTime);
}

FAnim_FootIKData UAnim_IKFootPlacement::CalculateFootIK(FName FootBoneName, float DeltaTime)
{
    FAnim_FootIKData FootIKData;

    if (!SkeletalMeshComponent)
    {
        return FootIKData;
    }

    // Get foot world location
    FVector FootWorldLocation = GetFootWorldLocation(FootBoneName);
    
    // Perform trace to find ground
    FVector HitLocation;
    FVector HitNormal;
    bool bHit = PerformFootTrace(FootWorldLocation, HitLocation, HitNormal);

    if (bHit)
    {
        // Calculate the offset needed
        float DistanceToGround = FootWorldLocation.Z - HitLocation.Z;
        float TargetOffset = DistanceToGround - FootOffset;

        // Smooth the IK alpha
        float TargetAlpha = FMath::Clamp(FMath::Abs(TargetOffset) / TraceDistance, 0.0f, 1.0f);
        FootIKData.IKAlpha = FMath::FInterpTo(FootIKData.IKAlpha, TargetAlpha, DeltaTime, IKInterpSpeed);

        // Set the foot location and rotation
        FootIKData.FootLocation = HitLocation + HitNormal * FootOffset;
        FootIKData.FootRotation = CalculateFootRotation(HitNormal, OwnerCharacter->GetActorForwardVector());
        FootIKData.JointTargetOffset = TargetOffset;
    }
    else
    {
        // No ground hit, fade out IK
        FootIKData.IKAlpha = FMath::FInterpTo(FootIKData.IKAlpha, 0.0f, DeltaTime, IKInterpSpeed);
        FootIKData.JointTargetOffset = 0.0f;
    }

    return FootIKData;
}

FVector UAnim_IKFootPlacement::GetFootWorldLocation(FName FootBoneName) const
{
    if (!SkeletalMeshComponent)
    {
        return FVector::ZeroVector;
    }

    return SkeletalMeshComponent->GetBoneLocation(FootBoneName, EBoneSpaces::WorldSpace);
}

bool UAnim_IKFootPlacement::PerformFootTrace(const FVector& StartLocation, FVector& OutHitLocation, FVector& OutHitNormal) const
{
    if (!GetWorld())
    {
        return false;
    }

    FVector TraceStart = StartLocation + FVector(0, 0, TraceDistance * 0.5f);
    FVector TraceEnd = StartLocation - FVector(0, 0, TraceDistance);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_WorldStatic,
        QueryParams
    );

    if (bHit)
    {
        OutHitLocation = HitResult.Location;
        OutHitNormal = HitResult.Normal;
    }

    return bHit;
}

FRotator UAnim_IKFootPlacement::CalculateFootRotation(const FVector& HitNormal, const FVector& FootForward) const
{
    // Calculate foot rotation to align with ground normal
    FVector FootRight = FVector::CrossProduct(HitNormal, FootForward).GetSafeNormal();
    FVector CorrectedForward = FVector::CrossProduct(FootRight, HitNormal).GetSafeNormal();

    return UKismetMathLibrary::MakeRotationFromAxes(CorrectedForward, FootRight, HitNormal);
}

void UAnim_IKFootPlacement::UpdatePelvisOffset(float LeftFootOffset, float RightFootOffset, float DeltaTime)
{
    // Use the smaller offset (higher foot) to avoid pelvis going too low
    TargetPelvisOffset = FMath::Min(LeftFootOffset, RightFootOffset);
    
    // Clamp the offset to reasonable values
    TargetPelvisOffset = FMath::Clamp(TargetPelvisOffset, -TraceDistance, TraceDistance);

    // Smooth the pelvis movement
    PelvisOffset = FMath::FInterpTo(PelvisOffset, TargetPelvisOffset, DeltaTime, IKInterpSpeed);
}

void UAnim_IKFootPlacement::DrawDebugInfo() const
{
    if (!GetWorld() || !OwnerCharacter)
    {
        return;
    }

    // Draw foot trace lines
    FVector LeftFootLoc = GetFootWorldLocation(LeftFootBoneName);
    FVector RightFootLoc = GetFootWorldLocation(RightFootBoneName);

    DrawDebugLine(GetWorld(), LeftFootLoc + FVector(0, 0, TraceDistance * 0.5f), 
                  LeftFootLoc - FVector(0, 0, TraceDistance), FColor::Red, false, 0.0f, 0, 2.0f);
    
    DrawDebugLine(GetWorld(), RightFootLoc + FVector(0, 0, TraceDistance * 0.5f), 
                  RightFootLoc - FVector(0, 0, TraceDistance), FColor::Blue, false, 0.0f, 0, 2.0f);

    // Draw IK target locations
    if (LeftFootIK.IKAlpha > 0.1f)
    {
        DrawDebugSphere(GetWorld(), LeftFootIK.FootLocation, 5.0f, 8, FColor::Green, false, 0.0f, 0, 2.0f);
    }
    
    if (RightFootIK.IKAlpha > 0.1f)
    {
        DrawDebugSphere(GetWorld(), RightFootIK.FootLocation, 5.0f, 8, FColor::Cyan, false, 0.0f, 0, 2.0f);
    }
}

void UAnim_IKFootPlacement::SetIKEnabled(bool bEnabled)
{
    bEnableFootIK = bEnabled;
}

FAnim_FootIKData UAnim_IKFootPlacement::GetLeftFootIKData() const
{
    return LeftFootIK;
}

FAnim_FootIKData UAnim_IKFootPlacement::GetRightFootIKData() const
{
    return RightFootIK;
}

float UAnim_IKFootPlacement::GetPelvisOffset() const
{
    return PelvisOffset;
}