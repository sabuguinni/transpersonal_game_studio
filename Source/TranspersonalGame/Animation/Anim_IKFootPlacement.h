#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Anim_IKFootPlacement.generated.h"

/**
 * IK Foot Placement Component for adaptive terrain walking
 * Adjusts foot positions to match ground surface for realistic movement
 * Essential for prehistoric terrain with uneven surfaces
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_IKFootPlacement : public UActorComponent
{
	GENERATED_BODY()

public:
	UAnim_IKFootPlacement();

	// IK settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
	float IKTraceDistance = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
	float IKInterpSpeed = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
	float FootOffset = 5.0f;

	// Foot bone names
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
	FName LeftFootBoneName = "foot_l";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
	FName RightFootBoneName = "foot_r";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
	FName PelvisBoneName = "pelvis";

	// IK output values (read by animation blueprint)
	UPROPERTY(BlueprintReadOnly, Category = "IK Output")
	float LeftFootIKOffset;

	UPROPERTY(BlueprintReadOnly, Category = "IK Output")
	float RightFootIKOffset;

	UPROPERTY(BlueprintReadOnly, Category = "IK Output")
	FRotator LeftFootIKRotation;

	UPROPERTY(BlueprintReadOnly, Category = "IK Output")
	FRotator RightFootIKRotation;

	UPROPERTY(BlueprintReadOnly, Category = "IK Output")
	float PelvisOffset;

	// Enable/disable IK
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
	bool bEnableIK = true;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	// Cached references
	UPROPERTY()
	class USkeletalMeshComponent* SkeletalMeshComponent;

	UPROPERTY()
	class UCharacterMovementComponent* MovementComponent;

	// IK calculation functions
	float PerformFootTrace(const FName& BoneName, FRotator& OutRotation);
	FVector GetBoneWorldLocation(const FName& BoneName);
	void UpdateIKValues(float DeltaTime);
	void ResetIKValues();

	// Smoothing values
	float LeftFootTargetOffset;
	float RightFootTargetOffset;
	float PelvisTargetOffset;
	FRotator LeftFootTargetRotation;
	FRotator RightFootTargetRotation;
};