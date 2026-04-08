#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Engine.h"
#include "GameplayTags.h"
#include "CombatAIArchitecture.h"
#include "TacticalCombatAnalyzer.generated.h"

/**
 * Advanced Tactical Combat Analyzer
 * Provides intelligent combat decision making for dinosaur AI
 * 
 * Key Features:
 * - Real-time threat assessment
 * - Tactical positioning analysis
 * - Pack coordination strategies
 * - Environmental exploitation
 * - Player behavior prediction
 */

UENUM(BlueprintType)
enum class ETacticalAdvantage : uint8
{
    None            UMETA(DisplayName = "No Advantage"),
    Positional      UMETA(DisplayName = "Positional Advantage"),
    Numerical       UMETA(DisplayName = "Numerical Advantage"),
    Environmental   UMETA(DisplayName = "Environmental Advantage"),
    Stealth         UMETA(DisplayName = "Stealth Advantage"),
    Speed           UMETA(DisplayName = "Speed Advantage"),
    Size            UMETA(DisplayName = "Size Advantage"),
    Terrain         UMETA(DisplayName = "Terrain Advantage")
};

UENUM(BlueprintType)
enum class ETacticalManeuver : uint8
{
    None            UMETA(DisplayName = "No Maneuver"),
    DirectAssault   UMETA(DisplayName = "Direct Assault"),
    Flanking        UMETA(DisplayName = "Flanking Attack"),
    Ambush          UMETA(DisplayName = "Ambush Setup"),
    Pincer          UMETA(DisplayName = "Pincer Movement"),
    Harassment      UMETA(DisplayName = "Hit and Run"),
    Encirclement    UMETA(DisplayName = "Encirclement"),
    FeintRetreat    UMETA(DisplayName = "Feint Retreat"),
    TerrainTrap     UMETA(DisplayName = "Terrain Trap"),
    Stalking        UMETA(DisplayName = "Stalking Approach")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FTacticalSituation
{
    GENERATED_BODY()

    // Target information
    UPROPERTY(BlueprintReadOnly)
    AActor* PrimaryTarget = nullptr;

    UPROPERTY(BlueprintReadOnly)
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly)
    FVector TargetVelocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly)
    float DistanceToTarget = 0.0f;

    // Threat assessment
    UPROPERTY(BlueprintReadOnly)
    EThreatLevel OverallThreatLevel = EThreatLevel::None;

    UPROPERTY(BlueprintReadOnly)
    TArray<AActor*> NearbyThreats;

    UPROPERTY(BlueprintReadOnly)
    TArray<AActor*> AlliedUnits;

    // Tactical analysis
    UPROPERTY(BlueprintReadOnly)
    TArray<ETacticalAdvantage> CurrentAdvantages;

    UPROPERTY(BlueprintReadOnly)
    TArray<ETacticalAdvantage> PotentialAdvantages;

    UPROPERTY(BlueprintReadOnly)
    ETacticalManeuver RecommendedManeuver = ETacticalManeuver::None;

    UPROPERTY(BlueprintReadOnly)
    TArray<FVector> OptimalPositions;

    UPROPERTY(BlueprintReadOnly)
    TArray<FVector> CoverPositions;

    UPROPERTY(BlueprintReadOnly)
    TArray<FVector> FlankingRoutes;

    // Environmental factors
    UPROPERTY(BlueprintReadOnly)
    bool bHasHighGround = false;

    UPROPERTY(BlueprintReadOnly)
    bool bHasCoverNearby = false;

    UPROPERTY(BlueprintReadOnly)
    bool bTargetHasEscapeRoute = true;

    UPROPERTY(BlueprintReadOnly)
    bool bCanUseAmbushTactics = false;

    UPROPERTY(BlueprintReadOnly)
    float TerrainDifficulty = 0.5f;

    // Timing factors
    UPROPERTY(BlueprintReadOnly)
    float TimeOfDay = 12.0f; // 0-24 hours

    UPROPERTY(BlueprintReadOnly)
    float Visibility = 1.0f; // 0-1 (fog, rain, etc.)

    UPROPERTY(BlueprintReadOnly)
    bool bIsNightTime = false;

    FTacticalSituation()
    {
        Reset();
    }

    void Reset()
    {
        PrimaryTarget = nullptr;
        TargetLocation = FVector::ZeroVector;
        TargetVelocity = FVector::ZeroVector;
        DistanceToTarget = 0.0f;
        OverallThreatLevel = EThreatLevel::None;
        NearbyThreats.Empty();
        AlliedUnits.Empty();
        CurrentAdvantages.Empty();
        PotentialAdvantages.Empty();
        RecommendedManeuver = ETacticalManeuver::None;
        OptimalPositions.Empty();
        CoverPositions.Empty();
        FlankingRoutes.Empty();
        bHasHighGround = false;
        bHasCoverNearby = false;
        bTargetHasEscapeRoute = true;
        bCanUseAmbushTactics = false;
        TerrainDifficulty = 0.5f;
        TimeOfDay = 12.0f;
        Visibility = 1.0f;
        bIsNightTime = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPlayerBehaviorProfile
{
    GENERATED_BODY()

    // Movement patterns
    UPROPERTY(BlueprintReadOnly)
    float AverageSpeed = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float MovementPredictability = 0.5f; // 0 = unpredictable, 1 = very predictable

    UPROPERTY(BlueprintReadOnly)
    bool bTendsToRun = false;

    UPROPERTY(BlueprintReadOnly)
    bool bTendsToHide = false;

    UPROPERTY(BlueprintReadOnly)
    bool bTendsToFight = false;

    // Combat behavior
    UPROPERTY(BlueprintReadOnly)
    float AggressionLevel = 0.5f;

    UPROPERTY(BlueprintReadOnly)
    float CautionLevel = 0.5f;

    UPROPERTY(BlueprintReadOnly)
    bool bUsesTools = false;

    UPROPERTY(BlueprintReadOnly)
    bool bUsesTraps = false;

    // Preferred locations
    UPROPERTY(BlueprintReadOnly)
    TArray<FVector> FrequentedAreas;

    UPROPERTY(BlueprintReadOnly)
    TArray<FVector> PreferredHidingSpots;

    // Timing patterns
    UPROPERTY(BlueprintReadOnly)
    TArray<float> ActiveHours;

    UPROPERTY(BlueprintReadOnly)
    float LastSeenTime = 0.0f;

    FPlayerBehaviorProfile()
    {
        Reset();
    }

    void Reset()
    {
        AverageSpeed = 0.0f;
        MovementPredictability = 0.5f;
        bTendsToRun = false;
        bTendsToHide = false;
        bTendsToFight = false;
        AggressionLevel = 0.5f;
        CautionLevel = 0.5f;
        bUsesTools = false;
        bUsesTraps = false;
        FrequentedAreas.Empty();
        PreferredHidingSpots.Empty();
        ActiveHours.Empty();
        LastSeenTime = 0.0f;
    }
};

/**
 * Tactical Combat Analyzer Component
 * Provides advanced AI decision making for combat scenarios
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UTacticalCombatAnalyzer : public UActorComponent
{
    GENERATED_BODY()

public:
    UTacticalCombatAnalyzer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core Analysis Functions
    UFUNCTION(BlueprintCallable, Category = "Tactical Analysis")
    FTacticalSituation AnalyzeCombatSituation(AActor* Target, const TArray<AActor*>& AlliedUnits = TArray<AActor*>());

    UFUNCTION(BlueprintCallable, Category = "Tactical Analysis")
    ETacticalManeuver DetermineOptimalManeuver(const FTacticalSituation& Situation, const FCombatPersonality& Personality);

    UFUNCTION(BlueprintCallable, Category = "Tactical Analysis")
    TArray<FVector> FindOptimalPositions(AActor* Target, float MinDistance = 300.0f, float MaxDistance = 800.0f, int32 MaxPositions = 5);

    UFUNCTION(BlueprintCallable, Category = "Tactical Analysis")
    TArray<FVector> FindFlankingRoutes(AActor* Target, float FlankDistance = 500.0f);

    UFUNCTION(BlueprintCallable, Category = "Tactical Analysis")
    bool CanExecuteAmbush(AActor* Target, float AmbushRange = 200.0f);

    UFUNCTION(BlueprintCallable, Category = "Tactical Analysis")
    FVector PredictTargetPosition(AActor* Target, float PredictionTime = 2.0f);

    // Player Behavior Analysis
    UFUNCTION(BlueprintCallable, Category = "Player Analysis")
    void UpdatePlayerBehaviorProfile(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Player Analysis")
    FPlayerBehaviorProfile GetPlayerBehaviorProfile() const { return PlayerProfile; }

    UFUNCTION(BlueprintCallable, Category = "Player Analysis")
    float PredictPlayerReaction(const FTacticalSituation& Situation);

    // Environmental Analysis
    UFUNCTION(BlueprintCallable, Category = "Environmental Analysis")
    TArray<FVector> FindCoverPositions(const FVector& FromLocation, float SearchRadius = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Environmental Analysis")
    bool HasLineOfSight(const FVector& FromLocation, const FVector& ToLocation);

    UFUNCTION(BlueprintCallable, Category = "Environmental Analysis")
    float CalculateTerrainDifficulty(const FVector& Location, float SearchRadius = 500.0f);

    UFUNCTION(BlueprintCallable, Category = "Environmental Analysis")
    bool HasHighGroundAdvantage(const FVector& MyLocation, const FVector& TargetLocation);

    // Pack Coordination
    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    TMap<AActor*, ETacticalManeuver> AssignPackRoles(const TArray<AActor*>& PackMembers, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    FVector CalculateOptimalPackPosition(AActor* PackMember, const TArray<AActor*>& OtherMembers, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    bool ShouldInitiatePackAttack(const TArray<AActor*>& PackMembers, AActor* Target);

    // Threat Assessment
    UFUNCTION(BlueprintCallable, Category = "Threat Assessment")
    EThreatLevel AssessThreatLevel(AActor* Target, const FCombatCapabilities& MyCapabilities);

    UFUNCTION(BlueprintCallable, Category = "Threat Assessment")
    float CalculateCombatEffectiveness(const FCombatCapabilities& MyCapabilities, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Threat Assessment")
    bool ShouldRetreat(const FTacticalSituation& Situation, const FCombatPersonality& Personality);

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Analysis Settings")
    float AnalysisRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Analysis Settings")
    float AnalysisFrequency = 0.5f; // How often to update analysis (seconds)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Analysis Settings")
    bool bEnablePlayerProfiling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Analysis Settings")
    bool bEnableEnvironmentalAnalysis = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Analysis Settings")
    bool bEnablePackCoordination = true;

    // Current state
    UPROPERTY(BlueprintReadOnly, Category = "Current Analysis")
    FTacticalSituation CurrentSituation;

    UPROPERTY(BlueprintReadOnly, Category = "Current Analysis")
    FPlayerBehaviorProfile PlayerProfile;

    UPROPERTY(BlueprintReadOnly, Category = "Current Analysis")
    float LastAnalysisTime = 0.0f;

    // Events
    UFUNCTION(BlueprintImplementableEvent, Category = "Tactical Events")
    void OnTacticalSituationChanged(const FTacticalSituation& NewSituation);

    UFUNCTION(BlueprintImplementableEvent, Category = "Tactical Events")
    void OnOptimalManeuverFound(ETacticalManeuver Maneuver, const TArray<FVector>& Positions);

    UFUNCTION(BlueprintImplementableEvent, Category = "Tactical Events")
    void OnPlayerBehaviorUpdated(const FPlayerBehaviorProfile& Profile);

private:
    // Internal analysis functions
    void UpdateAnalysis(float DeltaTime);
    TArray<ETacticalAdvantage> EvaluateAdvantages(const FTacticalSituation& Situation);
    void AnalyzeEnvironmentalFactors(FTacticalSituation& Situation);
    void UpdatePlayerProfile(AActor* Player, float DeltaTime);
    
    // Utility functions
    float CalculateDistance(const FVector& A, const FVector& B) const;
    bool IsValidPosition(const FVector& Position) const;
    FVector GetRandomPositionInRadius(const FVector& Center, float Radius) const;
    
    // Internal state
    float TimeSinceLastAnalysis = 0.0f;
    TArray<FVector> PlayerMovementHistory;
    TArray<float> PlayerSpeedHistory;
    float PlayerProfileUpdateTimer = 0.0f;
};