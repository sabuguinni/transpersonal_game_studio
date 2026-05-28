#include "Anim_FootIKSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_FootIKSystem::UAnim_FootIKSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Default settings
    TraceDistance = 150.0f;
    IKInterpSpeed = 15.0f;
    MaxFootOffset = 50.0f;
    bEnableFootIK = true;
    bShowDebugTraces = false;
    
    HipOffset = 0.0f;
    CurrentHipOffset = 0.0f;
    LastRootLocation = FVector::ZeroVector;
}

void UAnim_FootIKSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Get the skeletal mesh component
    if (AActor* Owner = GetOwner())
    {
        OwnerMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
        if (!OwnerMesh)
        {
            UE_LOG(LogTemp, Warning, TEXT("FootIKSystem: No SkeletalMeshComponent found on owner"));
        }
    }
}

void UAnim_FootIKSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bEnableFootIK && OwnerMesh)
    {
        UpdateFootIK(DeltaTime);
    }
}

void UAnim_FootIKSystem::UpdateFootIK(float DeltaTime)
{
    if (!OwnerMesh || !GetOwner())
    {
        return;
    }
    
    FVector RootLocation = GetOwner()->GetActorLocation();
    
    // Calculate foot IK for both feet
    LeftFootIK = CalculateFootIK(FName("foot_l"), RootLocation);
    RightFootIK = CalculateFootIK(FName("foot_r"), RootLocation);
    
    // Update hip offset to keep character grounded
    UpdateHipOffset(DeltaTime);
    
    LastRootLocation = RootLocation;
}

FAnim_FootIKData UAnim_FootIKSystem::CalculateFootIK(const FName& SocketName, const FVector& RootLocation)
{
    FAnim_FootIKData FootData;
    
    if (!OwnerMesh)
    {
        return FootData;
    }
    
    // Get foot socket location
    FVector SocketLocation = OwnerMesh->GetSocketLocation(SocketName);
    
    // Perform trace from above the foot to below
    FVector TraceStart = SocketLocation + FVector(0, 0, 50.0f);
    FVector TraceEnd = SocketLocation - FVector(0, 0, TraceDistance);
    
    FHitResult HitResult = PerformFootTrace(TraceStart, TraceEnd);
    
    if (HitResult.bBlockingHit)
    {
        // Calculate foot offset
        float DistanceFromGround = FVector::Dist(SocketLocation, HitResult.Location);
        FootData.DistanceFromGround = DistanceFromGround;
        
        // Calculate IK alpha based on distance
        FootData.IKAlpha = FMath::Clamp(DistanceFromGround / MaxFootOffset, 0.0f, 1.0f);
        
        // Set foot location
        FootData.FootLocation = HitResult.Location;
        
        // Calculate foot rotation from surface normal
        FootData.FootRotation = CalculateFootRotationFromNormal(HitResult.Normal);
        
        // Debug visualization
        if (bShowDebugTraces && GetWorld())
        {
            DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 0.1f);
            DrawDebugSphere(GetWorld(), HitResult.Location, 5.0f, 8, FColor::Green, false, 0.1f);
        }
    }
    else
    {
        // No ground hit - disable IK
        FootData.IKAlpha = 0.0f;
        FootData.FootLocation = SocketLocation;
        FootData.FootRotation = FRotator::ZeroRotator;
        FootData.DistanceFromGround = TraceDistance;
    }
    
    return FootData;
}

FHitResult UAnim_FootIKSystem::PerformFootTrace(const FVector& StartLocation, const FVector& EndLocation)
{
    FHitResult HitResult;
    
    if (!GetWorld())
    {
        return HitResult;
    }
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.bTraceComplex = false;
    
    GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );
    
    return HitResult;
}

FRotator UAnim_FootIKSystem::CalculateFootRotationFromNormal(const FVector& Normal)
{
    // Calculate rotation to align foot with surface normal
    FVector ForwardVector = GetOwner()->GetActorForwardVector();
    FVector RightVector = FVector::CrossProduct(Normal, ForwardVector).GetSafeNormal();
    ForwardVector = FVector::CrossProduct(RightVector, Normal).GetSafeNormal();
    
    return UKismetMathLibrary::MakeRotationFromAxes(ForwardVector, RightVector, Normal);
}

void UAnim_FootIKSystem::UpdateHipOffset(float DeltaTime)
{
    // Calculate target hip offset based on foot positions
    float LeftOffset = FMath::Max(0.0f, LeftFootIK.DistanceFromGround - MaxFootOffset);
    float RightOffset = FMath::Max(0.0f, RightFootIK.DistanceFromGround - MaxFootOffset);
    
    float TargetHipOffset = -FMath::Min(LeftOffset, RightOffset);
    
    // Smooth interpolation
    CurrentHipOffset = FMath::FInterpTo(CurrentHipOffset, TargetHipOffset, DeltaTime, IKInterpSpeed);
    HipOffset = CurrentHipOffset;
}