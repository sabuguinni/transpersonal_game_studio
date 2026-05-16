#include "Anim_IKFootPlacement.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_IKFootPlacement::UAnim_IKFootPlacement()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	
	// Initialize IK values
	LeftFootIKOffset = 0.0f;
	RightFootIKOffset = 0.0f;
	PelvisOffset = 0.0f;
	LeftFootIKRotation = FRotator::ZeroRotator;
	RightFootIKRotation = FRotator::ZeroRotator;
	
	// Initialize target values
	LeftFootTargetOffset = 0.0f;
	RightFootTargetOffset = 0.0f;
	PelvisTargetOffset = 0.0f;
	LeftFootTargetRotation = FRotator::ZeroRotator;
	RightFootTargetRotation = FRotator::ZeroRotator;
	
	// Default settings
	IKTraceDistance = 50.0f;
	IKInterpSpeed = 15.0f;
	FootOffset = 5.0f;
	bEnableIK = true;
	
	// Default bone names (common UE5 skeleton)
	LeftFootBoneName = "foot_l";
	RightFootBoneName = "foot_r";
	PelvisBoneName = "pelvis";
}

void UAnim_IKFootPlacement::BeginPlay()
{
	Super::BeginPlay();
	
	// Cache component references
	AActor* Owner = GetOwner();
	if (Owner)
	{
		SkeletalMeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
		MovementComponent = Owner->FindComponentByClass<UCharacterMovementComponent>();
	}
}

void UAnim_IKFootPlacement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (!bEnableIK || !SkeletalMeshComponent || !MovementComponent)
	{
		ResetIKValues();
		return;
	}
	
	// Only perform IK when character is on ground
	if (MovementComponent->IsMovingOnGround())
	{
		UpdateIKValues(DeltaTime);
	}
	else
	{
		ResetIKValues();
	}
}

void UAnim_IKFootPlacement::UpdateIKValues(float DeltaTime)
{
	if (!SkeletalMeshComponent)
	{
		return;
	}
	
	// Perform foot traces
	FRotator LeftFootRotation;
	FRotator RightFootRotation;
	
	float LeftFootTrace = PerformFootTrace(LeftFootBoneName, LeftFootRotation);
	float RightFootTrace = PerformFootTrace(RightFootBoneName, RightFootRotation);
	
	// Calculate target offsets
	LeftFootTargetOffset = LeftFootTrace;
	RightFootTargetOffset = RightFootTrace;
	LeftFootTargetRotation = LeftFootRotation;
	RightFootTargetRotation = RightFootRotation;
	
	// Calculate pelvis offset (average of both feet, but keep character level)
	float MaxFootOffset = FMath::Max(LeftFootTrace, RightFootTrace);
	PelvisTargetOffset = MaxFootOffset * 0.5f; // Adjust pelvis by half the maximum offset
	
	// Smooth interpolation
	LeftFootIKOffset = FMath::FInterpTo(LeftFootIKOffset, LeftFootTargetOffset, DeltaTime, IKInterpSpeed);
	RightFootIKOffset = FMath::FInterpTo(RightFootIKOffset, RightFootTargetOffset, DeltaTime, IKInterpSpeed);
	PelvisOffset = FMath::FInterpTo(PelvisOffset, PelvisTargetOffset, DeltaTime, IKInterpSpeed);
	
	LeftFootIKRotation = FMath::RInterpTo(LeftFootIKRotation, LeftFootTargetRotation, DeltaTime, IKInterpSpeed);
	RightFootIKRotation = FMath::RInterpTo(RightFootIKRotation, RightFootTargetRotation, DeltaTime, IKInterpSpeed);
}

float UAnim_IKFootPlacement::PerformFootTrace(const FName& BoneName, FRotator& OutRotation)
{
	if (!SkeletalMeshComponent)
	{
		OutRotation = FRotator::ZeroRotator;
		return 0.0f;
	}
	
	// Get bone world location
	FVector BoneLocation = GetBoneWorldLocation(BoneName);
	if (BoneLocation == FVector::ZeroVector)
	{
		OutRotation = FRotator::ZeroRotator;
		return 0.0f;
	}
	
	// Perform line trace downward
	FVector TraceStart = BoneLocation + FVector(0, 0, IKTraceDistance * 0.5f);
	FVector TraceEnd = BoneLocation - FVector(0, 0, IKTraceDistance);
	
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		QueryParams
	);
	
	if (bHit)
	{
		// Calculate offset from bone to ground
		float Offset = (BoneLocation.Z - HitResult.Location.Z) - FootOffset;
		
		// Calculate foot rotation based on surface normal
		FVector UpVector = FVector::UpVector;
		FVector SurfaceNormal = HitResult.Normal;
		
		// Create rotation to align foot with surface
		FVector ForwardVector = FVector::CrossProduct(UpVector, SurfaceNormal).GetSafeNormal();
		FVector RightVector = FVector::CrossProduct(SurfaceNormal, ForwardVector).GetSafeNormal();
		
		OutRotation = UKismetMathLibrary::MakeRotationFromAxes(ForwardVector, RightVector, SurfaceNormal);
		
		return -Offset; // Negative because we want to move foot down
	}
	
	OutRotation = FRotator::ZeroRotator;
	return 0.0f;
}

FVector UAnim_IKFootPlacement::GetBoneWorldLocation(const FName& BoneName)
{
	if (!SkeletalMeshComponent)
	{
		return FVector::ZeroVector;
	}
	
	int32 BoneIndex = SkeletalMeshComponent->GetBoneIndex(BoneName);
	if (BoneIndex == INDEX_NONE)
	{
		return FVector::ZeroVector;
	}
	
	return SkeletalMeshComponent->GetBoneLocation(BoneName, EBoneSpaces::WorldSpace);
}

void UAnim_IKFootPlacement::ResetIKValues()
{
	LeftFootTargetOffset = 0.0f;
	RightFootTargetOffset = 0.0f;
	PelvisTargetOffset = 0.0f;
	LeftFootTargetRotation = FRotator::ZeroRotator;
	RightFootTargetRotation = FRotator::ZeroRotator;
}