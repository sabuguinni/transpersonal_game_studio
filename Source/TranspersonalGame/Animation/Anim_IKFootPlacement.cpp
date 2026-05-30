#include "Anim_IKFootPlacement.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_IKFootPlacement::UAnim_IKFootPlacement()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    bEnableFootIK = true;
    TraceDistance = 50.0f;
    IKInterpSpeed = 15.0f;
    FootHeight = 5.0f;
    LeftFootSocketName = FName("foot_l");
    RightFootSocketName = FName("foot_r");
    HipOffset = 0.0f;
    
    OwningCharacter = nullptr;
    CharacterMesh = nullptr;
}

void UAnim_IKFootPlacement::BeginPlay()
{
    Super::BeginPlay();
    
    OwningCharacter = Cast<ACharacter>(GetOwner());
    if (OwningCharacter)
    {
        CharacterMesh = OwningCharacter->GetMesh();
    }
}

void UAnim_IKFootPlacement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bEnableFootIK && OwningCharacter && CharacterMesh)
    {
        UpdateFootIK(DeltaTime);
    }
}

void UAnim_IKFootPlacement::UpdateFootIK(float DeltaTime)
{
    if (!OwningCharacter || !CharacterMesh)
    {
        return;
    }
    
    // Calculate IK for both feet
    LeftFootIK = CalculateFootIK(LeftFootSocketName, DeltaTime);
    RightFootIK = CalculateFootIK(RightFootSocketName, DeltaTime);
    
    // Update hip offset to keep character level
    UpdateHipOffset(DeltaTime);
}

FAnim_FootIKData UAnim_IKFootPlacement::CalculateFootIK(const FName& FootSocketName, float DeltaTime)
{
    FAnim_FootIKData FootData;
    
    if (!CharacterMesh || !CharacterMesh->DoesSocketExist(FootSocketName))
    {
        return FootData;
    }
    
    // Get foot world location
    FVector FootLocation = GetFootWorldLocation(FootSocketName);
    
    // Perform line trace
    FHitResult HitResult;
    if (LineTraceFromFoot(FootLocation, HitResult))
    {
        // Calculate distance from ground
        float DistanceToGround = (FootLocation.Z - HitResult.Location.Z) - FootHeight;
        FootData.DistanceFromGround = DistanceToGround;
        
        // Calculate IK alpha based on distance
        if (DistanceToGround > 0.0f)
        {
            FootData.IKAlpha = FMath::Clamp(DistanceToGround / TraceDistance, 0.0f, 1.0f);
            
            // Set foot location and rotation
            FootData.FootLocation = FVector(0.0f, 0.0f, -DistanceToGround);
            FootData.FootRotation = CalculateFootRotation(HitResult);
        }
        else
        {
            FootData.IKAlpha = 0.0f;
            FootData.FootLocation = FVector::ZeroVector;
            FootData.FootRotation = FRotator::ZeroRotator;
        }
    }
    else
    {
        FootData.IKAlpha = 0.0f;
        FootData.FootLocation = FVector::ZeroVector;
        FootData.FootRotation = FRotator::ZeroRotator;
        FootData.DistanceFromGround = 0.0f;
    }
    
    return FootData;
}

FVector UAnim_IKFootPlacement::GetFootWorldLocation(const FName& FootSocketName) const
{
    if (CharacterMesh && CharacterMesh->DoesSocketExist(FootSocketName))
    {
        return CharacterMesh->GetSocketLocation(FootSocketName);
    }
    
    return FVector::ZeroVector;
}

bool UAnim_IKFootPlacement::LineTraceFromFoot(const FVector& FootLocation, FHitResult& HitResult) const
{
    if (!GetWorld())
    {
        return false;
    }
    
    FVector StartLocation = FootLocation + FVector(0.0f, 0.0f, 20.0f);
    FVector EndLocation = FootLocation - FVector(0.0f, 0.0f, TraceDistance);
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwningCharacter);
    
    return GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );
}

FRotator UAnim_IKFootPlacement::CalculateFootRotation(const FHitResult& HitResult) const
{
    FVector SurfaceNormal = HitResult.Normal;
    FVector ForwardVector = OwningCharacter->GetActorForwardVector();
    
    // Calculate foot rotation to align with surface
    FVector RightVector = FVector::CrossProduct(SurfaceNormal, ForwardVector).GetSafeNormal();
    FVector NewForward = FVector::CrossProduct(RightVector, SurfaceNormal).GetSafeNormal();
    
    return UKismetMathLibrary::MakeRotationFromAxes(NewForward, RightVector, SurfaceNormal);
}

void UAnim_IKFootPlacement::UpdateHipOffset(float DeltaTime)
{
    float LeftDistance = LeftFootIK.DistanceFromGround;
    float RightDistance = RightFootIK.DistanceFromGround;
    
    // Use the smaller distance (higher foot) to determine hip offset
    float TargetHipOffset = FMath::Min(LeftDistance, RightDistance);
    TargetHipOffset = FMath::Clamp(TargetHipOffset, -TraceDistance, 0.0f);
    
    // Interpolate to smooth hip movement
    HipOffset = FMath::FInterpTo(HipOffset, TargetHipOffset, DeltaTime, IKInterpSpeed);
}

FAnim_FootIKData UAnim_IKFootPlacement::GetFootIKData(bool bIsLeftFoot) const
{
    return bIsLeftFoot ? LeftFootIK : RightFootIK;
}