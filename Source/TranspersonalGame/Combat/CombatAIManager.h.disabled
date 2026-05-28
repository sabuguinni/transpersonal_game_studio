#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "../Core/SharedTypes.h"
#include "CombatAIManager.generated.h"

// Combat AI tactical states
UENUM(BlueprintType)
enum class ECombat_TacticalState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Patrol UMETA(DisplayName = "Patrol"),
    Alert UMETA(DisplayName = "Alert"),
    Hunt UMETA(DisplayName = "Hunt"),
    Attack UMETA(DisplayName = "Attack"),
    Flee UMETA(DisplayName = "Flee"),
    Territorial UMETA(DisplayName = "Territorial")
};

// Combat engagement range
UENUM(BlueprintType)
enum class ECombat_EngagementRange : uint8
{
    Close UMETA(DisplayName = "Close Range (0-5m)"),
    Medium UMETA(DisplayName = "Medium Range (5-15m)"),
    Long UMETA(DisplayName = "Long Range (15m+)")
};

// Combat AI data structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_AIData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    ECombat_TacticalState CurrentState = ECombat_TacticalState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float FearThreshold = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float TerritoryRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    ECombat_EngagementRange PreferredRange = ECombat_EngagementRange::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float AttackCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float LastAttackTime = 0.0f;
};

/**
 * Combat AI Manager - Orchestrates tactical combat behavior for dinosaurs and enemies
 * Manages threat assessment, pack coordination, and adaptive combat tactics
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCombatAIManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombatAIManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core combat AI functions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void InitializeCombatAI(AActor* OwnerActor);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateTacticalState(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_TacticalState EvaluateThreatLevel(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldEngageTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector CalculateOptimalPosition(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteAttackPattern(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CoordinatePackBehavior(TArray<AActor*> PackMembers);

    // Threat assessment
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    float CalculateThreatScore(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    TArray<AActor*> FindNearbyThreats(float SearchRadius);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    AActor* SelectPrimaryTarget(TArray<AActor*> PotentialTargets);

    // Combat properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    FCombat_AIData CombatData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float ThreatDetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float CombatUpdateInterval = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    bool bEnablePackCoordination = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    int32 MaxPackSize = 5;

private:
    // Internal state
    UPROPERTY()
    AActor* CurrentTarget;

    UPROPERTY()
    TArray<AActor*> KnownThreats;

    UPROPERTY()
    TArray<AActor*> PackMembers;

    float LastUpdateTime;
    float StateChangeTime;

    // Internal functions
    void UpdateThreatAssessment();
    void ProcessPackCoordination();
    bool IsWithinTerritory(FVector Position);
    FVector GetFleeDirection(AActor* Threat);
};