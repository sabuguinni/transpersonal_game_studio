#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Components/ActorComponent.h"
#include "../Core/SharedTypes.h"
#include "CombatAIManager.generated.h"

class AEnemyAIController;
class UCombatBehaviorComponent;

UENUM(BlueprintType)
enum class ECombat_TacticalState : uint8
{
    Patrol,
    Investigate,
    Engage,
    Flank,
    Retreat,
    Ambush,
    PackHunt,
    Defend
};

UENUM(BlueprintType)
enum class ECombat_ThreatLevel : uint8
{
    None,
    Low,
    Medium,
    High,
    Critical
};

USTRUCT(BlueprintType)
struct FCombat_TacticalData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    ECombat_TacticalState CurrentState = ECombat_TacticalState::Patrol;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    ECombat_ThreatLevel ThreatLevel = ECombat_ThreatLevel::None;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    FVector LastKnownPlayerLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    float LastPlayerSightTime = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    TArray<FVector> PatrolPoints;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    int32 CurrentPatrolIndex = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    bool bIsInCombat = false;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    float CombatStartTime = 0.0f;

    FCombat_TacticalData()
    {
        CurrentState = ECombat_TacticalState::Patrol;
        ThreatLevel = ECombat_ThreatLevel::None;
        LastKnownPlayerLocation = FVector::ZeroVector;
        LastPlayerSightTime = 0.0f;
        CurrentPatrolIndex = 0;
        bIsInCombat = false;
        CombatStartTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FCombat_PackData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Pack")
    TArray<AEnemyAIController*> PackMembers;

    UPROPERTY(BlueprintReadWrite, Category = "Pack")
    AEnemyAIController* PackLeader = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Pack")
    FVector PackTargetLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Pack")
    ECombat_TacticalState PackState = ECombat_TacticalState::Patrol;

    UPROPERTY(BlueprintReadWrite, Category = "Pack")
    float PackFormationRadius = 500.0f;

    FCombat_PackData()
    {
        PackLeader = nullptr;
        PackTargetLocation = FVector::ZeroVector;
        PackState = ECombat_TacticalState::Patrol;
        PackFormationRadius = 500.0f;
    }
};

/**
 * Combat AI Manager - Coordinates tactical AI behaviors for all enemies
 * Manages pack hunting, territorial behaviors, and combat state transitions
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCombatAIManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UCombatAIManager();

    // Subsystem overrides
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Combat AI Registration
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RegisterEnemyController(AEnemyAIController* Controller);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UnregisterEnemyController(AEnemyAIController* Controller);

    // Pack Management
    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void CreatePack(const TArray<AEnemyAIController*>& Members, AEnemyAIController* Leader);

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void DisbandPack(AEnemyAIController* PackMember);

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    FCombat_PackData* GetPackData(AEnemyAIController* Controller);

    // Tactical Coordination
    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void UpdateTacticalState(AEnemyAIController* Controller, ECombat_TacticalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void SetThreatLevel(AEnemyAIController* Controller, ECombat_ThreatLevel ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void ReportPlayerSighting(AEnemyAIController* Controller, const FVector& PlayerLocation);

    // Combat Coordination
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void InitiateCombat(AEnemyAIController* Controller, APawn* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void EndCombat(AEnemyAIController* Controller);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool IsInCombat(AEnemyAIController* Controller);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    TArray<AEnemyAIController*> GetNearbyEnemies(const FVector& Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector GetOptimalFlankingPosition(const FVector& TargetLocation, const FVector& EnemyLocation);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool CanSeePlayer(AEnemyAIController* Controller);

protected:
    // Registered enemy controllers
    UPROPERTY()
    TArray<AEnemyAIController*> RegisteredControllers;

    // Tactical data for each controller
    UPROPERTY()
    TMap<AEnemyAIController*, FCombat_TacticalData> TacticalDataMap;

    // Pack management
    UPROPERTY()
    TArray<FCombat_PackData> ActivePacks;

    // Combat parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float SightRange = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float HearingRange = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float CombatRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float FlankingDistance = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float PackCoordinationRadius = 1000.0f;

private:
    // Internal helper functions
    void UpdatePackBehaviors(float DeltaTime);
    void CoordinatePackHunting(FCombat_PackData& PackData);
    void AssignFlankingPositions(FCombat_PackData& PackData, const FVector& TargetLocation);
    bool ValidatePackIntegrity(FCombat_PackData& PackData);
    void CleanupInvalidControllers();

    // Timer handle for pack updates
    FTimerHandle PackUpdateTimerHandle;
};