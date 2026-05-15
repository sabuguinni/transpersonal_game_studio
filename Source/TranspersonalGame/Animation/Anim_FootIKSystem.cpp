#include "Anim_FootIKSystem.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_FootIKSystem::UAnim_FootIKSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Set default values
    bFootIKEnabled = true;
    TraceDistance = 100.0f;
    IKInterpSpeed = 15.0f;
    HipOffsetInterpSpeed = 10.0f;
    MaxHipOffset = 50.0f;
    
    // Default bone names (standard UE5 skeleton)
    LeftFootBoneName = TEXT("foot_l");
    RightFootBoneName = TEXT("foot_r");
    HipBoneName = TEXT("pelvis");
    
    // Initialize component references
    OwningCharacter = nullptr;
    SkeletalMeshComponent = nullptr;
    
    // Initialize IK data
    FootIKData = FAnim_FootIKData();
}

void UAnim_FootIKSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Get component references
    OwningCharacter = Cast<ACharacter>(GetOwner());
    if (OwningCharacter)
    {
        SkeletalMeshComponent = OwningCharacter->GetMesh();
        
        if (SkeletalMeshComponent)
        {
            UE_LOG(LogTemp, Warning, TEXT("Foot IK System initialized for character: %s"), 
                   *OwningCharacter->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("No skeletal mesh component found on character: %s"), 
                   *OwningCharacter->GetName());
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Foot IK System must be attached to a Character"));
    }
}

void UAnim_FootIKSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bFootIKEnabled && OwningCharacter && SkeletalMeshComponent)
    {
        UpdateFootIK(DeltaTime);
    }
}

void UAnim_FootIKSystem::SetFootIKEnabled(bool bEnabled)
{
    bFootIKEnabled = bEnabled;
    
    if (!bEnabled)
    {
        // Reset IK data when disabled
        FootIKData = FAnim_FootIKData();
    }
}

void UAnim_FootIKSystem::UpdateFootIK(float DeltaTime)
{
    if (!OwningCharacter || !SkeletalMeshComponent)
    {
        return;
    }
    
    // Only perform IK when character is on ground and not moving too fast
    UCharacterMovementComponent* MovementComp = OwningCharacter->GetCharacterMovement();
    if (!MovementComp || MovementComp->IsFalling())
    {
        // Gradually disable IK when in air
        FootIKData.LeftFootIKAlpha = FMath::FInterpTo(FootIKData.LeftFootIKAlpha, 0.0f, DeltaTime, IKInterpSpeed);
        FootIKData.RightFootIKAlpha = FMath::FInterpTo(FootIKData.RightFootIKAlpha, 0.0f, DeltaTime, IKInterpSpeed);
        FootIKData.HipOffset = FMath::FInterpTo(FootIKData.HipOffset, 0.0f, DeltaTime, HipOffsetInterpSpeed);
        return;
    }
    
    // Get current foot bone locations
    FVector LeftFootLocation = GetBoneWorldLocation(LeftFootBoneName);
    FVector RightFootLocation = GetBoneWorldLocation(RightFootBoneName);
    
    if (LeftFootLocation.IsZero() || RightFootLocation.IsZero())
    {
        UE_LOG(LogTemp, Warning, TEXT("Could not get foot bone locations for IK"));
        return;
    }
    
    // Perform ground traces for each foot
    bool bLeftFootHitGround, bRightFootHitGround;
    FRotator LeftGroundRotation, RightGroundRotation;
    
    FVector LeftFootTarget = PerformFootTrace(LeftFootLocation, bLeftFootHitGround, LeftGroundRotation);
    FVector RightFootTarget = PerformFootTrace(RightFootLocation, bRightFootHitGround, RightGroundRotation);
    
    // Calculate hip offset to keep character balanced
    float TargetHipOffset = CalculateHipOffset(LeftFootTarget, RightFootTarget);
    
    // Interpolate IK values
    FootIKData.LeftFootLocation = FMath::VInterpTo(FootIKData.LeftFootLocation, LeftFootTarget, DeltaTime, IKInterpSpeed);
    FootIKData.RightFootLocation = FMath::VInterpTo(FootIKData.RightFootLocation, RightFootTarget, DeltaTime, IKInterpSpeed);
    
    FootIKData.LeftFootRotation = FMath::RInterpTo(FootIKData.LeftFootRotation, LeftGroundRotation, DeltaTime, IKInterpSpeed);
    FootIKData.RightFootRotation = FMath::RInterpTo(FootIKData.RightFootRotation, RightGroundRotation, DeltaTime, IKInterpSpeed);
    
    FootIKData.HipOffset = FMath::FInterpTo(FootIKData.HipOffset, TargetHipOffset, DeltaTime, HipOffsetInterpSpeed);
    
    // Update IK alphas based on ground contact
    FootIKData.LeftFootIKAlpha = FMath::FInterpTo(FootIKData.LeftFootIKAlpha, bLeftFootHitGround ? 1.0f : 0.0f, DeltaTime, IKInterpSpeed);
    FootIKData.RightFootIKAlpha = FMath::FInterpTo(FootIKData.RightFootIKAlpha, bRightFootHitGround ? 1.0f : 0.0f, DeltaTime, IKInterpSpeed);
}

FVector UAnim_FootIKSystem::PerformFootTrace(const FVector& FootLocation, bool& bHitGround, FRotator& GroundRotation)
{
    if (!OwningCharacter)
    {
        bHitGround = false;
        GroundRotation = FRotator::ZeroRotator;
        return FootLocation;
    }
    
    // Setup trace parameters
    FVector TraceStart = FootLocation + FVector(0, 0, 50.0f);
    FVector TraceEnd = FootLocation - FVector(0, 0, TraceDistance);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwningCharacter);
    
    // Perform line trace
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );
    
    if (bHit)
    {
        bHitGround = true;
        
        // Calculate foot rotation from surface normal
        GroundRotation = CalculateFootRotationFromNormal(HitResult.Normal);
        
        return HitResult.Location;
    }
    else
    {
        bHitGround = false;
        GroundRotation = FRotator::ZeroRotator;
        return FootLocation;
    }
}

float UAnim_FootIKSystem::CalculateHipOffset(const FVector& LeftFootTarget, const FVector& RightFootTarget)
{
    if (!OwningCharacter)
    {
        return 0.0f;
    }
    
    // Get character's current location
    FVector CharacterLocation = OwningCharacter->GetActorLocation();
    
    // Calculate the lowest foot position
    float LowestFootZ = FMath::Min(LeftFootTarget.Z, RightFootTarget.Z);
    
    // Calculate desired hip offset to keep character grounded
    float DesiredOffset = LowestFootZ - CharacterLocation.Z;
    
    // Clamp to maximum allowed offset
    return FMath::Clamp(DesiredOffset, -MaxHipOffset, MaxHipOffset);
}

FRotator UAnim_FootIKSystem::CalculateFootRotationFromNormal(const FVector& Normal)
{
    // Calculate rotation to align foot with surface normal
    FVector ForwardVector = FVector::ForwardVector;
    FVector RightVector = FVector::CrossProduct(Normal, ForwardVector).GetSafeNormal();
    ForwardVector = FVector::CrossProduct(RightVector, Normal).GetSafeNormal();
    
    return UKismetMathLibrary::MakeRotationFromAxes(ForwardVector, RightVector, Normal);
}

FVector UAnim_FootIKSystem::GetBoneWorldLocation(const FName& BoneName) const
{
    if (!SkeletalMeshComponent)
    {
        return FVector::ZeroVector;
    }
    
    int32 BoneIndex = SkeletalMeshComponent->GetBoneIndex(BoneName);
    if (BoneIndex != INDEX_NONE)
    {
        return SkeletalMeshComponent->GetBoneLocation(BoneName, EBoneSpaces::WorldSpace);
    }
    
    return FVector::ZeroVector;
}

bool UAnim_FootIKSystem::IsCharacterMoving() const
{
    if (!OwningCharacter)
    {
        return false;
    }
    
    UCharacterMovementComponent* MovementComp = OwningCharacter->GetCharacterMovement();
    if (MovementComp)
    {
        return MovementComp->Velocity.Size2D() > 10.0f;
    }
    
    return false;
}