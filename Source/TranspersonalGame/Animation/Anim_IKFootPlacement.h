#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "../SharedTypes.h"
#include "Anim_IKFootPlacement.generated.h"

TRANSPERSONALGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogIKFootPlacement, Log, All);

/**
 * Foot placement data for a single foot
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FootPlacementData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Foot Placement")
	FVector FootLocation;

	UPROPERTY(BlueprintReadWrite, Category = "Foot Placement")
	FRotator FootRotation;

	UPROPERTY(BlueprintReadWrite, Category = "Foot Placement")
	float IKAlpha;

	UPROPERTY(BlueprintReadWrite, Category = "Foot Placement")
	bool bIsGrounded;

	UPROPERTY(BlueprintReadWrite, Category = "Foot Placement")
	float DistanceToGround;

	FAnim_FootPlacementData()
	{
		FootLocation = FVector::ZeroVector;
		FootRotation = FRotator::ZeroRotator;
		IKAlpha = 0.0f;
		bIsGrounded = false;
		DistanceToGround = 0.0f;
	}
};

/**
 * IK Foot Placement Component
 * Handles adaptive foot placement for uneven terrain
 * Integrates with character animation for realistic ground contact
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_IKFootPlacement : public UActorComponent
{
	GENERATED_BODY()

public:
	UAnim_IKFootPlacement();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// IK Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
	float TraceDistance = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
	float IKInterpSpeed = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
	float FootOffsetThreshold = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
	bool bEnableIK = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
	bool bDebugDraw = false;

	// Bone names for foot IK
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Settings")
	FName LeftFootBoneName = TEXT("foot_l");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Settings")
	FName RightFootBoneName = TEXT("foot_r");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Settings")
	FName PelvisBoneName = TEXT("pelvis");

	// Foot placement data
	UPROPERTY(BlueprintReadOnly, Category = "Foot Placement")
	FAnim_FootPlacementData LeftFootData;

	UPROPERTY(BlueprintReadOnly, Category = "Foot Placement")
	FAnim_FootPlacementData RightFootData;

	UPROPERTY(BlueprintReadOnly, Category = "Foot Placement")
	float PelvisOffset;

	// Public functions
	UFUNCTION(BlueprintCallable, Category = "IK Foot Placement")
	void UpdateFootPlacement();

	UFUNCTION(BlueprintCallable, Category = "IK Foot Placement")
	void SetIKEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "IK Foot Placement")
	FAnim_FootPlacementData GetLeftFootData() const { return LeftFootData; }

	UFUNCTION(BlueprintCallable, Category = "IK Foot Placement")
	FAnim_FootPlacementData GetRightFootData() const { return RightFootData; }

	UFUNCTION(BlueprintCallable, Category = "IK Foot Placement")
	float GetPelvisOffset() const { return PelvisOffset; }

protected:
	// Component references
	UPROPERTY()
	class USkeletalMeshComponent* SkeletalMeshComponent;

	UPROPERTY()
	class ACharacter* OwnerCharacter;

	// Internal foot placement calculation
	FAnim_FootPlacementData CalculateFootPlacement(const FName& BoneName, const FVector& FootSocketLocation);

	// Ground tracing
	bool TraceGround(const FVector& StartLocation, FVector& OutHitLocation, FVector& OutHitNormal);

	// IK calculation helpers
	FRotator CalculateFootRotationFromNormal(const FVector& GroundNormal);
	float CalculateIKAlpha(float DistanceToGround);
	void UpdatePelvisOffset();

	// Interpolation
	void InterpolateFootData(FAnim_FootPlacementData& CurrentData, const FAnim_FootPlacementData& TargetData, float DeltaTime);

	// Debug drawing
	void DrawDebugInfo();

private:
	// Previous frame data for interpolation
	FAnim_FootPlacementData PreviousLeftFootData;
	FAnim_FootPlacementData PreviousRightFootData;
	float PreviousPelvisOffset;

	// Trace parameters
	FCollisionQueryParams TraceParams;
};