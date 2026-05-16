#include "Anim_IKFootPlacement.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"

UAnim_IKFootPlacement::UAnim_IKFootPlacement()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize bone names
    LeftFootBoneName = FName("foot_l");
    RightFootBoneName = FName("foot_r");
    PelvisBoneName = FName("pelvis");
    
    // Initialize interpolation values
    TargetPelvisOffset = 0.0f;
    CurrentPelvisOffset = 0.0f;
    PelvisOffset = 0.0f;
    
    // Initialize component references
    SkeletalMeshComponent = nullptr;
    OwnerCharacter = nullptr;
}

void UAnim_IKFootPlacement::BeginPlay()
{
    Super::BeginPlay();
    
    // Get component references
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        SkeletalMeshComponent = OwnerCharacter->GetMesh();
        UE_LOG(LogTemp, Log, TEXT("IK Foot Placement initialized for character: %s"), 
               *OwnerCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("IK Foot Placement failed to get character reference"));
    }
}

void UAnim_IKFootPlacement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (IKSettings.bEnableFootIK && OwnerCharacter && SkeletalMeshComponent)
    {
        UpdateFootIK(DeltaTime);
    }
}

void UAnim_IKFootPlacement::UpdateFootIK(float DeltaTime)
{
    if (!OwnerCharacter || !SkeletalMeshComponent)
    {
        return;
    }
    
    // Only update IK when character is on ground
    UCharacterMovementComponent* MovementComponent = OwnerCharacter->GetCharacterMovement();
    if (!MovementComponent || MovementComponent->IsFalling())
    {
        // Reset IK when in air
        LeftFootData.IKAlpha = FMath::FInterpTo(LeftFootData.IKAlpha, 0.0f, DeltaTime, IKSettings.InterpSpeed);
        RightFootData.IKAlpha = FMath::FInterpTo(RightFootData.IKAlpha, 0.0f, DeltaTime, IKSettings.InterpSpeed);
        PelvisOffset = FMath::FInterpTo(PelvisOffset, 0.0f, DeltaTime, IKSettings.InterpSpeed);
        return;
    }
    
    // Calculate foot IK data
    TargetLeftFootData = CalculateFootIK(LeftFootBoneName, DeltaTime);
    TargetRightFootData = CalculateFootIK(RightFootBoneName, DeltaTime);
    
    // Calculate pelvis offset
    if (IKSettings.bEnablePelvisAdjustment)
    {
        TargetPelvisOffset = CalculatePelvisOffset();
    }
    
    // Interpolate to target values for smooth transitions
    LeftFootData.FootLocation = FMath::VInterpTo(LeftFootData.FootLocation, TargetLeftFootData.FootLocation, DeltaTime, IKSettings.InterpSpeed);
    LeftFootData.FootRotation = FMath::RInterpTo(LeftFootData.FootRotation, TargetLeftFootData.FootRotation, DeltaTime, IKSettings.InterpSpeed);
    LeftFootData.IKAlpha = FMath::FInterpTo(LeftFootData.IKAlpha, TargetLeftFootData.IKAlpha, DeltaTime, IKSettings.InterpSpeed);
    
    RightFootData.FootLocation = FMath::VInterpTo(RightFootData.FootLocation, TargetRightFootData.FootLocation, DeltaTime, IKSettings.InterpSpeed);
    RightFootData.FootRotation = FMath::RInterpTo(RightFootData.FootRotation, TargetRightFootData.FootRotation, DeltaTime, IKSettings.InterpSpeed);
    RightFootData.IKAlpha = FMath::FInterpTo(RightFootData.IKAlpha, TargetRightFootData.IKAlpha, DeltaTime, IKSettings.InterpSpeed);
    
    PelvisOffset = FMath::FInterpTo(PelvisOffset, TargetPelvisOffset, DeltaTime, IKSettings.InterpSpeed);
}

FAnim_FootData UAnim_IKFootPlacement::CalculateFootIK(const FName& FootBoneName, float DeltaTime)
{
    FAnim_FootData FootData;
    
    if (!SkeletalMeshComponent || FootBoneName == NAME_None)
    {
        return FootData;
    }
    
    // Get foot bone location in world space
    FVector FootBoneLocation = SkeletalMeshComponent->GetBoneLocation(FootBoneName, EBoneSpaces::WorldSpace);
    
    // Perform trace downward from foot location
    FHitResult HitResult = PerformFootTrace(FootBoneLocation);
    
    if (HitResult.bBlockingHit)
    {
        // Calculate distance from ground
        FootData.DistanceFromGround = FootBoneLocation.Z - HitResult.Location.Z;
        
        // Check if foot needs adjustment
        if (FMath::Abs(FootData.DistanceFromGround) <= IKSettings.MaxFootAdjustment)
        {
            // Calculate foot position adjustment
            FootData.FootLocation = FVector(0.0f, 0.0f, -(FootData.DistanceFromGround - IKSettings.FootOffset));
            
            // Calculate foot rotation from surface normal
            FootData.FootRotation = CalculateFootRotationFromNormal(HitResult.Normal);
            
            // Set IK alpha based on distance (closer to ground = higher alpha)
            FootData.IKAlpha = FMath::Clamp(1.0f - (FMath::Abs(FootData.DistanceFromGround) / IKSettings.MaxFootAdjustment), 0.0f, 1.0f);
            
            FootData.bValidFootPlacement = true;
        }
    }
    
    return FootData;
}

FHitResult UAnim_IKFootPlacement::PerformFootTrace(const FVector& FootLocation)
{
    FHitResult HitResult;
    
    if (!GetWorld())
    {
        return HitResult;
    }
    
    // Set up trace parameters
    FVector TraceStart = FootLocation + FVector(0.0f, 0.0f, IKSettings.TraceDistance * 0.5f);
    FVector TraceEnd = FootLocation - FVector(0.0f, 0.0f, IKSettings.TraceDistance);
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    QueryParams.bTraceComplex = false;
    QueryParams.bReturnPhysicalMaterial = true;
    
    // Perform line trace
    GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );
    
    // Debug draw trace (only in development builds)
    #if WITH_EDITOR
    if (CVarShowDebugAnimation.GetValueOnGameThread() > 0)
    {
        FColor TraceColor = HitResult.bBlockingHit ? FColor::Green : FColor::Red;
        DrawDebugLine(GetWorld(), TraceStart, TraceEnd, TraceColor, false, 0.1f, 0, 1.0f);
        
        if (HitResult.bBlockingHit)
        {
            DrawDebugSphere(GetWorld(), HitResult.Location, 3.0f, 8, FColor::Yellow, false, 0.1f);
        }
    }
    #endif
    
    return HitResult;
}

float UAnim_IKFootPlacement::CalculatePelvisOffset()
{
    // Calculate pelvis offset based on foot adjustments
    float LeftFootOffset = LeftFootData.FootLocation.Z;
    float RightFootOffset = RightFootData.FootLocation.Z;
    
    // Use the higher foot as reference to avoid pelvis going too low
    float MaxFootOffset = FMath::Max(LeftFootOffset, RightFootOffset);
    
    // Apply pelvis offset (negative because we want to lower the pelvis when feet are raised)
    return FMath::Clamp(-MaxFootOffset * 0.5f, -IKSettings.MaxFootAdjustment * 0.5f, IKSettings.MaxFootAdjustment * 0.5f);
}

FRotator UAnim_IKFootPlacement::CalculateFootRotationFromNormal(const FVector& SurfaceNormal)
{
    // Calculate foot rotation to align with surface normal
    FVector UpVector = FVector::UpVector;
    FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
    
    // Project forward vector onto surface plane
    FVector ProjectedForward = FVector::VectorPlaneProject(ForwardVector, SurfaceNormal).GetSafeNormal();
    
    // Calculate right vector
    FVector RightVector = FVector::CrossProduct(SurfaceNormal, ProjectedForward).GetSafeNormal();
    
    // Recalculate forward vector to ensure orthogonality
    ProjectedForward = FVector::CrossProduct(RightVector, SurfaceNormal).GetSafeNormal();
    
    // Create rotation matrix and convert to rotator
    FMatrix RotationMatrix = FMatrix(ProjectedForward, RightVector, SurfaceNormal, FVector::ZeroVector);
    FRotator SurfaceRotation = RotationMatrix.Rotator();
    
    // Return relative rotation from character's current rotation
    FRotator CharacterRotation = OwnerCharacter->GetActorRotation();
    return UKismetMathLibrary::NormalizedDeltaRotator(SurfaceRotation, CharacterRotation);
}

void UAnim_IKFootPlacement::SetIKEnabled(bool bEnabled)
{
    IKSettings.bEnableFootIK = bEnabled;
    
    if (!bEnabled)
    {
        // Reset IK data when disabled
        LeftFootData = FAnim_FootData();
        RightFootData = FAnim_FootData();
        PelvisOffset = 0.0f;
    }
}