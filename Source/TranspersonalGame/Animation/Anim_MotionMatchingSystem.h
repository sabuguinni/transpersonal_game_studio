#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Animation/AnimSequence.h"
#include "Animation/BlendSpace.h"
#include "../SharedTypes.h"
#include "Anim_MotionMatchingSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    UAnimSequence* AnimSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    float StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    float EndTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    float Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    ESurvivalState SurvivalState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    EMovementType MovementType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    float MatchingScore;

    FAnim_MotionData()
    {
        AnimSequence = nullptr;
        StartTime = 0.0f;
        EndTime = 0.0f;
        Velocity = FVector::ZeroVector;
        Speed = 0.0f;
        Direction = 0.0f;
        SurvivalState = ESurvivalState::Normal;
        MovementType = EMovementType::Walking;
        MatchingScore = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionQuery
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Query")
    FVector DesiredVelocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Query")
    float DesiredSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Query")
    float DesiredDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Query")
    ESurvivalState CurrentSurvivalState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Query")
    EMovementType PreferredMovementType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Query")
    float TerrainSteepness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Query")
    bool bIsInCombat;

    FAnim_MotionQuery()
    {
        DesiredVelocity = FVector::ZeroVector;
        DesiredSpeed = 0.0f;
        DesiredDirection = 0.0f;
        CurrentSurvivalState = ESurvivalState::Normal;
        PreferredMovementType = EMovementType::Walking;
        TerrainSteepness = 0.0f;
        bIsInCombat = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_MotionMatchingSystem : public UObject
{
    GENERATED_BODY()

public:
    UAnim_MotionMatchingSystem();

    // Core motion matching functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_MotionData FindBestMotionMatch(const FAnim_MotionQuery& Query);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void BuildMotionDatabase(const TArray<UAnimSequence*>& AnimSequences);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void AddMotionData(UAnimSequence* AnimSequence, float SampleRate = 30.0f);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculateMotionScore(const FAnim_MotionData& MotionData, const FAnim_MotionQuery& Query);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void ClearMotionDatabase();

    // Animation blending functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculateBlendWeight(const FAnim_MotionData& MotionA, const FAnim_MotionData& MotionB, float BlendTime);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    TArray<FAnim_MotionData> GetTopMotionMatches(const FAnim_MotionQuery& Query, int32 MaxResults = 3);

protected:
    // Motion database
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Database")
    TArray<FAnim_MotionData> MotionDatabase;

    // Matching weights
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Matching Weights")
    float VelocityWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Matching Weights")
    float SpeedWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Matching Weights")
    float DirectionWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Matching Weights")
    float SurvivalStateWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Matching Weights")
    float MovementTypeWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Matching Weights")
    float TerrainWeight;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxDatabaseSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MinMatchingThreshold;

private:
    // Helper functions
    void ExtractMotionFeatures(UAnimSequence* AnimSequence, float Time, FAnim_MotionData& OutMotionData);
    float CalculateVelocityScore(const FVector& MotionVelocity, const FVector& QueryVelocity);
    float CalculateDirectionScore(float MotionDirection, float QueryDirection);
    float CalculateStateScore(ESurvivalState MotionState, ESurvivalState QueryState);
    float CalculateMovementTypeScore(EMovementType MotionType, EMovementType QueryType);
    
    // Animation analysis
    FVector ExtractRootMotionVelocity(UAnimSequence* AnimSequence, float Time);
    float ExtractMovementDirection(UAnimSequence* AnimSequence, float Time);
    ESurvivalState DetermineAnimationSurvivalState(UAnimSequence* AnimSequence);
    EMovementType DetermineAnimationMovementType(UAnimSequence* AnimSequence);
};