#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Anim_IKFootPlacement.generated.h"

/**
 * IK Foot Placement system for realistic character foot positioning
 * Automatically adjusts foot placement to match terrain height and slope
 * Provides smooth interpolation for natural foot movement
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FootIKData
{
	GENERATED_BODY()

	// Foot bone name
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BoneName;

	// IK offset from ground
	UPROPERTY(BlueprintReadOnly)
	FVector IKOffset;

	// Foot rotation to match ground slope
	UPROPERTY(BlueprintReadOnly)
	FRotator IKRotation;

	// Distance from foot to ground
	UPROPERTY(BlueprintReadOnly)
	float DistanceToGround;

	// Whether foot is in contact with ground
	UPROPERTY(BlueprintReadOnly)
	bool bIsGrounded;

	FAnim_FootIKData()
	{
		BoneName = NAME_None;
		IKOffset = FVector::ZeroVector;
		IKRotation = FRotator::ZeroRotator;
		DistanceToGround = 0.0f;
		bIsGrounded = false;
	}
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_IKFootPlacement : public UActorComponent
{
	GENERATED_BODY()

public:
	UAnim_IKFootPlacement();

protected:
	// IK settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
	float TraceDistance = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
	float IKInterpSpeed = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
	float FootHeight = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
	bool bEnableIK = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
	bool bDebugDraw = false;

	// Foot bone names
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
	FName LeftFootBoneName = TEXT("foot_l");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
	FName RightFootBoneName = TEXT("foot_r");

	// IK data for each foot
	UPROPERTY(BlueprintReadOnly, Category = "IK Data")
	FAnim_FootIKData LeftFootIK;

	UPROPERTY(BlueprintReadOnly, Category = "IK Data")
	FAnim_FootIKData RightFootIK;

	// Hip offset to maintain character height
	UPROPERTY(BlueprintReadOnly, Category = "IK Data")
	float HipOffset;

	// Character references
	UPROPERTY()
	class ACharacter* OwnerCharacter;

	UPROPERTY()
	class USkeletalMeshComponent* CharacterMesh;

public:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// IK calculation functions
	UFUNCTION(BlueprintCallable, Category = "IK")
	void UpdateFootIK(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "IK")
	FAnim_FootIKData CalculateFootIK(const FName& BoneName, float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "IK")
	FVector PerformFootTrace(const FVector& FootLocation, float& OutDistance, FVector& OutNormal);

	UFUNCTION(BlueprintCallable, Category = "IK")
	void UpdateHipOffset(float DeltaTime);

	// Getters for animation blueprints
	UFUNCTION(BlueprintPure, Category = "IK Data")
	FAnim_FootIKData GetLeftFootIK() const { return LeftFootIK; }

	UFUNCTION(BlueprintPure, Category = "IK Data")
	FAnim_FootIKData GetRightFootIK() const { return RightFootIK; }

	UFUNCTION(BlueprintPure, Category = "IK Data")
	float GetHipOffset() const { return HipOffset; }

	UFUNCTION(BlueprintPure, Category = "IK Settings")
	bool IsIKEnabled() const { return bEnableIK; }

	// Configuration functions
	UFUNCTION(BlueprintCallable, Category = "IK Settings")
	void SetIKEnabled(bool bEnabled) { bEnableIK = bEnabled; }

	UFUNCTION(BlueprintCallable, Category = "IK Settings")
	void SetFootBoneNames(const FName& LeftFoot, const FName& RightFoot);

	UFUNCTION(BlueprintCallable, Category = "IK Settings")
	void SetIKParameters(float NewTraceDistance, float NewInterpSpeed, float NewFootHeight);

private:
	// Helper functions
	FVector GetBoneWorldLocation(const FName& BoneName) const;
	FRotator CalculateFootRotationFromNormal(const FVector& Normal) const;
	void DrawDebugIK(const FAnim_FootIKData& FootData, const FVector& TraceStart, const FVector& TraceEnd) const;
};