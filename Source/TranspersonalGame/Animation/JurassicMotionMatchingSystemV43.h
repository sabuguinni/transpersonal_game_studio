#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "PoseSearch/PoseSearchSchema.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "JurassicMotionMatchingSystemV43.generated.h"

class ACharacter;
class UCharacterMovementComponent;
class UPoseSearchDatabase;
class UPoseSearchSchema;

/**
 * Character Movement Archetype Definitions
 * Each archetype has unique movement characteristics and body language
 */
UENUM(BlueprintType)
enum class EJurassicMovementArchetype : uint8
{
    Paleontologist      UMETA(DisplayName = "Paleontologist - Scientific Observer"),
    TribalSurvivor      UMETA(DisplayName = "Tribal Survivor - Natural Navigator"), 
    ModernLost          UMETA(DisplayName = "Modern Lost - Urban Displaced"),
    ElderWise           UMETA(DisplayName = "Elder Wise - Ancient Knowledge"),
    YoungAdaptive       UMETA(DisplayName = "Young Adaptive - Quick Learner")
};

/**
 * Emotional States that affect movement and body language
 * Based on transpersonal psychology and survival psychology
 */
UENUM(BlueprintType)
enum class EJurassicEmotionalState : uint8
{
    Neutral             UMETA(DisplayName = "Neutral - Baseline State"),
    Curious             UMETA(DisplayName = "Curious - Wonder and Exploration"),
    Cautious            UMETA(DisplayName = "Cautious - Heightened Awareness"),
    Fearful             UMETA(DisplayName = "Fearful - Threat Response"),
    Determined          UMETA(DisplayName = "Determined - Goal-Focused"),
    Exhausted           UMETA(DisplayName = "Exhausted - Physical Depletion"),
    Hopeful             UMETA(DisplayName = "Hopeful - Positive Expectation"),
    Desperate           UMETA(DisplayName = "Desperate - Survival Mode"),
    Transcendent        UMETA(DisplayName = "Transcendent - Spiritual Connection")
};

/**
 * Terrain Adaptation Levels for IK foot placement
 */
UENUM(BlueprintType)
enum class EJurassicTerrainType : uint8
{
    FlatGround          UMETA(DisplayName = "Flat Ground - Minimal Adaptation"),
    RockyUneven         UMETA(DisplayName = "Rocky Uneven - High Adaptation"),
    SteepIncline        UMETA(DisplayName = "Steep Incline - Forward Lean"),
    SteepDecline        UMETA(DisplayName = "Steep Decline - Backward Lean"),
    MuddySoft           UMETA(DisplayName = "Muddy Soft - Careful Placement"),
    WaterShallow        UMETA(DisplayName = "Water Shallow - Splash Avoidance"),
    VegetationDense     UMETA(DisplayName = "Vegetation Dense - Path Finding")
};

/**
 * Motion Matching Database Configuration
 * Defines which animation database to use for different movement contexts
 */
USTRUCT(BlueprintType)
struct FJurassicMotionMatchingConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TSoftObjectPtr<UPoseSearchDatabase> LocomotionDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TSoftObjectPtr<UPoseSearchDatabase> StealthDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TSoftObjectPtr<UPoseSearchDatabase> InteractionDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TSoftObjectPtr<UPoseSearchDatabase> EmotionalDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TSoftObjectPtr<UPoseSearchDatabase> CombatDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TSoftObjectPtr<UPoseSearchSchema> DefaultSchema;

    FJurassicMotionMatchingConfig()
    {
        // Default constructor
    }
};

/**
 * Character Trajectory Data for Motion Matching
 * Stores past and predicted movement for pose selection
 */
USTRUCT(BlueprintType)
struct FJurassicCharacterTrajectory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Trajectory")
    TArray<FVector> HistoryPositions;

    UPROPERTY(BlueprintReadOnly, Category = "Trajectory")
    TArray<FVector> PredictedPositions;

    UPROPERTY(BlueprintReadOnly, Category = "Trajectory")
    TArray<FVector> HistoryVelocities;

    UPROPERTY(BlueprintReadOnly, Category = "Trajectory")
    TArray<FVector> PredictedVelocities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
    float PredictionTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
    float HistoryTime = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
    int32 SampleRate = 12;

    FJurassicCharacterTrajectory()
    {
        PredictionTime = 2.0f;
        HistoryTime = 1.0f;
        SampleRate = 12;
    }
};

/**
 * IK Foot Placement Data for terrain adaptation
 */
USTRUCT(BlueprintType)
struct FJurassicFootIKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Foot IK")
    FVector LeftFootPosition;

    UPROPERTY(BlueprintReadWrite, Category = "Foot IK")
    FVector RightFootPosition;

    UPROPERTY(BlueprintReadWrite, Category = "Foot IK")
    FRotator LeftFootRotation;

    UPROPERTY(BlueprintReadWrite, Category = "Foot IK")
    FRotator RightFootRotation;

    UPROPERTY(BlueprintReadWrite, Category = "Foot IK")
    float LeftFootAlpha = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Foot IK")
    float RightFootAlpha = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Foot IK")
    float HipOffset = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Foot IK")
    EJurassicTerrainType CurrentTerrainType = EJurassicTerrainType::FlatGround;

    FJurassicFootIKData()
    {
        LeftFootPosition = FVector::ZeroVector;
        RightFootPosition = FVector::ZeroVector;
        LeftFootRotation = FRotator::ZeroRotator;
        RightFootRotation = FRotator::ZeroRotator;
        LeftFootAlpha = 0.0f;
        RightFootAlpha = 0.0f;
        HipOffset = 0.0f;
        CurrentTerrainType = EJurassicTerrainType::FlatGround;
    }
};

/**
 * Archetype-Specific Animation Parameters
 * Defines how each character type moves and expresses themselves
 */
USTRUCT(BlueprintType)
struct FJurassicArchetypeAnimationData
{
    GENERATED_BODY()

    // Movement characteristics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float BaseWalkSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float BaseRunSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float StealthSpeedMultiplier = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float PanicSpeedMultiplier = 1.3f;

    // Body language parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body Language")
    float PostureConfidence = 0.5f; // 0 = hunched, 1 = upright

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body Language")
    float GestureFrequency = 0.5f; // How often character makes gestures

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body Language")
    float AlertnessLevel = 0.5f; // How alert/scanning the character is

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body Language")
    float MovementEfficiency = 0.5f; // How economical their movements are

    // Emotional response modifiers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Response")
    float FearSensitivity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Response")
    float CuriositySensitivity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Response")
    float AdaptabilityRate = 0.5f;

    FJurassicArchetypeAnimationData()
    {
        BaseWalkSpeed = 150.0f;
        BaseRunSpeed = 400.0f;
        StealthSpeedMultiplier = 0.5f;
        PanicSpeedMultiplier = 1.3f;
        PostureConfidence = 0.5f;
        GestureFrequency = 0.5f;
        AlertnessLevel = 0.5f;
        MovementEfficiency = 0.5f;
        FearSensitivity = 0.5f;
        CuriositySensitivity = 0.5f;
        AdaptabilityRate = 0.5f;
    }
};

/**
 * Jurassic Motion Matching System V43
 * 
 * Advanced character animation system that combines Motion Matching with
 * archetype-specific movement patterns and emotional body language.
 * 
 * Key Features:
 * - Motion Matching for fluid, responsive movement
 * - Archetype-based movement personalities
 * - Emotional state-driven body language
 * - Terrain-adaptive IK foot placement
 * - Contextual animation database selection
 * 
 * This system ensures every character moves with purpose and personality,
 * reflecting both their background and current emotional state.
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UJurassicMotionMatchingSystemV43 : public UActorComponent
{
    GENERATED_BODY()

public:
    UJurassicMotionMatchingSystemV43();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === CORE MOTION MATCHING INTERFACE ===

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void InitializeMotionMatchingSystem(ACharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetCharacterArchetype(EJurassicMovementArchetype NewArchetype);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetEmotionalState(EJurassicEmotionalState NewState, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateTerrainAdaptation(EJurassicTerrainType TerrainType);

    // === ANIMATION DATABASE MANAGEMENT ===

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    UPoseSearchDatabase* GetActiveDatabase() const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetActiveDatabase(UPoseSearchDatabase* NewDatabase);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SwitchToDatabaseByContext(const FString& Context);

    // === TRAJECTORY SYSTEM ===

    UFUNCTION(BlueprintCallable, Category = "Trajectory")
    void UpdateCharacterTrajectory(float DeltaTime);

    UFUNCTION(BlueprintPure, Category = "Trajectory")
    FJurassicCharacterTrajectory GetCurrentTrajectory() const { return CharacterTrajectory; }

    UFUNCTION(BlueprintCallable, Category = "Trajectory")
    void PredictMovementTrajectory(float PredictionTime);

    // === IK FOOT PLACEMENT ===

    UFUNCTION(BlueprintCallable, Category = "IK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintPure, Category = "IK")
    FJurassicFootIKData GetFootIKData() const { return FootIKData; }

    UFUNCTION(BlueprintCallable, Category = "IK")
    void PerformFootTrace(bool bLeftFoot, FVector& OutFootPosition, FRotator& OutFootRotation, float& OutAlpha);

    // === ARCHETYPE SYSTEM ===

    UFUNCTION(BlueprintPure, Category = "Archetype")
    EJurassicMovementArchetype GetCurrentArchetype() const { return CurrentArchetype; }

    UFUNCTION(BlueprintPure, Category = "Archetype")
    FJurassicArchetypeAnimationData GetArchetypeData() const;

    UFUNCTION(BlueprintCallable, Category = "Archetype")
    void ApplyArchetypeModifiers();

    // === EMOTIONAL SYSTEM ===

    UFUNCTION(BlueprintPure, Category = "Emotional")
    EJurassicEmotionalState GetCurrentEmotionalState() const { return CurrentEmotionalState; }

    UFUNCTION(BlueprintPure, Category = "Emotional")
    float GetEmotionalIntensity() const { return EmotionalIntensity; }

    UFUNCTION(BlueprintCallable, Category = "Emotional")
    void BlendEmotionalStates(EJurassicEmotionalState TargetState, float BlendRate);

    // === DEBUGGING AND VISUALIZATION ===

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawDebugTrajectory(bool bShowHistory = true, bool bShowPrediction = true);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawDebugFootIK();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogAnimationState();

protected:
    // === CORE REFERENCES ===

    UPROPERTY()
    TObjectPtr<ACharacter> OwnerCharacter;

    UPROPERTY()
    TObjectPtr<UCharacterMovementComponent> MovementComponent;

    UPROPERTY()
    TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;

    // === MOTION MATCHING CONFIGURATION ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FJurassicMotionMatchingConfig MotionMatchingConfig;

    UPROPERTY()
    TObjectPtr<UPoseSearchDatabase> ActiveDatabase;

    // === CHARACTER STATE ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character State")
    EJurassicMovementArchetype CurrentArchetype = EJurassicMovementArchetype::Paleontologist;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character State")
    EJurassicEmotionalState CurrentEmotionalState = EJurassicEmotionalState::Neutral;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character State")
    float EmotionalIntensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character State")
    EJurassicTerrainType CurrentTerrainType = EJurassicTerrainType::FlatGround;

    // === TRAJECTORY DATA ===

    UPROPERTY(BlueprintReadOnly, Category = "Trajectory")
    FJurassicCharacterTrajectory CharacterTrajectory;

    // === IK DATA ===

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FJurassicFootIKData FootIKData;

    // === ARCHETYPE CONFIGURATIONS ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetypes")
    TMap<EJurassicMovementArchetype, FJurassicArchetypeAnimationData> ArchetypeConfigurations;

    // === RUNTIME PARAMETERS ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Runtime")
    float IKTraceDistance = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Runtime")
    float IKInterpSpeed = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Runtime")
    float EmotionalBlendSpeed = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Runtime")
    bool bEnableDebugDrawing = false;

private:
    // === INTERNAL HELPER FUNCTIONS ===

    void InitializeArchetypeConfigurations();
    void UpdateMotionMatchingQuery(float DeltaTime);
    void UpdateEmotionalBlending(float DeltaTime);
    void CalculateTerrainAdaptation();
    FVector PredictFuturePosition(float TimeAhead) const;
    void SampleTrajectoryHistory(float DeltaTime);
};