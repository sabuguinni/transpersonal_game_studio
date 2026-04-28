// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Animation/AnimSequence.h"
#include "Animation/PoseAsset.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "PoseSearch/PoseSearchSchema.h"
#include "MotionMatchingDatabaseV44.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FMotionMatchingAnimData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimSequence* AnimSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    FString AnimationTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float Weight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bMirrorable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    FVector2D SpeedRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    FVector2D DirectionRange;

    FMotionMatchingAnimData()
    {
        AnimSequence = nullptr;
        Weight = 1.0f;
        bMirrorable = false;
        SpeedRange = FVector2D(0.0f, 600.0f);
        DirectionRange = FVector2D(-180.0f, 180.0f);
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCharacterArchetypeMotionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
    FString ArchetypeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    TArray<FMotionMatchingAnimData> IdleAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    TArray<FMotionMatchingAnimData> WalkAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    TArray<FMotionMatchingAnimData> RunAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    TArray<FMotionMatchingAnimData> TurnAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    TArray<FMotionMatchingAnimData> StartAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    TArray<FMotionMatchingAnimData> StopAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional")
    TArray<FMotionMatchingAnimData> EmotionalVariations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    TArray<FMotionMatchingAnimData> SurvivalAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    TArray<FMotionMatchingAnimData> InteractionAnimations;
};

/**
 * Advanced Motion Matching Database for Jurassic survival game
 * Supports character archetypes, emotional states, and survival behaviors
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UMotionMatchingDatabaseV44 : public UDataAsset
{
    GENERATED_BODY()

public:
    UMotionMatchingDatabaseV44();

    // Core database configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Database")
    UPoseSearchDatabase* PoseSearchDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Database")
    UPoseSearchSchema* PoseSearchSchema;

    // Character archetype data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetypes")
    TArray<FCharacterArchetypeMotionData> ArchetypeMotionData;

    // Global animation settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float BlendTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float SearchThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    int32 MaxSearchResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bEnableDebugVisualization;

    // Runtime query parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query")
    float TrajectoryWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query")
    float PoseWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query")
    float VelocityWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query")
    float AngularVelocityWeight;

    // Emotional state modifiers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float FearIntensityMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float CalmIntensityMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float AlertnessMultiplier;

    // Survival context modifiers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float HungerAnimationBias;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float FatigueAnimationBias;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float InjuryAnimationBias;

    // Blueprint interface
// [UHT-FIX]     UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FCharacterArchetypeMotionData* GetArchetypeData(const FString& ArchetypeName);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    TArray<FMotionMatchingAnimData> GetAnimationsForState(const FString& ArchetypeName, const FString& StateName);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateDatabaseWeights(float NewTrajectoryWeight, float NewPoseWeight, float NewVelocityWeight);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetEmotionalModifiers(float Fear, float Calm, float Alertness);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetSurvivalModifiers(float Hunger, float Fatigue, float Injury);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    bool ValidateDatabase();

    // Editor utilities
#if WITH_EDITOR
    UFUNCTION(CallInEditor, Category = "Database")
    void RebuildDatabase();

    UFUNCTION(CallInEditor, Category = "Database")
    void ValidateAllAnimations();

    UFUNCTION(CallInEditor, Category = "Database")
    void GenerateArchetypeFromAnimations();
#endif

protected:
    // Internal helper functions
    void InitializeDefaultSettings();
    void BuildPoseSearchDatabase();
    void ProcessAnimationTags(const FString& Tags, FMotionMatchingAnimData& AnimData);

    // Runtime caching
    UPROPERTY(Transient)
    TMap<FString, int32> ArchetypeIndexCache;

    UPROPERTY(Transient)
    bool bDatabaseInitialized;
};