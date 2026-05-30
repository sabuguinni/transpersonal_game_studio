#include "Anim_IKFootPlacement.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "CollisionQueryParams.h"

UAnim_IKFootPlacement::UAnim_IKFootPlacement()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize bone names with common defaults
    LeftFootBoneName = FName("foot_l");
    RightFootBoneName = FName("foot_r");
    PelvisBoneName = FName("pelvis");
    
    // Initialize cached values
    LastLeftFootLocation = FVector::ZeroVector;
    LastRightFootLocation = FVector::ZeroVector;
    LastLeftFootRotation = FRotator::ZeroRotator;
    LastRightFootRotation = FRotator::ZeroRotator;
    PelvisOffset = 0.0f;
    LastPelvisOffset = 0.0f;
    
    OwnerCharacter = nullptr;
    CharacterMesh = nullptr;
}

void UAnim_IKFootPlacement::BeginPlay()
{
    Super::BeginPlay();
    
    // Get character reference
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        CharacterMesh = OwnerCharacter->GetMesh();
        
        if (CharacterMesh)
        {
            // Initialize cached positions with current foot locations
            if (CharacterMesh->GetBoneIndex(LeftFootBoneName) != INDEX_NONE)
            {
                LastLeftFootLocation = CharacterMesh->GetBoneLocation(LeftFootBoneName);
            }
            
            if (CharacterMesh->GetBoneIndex(RightFootBoneName) != INDEX_NONE)
            {
                LastRightFootLocation = CharacterMesh->GetBoneLocation(RightFootBoneName);
            }
            
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
                    TEXT("IK Foot Placement component initialized"));
            }
        }
    }
}

void UAnim_IKFootPlacement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (IKSettings.bEnableFootIK && OwnerCharacter && CharacterMesh)
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
    
    // Calculate IK for both feet
    LeftFootIK = CalculateFootIK(LeftFootBoneName, DeltaTime);
    RightFootIK = CalculateFootIK(RightFootBoneName, DeltaTime);
    
    // Calculate pelvis offset to keep character grounded
    float LeftDistance = LeftFootIK.DistanceFromGround;
    float RightDistance = RightFootIK.DistanceFromGround;
    
    // Use the higher foot to determine pelvis adjustment
    float TargetPelvisOffset = FMath::Max(LeftDistance, RightDistance);
    TargetPelvisOffset = FMath::Clamp(TargetPelvisOffset, -IKSettings.TraceDistance, IKSettings.TraceDistance);
    
    // Smooth pelvis offset interpolation
    PelvisOffset = FMath::FInterpTo(LastPelvisOffset, TargetPelvisOffset, DeltaTime, IKSettings.IKInterpSpeed);
    LastPelvisOffset = PelvisOffset;
}

FAnim_FootIKData UAnim_IKFootPlacement::CalculateFootIK(const FName& FootBoneName, float DeltaTime)
{
    FAnim_FootIKData FootData;
    
    if (!CharacterMesh || CharacterMesh->GetBoneIndex(FootBoneName) == INDEX_NONE)
    {
        return FootData;
    }
    
    // Get current foot location in world space
    FVector FootLocation = CharacterMesh->GetBoneLocation(FootBoneName);
    
    // Perform ground trace
    float DistanceFromGround;
    FVector ImpactLocation = PerformFootTrace(FootLocation, DistanceFromGround);
    
    // Calculate IK alpha based on distance
    FootData.IKAlpha = CalculateIKAlpha(DistanceFromGround);
    FootData.DistanceFromGround = DistanceFromGround;
    
    // Set foot location with offset
    FootData.FootLocation = ImpactLocation + FVector(0, 0, IKSettings.FootOffset);
    
    // Calculate foot rotation if enabled
    if (IKSettings.bEnableFootRotation)
    {
        FVector ImpactNormal = FVector::UpVector; // Default up vector
        
        // Perform another trace to get surface normal
        FHitResult HitResult;
        FVector StartLocation = FootLocation + FVector(0, 0, IKSettings.TraceDistance);
        FVector EndLocation = FootLocation - FVector(0, 0, IKSettings.TraceDistance);
        
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(OwnerCharacter);
        
        if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, 
            ECC_Visibility, QueryParams))
        {
            ImpactNormal = HitResult.Normal;
        }
        
        FootData.FootRotation = CalculateFootRotation(ImpactLocation, ImpactNormal);
    }
    
    // Apply interpolation for smooth movement
    if (FootBoneName == LeftFootBoneName)
    {
        FootData.FootLocation = InterpFootLocation(LastLeftFootLocation, FootData.FootLocation, DeltaTime);
        FootData.FootRotation = InterpFootRotation(LastLeftFootRotation, FootData.FootRotation, DeltaTime);
        LastLeftFootLocation = FootData.FootLocation;
        LastLeftFootRotation = FootData.FootRotation;
    }
    else if (FootBoneName == RightFootBoneName)
    {
        FootData.FootLocation = InterpFootLocation(LastRightFootLocation, FootData.FootLocation, DeltaTime);
        FootData.FootRotation = InterpFootRotation(LastRightFootRotation, FootData.FootRotation, DeltaTime);
        LastRightFootLocation = FootData.FootLocation;
        LastRightFootRotation = FootData.FootRotation;
    }
    
    return FootData;
}

FVector UAnim_IKFootPlacement::PerformFootTrace(const FVector& FootLocation, float& OutDistance)
{
    if (!GetWorld())
    {
        OutDistance = 0.0f;
        return FootLocation;
    }
    
    FHitResult HitResult;
    FVector StartLocation = FootLocation + FVector(0, 0, IKSettings.TraceDistance);
    FVector EndLocation = FootLocation - FVector(0, 0, IKSettings.TraceDistance);
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    QueryParams.bTraceComplex = false;
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, 
        ECC_Visibility, QueryParams);
    
    if (bHit)
    {
        OutDistance = (FootLocation.Z - HitResult.Location.Z);
        return HitResult.Location;
    }
    else
    {
        OutDistance = IKSettings.TraceDistance;
        return FootLocation - FVector(0, 0, IKSettings.TraceDistance);
    }
}

FRotator UAnim_IKFootPlacement::CalculateFootRotation(const FVector& ImpactLocation, const FVector& ImpactNormal)
{
    // Calculate rotation to align foot with surface normal
    FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
    FVector RightVector = FVector::CrossProduct(ImpactNormal, ForwardVector).GetSafeNormal();
    ForwardVector = FVector::CrossProduct(RightVector, ImpactNormal).GetSafeNormal();
    
    return UKismetMathLibrary::MakeRotFromXZ(ForwardVector, ImpactNormal);
}

float UAnim_IKFootPlacement::CalculateIKAlpha(float DistanceFromGround)
{
    // Calculate alpha based on distance - closer to ground = higher alpha
    float Alpha = FMath::Clamp(FMath::Abs(DistanceFromGround) / IKSettings.TraceDistance, 0.0f, 1.0f);
    return 1.0f - Alpha; // Invert so closer = higher alpha
}

FVector UAnim_IKFootPlacement::InterpFootLocation(const FVector& Current, const FVector& Target, float DeltaTime)
{
    return FMath::VInterpTo(Current, Target, DeltaTime, IKSettings.IKInterpSpeed);
}

FRotator UAnim_IKFootPlacement::InterpFootRotation(const FRotator& Current, const FRotator& Target, float DeltaTime)
{
    return FMath::RInterpTo(Current, Target, DeltaTime, IKSettings.IKInterpSpeed);
}

float UAnim_IKFootPlacement::GetPelvisOffset() const
{
    return PelvisOffset;
}

void UAnim_IKFootPlacement::SetIKEnabled(bool bEnabled)
{
    IKSettings.bEnableFootIK = bEnabled;
    
    if (!bEnabled)
    {
        ResetIK();
    }
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange, 
            FString::Printf(TEXT("IK Foot Placement %s"), 
            bEnabled ? TEXT("Enabled") : TEXT("Disabled")));
    }
}

void UAnim_IKFootPlacement::ResetIK()
{
    LeftFootIK = FAnim_FootIKData();
    RightFootIK = FAnim_FootIKData();
    PelvisOffset = 0.0f;
    LastPelvisOffset = 0.0f;
    
    if (CharacterMesh)
    {
        LastLeftFootLocation = CharacterMesh->GetBoneLocation(LeftFootBoneName);
        LastRightFootLocation = CharacterMesh->GetBoneLocation(RightFootBoneName);
        LastLeftFootRotation = FRotator::ZeroRotator;
        LastRightFootRotation = FRotator::ZeroRotator;
    }
}