#include "Anim_IKFootPlacement.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UAnim_IKFootPlacement::UAnim_IKFootPlacement()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    
    PelvisOffset = 0.0f;
    PreviousLeftFootOffset = 0.0f;
    PreviousRightFootOffset = 0.0f;
    PreviousPelvisOffset = 0.0f;
    OwnerMesh = nullptr;
}

void UAnim_IKFootPlacement::BeginPlay()
{
    Super::BeginPlay();
    
    // Get the skeletal mesh component from the owner
    if (AActor* Owner = GetOwner())
    {
        OwnerMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
        if (!OwnerMesh)
        {
            // Try to get it from character
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
    
    if (IKSettings.bEnableFootIK && OwnerMesh)
    {
        UpdateFootIK(DeltaTime);
    }
}

void UAnim_IKFootPlacement::UpdateFootIK(float DeltaTime)
{
    if (!OwnerMesh || !GetWorld())
    {
        return;
    }
    
    // Update foot IK data
    LeftFootIKData = CalculateFootIK(IKSettings.LeftFootSocketName, DeltaTime);
    RightFootIKData = CalculateFootIK(IKSettings.RightFootSocketName, DeltaTime);
    
    // Calculate pelvis offset to keep character grounded
    float LeftOffset = LeftFootIKData.FootOffset;
    float RightOffset = RightFootIKData.FootOffset;
    
    // Use the higher foot as reference for pelvis adjustment
    float TargetPelvisOffset = FMath::Max(LeftOffset, RightOffset);
    
    // Smooth interpolation
    PelvisOffset = FMath::FInterpTo(PreviousPelvisOffset, TargetPelvisOffset, DeltaTime, IKSettings.InterpSpeed);
    PreviousPelvisOffset = PelvisOffset;
    
    // Adjust foot offsets relative to pelvis
    LeftFootIKData.FootOffset = LeftOffset - PelvisOffset;
    RightFootIKData.FootOffset = RightOffset - PelvisOffset;
}

FAnim_FootIKData UAnim_IKFootPlacement::CalculateFootIK(const FName& SocketName, float DeltaTime)
{
    FAnim_FootIKData FootData;
    
    if (!OwnerMesh || !OwnerMesh->DoesSocketExist(SocketName))
    {
        return FootData;
    }
    
    // Get foot socket location in world space
    FVector FootLocation = OwnerMesh->GetSocketLocation(SocketName);
    FRotator FootRotation = OwnerMesh->GetSocketRotation(SocketName);
    
    // Perform trace to find ground
    bool bHit = false;
    FVector HitNormal;
    FVector HitLocation = PerformFootTrace(FootLocation, bHit, HitNormal);
    
    if (bHit)
    {
        // Calculate foot offset
        float FootOffset = CalculateFootOffset(FootLocation, HitLocation);
        
        // Smooth interpolation
        float PreviousOffset = (SocketName == IKSettings.LeftFootSocketName) ? PreviousLeftFootOffset : PreviousRightFootOffset;
        FootOffset = FMath::FInterpTo(PreviousOffset, FootOffset, DeltaTime, IKSettings.InterpSpeed);
        
        // Store previous offset
        if (SocketName == IKSettings.LeftFootSocketName)
        {
            PreviousLeftFootOffset = FootOffset;
        }
        else
        {
            PreviousRightFootOffset = FootOffset;
        }
        
        // Calculate foot rotation based on surface normal
        FRotator TargetRotation = CalculateFootRotation(HitNormal, FootRotation);
        
        // Fill foot data
        FootData.FootLocation = HitLocation;
        FootData.FootRotation = TargetRotation;
        FootData.FootOffset = FootOffset;
        FootData.IKAlpha = FMath::Clamp(FMath::Abs(FootOffset) / IKSettings.MaxFootOffset, 0.0f, 1.0f);
    }
    
    return FootData;
}

FVector UAnim_IKFootPlacement::PerformFootTrace(const FVector& FootLocation, bool& bHit, FVector& HitNormal)
{
    if (!GetWorld())
    {
        bHit = false;
        return FootLocation;
    }
    
    // Trace parameters
    FVector TraceStart = FootLocation + FVector(0, 0, IKSettings.TraceDistance * 0.5f);
    FVector TraceEnd = FootLocation - FVector(0, 0, IKSettings.TraceDistance);
    
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;
    QueryParams.AddIgnoredActor(GetOwner());
    
    FHitResult HitResult;
    bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECollisionChannel::ECC_WorldStatic,
        QueryParams
    );
    
    // Debug visualization
    if (IKSettings.bShowDebugTraces)
    {
        FColor DebugColor = bHit ? FColor::Green : FColor::Red;
        DrawDebugLine(GetWorld(), TraceStart, TraceEnd, DebugColor, false, 0.1f, 0, 1.0f);
        
        if (bHit)
        {
            DrawDebugSphere(GetWorld(), HitResult.Location, 5.0f, 8, FColor::Yellow, false, 0.1f);
        }
    }
    
    if (bHit)
    {
        HitNormal = HitResult.Normal;
        return HitResult.Location;
    }
    
    HitNormal = FVector::UpVector;
    return FootLocation;
}

float UAnim_IKFootPlacement::CalculateFootOffset(const FVector& FootLocation, const FVector& HitLocation)
{
    float Offset = HitLocation.Z - FootLocation.Z;
    return FMath::Clamp(Offset, -IKSettings.MaxFootOffset, IKSettings.MaxFootOffset);
}

FRotator UAnim_IKFootPlacement::CalculateFootRotation(const FVector& HitNormal, const FRotator& CurrentRotation)
{
    // Calculate rotation to align foot with surface normal
    FVector ForwardVector = CurrentRotation.Vector();
    FVector RightVector = FVector::CrossProduct(HitNormal, ForwardVector).GetSafeNormal();
    ForwardVector = FVector::CrossProduct(RightVector, HitNormal).GetSafeNormal();
    
    FRotator TargetRotation = UKismetMathLibrary::MakeRotationFromAxes(ForwardVector, RightVector, HitNormal);
    
    // Limit rotation to prevent extreme angles
    FRotator DeltaRotation = TargetRotation - CurrentRotation;
    DeltaRotation.Pitch = FMath::Clamp(DeltaRotation.Pitch, -45.0f, 45.0f);
    DeltaRotation.Roll = FMath::Clamp(DeltaRotation.Roll, -45.0f, 45.0f);
    
    return CurrentRotation + DeltaRotation;
}