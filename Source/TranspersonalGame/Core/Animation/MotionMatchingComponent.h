#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimSequence.h"
#include "Animation/PoseAsset.h"
#include "GameFramework/Character.h"
#include "MotionMatchingComponent.generated.h"

USTRUCT(BlueprintType)
struct FMotionFeature
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Feature")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Feature")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Feature")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Feature")
    float Timestamp;

    FMotionFeature()
    {
        Position = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Timestamp = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FMotionCandidate
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Candidate")
    UAnimSequence* Animation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Candidate")
    float StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Candidate")
    float Score;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Candidate")
    FMotionFeature Feature;

    FMotionCandidate()
    {
        Animation = nullptr;
        StartTime = 0.0f;
        Score = FLT_MAX;
    }
};

USTRUCT(BlueprintType)
struct FTrajectoryPoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
    float Time;

    FTrajectoryPoint()
    {
        Position = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        Time = 0.0f;
    }
};

/**
 * Componente especializado para Motion Matching
 * Implementa algoritmos de busca e seleção de animações baseados em features de movimento
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UMotionMatchingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMotionMatchingComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Motion Matching Core Functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void InitializeMotionDatabase(const TArray<UAnimSequence*>& Animations);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FMotionCandidate FindBestMatch(const FMotionFeature& QueryFeature);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateCurrentQuery(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetMotionMatchingEnabled(bool bEnabled);

    // Trajectory System
    UFUNCTION(BlueprintCallable, Category = "Trajectory")
    void UpdateTrajectory(const FVector& InputVector, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Trajectory")
    TArray<FTrajectoryPoint> GetPredictedTrajectory(float PredictionTime, int32 NumSamples);

    UFUNCTION(BlueprintCallable, Category = "Trajectory")
    void DrawTrajectoryDebug(bool bDraw);

    // Feature Extraction
    UFUNCTION(BlueprintCallable, Category = "Feature Extraction")
    FMotionFeature ExtractCurrentFeature();

    UFUNCTION(BlueprintCallable, Category = "Feature Extraction")
    FMotionFeature ExtractFeatureFromAnimation(UAnimSequence* Animation, float Time);

    UFUNCTION(BlueprintCallable, Category = "Feature Extraction")
    void SetFeatureWeights(float PositionWeight, float VelocityWeight, float RotationWeight);

    // Database Management
    UFUNCTION(BlueprintCallable, Category = "Database")
    void AddAnimationToDatabase(UAnimSequence* Animation);

    UFUNCTION(BlueprintCallable, Category = "Database")
    void RemoveAnimationFromDatabase(UAnimSequence* Animation);

    UFUNCTION(BlueprintCallable, Category = "Database")
    void RebuildDatabase();

    UFUNCTION(BlueprintCallable, Category = "Database")
    int32 GetDatabaseSize() const { return MotionDatabase.Num(); }

    // Performance and Quality
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetSearchQuality(int32 Quality);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetMaxCandidates(int32 MaxCandidates);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableAdaptiveQuality(bool bEnable);

protected:
    // Motion Matching Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bMotionMatchingEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float BlendTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float SearchRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    int32 MaxSearchCandidates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float MinimumScore;

    // Feature Weights
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feature Weights")
    float PositionWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feature Weights")
    float VelocityWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feature Weights")
    float RotationWeight;

    // Trajectory Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
    int32 TrajectoryHistorySize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
    float TrajectoryPredictionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
    int32 TrajectoryPredictionSamples;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
    bool bDrawTrajectoryDebug;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 SearchQuality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bAdaptiveQualityEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float QualityAdaptationThreshold;

private:
    // Motion Database
    TArray<FMotionCandidate> MotionDatabase;
    TArray<UAnimSequence*> SourceAnimations;

    // Current State
    FMotionFeature CurrentQuery;
    FMotionCandidate LastSelectedCandidate;
    TArray<FTrajectoryPoint> TrajectoryHistory;
    
    // Character Reference
    UPROPERTY()
    ACharacter* OwnerCharacter;

    // Performance Tracking
    float LastUpdateTime;
    float AverageSearchTime;
    int32 FramesSinceLastUpdate;

    // Helper Functions
    void BuildMotionDatabase();
    float CalculateFeatureDistance(const FMotionFeature& A, const FMotionFeature& B);
    void UpdateTrajectoryHistory(const FVector& CurrentPosition, const FVector& CurrentVelocity);
    FVector PredictFuturePosition(float Time);
    void OptimizeSearchQuality();
    void ValidateDatabase();
    
    // Debug Functions
    void DrawFeatureDebug(const FMotionFeature& Feature, const FColor& Color);
    void LogMotionMatchingStats();
};