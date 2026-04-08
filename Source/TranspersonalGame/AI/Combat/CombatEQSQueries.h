#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "CombatAIArchitecture.h"
#include "CombatEQSQueries.generated.h"

/**
 * Environmental Query System contexts and tests specifically designed for tactical combat
 * Provides intelligent positioning, ambush points, and escape routes for dinosaur AI
 */

/**
 * EQS Context: Player Character
 * Provides the player's location as context for combat queries
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEnvQueryContext_Player : public UEnvQueryContext
{
    GENERATED_BODY()

public:
    virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
};

/**
 * EQS Context: Pack Leader
 * Provides the pack leader's location for coordinated attacks
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEnvQueryContext_PackLeader : public UEnvQueryContext
{
    GENERATED_BODY()

public:
    virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
};

/**
 * EQS Context: Territory Center
 * Provides the dinosaur's territory center for defensive positioning
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEnvQueryContext_TerritoryCenter : public UEnvQueryContext
{
    GENERATED_BODY()

public:
    virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
};

/**
 * EQS Context: Last Known Player Location
 * Provides the last known position of the player for investigation
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEnvQueryContext_LastKnownPlayerLocation : public UEnvQueryContext
{
    GENERATED_BODY()

public:
    virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
};

/**
 * EQS Test: Ambush Potential
 * Evaluates locations based on their potential for ambush attacks
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEnvQueryTest_AmbushPotential : public UEnvQueryTest
{
    GENERATED_BODY()

public:
    UEnvQueryTest_AmbushPotential();

    virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;

    virtual FText GetDescriptionTitle() const override;
    virtual FText GetDescriptionDetails() const override;

protected:
    // Minimum cover height required for ambush
    UPROPERTY(EditDefaultsOnly, Category = "Ambush")
    float MinCoverHeight = 200.0f;

    // Maximum distance from cover to be considered valid
    UPROPERTY(EditDefaultsOnly, Category = "Ambush")
    float MaxCoverDistance = 100.0f;

    // Preferred angle to target (0 = directly behind, 180 = directly in front)
    UPROPERTY(EditDefaultsOnly, Category = "Ambush")
    float PreferredAngleToTarget = 45.0f;

    // Angle tolerance for ambush positioning
    UPROPERTY(EditDefaultsOnly, Category = "Ambush")
    float AngleTolerance = 30.0f;
};

/**
 * EQS Test: Pack Coordination
 * Evaluates positions based on how well they support pack tactics
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEnvQueryTest_PackCoordination : public UEnvQueryTest
{
    GENERATED_BODY()

public:
    UEnvQueryTest_PackCoordination();

    virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;

    virtual FText GetDescriptionTitle() const override;
    virtual FText GetDescriptionDetails() const override;

protected:
    // Ideal distance from pack members
    UPROPERTY(EditDefaultsOnly, Category = "Pack")
    float IdealPackDistance = 500.0f;

    // Distance tolerance
    UPROPERTY(EditDefaultsOnly, Category = "Pack")
    float DistanceTolerance = 200.0f;

    // Whether to prefer flanking positions
    UPROPERTY(EditDefaultsOnly, Category = "Pack")
    bool bPreferFlankingPositions = true;

    // Minimum angle between pack members around target
    UPROPERTY(EditDefaultsOnly, Category = "Pack")
    float MinFlankingAngle = 60.0f;
};

/**
 * EQS Test: Escape Route Quality
 * Evaluates positions based on available escape routes
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEnvQueryTest_EscapeRouteQuality : public UEnvQueryTest
{
    GENERATED_BODY()

public:
    UEnvQueryTest_EscapeRouteQuality();

    virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;

    virtual FText GetDescriptionTitle() const override;
    virtual FText GetDescriptionDetails() const override;

protected:
    // Number of escape routes to evaluate
    UPROPERTY(EditDefaultsOnly, Category = "Escape")
    int32 NumEscapeRoutes = 3;

    // Minimum length of escape route
    UPROPERTY(EditDefaultsOnly, Category = "Escape")
    float MinEscapeDistance = 1000.0f;

    // Preferred escape direction (away from threats)
    UPROPERTY(EditDefaultsOnly, Category = "Escape")
    bool bPreferAwayFromThreats = true;

    // Whether to consider terrain obstacles
    UPROPERTY(EditDefaultsOnly, Category = "Escape")
    bool bConsiderObstacles = true;
};

/**
 * EQS Test: Territorial Advantage
 * Evaluates positions based on territorial control advantages
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEnvQueryTest_TerritorialAdvantage : public UEnvQueryTest
{
    GENERATED_BODY()

public:
    UEnvQueryTest_TerritorialAdvantage();

    virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;

    virtual FText GetDescriptionTitle() const override;
    virtual FText GetDescriptionDetails() const override;

protected:
    // Bonus for being within own territory
    UPROPERTY(EditDefaultsOnly, Category = "Territory")
    float TerritoryBonus = 1.0f;

    // Penalty for being in rival territory
    UPROPERTY(EditDefaultsOnly, Category = "Territory")
    float RivalTerritoryPenalty = 0.5f;

    // Bonus for high ground advantage
    UPROPERTY(EditDefaultsOnly, Category = "Territory")
    float HighGroundBonus = 0.3f;

    // Minimum height difference for high ground
    UPROPERTY(EditDefaultsOnly, Category = "Territory")
    float MinHighGroundDifference = 100.0f;
};

/**
 * EQS Test: Stalking Position
 * Evaluates positions for optimal stalking behavior
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEnvQueryTest_StalkingPosition : public UEnvQueryTest
{
    GENERATED_BODY()

public:
    UEnvQueryTest_StalkingPosition();

    virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;

    virtual FText GetDescriptionTitle() const override;
    virtual FText GetDescriptionDetails() const override;

protected:
    // Ideal stalking distance
    UPROPERTY(EditDefaultsOnly, Category = "Stalking")
    float IdealStalkingDistance = 800.0f;

    // Distance tolerance
    UPROPERTY(EditDefaultsOnly, Category = "Stalking")
    float DistanceTolerance = 200.0f;

    // Whether line of sight is required
    UPROPERTY(EditDefaultsOnly, Category = "Stalking")
    bool bRequireLineOfSight = true;

    // Whether to prefer cover
    UPROPERTY(EditDefaultsOnly, Category = "Stalking")
    bool bPreferCover = true;

    // Minimum cover rating (0.0 = no cover, 1.0 = full cover)
    UPROPERTY(EditDefaultsOnly, Category = "Stalking")
    float MinCoverRating = 0.3f;

    // Whether to consider wind direction for scent
    UPROPERTY(EditDefaultsOnly, Category = "Stalking")
    bool bConsiderWindDirection = true;
};

/**
 * Combat EQS Query Manager
 * Manages and executes combat-specific environmental queries
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCombatEQSManager : public UObject
{
    GENERATED_BODY()

public:
    // Query for optimal attack positions
    UFUNCTION(BlueprintCallable, Category = "Combat EQS")
    static void FindAttackPosition(AActor* Querier, AActor* Target, EDinosaurCombatRole CombatRole, 
                                 FOnEQSRequestComplete OnComplete);

    // Query for ambush positions
    UFUNCTION(BlueprintCallable, Category = "Combat EQS")
    static void FindAmbushPosition(AActor* Querier, AActor* Target, FOnEQSRequestComplete OnComplete);

    // Query for pack coordination positions
    UFUNCTION(BlueprintCallable, Category = "Combat EQS")
    static void FindPackPosition(AActor* Querier, AActor* Target, const TArray<AActor*>& PackMembers, 
                               FOnEQSRequestComplete OnComplete);

    // Query for escape routes
    UFUNCTION(BlueprintCallable, Category = "Combat EQS")
    static void FindEscapeRoute(AActor* Querier, const TArray<AActor*>& Threats, FOnEQSRequestComplete OnComplete);

    // Query for stalking positions
    UFUNCTION(BlueprintCallable, Category = "Combat EQS")
    static void FindStalkingPosition(AActor* Querier, AActor* Target, FOnEQSRequestComplete OnComplete);

    // Query for territorial defense positions
    UFUNCTION(BlueprintCallable, Category = "Combat EQS")
    static void FindDefensivePosition(AActor* Querier, FVector TerritoryCenter, AActor* Intruder, 
                                    FOnEQSRequestComplete OnComplete);

protected:
    // EQS Query templates
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EQS Templates")
    class UEnvQuery* AttackPositionQuery;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EQS Templates")
    class UEnvQuery* AmbushPositionQuery;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EQS Templates")
    class UEnvQuery* PackCoordinationQuery;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EQS Templates")
    class UEnvQuery* EscapeRouteQuery;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EQS Templates")
    class UEnvQuery* StalkingPositionQuery;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EQS Templates")
    class UEnvQuery* DefensivePositionQuery;
};