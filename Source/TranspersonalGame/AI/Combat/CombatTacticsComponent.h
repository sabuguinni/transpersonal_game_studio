#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameplayTags.h"
#include "CombatTacticsComponent.generated.h"

UENUM(BlueprintType)
enum class ETacticalManeuver : uint8
{
    None,
    DirectAssault,
    FlankingLeft,
    FlankingRight,
    Ambush,
    Retreat,
    CircleStrafe,
    FeintAttack,
    PincerMovement,
    HitAndRun,
    Surround
};

UENUM(BlueprintType)
enum class ECombatFormation : uint8
{
    None,
    Line,
    Wedge,
    Circle,
    Column,
    Scattered,
    Pincer,
    Ambush
};

USTRUCT(BlueprintType)
struct FTacticalPosition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    FVector Position = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    float AdvantageScore = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    bool bHasCover = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    bool bHasEscapeRoute = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    float DistanceToTarget = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    float SafetyRating = 0.0f;

    FTacticalPosition()
    {
        Position = FVector::ZeroVector;
        AdvantageScore = 0.0f;
        bHasCover = false;
        bHasEscapeRoute = false;
        DistanceToTarget = 0.0f;
        SafetyRating = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FCombatPattern
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patterns")
    ETacticalManeuver PreferredManeuver = ETacticalManeuver::DirectAssault;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patterns")
    float SuccessRate = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patterns")
    int32 TimesUsed = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patterns")
    float AverageEffectiveness = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patterns")
    TArray<FGameplayTag> RequiredConditions;

    FCombatPattern()
    {
        PreferredManeuver = ETacticalManeuver::DirectAssault;
        SuccessRate = 0.0f;
        TimesUsed = 0;
        AverageEffectiveness = 0.0f;
    }
};

/**
 * Component that handles tactical decision-making for combat AI
 * Analyzes battlefield conditions and selects optimal maneuvers
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombatTacticsComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombatTacticsComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Tactical Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    float TacticalUpdateInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    float PositionAnalysisRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    int32 MaxTacticalPositions = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    float CoverDetectionRadius = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    float FlankingAngle = 90.0f;

    // Current Tactical State
    UPROPERTY(BlueprintReadOnly, Category = "Tactics")
    ETacticalManeuver CurrentManeuver;

    UPROPERTY(BlueprintReadOnly, Category = "Tactics")
    ECombatFormation CurrentFormation;

    UPROPERTY(BlueprintReadOnly, Category = "Tactics")
    TArray<FTacticalPosition> AnalyzedPositions;

    UPROPERTY(BlueprintReadOnly, Category = "Tactics")
    FTacticalPosition OptimalPosition;

    // Combat Learning
    UPROPERTY(BlueprintReadOnly, Category = "Learning")
    TMap<ETacticalManeuver, FCombatPattern> LearnedPatterns;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning")
    float LearningRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning")
    bool bEnableLearning = true;

public:
    // Tactical Analysis
    UFUNCTION(BlueprintCallable, Category = "Tactics")
    void AnalyzeBattlefield(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Tactics")
    ETacticalManeuver SelectOptimalManeuver(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Tactics")
    FTacticalPosition FindBestPosition(AActor* Target, ETacticalManeuver Maneuver);

    UFUNCTION(BlueprintPure, Category = "Tactics")
    bool CanExecuteManeuver(ETacticalManeuver Maneuver, AActor* Target) const;

    // Position Analysis
    UFUNCTION(BlueprintCallable, Category = "Tactics")
    TArray<FTacticalPosition> AnalyzePositionsAroundTarget(AActor* Target, float Radius);

    UFUNCTION(BlueprintPure, Category = "Tactics")
    float CalculatePositionAdvantage(FVector Position, AActor* Target) const;

    UFUNCTION(BlueprintPure, Category = "Tactics")
    bool HasCoverAtPosition(FVector Position) const;

    UFUNCTION(BlueprintPure, Category = "Tactics")
    bool HasEscapeRouteFromPosition(FVector Position) const;

    // Maneuver Execution
    UFUNCTION(BlueprintCallable, Category = "Tactics")
    void ExecuteManeuver(ETacticalManeuver Maneuver, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Tactics")
    FVector CalculateFlankingPosition(AActor* Target, bool bLeftFlank = true);

    UFUNCTION(BlueprintCallable, Category = "Tactics")
    FVector CalculateAmbushPosition(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Tactics")
    FVector CalculateRetreatPosition();

    // Formation Management
    UFUNCTION(BlueprintCallable, Category = "Formation")
    void SetFormation(ECombatFormation Formation);

    UFUNCTION(BlueprintCallable, Category = "Formation")
    FVector GetFormationPosition(int32 UnitIndex, ECombatFormation Formation);

    UFUNCTION(BlueprintPure, Category = "Formation")
    bool IsFormationValid(ECombatFormation Formation) const;

    // Learning System
    UFUNCTION(BlueprintCallable, Category = "Learning")
    void RecordManeuverResult(ETacticalManeuver Maneuver, bool bSuccessful, float Effectiveness);

    UFUNCTION(BlueprintCallable, Category = "Learning")
    void UpdateCombatPatterns();

    UFUNCTION(BlueprintPure, Category = "Learning")
    float GetManeuverSuccessRate(ETacticalManeuver Maneuver) const;

    // Utility Functions
    UFUNCTION(BlueprintPure, Category = "Tactics")
    float GetDistanceToTarget(AActor* Target) const;

    UFUNCTION(BlueprintPure, Category = "Tactics")
    bool HasLineOfSightToTarget(AActor* Target) const;

    UFUNCTION(BlueprintPure, Category = "Tactics")
    FVector GetPredictedTargetPosition(AActor* Target, float PredictionTime) const;

    // Debugging
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugDrawTacticalInfo() const;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugDrawAnalyzedPositions() const;

protected:
    // Internal tactical logic
    void InitializeTacticalPatterns();
    void UpdateTacticalAnalysis();
    ETacticalManeuver EvaluateBestManeuver(AActor* Target);
    float CalculateManeuverScore(ETacticalManeuver Maneuver, AActor* Target);
    
    // Position evaluation
    float EvaluatePosition(FVector Position, AActor* Target);
    bool IsPositionSafe(FVector Position);
    bool IsPositionNavigable(FVector Position);
    
    // Combat intelligence
    void AdaptTactics(AActor* Target);
    void AnalyzeEnemyBehavior(AActor* Target);
    void PredictEnemyMovement(AActor* Target);

    // Timers
    float TacticalUpdateTimer = 0.0f;
    float LearningUpdateTimer = 0.0f;

    // Cached references
    class ACombatAIController* OwnerController;
    class UWorld* CachedWorld;
    class UNavigationSystemV1* NavSystem;

    // Combat history for learning
    TArray<ETacticalManeuver> RecentManeuvers;
    TArray<float> RecentEffectiveness;
    TMap<AActor*, FVector> EnemyMovementHistory;
};