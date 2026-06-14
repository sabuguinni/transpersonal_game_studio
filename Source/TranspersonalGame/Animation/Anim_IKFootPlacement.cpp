#include "Anim_IKFootPlacement.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_IKFootPlacement::UAnim_IKFootPlacement()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    OwnerCharacter = nullptr;
    CharacterMesh = nullptr;
    
    LeftFootBoneName = TEXT("foot_l");
    RightFootBoneName = TEXT("foot_r");
    LeftFootSocketName = TEXT("LeftFootSocket");
    RightFootSocketName = TEXT("RightFootSocket");
}

void UAnim_IKFootPlacement::BeginPlay()
{
    Super::BeginPlay();
    InitializeCharacterReferences();
}

void UAnim_IKFootPlacement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (IKSettings.bEnableIK)
    {
        UpdateFootIK(DeltaTime);
    }
    
    if (IKSettings.bDebugDraw)
    {
        DrawDebugInfo();
    }
}

void UAnim_IKFootPlacement::InitializeCharacterReferences()
{
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        CharacterMesh = OwnerCharacter->GetMesh();
    }
}

void UAnim_IKFootPlacement::UpdateFootIK(float DeltaTime)
{
    if (!OwnerCharacter || !CharacterMesh)
    {
        InitializeCharacterReferences();
        return;
    }

    // Update both feet
    LeftFootIK = CalculateFootIK(LeftFootBoneName, LeftFootSocketName, DeltaTime);
    RightFootIK = CalculateFootIK(RightFootBoneName, RightFootSocketName, DeltaTime);
}

FAnim_FootIKData UAnim_IKFootPlacement::CalculateFootIK(const FName& FootBoneName, const FName& FootSocketName, float DeltaTime)
{
    FAnim_FootIKData FootData;
    
    if (!CharacterMesh)
    {
        return FootData;
    }

    // Get foot world location
    FVector FootLocation = GetFootWorldLocation(FootBoneName);
    if (FootLocation.IsZero())
    {
        return FootData;
    }

    // Perform trace downward from foot
    FVector TraceStart = FootLocation + FVector(0, 0, IKSettings.TraceDistance * 0.5f);
    FHitResult HitResult = PerformFootTrace(TraceStart);

    if (HitResult.bBlockingHit)
    {
        // Calculate distance from ground
        float DistanceToGround = FootLocation.Z - HitResult.Location.Z;
        FootData.DistanceFromGround = DistanceToGround;

        // Calculate IK adjustment (clamp to max adjustment)
        float IKAdjustment = FMath::Clamp(-DistanceToGround, -IKSettings.MaxIKAdjustment, IKSettings.MaxIKAdjustment);
        
        // Interpolate to target position
        FootData.FootLocation.Z = InterpToTarget(FootData.FootLocation.Z, IKAdjustment, IKSettings.IKInterpSpeed, DeltaTime);
        
        // Calculate foot rotation from surface normal
        FootData.FootRotation = CalculateFootRotationFromNormal(HitResult.Normal);
        
        // Set IK alpha based on how much adjustment is needed
        float AdjustmentRatio = FMath::Abs(IKAdjustment) / IKSettings.MaxIKAdjustment;
        FootData.IKAlpha = InterpToTarget(FootData.IKAlpha, AdjustmentRatio, IKSettings.IKInterpSpeed, DeltaTime);
    }
    else
    {
        // No ground hit - fade out IK
        FootData.IKAlpha = InterpToTarget(FootData.IKAlpha, 0.0f, IKSettings.IKInterpSpeed, DeltaTime);
        FootData.FootLocation = FVector::ZeroVector;
        FootData.FootRotation = FRotator::ZeroRotator;
        FootData.DistanceFromGround = 0.0f;
    }

    return FootData;
}

FHitResult UAnim_IKFootPlacement::PerformFootTrace(const FVector& StartLocation) const
{
    FHitResult HitResult;
    
    if (!GetWorld())
    {
        return HitResult;
    }

    FVector EndLocation = StartLocation - FVector(0, 0, IKSettings.TraceDistance);
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    QueryParams.bTraceComplex = false;
    QueryParams.bReturnPhysicalMaterial = false;

    GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECollisionChannel::ECC_WorldStatic,
        QueryParams
    );

    return HitResult;
}

FRotator UAnim_IKFootPlacement::CalculateFootRotationFromNormal(const FVector& SurfaceNormal) const
{
    // Calculate rotation to align foot with surface
    FVector UpVector = SurfaceNormal;
    FVector ForwardVector = OwnerCharacter ? OwnerCharacter->GetActorForwardVector() : FVector::ForwardVector;
    
    // Project forward vector onto the surface plane
    FVector ProjectedForward = ForwardVector - FVector::DotProduct(ForwardVector, UpVector) * UpVector;
    ProjectedForward.Normalize();
    
    // Calculate right vector
    FVector RightVector = FVector::CrossProduct(UpVector, ProjectedForward);
    RightVector.Normalize();
    
    // Create rotation matrix
    FMatrix RotationMatrix = FMatrix(ProjectedForward, RightVector, UpVector, FVector::ZeroVector);
    
    return RotationMatrix.Rotator();
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
    
    if (!LeftFootLoc.IsZero())
    {
        FVector TraceStart = LeftFootLoc + FVector(0, 0, IKSettings.TraceDistance * 0.5f);
        FVector TraceEnd = TraceStart - FVector(0, 0, IKSettings.TraceDistance);
        DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 0.0f, 0, 2.0f);
    }
    
    if (!RightFootLoc.IsZero())
    {
        FVector TraceStart = RightFootLoc + FVector(0, 0, IKSettings.TraceDistance * 0.5f);
        FVector TraceEnd = TraceStart - FVector(0, 0, IKSettings.TraceDistance);
        DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Blue, false, 0.0f, 0, 2.0f);
    }

    // Draw IK adjustment visualization
    FVector CharacterLocation = OwnerCharacter->GetActorLocation();
    DrawDebugString(GetWorld(), CharacterLocation + FVector(0, 0, 200), 
        FString::Printf(TEXT("L_IK: %.1f | R_IK: %.1f"), LeftFootIK.IKAlpha, RightFootIK.IKAlpha),
        nullptr, FColor::White, 0.0f);
}

FVector UAnim_IKFootPlacement::GetFootWorldLocation(const FName& BoneName) const
{
    if (!CharacterMesh)
    {
        return FVector::ZeroVector;
    }

    // Try to get bone location
    if (CharacterMesh->GetBoneIndex(BoneName) != INDEX_NONE)
    {
        return CharacterMesh->GetBoneLocation(BoneName, EBoneSpaces::WorldSpace);
    }

    return FVector::ZeroVector;
}

float UAnim_IKFootPlacement::InterpToTarget(float Current, float Target, float InterpSpeed, float DeltaTime) const
{
    return FMath::FInterpTo(Current, Target, DeltaTime, InterpSpeed);
}