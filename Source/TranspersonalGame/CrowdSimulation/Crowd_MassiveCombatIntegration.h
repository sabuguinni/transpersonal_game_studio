#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "MassEntityTypes.h"
#include "MassProcessor.h"
#include "MassExecutionContext.h"
#include "Crowd_SharedTypes.h"
#include "Crowd_MassiveCombatIntegration.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_CombatZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Zone")
    FString ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Zone")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Zone")
    float ZoneRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Zone")
    int32 MaxEntityCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Zone")
    ECrowd_CombatState DefaultCombatState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Zone")
    float ThreatLevel;

    FCrowd_CombatZoneConfig()
    {
        ZoneName = TEXT("DefaultZone");
        Location = FVector::ZeroVector;
        ZoneRadius = 1000.0f;
        MaxEntityCount = 10000;
        DefaultCombatState = ECrowd_CombatState::Neutral;
        ThreatLevel = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_MassiveCombatFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECrowd_CombatState CombatState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FVector LastKnownThreatLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float CombatReadiness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int32 AlliesInRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float TimeInCombatState;

    FCrowd_MassiveCombatFragment()
    {
        CombatState = ECrowd_CombatState::Neutral;
        ThreatLevel = 0.0f;
        LastKnownThreatLocation = FVector::ZeroVector;
        CombatReadiness = 0.0f;
        AlliesInRange = 0;
        TimeInCombatState = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_CombatFormationFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    ECrowd_FormationType FormationType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    FVector FormationCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    int32 FormationPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    float FormationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    bool bIsFormationLeader;

    FCrowd_CombatFormationFragment()
    {
        FormationType = ECrowd_FormationType::Scattered;
        FormationCenter = FVector::ZeroVector;
        FormationPosition = 0;
        FormationRadius = 500.0f;
        bIsFormationLeader = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowd_MassiveCombatProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UCrowd_MassiveCombatProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery CombatEntityQuery;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Processing", meta = (AllowPrivateAccess = "true"))
    float CombatUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Processing", meta = (AllowPrivateAccess = "true"))
    float ThreatDetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Processing", meta = (AllowPrivateAccess = "true"))
    float FormationUpdateRadius;

    void ProcessCombatBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void UpdateFormations(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void HandleThreatResponse(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_MassiveCombatZone : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_MassiveCombatZone();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ZoneVisualization;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPointLightComponent* ThreatIndicator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Zone")
    FCrowd_CombatZoneConfig ZoneConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Zone")
    TArray<FCrowd_CombatZoneConfig> SubZones;

    UPROPERTY(BlueprintReadOnly, Category = "Combat Zone")
    int32 CurrentEntityCount;

    UPROPERTY(BlueprintReadOnly, Category = "Combat Zone")
    float CurrentThreatLevel;

public:
    UFUNCTION(BlueprintCallable, Category = "Combat Zone")
    void InitializeCombatZone(const FCrowd_CombatZoneConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Combat Zone")
    void SpawnMassiveEntityGroup(int32 EntityCount, ECrowd_DinosaurType DinosaurType);

    UFUNCTION(BlueprintCallable, Category = "Combat Zone")
    void UpdateThreatLevel(float NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Combat Zone")
    void TriggerCombatEvent(ECrowd_CombatState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat Zone")
    bool CanSpawnMoreEntities() const;

    UFUNCTION(BlueprintPure, Category = "Combat Zone")
    float GetZoneCapacityPercentage() const;

private:
    void UpdateVisualization();
    void ProcessZoneLogic(float DeltaTime);
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCrowd_MassiveCombatManager : public UObject
{
    GENERATED_BODY()

public:
    UCrowd_MassiveCombatManager();

    UFUNCTION(BlueprintCallable, Category = "Massive Combat")
    void InitializeCombatSystem(UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "Massive Combat")
    void CreateMassiveCombatZone(const FCrowd_CombatZoneConfig& ZoneConfig);

    UFUNCTION(BlueprintCallable, Category = "Massive Combat")
    void SpawnMassiveEntityArmy(int32 EntityCount, ECrowd_DinosaurType DinosaurType, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Massive Combat")
    void TriggerGlobalCombatEvent(ECrowd_CombatState CombatState);

    UFUNCTION(BlueprintPure, Category = "Massive Combat")
    int32 GetTotalActiveEntities() const;

    UFUNCTION(BlueprintPure, Category = "Massive Combat")
    float GetSystemPerformanceMetric() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Combat Zones")
    TArray<ACrowd_MassiveCombatZone*> ActiveCombatZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxSimultaneousEntities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceThreshold;

    UPROPERTY(BlueprintReadOnly, Category = "Statistics")
    int32 TotalEntitiesSpawned;

    UPROPERTY(BlueprintReadOnly, Category = "Statistics")
    float AverageFrameTime;

private:
    UWorld* CachedWorld;
    
    void OptimizePerformance();
    void UpdateStatistics();
};