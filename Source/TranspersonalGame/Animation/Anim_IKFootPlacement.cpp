#include "Anim_IKFootPlacement.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UAnim_IKFootPlacement::UAnim_IKFootPlacement()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    OwnerCharacter = nullptr;
    CharacterMesh = nullptr;
    
    HipOffset = 0.0f;
    TargetHipOffset = 0.0f;
    CurrentHipOffset = 0.0f;
    
    LeftFootBoneName = TEXT("foot_l");
    RightFootBoneName = TEXT("foot_r");
    TraceChannel = ECC_Visibility;
}

void UAnim_IKFootPlacement::BeginPlay()
{
    Super::BeginPlay();
    
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        CharacterMesh = OwnerCharacter->GetMesh();
    }
}

void UAnim_IKFootPlacement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (IKSettings.bEnableIK && OwnerCharacter && CharacterMesh)
    {
        UpdateFootIK(DeltaTime);
    }
}

void UAnim_IKFootPlacement::UpdateFootIK(float DeltaTime)
{
    if (!OwnerCharacter || !CharacterMesh)
    {
        return;
    }
    
    // Update foot data for both feet
    UpdateFootData(LeftFootData, LeftFootBoneName, DeltaTime);
    UpdateFootData(RightFootData, RightFootBoneName, DeltaTime);
    
    // Calculate hip offset to keep character level
    CalculateHipOffset();
    
    // Interpolate hip offset smoothly
    CurrentHipOffset = FMath::FInterpTo(CurrentHipOffset, TargetHipOffset, DeltaTime, IKSettings.InterpSpeed);
    HipOffset = CurrentHipOffset;
}

void UAnim_IKFootPlacement::UpdateFootData(FAnim_FootData& FootData, const FName& BoneName, float DeltaTime)
{
    if (!CharacterMesh)
    {
        return;
    }
    
    // Get foot bone location in world space
    FVector FootBoneLocation = CharacterMesh->GetBoneLocation(BoneName, EBoneSpaces::WorldSpace);
    
    // Perform ground trace
    bool bHitGround = false;
    FVector GroundLocation = PerformFootTrace(FootBoneLocation, bHitGround);
    
    FootData.bIsGrounded = bHitGround;
    
    if (bHitGround)
    {
        // Calculate the offset needed
        float DistanceToGround = FootBoneLocation.Z - GroundLocation.Z;
        
        // Only apply IK if within reasonable distance
        if (FMath::Abs(DistanceToGround) <= IKSettings.MaxIKDistance)
        {
            // Target foot location with offset
            FVector TargetLocation = GroundLocation + FVector(0, 0, IKSettings.FootOffset);
            
            // Interpolate foot location
            FootData.FootLocation = FMath::VInterpTo(FootData.FootLocation, TargetLocation, DeltaTime, IKSettings.InterpSpeed);
            
            // Calculate foot rotation based on ground normal
            FHitResult HitResult;
            FVector TraceStart = FootBoneLocation + FVector(0, 0, IKSettings.TraceDistance);
            FVector TraceEnd = FootBoneLocation - FVector(0, 0, IKSettings.TraceDistance);
            
            if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, TraceChannel))
            {
                FVector GroundNormal = HitResult.Normal;
                FRotator TargetRotation = UKismetMathLibrary::MakeRotFromZX(GroundNormal, OwnerCharacter->GetActorForwardVector());
                FootData.FootRotation = FMath::RInterpTo(FootData.FootRotation, TargetRotation, DeltaTime, IKSettings.InterpSpeed);
            }
            
            // Calculate IK alpha based on distance
            float Alpha = 1.0f - (FMath::Abs(DistanceToGround) / IKSettings.MaxIKDistance);
            FootData.IKAlpha = FMath::FInterpTo(FootData.IKAlpha, Alpha, DeltaTime, IKSettings.InterpSpeed * 2.0f);
        }
        else
        {
            // Too far from ground, disable IK
            FootData.IKAlpha = FMath::FInterpTo(FootData.IKAlpha, 0.0f, DeltaTime, IKSettings.InterpSpeed * 2.0f);
        }
    }
    else
    {
        // No ground hit, disable IK
        FootData.IKAlpha = FMath::FInterpTo(FootData.IKAlpha, 0.0f, DeltaTime, IKSettings.InterpSpeed * 2.0f);
    }
}

FVector UAnim_IKFootPlacement::PerformFootTrace(const FVector& FootLocation, bool& bHitGround)
{
    if (!GetWorld())
    {
        bHitGround = false;
        return FVector::ZeroVector;
    }
    
    FHitResult HitResult;
    FVector TraceStart = FootLocation + FVector(0, 0, IKSettings.TraceDistance);
    FVector TraceEnd = FootLocation - FVector(0, 0, IKSettings.TraceDistance);
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    QueryParams.bTraceComplex = false;
    
    bHitGround = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        TraceChannel,
        QueryParams
    );
    
    // Debug visualization (only in development builds)
    #if WITH_EDITOR
    if (IKSettings.bEnableIK)
    {
        DrawDebugLine(GetWorld(), TraceStart, TraceEnd, bHitGround ? FColor::Green : FColor::Red, false, 0.1f, 0, 1.0f);
        if (bHitGround)
        {
            DrawDebugSphere(GetWorld(), HitResult.Location, 2.0f, 8, FColor::Yellow, false, 0.1f);
        }
    }
    #endif
    
    return bHitGround ? HitResult.Location : FootLocation;
}

void UAnim_IKFootPlacement::CalculateHipOffset()
{
    // Calculate the lowest foot position to adjust hip
    float LeftFootOffset = LeftFootData.bIsGrounded ? (LeftFootData.FootLocation.Z - CharacterMesh->GetBoneLocation(LeftFootBoneName, EBoneSpaces::WorldSpace).Z) : 0.0f;
    float RightFootOffset = RightFootData.bIsGrounded ? (RightFootData.FootLocation.Z - CharacterMesh->GetBoneLocation(RightFootBoneName, EBoneSpaces::WorldSpace).Z) : 0.0f;
    
    // Use the smaller (more negative) offset to keep both feet on ground
    TargetHipOffset = FMath::Min(LeftFootOffset, RightFootOffset);
    
    // Clamp the hip offset to reasonable values
    TargetHipOffset = FMath::Clamp(TargetHipOffset, -IKSettings.MaxIKDistance, IKSettings.MaxIKDistance);
}

void UAnim_IKFootPlacement::SetIKEnabled(bool bEnabled)
{
    IKSettings.bEnableIK = bEnabled;
    
    if (!bEnabled)
    {
        // Reset all IK data when disabled
        LeftFootData.IKAlpha = 0.0f;
        RightFootData.IKAlpha = 0.0f;
        TargetHipOffset = 0.0f;
    }
}