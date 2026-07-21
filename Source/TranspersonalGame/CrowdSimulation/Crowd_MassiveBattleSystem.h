#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "MassEntitySubsystem.h"
#include "MassProcessor.h"
#include "MassEntityTemplate.h"
#include "MassSpawnerSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Crowd_SharedTypes.h"
#include "Crowd_MassiveBattleSystem.generated.h"

UENUM(BlueprintType)
enum class ECrowd_BattleFaction : uint8
{
    HerbivoreHerds      UMETA(DisplayName = "Herbivore Herds"),
    PredatorPacks       UMETA(DisplayName = "Predator Packs"),
    AlphaPredators      UMETA(DisplayName = "Alpha Predators"),
    ScavengerGroups     UMETA(DisplayName = "Scavenger Groups"),
    MigratingHerds      UMETA(DisplayName = "Migrating Herds"),
    TerritorialGuards   UMETA(DisplayName = "Territorial Guards"),
    NestingFamilies     UMETA(DisplayName = "Nesting Families"),
    EggRaiders          UMETA(DisplayName = "Egg Raiders"),
    RexTerritory        UMETA(DisplayName = "Rex Territory"),
    ChallengerCoalition UMETA(DisplayName = "Challenger Coalition")
};

UENUM(BlueprintType)
enum class ECrowd_BattleState : uint8
{
    Preparation     UMETA(DisplayName = "Preparation"),
    Engagement      UMETA(DisplayName = "Engagement"),
    Escalation      UMETA(DisplayName = "Escalation"),
    Climax          UMETA(DisplayName = "Climax"),
    Resolution      UMETA(DisplayName = "Resolution"),
    Aftermath       UMETA(DisplayName = "Aftermath")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_BattleZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Zone")
    FString ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Zone")
    FVector Center;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Zone")
    float Size;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Zone")
    ECrowd_BattleFaction FactionA;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Zone")
    ECrowd_BattleFaction FactionB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Zone")
    int32 EntityCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Zone")
    ECrowd_BattleState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Zone")
    float BattleIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Zone")
    float CasualtyRate;

    FCrowd_BattleZone()
    {
        ZoneName = TEXT("DefaultBattleZone");
        Center = FVector::ZeroVector;
        Size = 1000.0f;
        FactionA = ECrowd_BattleFaction::HerbivoreHerds;
        FactionB = ECrowd_BattleFaction::PredatorPacks;
        EntityCount = 1000;
        CurrentState = ECrowd_BattleState::Preparation;
        BattleIntensity = 0.0f;
        CasualtyRate = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_MassiveBattleConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Config")
    int32 MaxSimultaneousEntities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Config")
    float BattleDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Config")
    float IntensityRampUpTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Config")
    float MaxCasualtyRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Config")
    bool bEnableMassiveCollisions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Config")
    bool bEnableAdvancedAI;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Config")
    TArray<FCrowd_BattleZone> BattleZones;

    FCrowd_MassiveBattleConfig()
    {
        MaxSimultaneousEntities = 50000;
        BattleDuration = 300.0f;
        IntensityRampUpTime = 60.0f;
        MaxCasualtyRate = 0.3f;
        bEnableMassiveCollisions = true;
        bEnableAdvancedAI = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_MassiveBattleSystem : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_MassiveBattleSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle System")
    FCrowd_MassiveBattleConfig BattleConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle System")
    TArray<FCrowd_BattleZone> ActiveBattleZones;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle System")
    int32 CurrentActiveEntities;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle System")
    float CurrentBattleTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle System")
    bool bBattleInProgress;

    UFUNCTION(BlueprintCallable, Category = "Battle System")
    void StartMassiveBattle();

    UFUNCTION(BlueprintCallable, Category = "Battle System")
    void EndMassiveBattle();

    UFUNCTION(BlueprintCallable, Category = "Battle System")
    void AddBattleZone(const FCrowd_BattleZone& NewZone);

    UFUNCTION(BlueprintCallable, Category = "Battle System")
    void RemoveBattleZone(const FString& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Battle System")
    void UpdateBattleIntensity(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Battle System")
    void ProcessBattleZones(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Battle System")
    void SpawnBattleEntities(const FCrowd_BattleZone& Zone);

    UFUNCTION(BlueprintCallable, Category = "Battle System")
    void UpdateBattleState(FCrowd_BattleZone& Zone, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Battle System")
    float CalculateBattleIntensity(const FCrowd_BattleZone& Zone) const;

    UFUNCTION(BlueprintCallable, Category = "Battle System")
    void HandleCasualties(FCrowd_BattleZone& Zone, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Battle System")
    bool CanSpawnMoreEntities() const;

    UFUNCTION(BlueprintCallable, Category = "Battle System")
    void OptimizeBattlePerformance();

    UFUNCTION(BlueprintCallable, Category = "Battle System")
    TArray<FVector> GetBattleHotspots() const;

    UFUNCTION(BlueprintCallable, Category = "Battle System")
    void CreateBattleEffects(const FVector& Location, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Battle System")
    void UpdateBattleAudio(float DeltaTime);

private:
    UPROPERTY()
    class UMassEntitySubsystem* MassEntitySubsystem;

    UPROPERTY()
    class UMassSpawnerSubsystem* MassSpawnerSubsystem;

    UPROPERTY()
    TArray<FMassEntityHandle> BattleEntities;

    float BattleStartTime;
    float LastIntensityUpdate;
    int32 TotalCasualties;

    void InitializeBattleSystem();
    void CleanupBattleSystem();
    void UpdateEntityLOD(float DeltaTime);
    void ProcessBattleLogic(float DeltaTime);
    void HandleBattleTransitions(FCrowd_BattleZone& Zone);
    void SpawnFactionEntities(ECrowd_BattleFaction Faction, const FVector& Location, int32 Count);
    void UpdateFactionBehavior(ECrowd_BattleFaction Faction, float DeltaTime);
    bool IsValidBattleZone(const FCrowd_BattleZone& Zone) const;
    void LogBattleStatistics() const;
};