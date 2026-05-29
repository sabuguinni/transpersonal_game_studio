#include "Anim_IKFootPlacement.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_IKFootPlacement::UAnim_IKFootPlacement()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;

    // Initialize settings
    TraceDistance = 150.0f;
    IKInterpSpeed = 15.0f;
    FootHeight = 13.0f;
    bEnableIK = true;
    bDrawDebugTraces = false;

    // Initialize bone names
    LeftFootBoneName = TEXT("foot_l");
    RightFootBoneName = TEXT("foot_r");
    PelvisBoneName = TEXT("pelvis");

    // Initialize IK data
    LeftFootIK = FAnim_FootIKData();
    RightFootIK = FAnim_FootIKData();
    PelvisOffset = 0.0f;

    // Initialize cached references
    SkeletalMeshComponent = nullptr;
    OwnerCharacter = nullptr;
}

void UAnim_IKFootPlacement::BeginPlay()
{
    Super::BeginPlay();

    // Cache references
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        SkeletalMeshComponent = OwnerCharacter->GetMesh();
        UE_LOG(LogTemp, Warning, TEXT("IK Foot Placement initialized for %s"), *OwnerCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("IK Foot Placement: Owner is not a Character!"));
    }
}

void UAnim_IKFootPlacement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bEnableIK && SkeletalMeshComponent && OwnerCharacter)
    {
        UpdateFootIK();
    }
}

void UAnim_IKFootPlacement::UpdateFootIK()
{
    if (!SkeletalMeshComponent || !OwnerCharacter)
    {
        return;
    }

    float DeltaTime = GetWorld()->GetDeltaSeconds();

    // Update foot IK data
    LeftFootIK = CalculateFootIK(LeftFootBoneName, DeltaTime);
    RightFootIK = CalculateFootIK(RightFootBoneName, DeltaTime);

    // Update pelvis offset
    UpdatePelvisOffset(DeltaTime);
}

FAnim_FootIKData UAnim_IKFootPlacement::CalculateFootIK(const FName& FootBoneName, float DeltaTime)
{
    FAnim_FootIKData FootData;

    if (!SkeletalMeshComponent)
    {
        return FootData;
    }

    // Get foot world location
    FVector FootWorldLocation = GetFootWorldLocation(FootBoneName);
    
    if (FootWorldLocation == FVector::ZeroVector)
    {
        return FootData;
    }

    // Perform ground trace
    FVector HitLocation;
    FVector HitNormal;
    bool bHitGround = PerformGroundTrace(FootWorldLocation, HitLocation, HitNormal);

    if (bHitGround)
    {
        FootData.bIsGrounded = true;
        FootData.DistanceFromGround = FootWorldLocation.Z - HitLocation.Z;
        
        // Calculate target foot location (adjust for foot height)
        FVector TargetLocation = HitLocation + FVector(0, 0, FootHeight);
        
        // Convert to component space
        FTransform ComponentTransform = SkeletalMeshComponent->GetComponentTransform();
        FootData.FootLocation = ComponentTransform.InverseTransformPosition(TargetLocation);
        
        // Calculate foot rotation from ground normal
        FootData.FootRotation = CalculateFootRotationFromNormal(HitNormal);
        
        // Calculate IK alpha based on distance from ground
        float MaxIKDistance = TraceDistance * 0.5f;
        FootData.IKAlpha = FMath::Clamp(
            1.0f - (FMath::Abs(FootData.DistanceFromGround) / MaxIKDistance),
            0.0f, 1.0f
        );
    }
    else
    {
        FootData.bIsGrounded = false;
        FootData.IKAlpha = 0.0f;
        FootData.DistanceFromGround = TraceDistance;
    }

    return FootData;
}

FVector UAnim_IKFootPlacement::GetFootWorldLocation(const FName& FootBoneName) const
{
    if (!SkeletalMeshComponent)
    {
        return FVector::ZeroVector;
    }

    int32 BoneIndex = SkeletalMeshComponent->GetBoneIndex(FootBoneName);
    if (BoneIndex == INDEX_NONE)
    {
        UE_LOG(LogTemp, Warning, TEXT("IK Foot Placement: Bone '%s' not found!"), *FootBoneName.ToString());
        return FVector::ZeroVector;
    }

    return SkeletalMeshComponent->GetBoneLocation(FootBoneName, EBoneSpaces::WorldSpace);
}

bool UAnim_IKFootPlacement::PerformGroundTrace(const FVector& StartLocation, FVector& OutHitLocation, FVector& OutHitNormal) const
{
    if (!GetWorld())
    {
        return false;
    }

    FVector TraceStart = StartLocation + FVector(0, 0, TraceDistance * 0.5f);
    FVector TraceEnd = StartLocation - FVector(0, 0, TraceDistance);

    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;
    QueryParams.AddIgnoredActor(OwnerCharacter);

    FHitResult HitResult;
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_Visibility,
        QueryParams
    );

    if (bDrawDebugTraces)
    {
        FColor TraceColor = bHit ? FColor::Green : FColor::Red;
        DrawDebugLine(GetWorld(), TraceStart, TraceEnd, TraceColor, false, 0.1f, 0, 1.0f);
        
        if (bHit)
        {
            DrawDebugSphere(GetWorld(), HitResult.Location, 5.0f, 8, FColor::Yellow, false, 0.1f);
        }
    }

    if (bHit)
    {
        OutHitLocation = HitResult.Location;
        OutHitNormal = HitResult.Normal;
        return true;
    }

    return false;
}

FRotator UAnim_IKFootPlacement::CalculateFootRotationFromNormal(const FVector& Normal) const
{
    // Calculate rotation to align foot with ground normal
    FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
    FVector RightVector = FVector::CrossProduct(Normal, ForwardVector).GetSafeNormal();
    ForwardVector = FVector::CrossProduct(RightVector, Normal).GetSafeNormal();

    return UKismetMathLibrary::MakeRotFromXZ(ForwardVector, Normal);
}

void UAnim_IKFootPlacement::UpdatePelvisOffset(float DeltaTime)
{
    if (!bEnableIK)
    {
        PelvisOffset = 0.0f;
        return;
    }

    // Calculate target pelvis offset based on foot positions
    float LeftFootOffset = LeftFootIK.bIsGrounded ? FMath::Min(LeftFootIK.DistanceFromGround, 0.0f) : 0.0f;
    float RightFootOffset = RightFootIK.bIsGrounded ? FMath::Min(RightFootIK.DistanceFromGround, 0.0f) : 0.0f;
    
    float TargetPelvisOffset = FMath::Max(LeftFootOffset, RightFootOffset);
    
    // Smooth pelvis offset changes
    PelvisOffset = FMath::FInterpTo(PelvisOffset, TargetPelvisOffset, DeltaTime, IKInterpSpeed);
}