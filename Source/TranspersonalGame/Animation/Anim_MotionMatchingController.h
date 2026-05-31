#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Engine.h"
#include "Animation/AnimSequence.h"
#include "Animation/PoseAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "Anim_MotionMatchingController.generated.h"

/**
 * Motion data structure for storing animation poses and metadata
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
	class UAnimSequence* Animation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
	float TimeStamp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
	FVector Velocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
	FVector Acceleration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
	float AngularVelocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
	TArray<FTransform> BoneTransforms;

	FAnim_MotionData()
	{
		Animation = nullptr;
		TimeStamp = 0.0f;
		Velocity = FVector::ZeroVector;
		Acceleration = FVector::ZeroVector;
		AngularVelocity = 0.0f;
	}
};

/**
 * Motion matching query structure for finding best animation match
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionQuery
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Query")
	FVector DesiredVelocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Query")
	FVector DesiredAcceleration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Query")
	float DesiredAngularVelocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Query")
	TArray<FTransform> CurrentPose;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Query")
	float VelocityWeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Query")
	float AccelerationWeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Query")
	float AngularWeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Query")
	float PoseWeight;

	FAnim_MotionQuery()
	{
		DesiredVelocity = FVector::ZeroVector;
		DesiredAcceleration = FVector::ZeroVector;
		DesiredAngularVelocity = 0.0f;
		VelocityWeight = 1.0f;
		AccelerationWeight = 0.5f;
		AngularWeight = 0.3f;
		PoseWeight = 0.8f;
	}
};

/**
 * Motion Matching Controller for fluid character animations
 * Implements simplified motion matching system for natural movement
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_MotionMatchingController : public UObject
{
	GENERATED_BODY()

public:
	UAnim_MotionMatchingController();

protected:
	// Motion database
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Database")
	TArray<FAnim_MotionData> MotionDatabase;

	// Current animation state
	UPROPERTY(BlueprintReadOnly, Category = "Current State", meta = (AllowPrivateAccess = "true"))
	int32 CurrentMotionIndex;

	UPROPERTY(BlueprintReadOnly, Category = "Current State", meta = (AllowPrivateAccess = "true"))
	float CurrentPlayTime;

	UPROPERTY(BlueprintReadOnly, Category = "Current State", meta = (AllowPrivateAccess = "true"))
	class UAnimSequence* CurrentAnimation;

	// Motion matching settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching Settings")
	float SearchRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching Settings")
	float MinimumMatchScore;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching Settings")
	float BlendTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching Settings")
	bool bEnableDebugOutput;

public:
	// Motion matching interface
	UFUNCTION(BlueprintCallable, Category = "Motion Matching")
	void InitializeMotionDatabase();

	UFUNCTION(BlueprintCallable, Category = "Motion Matching")
	void AddMotionData(UAnimSequence* Animation, float TimeStamp, const FVector& Velocity, const FVector& Acceleration);

	UFUNCTION(BlueprintCallable, Category = "Motion Matching")
	int32 FindBestMotionMatch(const FAnim_MotionQuery& Query);

	UFUNCTION(BlueprintCallable, Category = "Motion Matching")
	float CalculateMotionScore(const FAnim_MotionData& MotionData, const FAnim_MotionQuery& Query);

	UFUNCTION(BlueprintCallable, Category = "Motion Matching")
	void UpdateMotionMatching(float DeltaTime, const FAnim_MotionQuery& Query);

	// Animation control
	UFUNCTION(BlueprintCallable, Category = "Animation Control")
	void PlayMotion(int32 MotionIndex, float BlendInTime = 0.2f);

	UFUNCTION(BlueprintCallable, Category = "Animation Control")
	void StopCurrentMotion();

	UFUNCTION(BlueprintCallable, Category = "Animation Control")
	bool IsMotionPlaying() const;

	// Utility functions
	UFUNCTION(BlueprintCallable, Category = "Motion Matching")
	void ExtractPoseFromAnimation(UAnimSequence* Animation, float Time, TArray<FTransform>& OutPose);

	UFUNCTION(BlueprintCallable, Category = "Motion Matching")
	float CalculatePoseDistance(const TArray<FTransform>& PoseA, const TArray<FTransform>& PoseB);

	UFUNCTION(BlueprintCallable, Category = "Motion Matching")
	void ClearMotionDatabase();

	// Getters
	UFUNCTION(BlueprintPure, Category = "Motion Matching")
	int32 GetCurrentMotionIndex() const { return CurrentMotionIndex; }

	UFUNCTION(BlueprintPure, Category = "Motion Matching")
	float GetCurrentPlayTime() const { return CurrentPlayTime; }

	UFUNCTION(BlueprintPure, Category = "Motion Matching")
	UAnimSequence* GetCurrentAnimation() const { return CurrentAnimation; }

	UFUNCTION(BlueprintPure, Category = "Motion Matching")
	int32 GetMotionDatabaseSize() const { return MotionDatabase.Num(); }

private:
	// Internal helper functions
	void LogDebugInfo(const FString& Message);
	bool IsValidMotionIndex(int32 Index) const;
};