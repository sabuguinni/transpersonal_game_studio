#include "FootIKComponent.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"

UFootIKComponent::UFootIKComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.016f; // ~60fps
}

void UFootIKComponent::BeginPlay()
{
    Super::BeginPlay();

    // Cache the skeletal mesh from owner character
    if (ACharacter* OwnerChar = Cast<ACharacter>(GetOwner()))
    {
        OwnerMesh = OwnerChar->GetMesh();
        if (OwnerMesh)
        {
            UE_LOG(LogTemp, Log, TEXT("FootIKComponent: Cached SkeletalMesh '%s' on '%s'"),
                *OwnerMesh->GetName(), *GetOwner()->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("FootIKComponent: No SkeletalMesh found on owner '%s'"),
                *GetOwner()->GetName());
        }
    }
}

void UFootIKComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bEnableFootIK || !OwnerMesh)
    {
        return;
    }

    // Get foot bone world locations
    FVector LeftFootWorld = OwnerMesh->GetBoneLocation(LeftFootBoneName, EBoneSpaces::WorldSpace);
    FVector RightFootWorld = OwnerMesh->GetBoneLocation(RightFootBoneName, EBoneSpaces::WorldSpace);

    // Trace left foot
    FVector LeftHitLoc, LeftHitNormal;
    float TargetLeftOffset = 0.0f;
    if (TraceFootToGround(LeftFootWorld, LeftHitLoc, LeftHitNormal))
    {
        TargetLeftOffset = LeftHitLoc.Z - LeftFootWorld.Z + FootHeight;
        CurrentIKData.LeftFootLocation = LeftHitLoc;
        CurrentIKData.LeftFootRotation = FRotator(
            FMath::RadiansToDegrees(FMath::Atan2(LeftHitNormal.X, LeftHitNormal.Z)),
            0.0f,
            FMath::RadiansToDegrees(FMath::Atan2(LeftHitNormal.Y, LeftHitNormal.Z)) * -1.0f
        );
    }

    // Trace right foot
    FVector RightHitLoc, RightHitNormal;
    float TargetRightOffset = 0.0f;
    if (TraceFootToGround(RightFootWorld, RightHitLoc, RightHitNormal))
    {
        TargetRightOffset = RightHitLoc.Z - RightFootWorld.Z + FootHeight;
        CurrentIKData.RightFootLocation = RightHitLoc;
        CurrentIKData.RightFootRotation = FRotator(
            FMath::RadiansToDegrees(FMath::Atan2(RightHitNormal.X, RightHitNormal.Z)),
            0.0f,
            FMath::RadiansToDegrees(FMath::Atan2(RightHitNormal.Y, RightHitNormal.Z)) * -1.0f
        );
    }

    // Smooth interpolation
    CurrentIKData.LeftFootOffset = InterpFootOffset(CurrentIKData.LeftFootOffset, TargetLeftOffset, DeltaTime);
    CurrentIKData.RightFootOffset = InterpFootOffset(CurrentIKData.RightFootOffset, TargetRightOffset, DeltaTime);

    // Pelvis correction
    CurrentIKData.PelvisOffset = ComputePelvisOffset(CurrentIKData.LeftFootOffset, CurrentIKData.RightFootOffset);

    // Detect uneven terrain
    float OffsetDifference = FMath::Abs(CurrentIKData.LeftFootOffset - CurrentIKData.RightFootOffset);
    CurrentIKData.bIsOnUnevenTerrain = OffsetDifference > 5.0f;
}

FAnim_FootIKData UFootIKComponent::GetFootIKData() const
{
    return CurrentIKData;
}

bool UFootIKComponent::TraceFootToGround(const FVector& FootWorldLocation, FVector& OutHitLocation, FVector& OutHitNormal) const
{
    if (!GetWorld())
    {
        return false;
    }

    FVector TraceStart = FootWorldLocation + FVector(0.0f, 0.0f, TraceDistance * 0.5f);
    FVector TraceEnd = FootWorldLocation - FVector(0.0f, 0.0f, TraceDistance);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.bTraceComplex = false;

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );

    if (bHit)
    {
        OutHitLocation = HitResult.ImpactPoint;
        OutHitNormal = HitResult.ImpactNormal;
        return true;
    }

    return false;
}

float UFootIKComponent::ComputePelvisOffset(float LeftOffset, float RightOffset) const
{
    // Pelvis drops to accommodate the lower foot
    float MinOffset = FMath::Min(LeftOffset, RightOffset);
    return FMath::Clamp(MinOffset, -30.0f, 0.0f);
}

float UFootIKComponent::InterpFootOffset(float Current, float Target, float DeltaTime) const
{
    return FMath::FInterpTo(Current, Target, DeltaTime, InterpSpeed);
}
