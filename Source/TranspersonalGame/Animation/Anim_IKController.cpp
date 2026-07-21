#include "Anim_IKController.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_IKController::UAnim_IKController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    IKSettings = FAnim_IKSettings();
    LeftFootIKData = FAnim_FootIKData();
    RightFootIKData = FAnim_FootIKData();
    PelvisOffset = 0.0f;
    
    LeftHandIKLocation = FVector::ZeroVector;
    RightHandIKLocation = FVector::ZeroVector;
    LeftHandIKRotation = FRotator::ZeroRotator;
    RightHandIKRotation = FRotator::ZeroRotator;
    LeftHandIKAlpha = 0.0f;
    RightHandIKAlpha = 0.0f;
}

void UAnim_IKController::BeginPlay()
{
    Super::BeginPlay();
    
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        SkeletalMeshComponent = OwnerCharacter->GetMesh();
    }
    
    if (!OwnerCharacter || !SkeletalMeshComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("Anim_IKController: Invalid owner character or skeletal mesh"));
    }
}

void UAnim_IKController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (IKSettings.bEnableFootIK && OwnerCharacter && SkeletalMeshComponent)
    {
        UpdateFootIK(DeltaTime);
    }
}

void UAnim_IKController::UpdateFootIK(float DeltaTime)
{
    if (!OwnerCharacter || !SkeletalMeshComponent)
    {
        return;
    }
    
    // Calculate foot IK for both feet
    LeftFootIKData = CalculateFootIK(LeftFootBoneName, DeltaTime);
    RightFootIKData = CalculateFootIK(RightFootBoneName, DeltaTime);
    
    // Calculate pelvis offset to keep character grounded
    float TargetPelvisOffset = CalculatePelvisOffset();
    PelvisOffset = FMath::FInterpTo(PelvisOffset, TargetPelvisOffset, DeltaTime, IKSettings.InterpSpeed);
}

FAnim_FootIKData UAnim_IKController::CalculateFootIK(FName FootBoneName, float DeltaTime)
{
    FAnim_FootIKData FootData;
    
    if (!SkeletalMeshComponent || FootBoneName == NAME_None)
    {
        return FootData;
    }
    
    // Get foot bone location in world space
    FVector FootBoneLocation = SkeletalMeshComponent->GetBoneLocation(FootBoneName, EBoneSpaces::WorldSpace);
    
    // Perform trace from foot bone downward
    FHitResult HitResult = PerformFootTrace(FootBoneLocation);
    
    if (HitResult.bBlockingHit)
    {
        // Calculate distance from ground
        float DistanceFromGround = FootBoneLocation.Z - HitResult.Location.Z;
        FootData.DistanceFromGround = DistanceFromGround;
        
        // Calculate IK alpha based on distance
        float IKAlpha = 0.0f;
        if (FMath::Abs(DistanceFromGround) < IKSettings.MaxIKDistance)
        {
            IKAlpha = 1.0f - (FMath::Abs(DistanceFromGround) / IKSettings.MaxIKDistance);
        }
        
        // Smooth IK alpha changes
        FootData.IKAlpha = FMath::FInterpTo(FootData.IKAlpha, IKAlpha, DeltaTime, IKSettings.InterpSpeed);
        
        // Calculate foot location adjustment
        FVector TargetLocation = FVector(0, 0, DistanceFromGround + IKSettings.FootHeight);
        FootData.FootLocation = FMath::VInterpTo(FootData.FootLocation, TargetLocation, DeltaTime, IKSettings.InterpSpeed);
        
        // Calculate foot rotation to match surface normal
        FVector SurfaceNormal = HitResult.Normal;
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector RightVector = FVector::CrossProduct(SurfaceNormal, ForwardVector).GetSafeNormal();
        ForwardVector = FVector::CrossProduct(RightVector, SurfaceNormal).GetSafeNormal();
        
        FRotator TargetRotation = UKismetMathLibrary::MakeRotationFromAxes(ForwardVector, RightVector, SurfaceNormal);
        FRotator CurrentRotation = SkeletalMeshComponent->GetBoneQuaternion(FootBoneName, EBoneSpaces::WorldSpace).Rotator();
        FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(TargetRotation, CurrentRotation);
        
        FootData.FootRotation = FMath::RInterpTo(FootData.FootRotation, DeltaRotation, DeltaTime, IKSettings.InterpSpeed);
    }
    else
    {
        // No ground hit, gradually return to default position
        FootData.IKAlpha = FMath::FInterpTo(FootData.IKAlpha, 0.0f, DeltaTime, IKSettings.InterpSpeed);
        FootData.FootLocation = FMath::VInterpTo(FootData.FootLocation, FVector::ZeroVector, DeltaTime, IKSettings.InterpSpeed);
        FootData.FootRotation = FMath::RInterpTo(FootData.FootRotation, FRotator::ZeroRotator, DeltaTime, IKSettings.InterpSpeed);
    }
    
    return FootData;
}

FHitResult UAnim_IKController::PerformFootTrace(FVector StartLocation) const
{
    FHitResult HitResult;
    
    if (!GetWorld())
    {
        return HitResult;
    }
    
    FVector TraceStart = StartLocation + FVector(0, 0, IKSettings.TraceDistance * 0.5f);
    FVector TraceEnd = StartLocation - FVector(0, 0, IKSettings.TraceDistance);
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    QueryParams.bTraceComplex = false;
    
    GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECollisionChannel::ECC_WorldStatic,
        QueryParams
    );
    
    // Debug drawing (only in development builds)
    #if WITH_EDITOR
    if (CVarShowDebugAnimation.GetValueOnGameThread() > 0)
    {
        FColor TraceColor = HitResult.bBlockingHit ? FColor::Green : FColor::Red;
        DrawDebugLine(GetWorld(), TraceStart, TraceEnd, TraceColor, false, 0.1f, 0, 1.0f);
        
        if (HitResult.bBlockingHit)
        {
            DrawDebugSphere(GetWorld(), HitResult.Location, 5.0f, 8, FColor::Yellow, false, 0.1f);
        }
    }
    #endif
    
    return HitResult;
}

float UAnim_IKController::CalculatePelvisOffset() const
{
    // Calculate the lowest foot position to adjust pelvis
    float LeftFootOffset = LeftFootIKData.FootLocation.Z * LeftFootIKData.IKAlpha;
    float RightFootOffset = RightFootIKData.FootLocation.Z * RightFootIKData.IKAlpha;
    
    // Use the lower foot as reference for pelvis adjustment
    return FMath::Min(LeftFootOffset, RightFootOffset);
}

void UAnim_IKController::SetLeftHandIKTarget(FVector TargetLocation, FRotator TargetRotation, float Alpha)
{
    LeftHandIKLocation = TargetLocation;
    LeftHandIKRotation = TargetRotation;
    LeftHandIKAlpha = FMath::Clamp(Alpha, 0.0f, 1.0f);
}

void UAnim_IKController::SetRightHandIKTarget(FVector TargetLocation, FRotator TargetRotation, float Alpha)
{
    RightHandIKLocation = TargetLocation;
    RightHandIKRotation = TargetRotation;
    RightHandIKAlpha = FMath::Clamp(Alpha, 0.0f, 1.0f);
}

void UAnim_IKController::ClearHandIKTargets()
{
    LeftHandIKAlpha = 0.0f;
    RightHandIKAlpha = 0.0f;
    LeftHandIKLocation = FVector::ZeroVector;
    RightHandIKLocation = FVector::ZeroVector;
    LeftHandIKRotation = FRotator::ZeroRotator;
    RightHandIKRotation = FRotator::ZeroRotator;
}