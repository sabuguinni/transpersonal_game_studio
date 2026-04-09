// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "PoseSearch/PoseSearchSchema.h"
#include "PoseSearch/PoseSearchLibrary.h"
#include "Animation/AnimSequence.h"
#include "Animation/BlendSpace.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "MotionMatchingSystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMotionMatching, Log, All);

/**
 * Motion Matching Database Configuration
 * Defines different animation databases for different character states
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FMotionMatchingDatabaseConfig
{
    GENERATED_BODY()

    // Database asset reference
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Database")
    class UPoseSearchDatabase* Database;

    // When this database should be active
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activation")
    float MinSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activation")
    float MaxSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activation")
    bool bRequireGrounded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activation")
    bool bRequireInAir;

    // Blend settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blending")
    float BlendTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blending")
    float Priority;

    FMotionMatchingDatabaseConfig()
    {
        Database = nullptr;
        MinSpeed = 0.0f;
        MaxSpeed = 1000.0f;
        bRequireGrounded = false;
        bRequireInAir = false;
        BlendTime = 0.2f;
        Priority = 1.0f;
    }
};

/**
 * Character Movement Profile
 * Defines how a character moves - their unique "gait signature"
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCharacterMovementProfile
{
    GENERATED_BODY()

    // Basic movement characteristics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Style")
    float StrideLength; // How long their steps are

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Style")
    float StepFrequency; // How fast they step

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Style")
    float PosturalSway; // How much they sway while walking

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Style")
    float ArmSwing; // How much their arms swing

    // Personality-driven movement
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Confidence; // Affects chest position, stride confidence

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Nervousness; // Affects fidgeting, irregular timing

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Determination; // Affects forward lean, purposeful movement

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Caution; // Affects head movement, step placement

    // Physical characteristics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    float Age; // Affects movement speed and stability

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    float Fitness; // Affects endurance and movement efficiency

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    float Injury; // Creates asymmetric movement patterns

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    float Fatigue; // Accumulates over time, affects everything

    FCharacterMovementProfile()
    {
        StrideLength = 1.0f;
        StepFrequency = 1.0f;
        PosturalSway = 0.5f;
        ArmSwing = 1.0f;
        
        Confidence = 0.7f;
        Nervousness = 0.2f;
        Determination = 0.8f;
        Caution = 0.5f;
        
        Age = 0.5f; // 0 = young, 1 = old
        Fitness = 0.8f;
        Injury = 0.0f;
        Fatigue = 0.0f;
    }
};

/**
 * Motion Matching System
 * Handles query-based animation selection using character movement profiles
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UMotionMatchingSystem : public UObject
{
    GENERATED_BODY()

public:
    UMotionMatchingSystem();

    // Database Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Databases")
    TArray<FMotionMatchingDatabaseConfig> DatabaseConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schema")
    class UPoseSearchSchema* DefaultSchema;

    // Character Profile
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Profile")
    FCharacterMovementProfile MovementProfile;

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    class UPoseSearchDatabase* ActiveDatabase;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    float CurrentBlendWeight;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    int32 SelectedPoseIndex;

    // Query Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query")
    float QueryRadius; // How far ahead/behind to look

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query")
    float TrajectoryWeight; // How much to weight future trajectory

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query")
    float PoseWeight; // How much to weight current pose

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query")
    float VelocityWeight; // How much to weight velocity matching

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency; // How often to run motion matching (Hz)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxCandidates; // Maximum poses to consider per query

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableDebugDraw;

    // Core Functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionMatching(class ACharacter* Character, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    class UPoseSearchDatabase* SelectBestDatabase(float Speed, bool bIsGrounded, bool bIsInAir);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    int32 QueryBestPose(class UPoseSearchDatabase* Database, const FVector& Velocity, const FVector& Acceleration);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void ApplyMovementProfile(FVector& Velocity, FRotator& Rotation, float DeltaTime);

    // Character Profile Functions
    UFUNCTION(BlueprintCallable, Category = "Character Profile")
    void UpdateFatigue(float MovementIntensity, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Character Profile")
    void ApplyPersonalityToMovement(FVector& MovementVector, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Character Profile")
    void ApplyPhysicalTraitsToMovement(FVector& MovementVector, float DeltaTime);

    // Trajectory Prediction
    UFUNCTION(BlueprintCallable, Category = "Trajectory")
    TArray<FVector> PredictTrajectory(const FVector& CurrentVelocity, const FVector& InputVector, float PredictionTime);

    UFUNCTION(BlueprintCallable, Category = "Trajectory")
    FVector CalculateDesiredVelocity(const FVector& InputVector, float MaxSpeed);

    // Database Utilities
    UFUNCTION(BlueprintCallable, Category = "Database Utilities")
    void AddDatabaseConfig(class UPoseSearchDatabase* Database, float MinSpeed, float MaxSpeed, float BlendTime);

    UFUNCTION(BlueprintCallable, Category = "Database Utilities")
    void RemoveDatabaseConfig(class UPoseSearchDatabase* Database);

    UFUNCTION(BlueprintCallable, Category = "Database Utilities")
    void SortDatabasesByPriority();

    // Debug Functions
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawDebugInfo(class UWorld* World, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogMotionMatchingState();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    FString GetCurrentStateDescription();

protected:
    // Internal state
    float TimeSinceLastUpdate;
    FVector LastVelocity;
    FVector LastAcceleration;
    float LastSpeed;
    
    // Performance optimization
    int32 FramesSinceLastQuery;
    bool bNeedsUpdate;
    
    // Trajectory history for better matching
    TArray<FVector> TrajectoryHistory;
    int32 MaxTrajectoryHistorySize;

private:
    // Internal helper functions
    float CalculateDatabaseScore(const FMotionMatchingDatabaseConfig& Config, float Speed, bool bIsGrounded, bool bIsInAir);
    void UpdateTrajectoryHistory(const FVector& CurrentVelocity);
    FVector ApplyPersonalityNoise(const FVector& BaseMovement, float DeltaTime);
};

/**
 * Motion Matching Animation Node
 * Custom animation node that integrates with the Motion Matching System
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnimNode_MotionMatching
{
    GENERATED_BODY()

    // Input pose (usually from previous nodes)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    struct FPoseLink BasePose;

    // Motion Matching System reference
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UMotionMatchingSystem* MotionMatchingSystem;

    // Override settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bOverrideBlendTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (EditCondition = "bOverrideBlendTime"))
    float BlendTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bIgnoreRootMotion;

    // Debug settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowDebugInfo;

    FAnimNode_MotionMatching()
    {
        MotionMatchingSystem = nullptr;
        bOverrideBlendTime = false;
        BlendTime = 0.2f;
        bIgnoreRootMotion = false;
        bShowDebugInfo = false;
    }
};

/**
 * Protagonist-specific Motion Matching System
 * Adds scientific observation behaviors and stress responses
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UProtagonistMotionMatching : public UMotionMatchingSystem
{
    GENERATED_BODY()

public:
    UProtagonistMotionMatching();

    // Protagonist-specific states
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Protagonist State")
    bool bIsObservingDinosaur;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Protagonist State")
    float StressLevel; // 0-1, affects movement jitter

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Protagonist State")
    float CuriosityLevel; // 0-1, affects head movement frequency

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Protagonist State")
    bool bIsHoldingTool;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Protagonist State")
    bool bIsCrafting;

    // Scientific behavior databases
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scientific Databases")
    class UPoseSearchDatabase* ObservationDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scientific Databases")
    class UPoseSearchDatabase* CraftingDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scientific Databases")
    class UPoseSearchDatabase* ToolUseDatabase;

    // Stress response parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stress Response")
    float StressMovementJitter; // How much stress affects movement precision

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stress Response")
    float StressBreathingRate; // Affects idle animation breathing

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stress Response")
    float StressFidgeting; // Frequency of fidget animations

    // Override base update function
    virtual void UpdateMotionMatching(class ACharacter* Character, float DeltaTime) override;

    // Protagonist-specific functions
    UFUNCTION(BlueprintCallable, Category = "Protagonist Behavior")
    void UpdateStressLevel(float ThreatLevel, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Protagonist Behavior")
    void ApplyStressToMovement(FVector& MovementVector, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Protagonist Behavior")
    void UpdateScientificBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Protagonist Behavior")
    class UPoseSearchDatabase* SelectScientificDatabase();

protected:
    // Internal stress tracking
    float StressAccumulation;
    float LastThreatLevel;
    float TimeSinceLastThreat;

    // Scientific behavior timing
    float ObservationTimer;
    float CraftingTimer;
    bool bInScientificMode;
};