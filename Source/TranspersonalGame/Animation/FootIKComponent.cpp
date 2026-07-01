#include "FootIKComponent.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

UFootIKComponent::UFootIKComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.0f;
}

void UFootIKComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentIKData = FAnim_FootIKData();
}

void UFootIKComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateFootIK(DeltaTime);
}

FAnim_FootIKData UFootIKComponent::GetFootIKData() const
{
	return CurrentIKData;
}

void UFootIKComponent::UpdateFootIK(float DeltaTime)
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		return;
	}

	USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
	if (!MeshComp)
	{
		return;
	}

	// Get bone world locations
	FVector LeftFootWorld = MeshComp->GetBoneLocation(LeftFootBoneName, EBoneSpaces::WorldSpace);
	FVector RightFootWorld = MeshComp->GetBoneLocation(RightFootBoneName, EBoneSpaces::WorldSpace);

	// Trace for each foot
	FVector LeftHitLoc, LeftHitNormal;
	FVector RightHitLoc, RightHitNormal;

	bool bLeftHit = TraceForFoot(LeftFootWorld, LeftHitLoc, LeftHitNormal);
	bool bRightHit = TraceForFoot(RightFootWorld, RightHitLoc, RightHitNormal);

	// Calculate offsets
	float TargetLeftOffset = 0.0f;
	float TargetRightOffset = 0.0f;
	FRotator TargetLeftRot = FRotator::ZeroRotator;
	FRotator TargetRightRot = FRotator::ZeroRotator;

	if (bLeftHit)
	{
		TargetLeftOffset = FMath::Clamp(LeftHitLoc.Z - LeftFootWorld.Z, -MaxFootOffset, MaxFootOffset);
		// Align foot to surface normal
		FVector Forward = OwnerCharacter->GetActorForwardVector();
		FVector Right = FVector::CrossProduct(LeftHitNormal, Forward).GetSafeNormal();
		FVector AdjustedForward = FVector::CrossProduct(Right, LeftHitNormal).GetSafeNormal();
		TargetLeftRot = UKismetMathLibrary::MakeRotFromXZ(AdjustedForward, LeftHitNormal);
	}

	if (bRightHit)
	{
		TargetRightOffset = FMath::Clamp(RightHitLoc.Z - RightFootWorld.Z, -MaxFootOffset, MaxFootOffset);
		FVector Forward = OwnerCharacter->GetActorForwardVector();
		FVector Right = FVector::CrossProduct(RightHitNormal, Forward).GetSafeNormal();
		FVector AdjustedForward = FVector::CrossProduct(Right, RightHitNormal).GetSafeNormal();
		TargetRightRot = UKismetMathLibrary::MakeRotFromXZ(AdjustedForward, RightHitNormal);
	}

	// Interpolate smoothly
	CurrentIKData.LeftFootOffset = FMath::FInterpTo(CurrentIKData.LeftFootOffset, TargetLeftOffset, DeltaTime, InterpSpeed);
	CurrentIKData.RightFootOffset = FMath::FInterpTo(CurrentIKData.RightFootOffset, TargetRightOffset, DeltaTime, InterpSpeed);
	CurrentIKData.LeftFootRotation = FMath::RInterpTo(CurrentIKData.LeftFootRotation, TargetLeftRot, DeltaTime, InterpSpeed);
	CurrentIKData.RightFootRotation = FMath::RInterpTo(CurrentIKData.RightFootRotation, TargetRightRot, DeltaTime, InterpSpeed);

	// Update pelvis offset
	float TargetPelvis = CalculatePelvisOffset(CurrentIKData.LeftFootOffset, CurrentIKData.RightFootOffset);
	CurrentIKData.PelvisOffset = FMath::FInterpTo(CurrentIKData.PelvisOffset, TargetPelvis, DeltaTime, InterpSpeed * 0.5f);

	// Update foot world locations
	CurrentIKData.LeftFootLocation = LeftFootWorld;
	CurrentIKData.RightFootLocation = RightFootWorld;

	// Detect uneven terrain
	float OffsetDiff = FMath::Abs(CurrentIKData.LeftFootOffset - CurrentIKData.RightFootOffset);
	CurrentIKData.bIsOnUnevenTerrain = OffsetDiff > 5.0f;
}

bool UFootIKComponent::TraceForFoot(const FVector& FootLocation, FVector& OutHitLocation, FVector& OutHitNormal) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	FVector TraceStart = FootLocation + FVector(0.0f, 0.0f, TraceDistance);
	FVector TraceEnd = FootLocation - FVector(0.0f, 0.0f, TraceDistance);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	QueryParams.bTraceComplex = false;

	bool bHit = World->LineTraceSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		QueryParams
	);

	if (bHit)
	{
		OutHitLocation = HitResult.ImpactPoint;
		OutHitNormal = HitResult.ImpactNormal;
	}

	return bHit;
}

float UFootIKComponent::CalculatePelvisOffset(float LeftOffset, float RightOffset) const
{
	// Pelvis drops to the lower foot to prevent stretching
	return FMath::Min(LeftOffset, RightOffset);
}
