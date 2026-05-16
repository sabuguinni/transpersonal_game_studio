#include "Anim_IKFootPlacement.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/CapsuleComponent.h"

UAnim_IKFootPlacement::UAnim_IKFootPlacement()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    // Initialize IK settings
    IKSettings.TraceDistance = 50.0f;
    IKSettings.InterpSpeed = 15.0f;
    IKSettings.FootOffset = 5.0f;
    IKSettings.MaxIKAdjustment = 30.0f;
    IKSettings.bEnableFootRotation = true;
    IKSettings.bEnableHipAdjustment = true;

    // Initialize bone names (standard UE5 skeleton)
    LeftFootBoneName = TEXT("foot_l");
    RightFootBoneName = TEXT("foot_r");
    HipBoneName = TEXT("pelvis");

    // Initialize state
    bIKEnabled = true;
    bShowDebugTraces = false;
    HipOffset = 0.0f;
    TargetHipOffset = 0.0f;
    PreviousHipOffset = 0.0f;

    // Initialize cached references
    OwnerCharacter = nullptr;
    MeshComponent = nullptr;
}

void UAnim_IKFootPlacement::BeginPlay()
{
    Super::BeginPlay();
    CacheComponentReferences();
}

void UAnim_IKFootPlacement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIKEnabled && OwnerCharacter && MeshComponent)
    {
        UpdateFootIK(DeltaTime);
        
        if (bShowDebugTraces)
        {
            DrawDebugInfo();
        }
    }
}

void UAnim_IKFootPlacement::UpdateFootIK(float DeltaTime)
{
    if (!MeshComponent || !OwnerCharacter)
    {
        return;
    }

    // Update foot IK data
    LeftFootIK = CalculateFootIK(LeftFootBoneName, PreviousLeftFootIK, DeltaTime);
    RightFootIK = CalculateFootIK(RightFootBoneName, PreviousRightFootIK, DeltaTime);

    // Update hip adjustment
    if (IKSettings.bEnableHipAdjustment)
    {
        UpdateHipAdjustment(DeltaTime);
    }

    // Store previous values for next frame
    PreviousLeftFootIK = LeftFootIK;
    PreviousRightFootIK = RightFootIK;
    PreviousHipOffset = HipOffset;
}

FAnim_FootIKData UAnim_IKFootPlacement::CalculateFootIK(const FName& BoneName, const FAnim_FootIKData& PreviousData, float DeltaTime)
{
    FAnim_FootIKData NewIKData = PreviousData;

    // Get current foot bone world location
    FVector FootBoneLocation = GetBoneWorldLocation(BoneName);
    if (FootBoneLocation.IsZero())
    {
        return NewIKData;
    }

    // Perform ground trace from foot location
    FVector HitLocation;
    FVector HitNormal;
    bool bHitGround = PerformGroundTrace(FootBoneLocation, HitLocation, HitNormal);

    if (bHitGround)
    {
        // Calculate distance from ground
        float DistanceFromGround = FootBoneLocation.Z - HitLocation.Z;
        NewIKData.DistanceFromGround = DistanceFromGround;
        NewIKData.bIsGrounded = FMath::Abs(DistanceFromGround) <= IKSettings.MaxIKAdjustment;

        if (NewIKData.bIsGrounded)
        {
            // Calculate target foot location with offset
            FVector TargetFootLocation = HitLocation + (HitNormal * IKSettings.FootOffset);
            
            // Interpolate foot location
            NewIKData.FootLocation = FMath::VInterpTo(
                PreviousData.FootLocation,
                TargetFootLocation,
                DeltaTime,
                IKSettings.InterpSpeed
            );

            // Calculate foot rotation from ground normal
            if (IKSettings.bEnableFootRotation)
            {
                FRotator TargetFootRotation = CalculateFootRotationFromNormal(HitNormal);
                NewIKData.FootRotation = FMath::RInterpTo(
                    PreviousData.FootRotation,
                    TargetFootRotation,
                    DeltaTime,
                    IKSettings.InterpSpeed
                );
            }

            // Calculate IK alpha based on distance and settings
            float IKDistance = FMath::Abs(DistanceFromGround);
            NewIKData.IKAlpha = FMath::Clamp(IKDistance / IKSettings.MaxIKAdjustment, 0.0f, 1.0f);
        }
        else
        {
            // Gradually disable IK when too far from ground
            NewIKData.IKAlpha = FMath::FInterpTo(
                PreviousData.IKAlpha,
                0.0f,
                DeltaTime,
                IKSettings.InterpSpeed
            );
        }
    }
    else
    {
        // No ground hit, gradually disable IK
        NewIKData.bIsGrounded = false;
        NewIKData.IKAlpha = FMath::FInterpTo(
            PreviousData.IKAlpha,
            0.0f,
            DeltaTime,
            IKSettings.InterpSpeed
        );
    }

    return NewIKData;
}

void UAnim_IKFootPlacement::UpdateHipAdjustment(float DeltaTime)
{
    // Calculate target hip offset based on foot IK data
    float LeftFootAdjustment = LeftFootIK.bIsGrounded ? LeftFootIK.DistanceFromGround : 0.0f;
    float RightFootAdjustment = RightFootIK.bIsGrounded ? RightFootIK.DistanceFromGround : 0.0f;

    // Use the smaller adjustment to keep both feet grounded
    TargetHipOffset = FMath::Min(LeftFootAdjustment, RightFootAdjustment);
    TargetHipOffset = FMath::Clamp(TargetHipOffset, -IKSettings.MaxIKAdjustment, 0.0f);

    // Interpolate hip offset
    HipOffset = FMath::FInterpTo(
        PreviousHipOffset,
        TargetHipOffset,
        DeltaTime,
        IKSettings.InterpSpeed
    );
}

bool UAnim_IKFootPlacement::PerformGroundTrace(const FVector& StartLocation, FVector& HitLocation, FVector& HitNormal) const
{
    if (!OwnerCharacter || !GetWorld())
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
        ECollisionChannel::ECC_WorldStatic,
        QueryParams
    );

    if (bHit)
    {
        HitLocation = HitResult.Location;
        HitNormal = HitResult.Normal;
        return true;
    }

    return false;
}

FRotator UAnim_IKFootPlacement::CalculateFootRotationFromNormal(const FVector& GroundNormal) const
{
    if (!OwnerCharacter)
    {
        return FRotator::ZeroRotator;
    }

    // Get character's forward and right vectors
    FVector CharacterForward = OwnerCharacter->GetActorForwardVector();
    FVector CharacterRight = OwnerCharacter->GetActorRightVector();

    // Calculate foot forward vector projected onto the ground plane
    FVector FootForward = CharacterForward - (FVector::DotProduct(CharacterForward, GroundNormal) * GroundNormal);
    FootForward.Normalize();

    // Calculate foot right vector
    FVector FootRight = FVector::CrossProduct(GroundNormal, FootForward);
    FootRight.Normalize();

    // Create rotation matrix and convert to rotator
    FMatrix RotationMatrix = FMatrix(FootForward, FootRight, GroundNormal, FVector::ZeroVector);
    return RotationMatrix.Rotator();
}

FVector UAnim_IKFootPlacement::GetBoneWorldLocation(const FName& BoneName) const
{
    if (!MeshComponent)
    {
        return FVector::ZeroVector;
    }

    int32 BoneIndex = MeshComponent->GetBoneIndex(BoneName);
    if (BoneIndex == INDEX_NONE)
    {
        return FVector::ZeroVector;
    }

    return MeshComponent->GetBoneLocation(BoneName, EBoneSpaces::WorldSpace);
}

void UAnim_IKFootPlacement::CacheComponentReferences()
{
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MeshComponent = OwnerCharacter->GetMesh();
    }
}

void UAnim_IKFootPlacement::DrawDebugInfo() const
{
    if (!GetWorld() || !OwnerCharacter)
    {
        return;
    }

    // Draw foot bone locations
    FVector LeftFootLocation = GetBoneWorldLocation(LeftFootBoneName);
    FVector RightFootLocation = GetBoneWorldLocation(RightFootBoneName);

    if (!LeftFootLocation.IsZero())
    {
        DrawDebugSphere(GetWorld(), LeftFootLocation, 5.0f, 12, FColor::Blue, false, 0.0f);
        
        // Draw trace line for left foot
        FVector TraceStart = LeftFootLocation + FVector(0.0f, 0.0f, IKSettings.TraceDistance * 0.5f);
        FVector TraceEnd = LeftFootLocation - FVector(0.0f, 0.0f, IKSettings.TraceDistance);
        DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Green, false, 0.0f);
    }

    if (!RightFootLocation.IsZero())
    {
        DrawDebugSphere(GetWorld(), RightFootLocation, 5.0f, 12, FColor::Red, false, 0.0f);
        
        // Draw trace line for right foot
        FVector TraceStart = RightFootLocation + FVector(0.0f, 0.0f, IKSettings.TraceDistance * 0.5f);
        FVector TraceEnd = RightFootLocation - FVector(0.0f, 0.0f, IKSettings.TraceDistance);
        DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Orange, false, 0.0f);
    }

    // Draw IK target locations
    if (LeftFootIK.bIsGrounded)
    {
        DrawDebugSphere(GetWorld(), LeftFootIK.FootLocation, 3.0f, 8, FColor::Cyan, false, 0.0f);
    }

    if (RightFootIK.bIsGrounded)
    {
        DrawDebugSphere(GetWorld(), RightFootIK.FootLocation, 3.0f, 8, FColor::Magenta, false, 0.0f);
    }
}

// Getter functions
FAnim_FootIKData UAnim_IKFootPlacement::GetLeftFootIKData() const
{
    return LeftFootIK;
}

FAnim_FootIKData UAnim_IKFootPlacement::GetRightFootIKData() const
{
    return RightFootIK;
}

float UAnim_IKFootPlacement::GetHipOffset() const
{
    return HipOffset;
}

void UAnim_IKFootPlacement::SetIKEnabled(bool bEnabled)
{
    bIKEnabled = bEnabled;
}

bool UAnim_IKFootPlacement::IsIKEnabled() const
{
    return bIKEnabled;
}

// Terrain adaptation functions
FVector UAnim_IKFootPlacement::GetGroundNormal(const FVector& Location) const
{
    FVector HitLocation;
    FVector HitNormal;
    
    if (PerformGroundTrace(Location, HitLocation, HitNormal))
    {
        return HitNormal;
    }
    
    return FVector::UpVector;
}

float UAnim_IKFootPlacement::GetGroundHeight(const FVector& Location) const
{
    FVector HitLocation;
    FVector HitNormal;
    
    if (PerformGroundTrace(Location, HitLocation, HitNormal))
    {
        return HitLocation.Z;
    }
    
    return Location.Z;
}

bool UAnim_IKFootPlacement::IsOnValidGround(const FVector& Location) const
{
    FVector HitLocation;
    FVector HitNormal;
    return PerformGroundTrace(Location, HitLocation, HitNormal);
}