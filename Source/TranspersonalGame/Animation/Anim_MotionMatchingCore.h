#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "../SharedTypes.h"
#include "Anim_MotionMatchingCore.generated.h"

// Motion matching pose data structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionPoseData
{
    GENERATED_BODY()

    // Pose identifier
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    int32 PoseID = 0;

    // Animation sequence reference
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TSoftObjectPtr<UAnimSequence> AnimSequence;

    // Time within the animation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float AnimTime = 0.0f;

    // Velocity at this pose
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FVector Velocity = FVector::ZeroVector;

    // Acceleration at this pose
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FVector Acceleration = FVector::ZeroVector;

    // Root motion direction
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FVector RootMotionDirection = FVector::ForwardVector;

    // Pose quality score (0-1, higher is better)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float QualityScore = 1.0f;

    // Tags for this pose
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TArray<FString> PoseTags;

    FAnim_MotionPoseData()
    {
        PoseID = 0;
        AnimTime = 0.0f;
        Velocity = FVector::ZeroVector;
        Acceleration = FVector::ZeroVector;
        RootMotionDirection = FVector::ForwardVector;
        QualityScore = 1.0f;
    }
};

// Motion matching search parameters
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionSearchParams
{
    GENERATED_BODY()

    // Desired velocity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FVector DesiredVelocity = FVector::ZeroVector;

    // Desired acceleration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FVector DesiredAcceleration = FVector::ZeroVector;

    // Required tags
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TArray<FString> RequiredTags;

    // Excluded tags
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TArray<FString> ExcludedTags;

    // Velocity weight in matching
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float VelocityWeight = 0.7f;

    // Acceleration weight in matching
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AccelerationWeight = 0.3f;

    FAnim_MotionSearchParams()
    {
        DesiredVelocity = FVector::ZeroVector;
        DesiredAcceleration = FVector::ZeroVector;
        VelocityWeight = 0.7f;
        AccelerationWeight = 0.3f;
    }
};

/**
 * Core Motion Matching system for realistic character animation
 * Implements database-driven pose matching for fluid locomotion
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_MotionMatchingCore : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_MotionMatchingCore();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Motion matching database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TArray<FAnim_MotionPoseData> PoseDatabase;

    // Current best matching pose
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    FAnim_MotionPoseData CurrentPose;

    // Motion matching enabled
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bMotionMatchingEnabled = true;

    // Search frequency (searches per second)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching", meta = (ClampMin = "1.0", ClampMax = "60.0"))
    float SearchFrequency = 30.0f;

    // Minimum pose quality threshold
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MinQualityThreshold = 0.5f;

    // Blend time for pose transitions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BlendTime = 0.2f;

    // Owner character reference
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    ACharacter* OwnerCharacter;

    // Character movement component
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    UCharacterMovementComponent* MovementComponent;

    // Current search parameters
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    FAnim_MotionSearchParams CurrentSearchParams;

    // Search timer
    FTimerHandle SearchTimerHandle;

    // Last search time
    float LastSearchTime = 0.0f;

    /**
     * Initialize motion matching system
     */
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void InitializeMotionMatching();

    /**
     * Add pose to database
     */
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void AddPoseToDatabase(const FAnim_MotionPoseData& PoseData);

    /**
     * Clear pose database
     */
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void ClearPoseDatabase();

    /**
     * Find best matching pose
     */
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_MotionPoseData FindBestMatchingPose(const FAnim_MotionSearchParams& SearchParams);

    /**
     * Update search parameters based on character state
     */
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateSearchParameters();

    /**
     * Perform motion matching search
     */
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void PerformMotionSearch();

    /**
     * Calculate pose match score
     */
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculatePoseMatchScore(const FAnim_MotionPoseData& PoseData, const FAnim_MotionSearchParams& SearchParams);

    /**
     * Get current character velocity
     */
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FVector GetCharacterVelocity() const;

    /**
     * Get current character acceleration
     */
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FVector GetCharacterAcceleration() const;

    /**
     * Check if pose has required tags
     */
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    bool PoseHasRequiredTags(const FAnim_MotionPoseData& PoseData, const TArray<FString>& RequiredTags);

    /**
     * Check if pose has excluded tags
     */
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    bool PoseHasExcludedTags(const FAnim_MotionPoseData& PoseData, const TArray<FString>& ExcludedTags);

    /**
     * Build default pose database
     */
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void BuildDefaultPoseDatabase();

    /**
     * Enable/disable motion matching
     */
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetMotionMatchingEnabled(bool bEnabled);

    /**
     * Set search frequency
     */
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetSearchFrequency(float NewFrequency);

private:
    // Previous velocity for acceleration calculation
    FVector PreviousVelocity = FVector::ZeroVector;

    // Previous frame time
    float PreviousFrameTime = 0.0f;

    /**
     * Timer callback for motion search
     */
    void OnSearchTimer();

    /**
     * Validate pose database
     */
    bool ValidatePoseDatabase() const;

    /**
     * Get normalized velocity difference
     */
    float GetVelocityDifference(const FVector& A, const FVector& B) const;

    /**
     * Get normalized acceleration difference
     */
    float GetAccelerationDifference(const FVector& A, const FVector& B) const;
};