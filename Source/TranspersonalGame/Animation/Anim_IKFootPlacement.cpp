#include "Anim_IKFootPlacement.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_IKFootPlacement::UAnim_IKFootPlacement()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize default values
    bIKEnabled = true;
    LastUpdateTime = 0.0f;
    PelvisOffset = 0.0f;
    PreviousPelvisOffset = 0.0f;
    
    // Set default IK settings
    IKSettings = FAnim_IKSettings();
}

void UAnim_IKFootPlacement::BeginPlay()
{
    Super::BeginPlay();
    
    CacheComponents();
    InitializeIKData();
}

void UAnim_IKFootPlacement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIKEnabled && ShouldUpdateIK())
    {
        UpdateFootIK(DeltaTime);
        
        if (IKSettings.bEnableDebugDraw)
        {
            DrawDebugIK();
        }
    }
}

void UAnim_IKFootPlacement::CacheComponents()
{
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MeshComponent = OwnerCharacter->GetMesh();
        MovementComponent = OwnerCharacter->GetCharacterMovement();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UAnim_IKFootPlacement: Owner is not a Character"));
    }
}

void UAnim_IKFootPlacement::InitializeIKData()
{
    LeftFootIK = FAnim_FootIKData();
    RightFootIK = FAnim_FootIKData();
    PreviousLeftFootIK = FAnim_FootIKData();
    PreviousRightFootIK = FAnim_FootIKData();
    PelvisOffset = 0.0f;
    PreviousPelvisOffset = 0.0f;
}

void UAnim_IKFootPlacement::UpdateFootIK(float DeltaTime)
{
    if (!OwnerCharacter || !MeshComponent)
    {
        return;
    }
    
    // Store previous frame data
    PreviousLeftFootIK = LeftFootIK;
    PreviousRightFootIK = RightFootIK;
    PreviousPelvisOffset = PelvisOffset;
    
    // Calculate new IK data for both feet
    LeftFootIK = CalculateFootIK(IKSettings.LeftFootBoneName, DeltaTime);
    RightFootIK = CalculateFootIK(IKSettings.RightFootBoneName, DeltaTime);
    
    // Calculate pelvis offset to keep character grounded
    PelvisOffset = CalculatePelvisOffset();
    
    // Interpolate for smooth transitions
    LeftFootIK.FootLocation = InterpolateFootLocation(PreviousLeftFootIK.FootLocation, LeftFootIK.FootLocation, DeltaTime);
    LeftFootIK.FootRotation = InterpolateFootRotation(PreviousLeftFootIK.FootRotation, LeftFootIK.FootRotation, DeltaTime);
    
    RightFootIK.FootLocation = InterpolateFootLocation(PreviousRightFootIK.FootLocation, RightFootIK.FootLocation, DeltaTime);
    RightFootIK.FootRotation = InterpolateFootRotation(PreviousRightFootIK.FootRotation, RightFootIK.FootRotation, DeltaTime);
    
    PelvisOffset = FMath::FInterpTo(PreviousPelvisOffset, PelvisOffset, DeltaTime, IKSettings.IKInterpSpeed);
    
    // Trigger Blueprint event
    OnIKUpdated();
}

FAnim_FootIKData UAnim_IKFootPlacement::CalculateFootIK(FName FootBoneName, float DeltaTime)
{
    FAnim_FootIKData FootData;
    
    if (!MeshComponent)
    {
        return FootData;
    }
    
    // Get foot location in world space
    FVector FootLocation = GetFootLocation(FootBoneName);
    
    // Perform ground trace
    FVector HitLocation;
    FVector HitNormal;
    bool bHitGround = TraceForGround(FootLocation, HitLocation, HitNormal);
    
    if (bHitGround)
    {
        // Calculate foot position adjustment
        float DistanceToGround = FootLocation.Z - HitLocation.Z;
        float TargetOffset = DistanceToGround - IKSettings.FootOffset;
        
        // Clamp to maximum leg extension
        TargetOffset = FMath::Clamp(TargetOffset, -IKSettings.MaxLegExtension, IKSettings.MaxLegExtension);
        
        FootData.EffectorLocation = -TargetOffset;
        FootData.FootLocation = FVector(0.0f, 0.0f, FootData.EffectorLocation);
        
        // Calculate foot rotation to match ground normal
        FVector FootForward = MeshComponent->GetForwardVector();
        FootData.FootRotation = CalculateFootRotation(HitNormal, FootForward);
        
        // Set IK alpha based on ground distance
        float MaxDistance = IKSettings.TraceDistance;
        FootData.IKAlpha = FMath::Clamp(1.0f - (FMath::Abs(DistanceToGround) / MaxDistance), 0.0f, 1.0f);
        
        // Calculate joint target for knee/elbow positioning
        FootData.JointTarget = HitLocation + (HitNormal * IKSettings.FootOffset);
        
        // Trigger ground hit event
        OnGroundHit(HitLocation, HitNormal);
    }
    else
    {
        // No ground found, disable IK for this foot
        FootData.IKAlpha = 0.0f;
    }
    
    return FootData;
}

float UAnim_IKFootPlacement::CalculatePelvisOffset()
{
    if (!OwnerCharacter)
    {
        return 0.0f;
    }
    
    // Get the lowest foot position to adjust pelvis
    float LeftFootOffset = LeftFootIK.EffectorLocation;
    float RightFootOffset = RightFootIK.EffectorLocation;
    
    // Use the foot that's furthest down (most negative offset)
    float LowestOffset = FMath::Min(LeftFootOffset, RightFootOffset);
    
    // Only adjust pelvis if one foot is significantly lower
    float PelvisAdjustment = 0.0f;
    if (LowestOffset < -5.0f) // Only adjust if difference is significant
    {
        PelvisAdjustment = LowestOffset * 0.5f; // Adjust pelvis by half the foot offset
    }
    
    return PelvisAdjustment;
}

FVector UAnim_IKFootPlacement::GetFootLocation(FName FootBoneName) const
{
    if (!MeshComponent)
    {
        return FVector::ZeroVector;
    }
    
    // Get bone location in world space
    FVector BoneLocation = MeshComponent->GetBoneLocation(FootBoneName, EBoneSpaces::WorldSpace);
    
    return BoneLocation;
}

bool UAnim_IKFootPlacement::TraceForGround(const FVector& StartLocation, FVector& OutHitLocation, FVector& OutHitNormal) const
{
    if (!OwnerCharacter || !GetWorld())
    {
        return false;
    }
    
    // Setup trace parameters
    FVector TraceStart = StartLocation + FVector(0.0f, 0.0f, IKSettings.TraceDistance * 0.5f);
    FVector TraceEnd = StartLocation - FVector(0.0f, 0.0f, IKSettings.TraceDistance);
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    QueryParams.bTraceComplex = true;
    
    FHitResult HitResult;
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_Visibility,
        QueryParams
    );
    
    if (bHit)
    {
        OutHitLocation = HitResult.Location;
        OutHitNormal = HitResult.Normal;
        return true;
    }
    
    return false;
}

FRotator UAnim_IKFootPlacement::CalculateFootRotation(const FVector& HitNormal, const FVector& FootForward) const
{
    // Calculate foot rotation to align with ground normal
    FVector GroundRight = FVector::CrossProduct(HitNormal, FootForward).GetSafeNormal();
    FVector GroundForward = FVector::CrossProduct(GroundRight, HitNormal).GetSafeNormal();
    
    // Create rotation matrix from ground-aligned vectors
    FMatrix RotationMatrix = FMatrix(GroundForward, GroundRight, HitNormal, FVector::ZeroVector);
    FRotator GroundRotation = RotationMatrix.Rotator();
    
    // Convert to relative rotation
    FRotator CurrentRotation = MeshComponent ? MeshComponent->GetComponentRotation() : FRotator::ZeroRotator;
    FRotator RelativeRotation = GroundRotation - CurrentRotation;
    
    // Limit rotation angles to prevent extreme poses
    RelativeRotation.Pitch = FMath::Clamp(RelativeRotation.Pitch, -45.0f, 45.0f);
    RelativeRotation.Roll = FMath::Clamp(RelativeRotation.Roll, -45.0f, 45.0f);
    RelativeRotation.Yaw = 0.0f; // Don't rotate around Z axis
    
    return RelativeRotation;
}

void UAnim_IKFootPlacement::SetIKEnabled(bool bEnabled)
{
    bIKEnabled = bEnabled;
    
    if (!bEnabled)
    {
        // Reset IK data when disabled
        InitializeIKData();
    }
}

FVector UAnim_IKFootPlacement::InterpolateFootLocation(const FVector& CurrentLocation, const FVector& TargetLocation, float DeltaTime) const
{
    return FMath::VInterpTo(CurrentLocation, TargetLocation, DeltaTime, IKSettings.IKInterpSpeed);
}

FRotator UAnim_IKFootPlacement::InterpolateFootRotation(const FRotator& CurrentRotation, const FRotator& TargetRotation, float DeltaTime) const
{
    return FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, IKSettings.IKInterpSpeed);
}

void UAnim_IKFootPlacement::DrawDebugIK() const
{
    if (!GetWorld() || !OwnerCharacter)
    {
        return;
    }
    
    // Draw foot IK debug info
    FVector LeftFootLoc = GetFootLocation(IKSettings.LeftFootBoneName);
    FVector RightFootLoc = GetFootLocation(IKSettings.RightFootBoneName);
    
    // Draw foot positions
    DrawDebugSphere(GetWorld(), LeftFootLoc, 5.0f, 12, FColor::Red, false, 0.1f);
    DrawDebugSphere(GetWorld(), RightFootLoc, 5.0f, 12, FColor::Blue, false, 0.1f);
    
    // Draw IK targets
    if (LeftFootIK.IKAlpha > 0.0f)
    {
        FVector LeftTarget = LeftFootLoc + LeftFootIK.FootLocation;
        DrawDebugSphere(GetWorld(), LeftTarget, 3.0f, 8, FColor::Green, false, 0.1f);
        DrawDebugLine(GetWorld(), LeftFootLoc, LeftTarget, FColor::Green, false, 0.1f);
    }
    
    if (RightFootIK.IKAlpha > 0.0f)
    {
        FVector RightTarget = RightFootLoc + RightFootIK.FootLocation;
        DrawDebugSphere(GetWorld(), RightTarget, 3.0f, 8, FColor::Cyan, false, 0.1f);
        DrawDebugLine(GetWorld(), RightFootLoc, RightTarget, FColor::Cyan, false, 0.1f);
    }
    
    // Draw pelvis offset
    if (FMath::Abs(PelvisOffset) > 0.1f)
    {
        FVector PelvisLoc = MeshComponent ? MeshComponent->GetBoneLocation(IKSettings.PelvisBoneName, EBoneSpaces::WorldSpace) : FVector::ZeroVector;
        FVector PelvisTarget = PelvisLoc + FVector(0.0f, 0.0f, PelvisOffset);
        DrawDebugSphere(GetWorld(), PelvisTarget, 8.0f, 12, FColor::Yellow, false, 0.1f);
        DrawDebugLine(GetWorld(), PelvisLoc, PelvisTarget, FColor::Yellow, false, 0.1f);
    }
}

bool UAnim_IKFootPlacement::IsCharacterMoving() const
{
    if (!MovementComponent)
    {
        return false;
    }
    
    return MovementComponent->GetLastUpdateVelocity().Size() > 1.0f;
}

bool UAnim_IKFootPlacement::ShouldUpdateIK() const
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return false;
    }
    
    // Don't update IK if character is in air
    if (MovementComponent->IsFalling())
    {
        return false;
    }
    
    // Don't update IK if character is moving too fast
    float Speed = MovementComponent->GetLastUpdateVelocity().Size();
    if (Speed > 800.0f) // Running speed threshold
    {
        return false;
    }
    
    return true;
}