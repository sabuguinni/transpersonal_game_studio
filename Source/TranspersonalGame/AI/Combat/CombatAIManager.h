#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "../../SharedTypes.h"
#include "CombatAIManager.generated.h"

UENUM(BlueprintType)
enum class ECombat_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "No Threat"),
    Low         UMETA(DisplayName = "Low Threat"),
    Medium      UMETA(DisplayName = "Medium Threat"),
    High        UMETA(DisplayName = "High Threat"),
    Critical    UMETA(DisplayName = "Critical Threat")
};

UENUM(BlueprintType)
enum class ECombat_AIState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrolling"),
    Alert       UMETA(DisplayName = "Alert"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Combat      UMETA(DisplayName = "In Combat"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Feeding     UMETA(DisplayName = "Feeding")
};

USTRUCT(BlueprintType)
struct FCombat_ThreatData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    AActor* ThreatActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    ECombat_ThreatLevel ThreatLevel = ECombat_ThreatLevel::None;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    float Distance = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    float LastSeenTime = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    FVector LastKnownLocation = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct FCombat_AIProfile
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float AggressionLevel = 0.5f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float FearThreshold = 0.3f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float AttackRange = 500.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float DetectionRadius = 1500.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float FleeDistance = 2000.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    bool bIsPackHunter = false;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    bool bIsTerritorial = true;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float PackCoordinationRadius = 1000.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombatAIManager : public AActor
{
    GENERATED_BODY()

public:
    ACombatAIManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Combat AI Functions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RegisterCombatActor(AActor* Actor, const FCombat_AIProfile& Profile);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UnregisterCombatActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_ThreatLevel EvaluateThreat(AActor* Evaluator, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    TArray<AActor*> GetNearbyThreats(AActor* Actor, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateCombatState(AActor* Actor, ECombat_AIState NewState);

    // Pack Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    TArray<AActor*> GetPackMembers(AActor* PackLeader, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void CoordinatePackAttack(AActor* PackLeader, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    FVector CalculateFlankingPosition(AActor* Attacker, AActor* Target, AActor* PackLeader);

    // Territory and Zone Management
    UFUNCTION(BlueprintCallable, Category = "Territory")
    bool IsInTerritory(AActor* Actor, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Territory")
    void DefendTerritory(AActor* Defender, AActor* Intruder);

    UFUNCTION(BlueprintCallable, Category = "Combat Zones")
    void CreateCombatZone(FVector Center, float Radius, ECombat_ThreatLevel MinThreatLevel);

protected:
    // Internal tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    TMap<AActor*, FCombat_AIProfile> RegisteredActors;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    TMap<AActor*, ECombat_AIState> ActorStates;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    TArray<FCombat_ThreatData> ActiveThreats;

    // Combat parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float GlobalAggressionMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float ThreatUpdateInterval = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    int32 MaxSimultaneousCombats = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    bool bEnablePackBehavior = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    bool bEnableTerritorialBehavior = true;

private:
    float LastThreatUpdate = 0.0f;
    int32 CurrentActiveCombats = 0;

    // Internal helper functions
    void UpdateThreatAssessment();
    void ProcessCombatStates();
    void HandlePackCoordination();
    float CalculateThreatScore(AActor* Evaluator, AActor* Target);
    bool CanEngageInCombat() const;
};