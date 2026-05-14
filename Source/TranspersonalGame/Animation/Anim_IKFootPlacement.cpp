#include "Anim_IKFootPlacement.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Animation/AnimInstance.h"

UAnim_IKFootPlacement::UAnim_IKFootPlacement()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    // Initialize default settings
    IKSettings = FAnim_IKSettings();
    FootIKData = FAnim_FootIKData();

    // Initialize smoothed values
    SmoothedLeftFootOffset = 0.0f;
    SmoothedRightFootOffset = 0.0f;
    SmoothedLeftFootRotation = FRotator::ZeroRotator;
    SmoothedRightFootRotation = FRotator::ZeroRotator;
    SmoothedPelvisOffset = 0.0f;

    // Set default bone names (common MetaHuman/UE5 skeleton)
    LeftFootBoneName = TEXT("foot_l");
    RightFootBoneName = TEXT("foot_r");
    PelvisBoneName = TEXT("pelvis");
}

void UAnim_IKFootPlacement::BeginPlay()
{
    Super::BeginPlay();

    // Cache owner character and skeletal mesh
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        SkeletalMeshComponent = OwnerCharacter->GetMesh();
        
        if (!SkeletalMeshComponent)
        {
            UE_LOG(LogTemp, Warning, TEXT("UAnim_IKFootPlacement: No SkeletalMeshComponent found on %s"), 
                   *OwnerCharacter->GetName());
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("UAnim_IKFootPlacement: Component not attached to ACharacter"));
    }
}

void UAnim_IKFootPlacement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (IKSettings.bEnableIK && OwnerCharacter && SkeletalMeshComponent)
    {
        UpdateFootIK();
        SmoothIKValues(DeltaTime);
        
        if (IKSettings.bEnableDebugDraw)
        {
            DrawDebugInfo();
        }
    }
}

void UAnim_IKFootPlacement::UpdateFootIK()
{
    if (!OwnerCharacter || !SkeletalMeshComponent)
    {
        return;
    }

    // Get foot world locations
    FVector LeftFootLocation = GetFootWorldLocation(true);
    FVector RightFootLocation = GetFootWorldLocation(false);

    // Perform traces for each foot
    FRotator LeftFootRotation;
    FRotator RightFootRotation;
    
    float LeftOffset = PerformFootTrace(LeftFootLocation, LeftFootRotation);
    float RightOffset = PerformFootTrace(RightFootLocation, RightFootRotation);

    // Clamp offsets to maximum values
    LeftOffset = FMath::Clamp(LeftOffset, -IKSettings.MaxFootOffset, IKSettings.MaxFootOffset);
    RightOffset = FMath::Clamp(RightOffset, -IKSettings.MaxFootOffset, IKSettings.MaxFootOffset);

    // Update foot IK data
    FootIKData.LeftFootOffset = LeftOffset;
    FootIKData.RightFootOffset = RightOffset;
    FootIKData.LeftFootRotation = LeftFootRotation;
    FootIKData.RightFootRotation = RightFootRotation;

    // Calculate pelvis offset
    CalculatePelvisOffset();
}

float UAnim_IKFootPlacement::PerformFootTrace(const FVector& FootLocation, FRotator& OutFootRotation)
{
    if (!GetWorld())
    {
        OutFootRotation = FRotator::ZeroRotator;
        return 0.0f;
    }

    // Setup trace parameters
    FVector TraceStart = FootLocation + FVector(0, 0, IKSettings.TraceDistance);
    FVector TraceEnd = FootLocation - FVector(0, 0, IKSettings.TraceDistance);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    QueryParams.bTraceComplex = false;

    // Perform line trace
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_Visibility,
        QueryParams
    );

    if (bHit)
    {
        // Calculate foot offset
        float FootOffset = (HitResult.Location.Z - FootLocation.Z) + IKSettings.FootHeight;
        
        // Calculate foot rotation to match surface normal
        FVector SurfaceNormal = HitResult.Normal;
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector RightVector = FVector::CrossProduct(SurfaceNormal, ForwardVector).GetSafeNormal();
        ForwardVector = FVector::CrossProduct(RightVector, SurfaceNormal).GetSafeNormal();
        
        OutFootRotation = UKismetMathLibrary::MakeRotationFromAxes(ForwardVector, RightVector, SurfaceNormal);
        
        return FootOffset;
    }
    else
    {
        OutFootRotation = FRotator::ZeroRotator;
        return 0.0f;
    }
}

FVector UAnim_IKFootPlacement::GetFootWorldLocation(bool bIsLeftFoot) const
{
    if (!SkeletalMeshComponent)
    {
        return FVector::ZeroVector;
    }

    FName BoneName = bIsLeftFoot ? LeftFootBoneName : RightFootBoneName;
    
    // Get bone location in world space
    FVector BoneLocation = SkeletalMeshComponent->GetBoneLocation(BoneName, EBoneSpaces::WorldSpace);
    
    // If bone not found, use character location as fallback
    if (BoneLocation.IsZero())
    {
        FVector CharacterLocation = OwnerCharacter->GetActorLocation();
        float OffsetY = bIsLeftFoot ? 20.0f : -20.0f; // Approximate foot spacing
        BoneLocation = CharacterLocation + OwnerCharacter->GetActorRightVector() * OffsetY;
        
        UE_LOG(LogTemp, Warning, TEXT("Bone %s not found, using approximate location"), *BoneName.ToString());
    }
    
    return BoneLocation;
}

void UAnim_IKFootPlacement::CalculatePelvisOffset()
{
    // Calculate pelvis offset to keep character grounded
    // Use the higher foot as reference to avoid sinking into ground
    float MaxOffset = FMath::Max(FootIKData.LeftFootOffset, FootIKData.RightFootOffset);
    
    // Only apply negative offset (lowering pelvis) to accommodate higher foot
    if (MaxOffset > 0.0f)
    {
        FootIKData.PelvisOffset = -MaxOffset * 0.5f; // Reduce impact by half
    }
    else
    {
        FootIKData.PelvisOffset = 0.0f;
    }
}

void UAnim_IKFootPlacement::SmoothIKValues(float DeltaTime)
{
    float InterpSpeed = IKSettings.InterpSpeed;
    
    // Smooth foot offsets
    SmoothedLeftFootOffset = FMath::FInterpTo(SmoothedLeftFootOffset, FootIKData.LeftFootOffset, DeltaTime, InterpSpeed);
    SmoothedRightFootOffset = FMath::FInterpTo(SmoothedRightFootOffset, FootIKData.RightFootOffset, DeltaTime, InterpSpeed);
    
    // Smooth foot rotations
    SmoothedLeftFootRotation = FMath::RInterpTo(SmoothedLeftFootRotation, FootIKData.LeftFootRotation, DeltaTime, InterpSpeed);
    SmoothedRightFootRotation = FMath::RInterpTo(SmoothedRightFootRotation, FootIKData.RightFootRotation, DeltaTime, InterpSpeed);
    
    // Smooth pelvis offset
    SmoothedPelvisOffset = FMath::FInterpTo(SmoothedPelvisOffset, FootIKData.PelvisOffset, DeltaTime, InterpSpeed);
    
    // Update final IK data with smoothed values
    FootIKData.LeftFootOffset = SmoothedLeftFootOffset;
    FootIKData.RightFootOffset = SmoothedRightFootOffset;
    FootIKData.LeftFootRotation = SmoothedLeftFootRotation;
    FootIKData.RightFootRotation = SmoothedRightFootRotation;
    FootIKData.PelvisOffset = SmoothedPelvisOffset;
}

void UAnim_IKFootPlacement::SetIKEnabled(bool bEnabled)
{
    IKSettings.bEnableIK = bEnabled;
    
    if (!bEnabled)
    {
        // Reset IK data when disabled
        FootIKData = FAnim_FootIKData();
        SmoothedLeftFootOffset = 0.0f;
        SmoothedRightFootOffset = 0.0f;
        SmoothedLeftFootRotation = FRotator::ZeroRotator;
        SmoothedRightFootRotation = FRotator::ZeroRotator;
        SmoothedPelvisOffset = 0.0f;
    }
}

void UAnim_IKFootPlacement::SetIKAlpha(float Alpha)
{
    FootIKData.IKAlpha = FMath::Clamp(Alpha, 0.0f, 1.0f);
}

float UAnim_IKFootPlacement::GetFootOffset(bool bIsLeftFoot) const
{
    return bIsLeftFoot ? FootIKData.LeftFootOffset : FootIKData.RightFootOffset;
}

FRotator UAnim_IKFootPlacement::GetFootRotation(bool bIsLeftFoot) const
{
    return bIsLeftFoot ? FootIKData.LeftFootRotation : FootIKData.RightFootRotation;
}

void UAnim_IKFootPlacement::DrawDebugInfo() const
{
    if (!GetWorld() || !OwnerCharacter)
    {
        return;
    }

    // Draw foot trace lines
    FVector LeftFootLocation = GetFootWorldLocation(true);
    FVector RightFootLocation = GetFootWorldLocation(false);
    
    FVector TraceOffset = FVector(0, 0, IKSettings.TraceDistance);
    
    // Left foot trace
    DrawDebugLine(GetWorld(), LeftFootLocation + TraceOffset, LeftFootLocation - TraceOffset, 
                  FColor::Green, false, -1, 0, 2.0f);
    
    // Right foot trace
    DrawDebugLine(GetWorld(), RightFootLocation + TraceOffset, RightFootLocation - TraceOffset, 
                  FColor::Blue, false, -1, 0, 2.0f);
    
    // Draw foot offset indicators
    if (FootIKData.LeftFootOffset != 0.0f)
    {
        DrawDebugSphere(GetWorld(), LeftFootLocation + FVector(0, 0, FootIKData.LeftFootOffset), 
                        5.0f, 8, FColor::Red, false, -1, 0);
    }
    
    if (FootIKData.RightFootOffset != 0.0f)
    {
        DrawDebugSphere(GetWorld(), RightFootLocation + FVector(0, 0, FootIKData.RightFootOffset), 
                        5.0f, 8, FColor::Red, false, -1, 0);
    }
}