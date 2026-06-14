#include "Anim_IKFootPlacement.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"

UAnim_IKFootPlacement::UAnim_IKFootPlacement()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Default settings
    TraceDistance = 150.0f;
    InterpSpeed = 15.0f;
    FootHeight = 20.0f;
    LeftFootBoneName = TEXT("foot_l");
    RightFootBoneName = TEXT("foot_r");
    PelvisBoneName = TEXT("pelvis");
    
    PelvisOffset = 0.0f;
    OwnerMesh = nullptr;
    AnimInstance = nullptr;
}

void UAnim_IKFootPlacement::BeginPlay()
{
    Super::BeginPlay();
    
    // Get owner character and mesh
    if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
    {
        OwnerMesh = OwnerCharacter->GetMesh();
        if (OwnerMesh)
        {
            AnimInstance = OwnerMesh->GetAnimInstance();
        }
    }
}

void UAnim_IKFootPlacement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (OwnerMesh && AnimInstance)
    {
        UpdateFootIK();
    }
}

void UAnim_IKFootPlacement::UpdateFootIK()
{
    if (!OwnerMesh || !AnimInstance)
    {
        return;
    }

    // Calculate IK for both feet
    FVector LeftFootLocation = GetFootWorldLocation(LeftFootBoneName);
    FVector RightFootLocation = GetFootWorldLocation(RightFootBoneName);
    
    LeftFootIK = CalculateFootIK(LeftFootBoneName, LeftFootLocation);
    RightFootIK = CalculateFootIK(RightFootBoneName, RightFootLocation);
    
    // Calculate pelvis adjustment
    float LeftOffset = LeftFootIK.bIsGrounded ? LeftFootIK.FootOffset.Z : 0.0f;
    float RightOffset = RightFootIK.bIsGrounded ? RightFootIK.FootOffset.Z : 0.0f;
    float TargetPelvisOffset = FMath::Min(LeftOffset, RightOffset);
    
    // Smooth pelvis adjustment
    PelvisOffset = FMath::FInterpTo(PelvisOffset, TargetPelvisOffset, GetWorld()->GetDeltaSeconds(), InterpSpeed);
    
    // Adjust foot offsets relative to pelvis
    if (LeftFootIK.bIsGrounded)
    {
        LeftFootIK.FootOffset.Z -= PelvisOffset;
    }
    if (RightFootIK.bIsGrounded)
    {
        RightFootIK.FootOffset.Z -= PelvisOffset;
    }
}

FAnim_FootIKData UAnim_IKFootPlacement::CalculateFootIK(FName FootBoneName, const FVector& FootLocation)
{
    FAnim_FootIKData FootData;
    
    FHitResult HitResult;
    FVector TraceStart = FootLocation + FVector(0, 0, FootHeight);
    
    if (PerformFootTrace(TraceStart, HitResult))
    {
        FootData.bIsGrounded = true;
        
        // Calculate foot offset
        float DistanceToGround = (TraceStart - HitResult.Location).Z;
        FootData.FootOffset.Z = FootHeight - DistanceToGround;
        
        // Calculate foot rotation to match surface normal
        FVector UpVector = FVector::UpVector;
        FVector RightVector = FVector::CrossProduct(UpVector, HitResult.Normal).GetSafeNormal();
        FVector ForwardVector = FVector::CrossProduct(HitResult.Normal, RightVector).GetSafeNormal();
        
        FootData.FootRotation = UKismetMathLibrary::MakeRotationFromAxes(ForwardVector, RightVector, HitResult.Normal);
        FootData.IKAlpha = 1.0f;
        
        // Smooth interpolation
        float DeltaTime = GetWorld()->GetDeltaSeconds();
        FootData.FootOffset = FMath::VInterpTo(FootData.FootOffset, FootData.FootOffset, DeltaTime, InterpSpeed);
    }
    else
    {
        FootData.bIsGrounded = false;
        FootData.IKAlpha = 0.0f;
    }
    
    return FootData;
}

FVector UAnim_IKFootPlacement::GetFootWorldLocation(FName FootBoneName) const
{
    if (!OwnerMesh)
    {
        return FVector::ZeroVector;
    }
    
    return OwnerMesh->GetBoneLocation(FootBoneName, EBoneSpaces::WorldSpace);
}

bool UAnim_IKFootPlacement::PerformFootTrace(const FVector& StartLocation, FHitResult& HitResult) const
{
    if (!GetWorld())
    {
        return false;
    }
    
    FVector EndLocation = StartLocation - FVector(0, 0, TraceDistance);
    
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;
    QueryParams.AddIgnoredActor(GetOwner());
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECC_WorldStatic,
        QueryParams
    );
    
    // Debug visualization
    if (CVarShowDebugAnimation.GetValueOnGameThread())
    {
        FColor TraceColor = bHit ? FColor::Green : FColor::Red;
        DrawDebugLine(GetWorld(), StartLocation, EndLocation, TraceColor, false, 0.1f, 0, 1.0f);
        
        if (bHit)
        {
            DrawDebugSphere(GetWorld(), HitResult.Location, 5.0f, 8, FColor::Blue, false, 0.1f);
        }
    }
    
    return bHit;
}