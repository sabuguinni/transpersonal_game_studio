#include "Anim_IKFootPlacement.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UAnim_IKFootPlacement::UAnim_IKFootPlacement()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize IK data
    LeftFootIKData = FAnim_FootIKData();
    RightFootIKData = FAnim_FootIKData();
    
    PelvisOffset = 0.0f;
    TargetPelvisOffset = 0.0f;
    
    // Set default settings
    IKSettings = FAnim_IKSettings();
    
    // Default socket names (common UE5 skeleton naming)
    LeftFootSocketName = "foot_l";
    RightFootSocketName = "foot_r";
    
    // Control flags
    bEnableIK = true;
    bEnablePelvisAdjustment = true;
    
    // Collision settings
    TraceChannel = ECC_Visibility;
    
    // Interpolation speeds
    PelvisInterpSpeed = 10.0f;
    FootInterpSpeed = 15.0f;
}

void UAnim_IKFootPlacement::BeginPlay()
{
    Super::BeginPlay();
    
    // Get character and skeletal mesh component references
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        SkeletalMeshComponent = OwnerCharacter->GetMesh();
        if (SkeletalMeshComponent)
        {
            UE_LOG(LogTemp, Log, TEXT("IK Foot Placement initialized for character: %s"), *OwnerCharacter->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("IK Foot Placement: No SkeletalMeshComponent found!"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("IK Foot Placement: Owner is not a Character!"));
    }
}

void UAnim_IKFootPlacement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bEnableIK || !OwnerCharacter || !SkeletalMeshComponent)
    {
        return;
    }
    
    // Only update IK when character is on ground and moving slowly or stationary
    if (OwnerCharacter->GetCharacterMovement() && 
        !OwnerCharacter->GetCharacterMovement()->IsFalling() &&
        OwnerCharacter->GetVelocity().Size() < 600.0f) // Don't use IK when running fast
    {
        UpdateFootIK(DeltaTime);
    }
    else
    {
        // Reset IK when not applicable
        LeftFootIKData.IKAlpha = FMath::FInterpTo(LeftFootIKData.IKAlpha, 0.0f, DeltaTime, FootInterpSpeed);
        RightFootIKData.IKAlpha = FMath::FInterpTo(RightFootIKData.IKAlpha, 0.0f, DeltaTime, FootInterpSpeed);
        PelvisOffset = FMath::FInterpTo(PelvisOffset, 0.0f, DeltaTime, PelvisInterpSpeed);
    }
    
    // Draw debug info if enabled
    if (IKSettings.bDrawDebugTraces)
    {
        DrawDebugInfo();
    }
}

void UAnim_IKFootPlacement::UpdateFootIK(float DeltaTime)
{
    if (!SkeletalMeshComponent)
    {
        return;
    }
    
    // Calculate IK for both feet
    FAnim_FootIKData TargetLeftFoot = CalculateFootIK(LeftFootSocketName, DeltaTime);
    FAnim_FootIKData TargetRightFoot = CalculateFootIK(RightFootSocketName, DeltaTime);
    
    // Interpolate to target values
    InterpolateFootIKData(LeftFootIKData, TargetLeftFoot, DeltaTime);
    InterpolateFootIKData(RightFootIKData, TargetRightFoot, DeltaTime);
    
    // Update pelvis offset
    if (bEnablePelvisAdjustment)
    {
        UpdatePelvisOffset(DeltaTime);
    }
}

FAnim_FootIKData UAnim_IKFootPlacement::CalculateFootIK(const FName& SocketName, float DeltaTime)
{
    FAnim_FootIKData FootData;
    
    if (!SkeletalMeshComponent || !SkeletalMeshComponent->DoesSocketExist(SocketName))
    {
        return FootData;
    }
    
    // Get foot socket location in world space
    FVector FootSocketLocation = SkeletalMeshComponent->GetSocketLocation(SocketName);
    FRotator FootSocketRotation = SkeletalMeshComponent->GetSocketRotation(SocketName);
    
    // Trace for ground
    FVector HitLocation;
    FVector HitNormal;
    bool bHitGround = TraceForGround(FootSocketLocation, HitLocation, HitNormal);
    
    if (bHitGround)
    {
        // Calculate distance from foot to ground
        float DistanceToGround = FootSocketLocation.Z - HitLocation.Z;
        FootData.DistanceFromGround = DistanceToGround;
        FootData.GroundNormal = HitNormal;
        
        // Calculate IK alpha based on distance
        if (FMath::Abs(DistanceToGround) > IKSettings.FootOffset)
        {
            FootData.IKAlpha = 1.0f;
            
            // Calculate foot location adjustment
            FootData.FootLocation = FVector(0.0f, 0.0f, -DistanceToGround + IKSettings.FootOffset);
            
            // Calculate foot rotation if enabled
            if (IKSettings.bEnableFootRotation)
            {
                FootData.FootRotation = CalculateFootRotation(HitNormal, FootSocketRotation);
            }
        }
        else
        {
            FootData.IKAlpha = 0.0f;
        }
    }
    else
    {
        FootData.IKAlpha = 0.0f;
    }
    
    return FootData;
}

bool UAnim_IKFootPlacement::TraceForGround(const FVector& StartLocation, FVector& OutHitLocation, FVector& OutHitNormal)
{
    if (!GetWorld())
    {
        return false;
    }
    
    // Setup trace parameters
    FVector TraceStart = StartLocation + FVector(0.0f, 0.0f, IKSettings.TraceDistance * 0.5f);
    FVector TraceEnd = StartLocation - FVector(0.0f, 0.0f, IKSettings.TraceDistance);
    
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    
    FHitResult HitResult;
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        TraceChannel,
        QueryParams
    );
    
    if (bHit)
    {
        OutHitLocation = HitResult.Location;
        OutHitNormal = HitResult.Normal;
        
        // Draw debug trace if enabled
        if (IKSettings.bDrawDebugTraces)
        {
            DrawDebugLine(GetWorld(), TraceStart, HitResult.Location, FColor::Green, false, 0.1f, 0, 1.0f);
            DrawDebugLine(GetWorld(), HitResult.Location, TraceEnd, FColor::Red, false, 0.1f, 0, 1.0f);
            DrawDebugSphere(GetWorld(), HitResult.Location, 3.0f, 8, FColor::Yellow, false, 0.1f);
        }
    }
    else if (IKSettings.bDrawDebugTraces)
    {
        DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 0.1f, 0, 1.0f);
    }
    
    return bHit;
}

FRotator UAnim_IKFootPlacement::CalculateFootRotation(const FVector& GroundNormal, const FRotator& CurrentRotation)
{
    // Calculate rotation to align foot with ground normal
    FVector ForwardVector = CurrentRotation.Vector();
    FVector RightVector = FVector::CrossProduct(GroundNormal, ForwardVector).GetSafeNormal();
    ForwardVector = FVector::CrossProduct(RightVector, GroundNormal).GetSafeNormal();
    
    FRotator TargetRotation = UKismetMathLibrary::MakeRotationFromAxes(ForwardVector, RightVector, GroundNormal);
    
    // Limit the rotation angle
    FRotator DeltaRotation = TargetRotation - CurrentRotation;
    DeltaRotation.Pitch = FMath::Clamp(DeltaRotation.Pitch, -IKSettings.MaxFootAngle, IKSettings.MaxFootAngle);
    DeltaRotation.Roll = FMath::Clamp(DeltaRotation.Roll, -IKSettings.MaxFootAngle, IKSettings.MaxFootAngle);
    DeltaRotation.Yaw = 0.0f; // Don't rotate around Z axis
    
    return CurrentRotation + DeltaRotation;
}

void UAnim_IKFootPlacement::UpdatePelvisOffset(float DeltaTime)
{
    TargetPelvisOffset = CalculateOptimalPelvisOffset();
    PelvisOffset = FMath::FInterpTo(PelvisOffset, TargetPelvisOffset, DeltaTime, PelvisInterpSpeed);
}

float UAnim_IKFootPlacement::CalculateOptimalPelvisOffset()
{
    // Calculate pelvis offset based on foot IK adjustments
    float LeftFootOffset = LeftFootIKData.FootLocation.Z * LeftFootIKData.IKAlpha;
    float RightFootOffset = RightFootIKData.FootLocation.Z * RightFootIKData.IKAlpha;
    
    // Use the higher foot as reference (less negative value)
    float MaxOffset = FMath::Max(LeftFootOffset, RightFootOffset);
    
    // Only lower pelvis, never raise it
    return FMath::Min(MaxOffset * 0.5f, 0.0f);
}

void UAnim_IKFootPlacement::InterpolateFootIKData(FAnim_FootIKData& Current, const FAnim_FootIKData& Target, float DeltaTime)
{
    // Interpolate IK alpha
    Current.IKAlpha = FMath::FInterpTo(Current.IKAlpha, Target.IKAlpha, DeltaTime, IKSettings.InterpSpeed);
    
    // Interpolate foot location
    Current.FootLocation = FMath::VInterpTo(Current.FootLocation, Target.FootLocation, DeltaTime, IKSettings.InterpSpeed);
    
    // Interpolate foot rotation
    Current.FootRotation = FMath::RInterpTo(Current.FootRotation, Target.FootRotation, DeltaTime, IKSettings.InterpSpeed);
    
    // Update other data
    Current.DistanceFromGround = Target.DistanceFromGround;
    Current.GroundNormal = Target.GroundNormal;
}

bool UAnim_IKFootPlacement::IsCharacterMoving() const
{
    if (!OwnerCharacter)
    {
        return false;
    }
    
    return OwnerCharacter->GetVelocity().Size() > 10.0f;
}

void UAnim_IKFootPlacement::DrawDebugInfo() const
{
    if (!GetWorld() || !OwnerCharacter)
    {
        return;
    }
    
    FVector CharacterLocation = OwnerCharacter->GetActorLocation();
    
    // Draw pelvis offset
    if (FMath::Abs(PelvisOffset) > 0.1f)
    {
        DrawDebugSphere(GetWorld(), CharacterLocation + FVector(0, 0, PelvisOffset), 5.0f, 8, FColor::Blue, false, 0.1f);
    }
    
    // Draw foot IK info
    if (SkeletalMeshComponent)
    {
        if (LeftFootIKData.IKAlpha > 0.1f)
        {
            FVector LeftFootLoc = SkeletalMeshComponent->GetSocketLocation(LeftFootSocketName);
            DrawDebugSphere(GetWorld(), LeftFootLoc + LeftFootIKData.FootLocation, 3.0f, 8, FColor::Green, false, 0.1f);
        }
        
        if (RightFootIKData.IKAlpha > 0.1f)
        {
            FVector RightFootLoc = SkeletalMeshComponent->GetSocketLocation(RightFootSocketName);
            DrawDebugSphere(GetWorld(), RightFootLoc + RightFootIKData.FootLocation, 3.0f, 8, FColor::Green, false, 0.1f);
        }
    }
}