#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimInstance.h"
#include "Anim_MotionMatchingSystem.generated.h"

/**
 * Motion Matching System for fluid character movement
 * Selects best animation based on current movement context
 * Provides natural transitions between movement states
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionData
{
	GENERATED_BODY()

	// Animation sequence reference
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
	class UAnimSequence* AnimationSequence;

	// Movement characteristics
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
	float Speed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
	float Direction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
	bool bIsAccelerating;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
	bool bIsTurning;

	// Quality score for matching
	UPROPERTY(BlueprintReadOnly, Category = "Motion Data")
	float MatchScore;

	FAnim_MotionData()
	{
		AnimationSequence = nullptr;
		Speed = 0.0f;
		Direction = 0.0f;
		bIsAccelerating = false;
		bIsTurning = false;
		MatchScore = 0.0f;
	}
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_MotionMatchingSystem : public UActorComponent
{
	GENERATED_BODY()

public:
	UAnim_MotionMatchingSystem();

	// Motion database
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
	TArray<FAnim_MotionData> MotionDatabase;

	// Matching weights
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
	float SpeedWeight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
	float DirectionWeight = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
	float AccelerationWeight = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
	float TurningWeight = 0.7f;

	// Current best match
	UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
	FAnim_MotionData CurrentBestMatch;

	// Motion matching functions
	UFUNCTION(BlueprintCallable, Category = "Motion Matching")
	FAnim_MotionData FindBestMotionMatch(float CurrentSpeed, float CurrentDirection, bool bCurrentlyAccelerating, bool bCurrentlyTurning);

	UFUNCTION(BlueprintCallable, Category = "Motion Matching")
	void AddMotionToDatabase(UAnimSequence* Animation, float Speed, float Direction, bool bAccelerating, bool bTurning);

	UFUNCTION(BlueprintCallable, Category = "Motion Matching")
	void ClearMotionDatabase();

	UFUNCTION(BlueprintCallable, Category = "Motion Matching")
	void InitializeDefaultMotions();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	// Cached references
	UPROPERTY()
	class UCharacterMovementComponent* MovementComponent;

	UPROPERTY()
	class USkeletalMeshComponent* SkeletalMeshComponent;

	// Motion matching calculation
	float CalculateMotionScore(const FAnim_MotionData& MotionData, float Speed, float Direction, bool bAccelerating, bool bTurning);
	void UpdateCurrentMotion();

	// Previous frame data for comparison
	float PreviousSpeed;
	float PreviousDirection;
	bool bPreviouslyAccelerating;
	bool bPreviouslyTurning;
};