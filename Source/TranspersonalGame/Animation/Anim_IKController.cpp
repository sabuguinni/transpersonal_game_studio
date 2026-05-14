#include "Anim_IKController.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UAnim_IKController::UAnim_IKController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Default settings
    bEnableFootIK = true;
    FootTraceDistance = 50.0f;
    IKInterpSpeed = 15.0f;
    MaxFootOffset = 30.0f;
    
    // Default bone names (common MetaHuman/UE5 skeleton)
    LeftFootBoneName = TEXT("foot_l");
    RightFootBoneName = TEXT("foot_r");
    LeftThighBoneName = TEXT("thigh_l");
    RightThighBoneName = TEXT("thigh_r");
    LeftCalfBoneName = TEXT("calf_l");
    RightCalfBoneName = TEXT("calf_r");
    
    // Initialize state
    LeftFootIKOffset = 0.0f;
    RightFootIKOffset = 0.0f;
    LeftFootIKRotation = FRotator::ZeroRotator;
    RightFootIKRotation = FRotator::ZeroRotator;
    HipOffset = 0.0f;
    
    TargetLeftFootOffset = 0.0f;
    TargetRightFootOffset = 0.0f;
    TargetLeftFootRotation = FRotator::ZeroRotator;
    TargetRightFootRotation = FRotator::ZeroRotator;
    TargetHipOffset = 0.0f;
    
    // Trace settings
    TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic));
    TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldDynamic));
    bTraceComplex = false;
    bIgnoreSelf = true;
    
    OwnerCharacter = nullptr;
    SkeletalMeshComp = nullptr;
}

void UAnim_IKController::BeginPlay()
{
    Super::BeginPlay();
    InitializeComponent();
}

void UAnim_IKController::InitializeComponent()
{
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        SkeletalMeshComp = OwnerCharacter->GetMesh();
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green,
                TEXT("IK Controller initialized successfully"));
        }
    }
    else
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red,
                TEXT("IK Controller: Owner is not a Character!"));
        }
    }
}

void UAnim_IKController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bEnableFootIK && OwnerCharacter && SkeletalMeshComp)
    {
        UpdateFootIK(DeltaTime);
    }
}

void UAnim_IKController::UpdateFootIK(float DeltaTime)
{
    if (!OwnerCharacter || !SkeletalMeshComp)
    {
        return;
    }
    
    // Get foot world locations
    FVector LeftFootLocation = GetFootWorldLocation(LeftFootBoneName);
    FVector RightFootLocation = GetFootWorldLocation(RightFootBoneName);
    
    if (LeftFootLocation.IsZero() || RightFootLocation.IsZero())
    {
        // Bones not found, disable IK
        return;
    }
    
    // Trace for ground under each foot
    FHitResult LeftHit, RightHit;
    FVector TraceStart, TraceEnd;
    
    // Left foot trace
    TraceStart = LeftFootLocation + FVector(0, 0, 25.0f);
    TraceEnd = LeftFootLocation - FVector(0, 0, FootTraceDistance);
    float LeftGroundDistance = TraceForGround(TraceStart, TraceEnd, LeftHit);
    
    // Right foot trace
    TraceStart = RightFootLocation + FVector(0, 0, 25.0f);
    TraceEnd = RightFootLocation - FVector(0, 0, FootTraceDistance);
    float RightGroundDistance = TraceForGround(TraceStart, TraceEnd, RightHit);
    
    // Calculate target offsets
    TargetLeftFootOffset = FMath::Clamp(LeftGroundDistance, -MaxFootOffset, MaxFootOffset);
    TargetRightFootOffset = FMath::Clamp(RightGroundDistance, -MaxFootOffset, MaxFootOffset);
    
    // Calculate foot rotations based on ground normal
    if (LeftHit.bBlockingHit)
    {
        TargetLeftFootRotation = CalculateFootRotation(LeftHit);
    }
    if (RightHit.bBlockingHit)
    {
        TargetRightFootRotation = CalculateFootRotation(RightHit);
    }
    
    // Update hip offset to keep character level
    UpdateHipOffset();
    
    // Smoothly interpolate to target values
    LeftFootIKOffset = FMath::FInterpTo(LeftFootIKOffset, TargetLeftFootOffset, DeltaTime, IKInterpSpeed);
    RightFootIKOffset = FMath::FInterpTo(RightFootIKOffset, TargetRightFootOffset, DeltaTime, IKInterpSpeed);
    
    LeftFootIKRotation = FMath::RInterpTo(LeftFootIKRotation, TargetLeftFootRotation, DeltaTime, IKInterpSpeed);
    RightFootIKRotation = FMath::RInterpTo(RightFootIKRotation, TargetRightFootRotation, DeltaTime, IKInterpSpeed);
    
    HipOffset = FMath::FInterpTo(HipOffset, TargetHipOffset, DeltaTime, IKInterpSpeed);
}

float UAnim_IKController::TraceForGround(const FVector& StartLocation, const FVector& EndLocation, FHitResult& HitResult)
{
    TArray<AActor*> ActorsToIgnore;
    if (bIgnoreSelf && OwnerCharacter)
    {
        ActorsToIgnore.Add(OwnerCharacter);
    }
    
    bool bHit = UKismetSystemLibrary::LineTraceSingleForObjects(
        GetWorld(),
        StartLocation,
        EndLocation,
        TraceObjectTypes,
        bTraceComplex,
        ActorsToIgnore,
        EDrawDebugTrace::None, // Change to ForOneFrame for debugging
        HitResult,
        bIgnoreSelf
    );
    
    if (bHit)
    {
        // Return the distance from the original foot position to the ground
        return StartLocation.Z - HitResult.Location.Z - 25.0f; // Subtract the initial offset
    }
    
    return 0.0f;
}

FRotator UAnim_IKController::CalculateFootRotation(const FHitResult& HitResult)
{
    if (!HitResult.bBlockingHit)
    {
        return FRotator::ZeroRotator;
    }
    
    // Calculate rotation to align foot with ground normal
    FVector UpVector = FVector::UpVector;
    FVector GroundNormal = HitResult.Normal;
    
    // Calculate the rotation needed to align up vector with ground normal
    FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(UpVector, GroundNormal);
    
    // Limit the rotation to prevent extreme angles
    TargetRotation.Pitch = FMath::Clamp(TargetRotation.Pitch, -45.0f, 45.0f);
    TargetRotation.Roll = FMath::Clamp(TargetRotation.Roll, -45.0f, 45.0f);
    
    return TargetRotation;
}

void UAnim_IKController::UpdateHipOffset()
{
    // Calculate hip adjustment to keep character balanced
    float AverageFootOffset = (TargetLeftFootOffset + TargetRightFootOffset) * 0.5f;
    float MaxFootOffset_Local = FMath::Max(FMath::Abs(TargetLeftFootOffset), FMath::Abs(TargetRightFootOffset));
    
    // Adjust hip to keep the character level
    TargetHipOffset = -AverageFootOffset * 0.5f;
    
    // Clamp hip offset
    TargetHipOffset = FMath::Clamp(TargetHipOffset, -MaxFootOffset * 0.5f, MaxFootOffset * 0.5f);
}

float UAnim_IKController::CalculateHipAdjustment() const
{
    return HipOffset;
}

FVector UAnim_IKController::GetFootWorldLocation(const FName& BoneName) const
{
    if (!SkeletalMeshComp || BoneName == NAME_None)
    {
        return FVector::ZeroVector;
    }
    
    // Get bone index
    int32 BoneIndex = SkeletalMeshComp->GetBoneIndex(BoneName);
    if (BoneIndex == INDEX_NONE)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange,
                FString::Printf(TEXT("IK: Bone '%s' not found"), *BoneName.ToString()));
        }
        return FVector::ZeroVector;
    }
    
    // Get world space bone location
    FVector BoneLocation = SkeletalMeshComp->GetBoneLocation(BoneName, EBoneSpaces::WorldSpace);
    return BoneLocation;
}

void UAnim_IKController::SetIKEnabled(bool bEnabled)
{
    bEnableFootIK = bEnabled;
    
    if (!bEnabled)
    {
        // Reset all IK values when disabled
        TargetLeftFootOffset = 0.0f;
        TargetRightFootOffset = 0.0f;
        TargetLeftFootRotation = FRotator::ZeroRotator;
        TargetRightFootRotation = FRotator::ZeroRotator;
        TargetHipOffset = 0.0f;
    }
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan,
            FString::Printf(TEXT("Foot IK %s"), bEnabled ? TEXT("Enabled") : TEXT("Disabled")));
    }
}

void UAnim_IKController::SetFootBoneNames(const FName& LeftFoot, const FName& RightFoot)
{
    LeftFootBoneName = LeftFoot;
    RightFootBoneName = RightFoot;
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow,
            FString::Printf(TEXT("Foot bones set: L=%s, R=%s"), 
                *LeftFootBoneName.ToString(), *RightFootBoneName.ToString()));
    }
}

void UAnim_IKController::SetLegBoneNames(const FName& LeftThigh, const FName& LeftCalf, const FName& RightThigh, const FName& RightCalf)
{
    LeftThighBoneName = LeftThigh;
    LeftCalfBoneName = LeftCalf;
    RightThighBoneName = RightThigh;
    RightCalfBoneName = RightCalf;
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow,
            TEXT("Leg bone names updated for IK chain"));
    }
}