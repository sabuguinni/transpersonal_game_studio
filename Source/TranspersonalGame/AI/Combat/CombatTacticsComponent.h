// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTags.h"
#include "Engine/World.h"
#include "CombatTacticsComponent.generated.h"

UENUM(BlueprintType)
enum class ETacticType : uint8
{
    None,
    DirectAssault,
    Flanking,
    Ambush,
    Stalking,
    PackHunt,
    Retreat,
    Distraction
};

UENUM(BlueprintType)
enum class EFormation : uint8
{
    None,
    Line,
    Circle,
    Wedge,
    Scatter,
    Pincer
};

USTRUCT(BlueprintType)
struct FTacticalPosition
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FVector Position = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite)
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadWrite)
    ETacticType AssignedTactic = ETacticType::None;

    UPROPERTY(BlueprintReadWrite)
    float Priority = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    bool bIsOccupied = false;

    UPROPERTY(BlueprintReadWrite)
    AActor* OccupyingActor = nullptr;

    FTacticalPosition()
    {
        Position = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        AssignedTactic = ETacticType::None;
        Priority = 0.0f;
        bIsOccupied = false;
        OccupyingActor = nullptr;
    }
};

USTRUCT(BlueprintType)
struct FTacticalPlan
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    ETacticType PrimaryTactic = ETacticType::None;

    UPROPERTY(BlueprintReadWrite)
    EFormation Formation = EFormation::None;

    UPROPERTY(BlueprintReadWrite)
    TArray<FTacticalPosition> Positions;

    UPROPERTY(BlueprintReadWrite)
    AActor* TargetActor = nullptr;

    UPROPERTY(BlueprintReadWrite)
    float ExecutionTime = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    float SuccessChance = 0.0f;

    FTacticalPlan()
    {
        PrimaryTactic = ETacticType::None;
        Formation = EFormation::None;
        TargetActor = nullptr;
        ExecutionTime = 0.0f;
        SuccessChance = 0.0f;
    }
};

/**
 * Component that handles tactical combat decisions and positioning
 * Implements advanced AI tactics like flanking, ambushing, and pack coordination
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

    // Tactical Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    float TacticalRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    float FlankingAngle = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    float AmbushDistance = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    float PackCoordinationRadius = 2000.0f;

    // Current Tactical State
    UPROPERTY(BlueprintReadOnly, Category = "Tactics")
    FTacticalPlan CurrentPlan;

    UPROPERTY(BlueprintReadOnly, Category = "Tactics")
    ETacticType ActiveTactic = ETacticType::None;

    UPROPERTY(BlueprintReadOnly, Category = "Tactics")
    FVector TacticalTargetLocation = FVector::ZeroVector;

    // Tactical Intelligence
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    float TacticalIntelligence = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    float AdaptationSpeed = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    bool bCanUseAdvancedTactics = true;

public:
    // Tactical Planning
    UFUNCTION(BlueprintCallable, Category = "Combat Tactics")
    void UpdateTactics(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat Tactics")
    FTacticalPlan GenerateTacticalPlan(AActor* Target, ETacticType PreferredTactic = ETacticType::None);

    UFUNCTION(BlueprintCallable, Category = "Combat Tactics")
    bool ExecuteTacticalPlan(const FTacticalPlan& Plan);

    // Tactical Positioning
    UFUNCTION(BlueprintCallable, Category = "Combat Tactics")
    FVector GetFlankingPosition(AActor* Target, bool bLeftFlank = true);

    UFUNCTION(BlueprintCallable, Category = "Combat Tactics")
    FVector GetAmbushPosition(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat Tactics")
    FVector GetStalkingPosition(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat Tactics")
    TArray<FVector> GetPackFormationPositions(AActor* Target, EFormation Formation, int32 PackSize);

    // Tactical Analysis
    UFUNCTION(BlueprintPure, Category = "Combat Tactics")
    ETacticType GetBestTacticForTarget(AActor* Target) const;

    UFUNCTION(BlueprintPure, Category = "Combat Tactics")
    float EvaluateTacticEffectiveness(ETacticType Tactic, AActor* Target) const;

    UFUNCTION(BlueprintPure, Category = "Combat Tactics")
    bool CanExecuteTactic(ETacticType Tactic, AActor* Target) const;

    // Environmental Analysis
    UFUNCTION(BlueprintCallable, Category = "Combat Tactics")
    TArray<FVector> FindCoverPositions(FVector CenterLocation, float SearchRadius);

    UFUNCTION(BlueprintCallable, Category = "Combat Tactics")
    TArray<FVector> FindHighGroundPositions(FVector CenterLocation, float SearchRadius);

    UFUNCTION(BlueprintPure, Category = "Combat Tactics")
    bool HasLineOfSight(FVector FromLocation, FVector ToLocation) const;

    UFUNCTION(BlueprintPure, Category = "Combat Tactics")
    float GetTerrainAdvantage(FVector Position, AActor* Target) const;

    // Pack Coordination
    UFUNCTION(BlueprintCallable, Category = "Combat Tactics")
    void CoordinateWithPack(const TArray<AActor*>& PackMembers, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat Tactics")
    void AssignPackRoles(const TArray<AActor*>& PackMembers, AActor* Target);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Combat Tactics")
    ETacticType GetActiveTactic() const { return ActiveTactic; }

    UFUNCTION(BlueprintPure, Category = "Combat Tactics")
    FVector GetTacticalTargetLocation() const { return TacticalTargetLocation; }

    UFUNCTION(BlueprintPure, Category = "Combat Tactics")
    FTacticalPlan GetCurrentPlan() const { return CurrentPlan; }

protected:
    // Internal Functions
    void AnalyzeEnvironment(AActor* Target);
    void UpdateTacticalIntelligence(bool bTacticSuccessful);
    FVector FindOptimalPosition(AActor* Target, ETacticType Tactic);
    bool IsPositionValid(FVector Position, AActor* Target, ETacticType Tactic);
    float CalculatePositionScore(FVector Position, AActor* Target, ETacticType Tactic);

    // Tactical Timers
    FTimerHandle TacticalUpdateTimer;
    FTimerHandle EnvironmentAnalysisTimer;

    void OnTacticalUpdate();
    void OnEnvironmentAnalysis();

    // Cached Environmental Data
    UPROPERTY()
    TArray<FVector> CachedCoverPositions;

    UPROPERTY()
    TArray<FVector> CachedHighGroundPositions;

    UPROPERTY()
    float LastEnvironmentUpdateTime = 0.0f;

    // Tactical History
    UPROPERTY()
    TArray<ETacticType> RecentTactics;

    UPROPERTY()
    TArray<bool> TacticSuccessHistory;

    void RecordTacticResult(ETacticType Tactic, bool bSuccess);
    float GetTacticSuccessRate(ETacticType Tactic) const;
};