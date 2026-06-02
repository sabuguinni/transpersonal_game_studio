#include "Anim_AdvancedIKSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Animation/AnimInstance.h"

UAnim_AdvancedIKSystem::UAnim_AdvancedIKSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Foot IK defaults
    bEnableFootIK = true;
    FootIKTraceDistance = 150.0f;
    FootIKInterpSpeed = 15.0f;
    LeftFootBoneName = TEXT("foot_l");
    RightFootBoneName = TEXT("foot_r");
    PelvisBoneName = TEXT("pelvis");
    PelvisOffset = 0.0f;
    
    // Look At IK defaults
    bEnableLookAtIK = false;
    HeadBoneName = TEXT("head");
    LookAtTarget = FVector::ZeroVector;
    LookAtBlendSpeed = 5.0f;
    MaxLookAtAngle = 90.0f;
    
    // Spine IK defaults
    bEnableSpineIK = false;
    SpineBoneNames.Add(TEXT("spine_01"));
    SpineBoneNames.Add(TEXT("spine_02"));
    SpineBoneNames.Add(TEXT("spine_03"));
    SpineFlexibility = 0.3f;
    
    OwnerMeshComponent = nullptr;
}

void UAnim_AdvancedIKSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Get the skeletal mesh component from owner
    if (AActor* Owner = GetOwner())
    {
        OwnerMeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
        if (!OwnerMeshComponent)
        {
            UE_LOG(LogTemp, Warning, TEXT("UAnim_AdvancedIKSystem: No SkeletalMeshComponent found on owner %s"), *Owner->GetName());
        }
    }
}

void UAnim_AdvancedIKSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerMeshComponent || !OwnerMeshComponent->GetAnimInstance())
    {
        return;
    }
    
    if (bEnableFootIK)
    {
        UpdateFootIK(DeltaTime);
    }
    
    if (bEnableLookAtIK)
    {
        UpdateLookAtIK(DeltaTime);
    }
    
    if (bEnableSpineIK)
    {
        UpdateSpineIK(DeltaTime);
    }
}

void UAnim_AdvancedIKSystem::UpdateFootIK(float DeltaTime)
{
    if (!OwnerMeshComponent)
    {
        return;
    }
    
    // Get foot bone locations in world space
    FVector LeftFootWorldLocation = OwnerMeshComponent->GetBoneLocation(LeftFootBoneName, EBoneSpaces::WorldSpace);
    FVector RightFootWorldLocation = OwnerMeshComponent->GetBoneLocation(RightFootBoneName, EBoneSpaces::WorldSpace);
    
    // Perform traces for both feet
    bool bLeftFootHit = PerformFootTrace(LeftFootWorldLocation, LeftFootIKData);
    bool bRightFootHit = PerformFootTrace(RightFootWorldLocation, RightFootIKData);
    
    // Update pelvis offset based on foot positions
    UpdatePelvisOffset(DeltaTime);
    
    // Interpolate IK alpha values
    float TargetLeftAlpha = bLeftFootHit ? 1.0f : 0.0f;
    float TargetRightAlpha = bRightFootHit ? 1.0f : 0.0f;
    
    LeftFootIKData.IKAlpha = FMath::FInterpTo(LeftFootIKData.IKAlpha, TargetLeftAlpha, DeltaTime, FootIKInterpSpeed);
    RightFootIKData.IKAlpha = FMath::FInterpTo(RightFootIKData.IKAlpha, TargetRightAlpha, DeltaTime, FootIKInterpSpeed);
}

bool UAnim_AdvancedIKSystem::PerformFootTrace(const FVector& FootLocation, FAnim_FootIKData& FootData)
{
    if (!GetWorld())
    {
        return false;
    }
    
    FVector TraceStart = FootLocation + FVector(0, 0, 50.0f);
    FVector TraceEnd = FootLocation - FVector(0, 0, FootIKTraceDistance);
    
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
        FootData.FootLocation = HitResult.Location;
        FootData.FootRotation = CalculateFootRotationFromNormal(HitResult.Normal);
        FootData.GroundDistance = FVector::Dist(FootLocation, HitResult.Location);
        FootData.bHasValidGround = true;
        
        // Debug visualization
        if (CVarShowDebugIK.GetValueOnGameThread())
        {
            DrawDebugSphere(GetWorld(), HitResult.Location, 5.0f, 12, FColor::Green, false, 0.1f);
            DrawDebugLine(GetWorld(), TraceStart, HitResult.Location, FColor::Yellow, false, 0.1f);
        }
        
        return true;
    }
    else
    {
        FootData.bHasValidGround = false;
        return false;
    }
}

FRotator UAnim_AdvancedIKSystem::CalculateFootRotationFromNormal(const FVector& Normal)
{
    FVector ForwardVector = GetOwner()->GetActorForwardVector();
    FVector RightVector = FVector::CrossProduct(Normal, ForwardVector).GetSafeNormal();
    ForwardVector = FVector::CrossProduct(RightVector, Normal).GetSafeNormal();
    
    return UKismetMathLibrary::MakeRotationFromAxes(ForwardVector, RightVector, Normal);
}

void UAnim_AdvancedIKSystem::UpdatePelvisOffset(float DeltaTime)
{
    float LeftFootOffset = LeftFootIKData.bHasValidGround ? LeftFootIKData.GroundDistance : 0.0f;
    float RightFootOffset = RightFootIKData.bHasValidGround ? RightFootIKData.GroundDistance : 0.0f;
    
    float TargetPelvisOffset = FMath::Min(LeftFootOffset, RightFootOffset) * -0.5f;
    PelvisOffset = FMath::FInterpTo(PelvisOffset, TargetPelvisOffset, DeltaTime, FootIKInterpSpeed);
}

void UAnim_AdvancedIKSystem::UpdateLookAtIK(float DeltaTime)
{
    if (!IsLookAtTargetValid())
    {
        return;
    }
    
    FVector HeadLocation = OwnerMeshComponent->GetBoneLocation(HeadBoneName, EBoneSpaces::WorldSpace);
    FRotator TargetRotation = CalculateLookAtRotation(HeadLocation, LookAtTarget);
    
    // Apply look at rotation with blending
    // This would typically be applied in the animation blueprint
}

FRotator UAnim_AdvancedIKSystem::CalculateLookAtRotation(const FVector& CurrentLocation, const FVector& TargetLocation)
{
    FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();
    return UKismetMathLibrary::FindLookAtRotation(CurrentLocation, TargetLocation);
}

bool UAnim_AdvancedIKSystem::IsLookAtTargetValid()
{
    return LookAtTarget != FVector::ZeroVector;
}

void UAnim_AdvancedIKSystem::UpdateSpineIK(float DeltaTime)
{
    if (SpineBoneNames.Num() == 0)
    {
        return;
    }
    
    // Calculate spine bending based on movement direction and terrain
    FVector MovementDirection = GetOwner()->GetVelocity().GetSafeNormal();
    TArray<FRotator> SpineRotations;
    CalculateSpineBendRotations(MovementDirection, SpineRotations);
    
    // Apply spine rotations (would typically be done in animation blueprint)
}

void UAnim_AdvancedIKSystem::CalculateSpineBendRotations(const FVector& TargetDirection, TArray<FRotator>& OutRotations)
{
    OutRotations.Empty();
    
    for (int32 i = 0; i < SpineBoneNames.Num(); i++)
    {
        float BendAmount = SpineFlexibility * (i + 1) / SpineBoneNames.Num();
        FRotator BendRotation = FRotator(0, 0, BendAmount * TargetDirection.Y * 10.0f);
        OutRotations.Add(BendRotation);
    }
}

void UAnim_AdvancedIKSystem::SetLookAtTarget(const FVector& NewTarget)
{
    LookAtTarget = NewTarget;
}

void UAnim_AdvancedIKSystem::AddIKBoneTarget(const FName& BoneName, const FVector& TargetLocation, const FRotator& TargetRotation, float BlendWeight)
{
    // Remove existing target for this bone
    RemoveIKBoneTarget(BoneName);
    
    FAnim_IKBoneTarget NewTarget;
    NewTarget.BoneName = BoneName;
    NewTarget.TargetLocation = TargetLocation;
    NewTarget.TargetRotation = TargetRotation;
    NewTarget.BlendWeight = BlendWeight;
    NewTarget.bIsActive = true;
    
    IKBoneTargets.Add(NewTarget);
}

void UAnim_AdvancedIKSystem::RemoveIKBoneTarget(const FName& BoneName)
{
    IKBoneTargets.RemoveAll([BoneName](const FAnim_IKBoneTarget& Target)
    {
        return Target.BoneName == BoneName;
    });
}

void UAnim_AdvancedIKSystem::EnableFootIK(bool bEnable)
{
    bEnableFootIK = bEnable;
}

void UAnim_AdvancedIKSystem::EnableLookAtIK(bool bEnable)
{
    bEnableLookAtIK = bEnable;
}

void UAnim_AdvancedIKSystem::EnableSpineIK(bool bEnable)
{
    bEnableSpineIK = bEnable;
}