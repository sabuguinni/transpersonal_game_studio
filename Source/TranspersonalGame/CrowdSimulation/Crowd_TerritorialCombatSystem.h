#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "MassEntityTypes.h"
#include "MassProcessor.h"
#include "MassEntitySubsystem.h"
#include "Crowd_SharedTypes.h"
#include "Crowd_TerritorialCombatSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_TerritorialZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    FVector ZoneCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    float ZoneRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    int32 MaxOccupants;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    int32 CurrentOccupants;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    ECrowd_FactionType ControllingFaction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    float ContestedLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    bool bIsUnderAttack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    TArray<FMassEntityHandle> DefendingEntities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    TArray<FMassEntityHandle> AttackingEntities;

    FCrowd_TerritorialZone()
    {
        ZoneCenter = FVector::ZeroVector;
        ZoneRadius = 1000.0f;
        MaxOccupants = 100;
        CurrentOccupants = 0;
        ControllingFaction = ECrowd_FactionType::Neutral;
        ContestedLevel = 0.0f;
        bIsUnderAttack = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_CombatFormation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    ECrowd_FormationType FormationType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    TArray<FMassEntityHandle> FormationMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    FVector FormationCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    float FormationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    FVector TargetDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    bool bIsEngaged;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    float CohesionStrength;

    FCrowd_CombatFormation()
    {
        FormationType = ECrowd_FormationType::Loose;
        FormationCenter = FVector::ZeroVector;
        FormationRadius = 500.0f;
        TargetDirection = FVector::ForwardVector;
        bIsEngaged = false;
        CohesionStrength = 0.8f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_TerritorialCombatSystem : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_TerritorialCombatSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* SystemMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* InfluenceRadius;

    // Territorial Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territorial Combat")
    TArray<FCrowd_TerritorialZone> TerritorialZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territorial Combat")
    int32 MaxTerritorialZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territorial Combat")
    float ZoneConflictRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territorial Combat")
    float TerritoryExpansionRate;

    // Combat Formation Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Formations")
    TArray<FCrowd_CombatFormation> ActiveFormations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Formations")
    int32 MaxCombatFormations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Formations")
    float FormationCoordinationRange;

    // Mass Entity Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Integration")
    int32 MaxCombatEntities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Integration")
    float CombatEntitySpawnRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Integration")
    TArray<FMassEntityHandle> CombatEntities;

    // Faction Warfare
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction Warfare")
    TMap<ECrowd_FactionType, int32> FactionPopulations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction Warfare")
    TMap<ECrowd_FactionType, float> FactionStrengths;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction Warfare")
    float FactionConflictIntensity;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxProcessingEntitiesPerFrame;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableLODOptimization;

public:
    // Territorial Management Functions
    UFUNCTION(BlueprintCallable, Category = "Territorial Combat")
    void CreateTerritorialZone(FVector Center, float Radius, ECrowd_FactionType Faction);

    UFUNCTION(BlueprintCallable, Category = "Territorial Combat")
    void UpdateTerritorialControl(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Territorial Combat")
    bool IsZoneContested(int32 ZoneIndex) const;

    UFUNCTION(BlueprintCallable, Category = "Territorial Combat")
    void InitiateTerritorialConflict(int32 ZoneIndex, ECrowd_FactionType AttackingFaction);

    // Combat Formation Functions
    UFUNCTION(BlueprintCallable, Category = "Combat Formations")
    int32 CreateCombatFormation(ECrowd_FormationType Type, FVector Center, const TArray<FMassEntityHandle>& Members);

    UFUNCTION(BlueprintCallable, Category = "Combat Formations")
    void UpdateFormationPositions(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Combat Formations")
    void EngageFormation(int32 FormationIndex, FVector TargetPosition);

    UFUNCTION(BlueprintCallable, Category = "Combat Formations")
    void DissolveFormation(int32 FormationIndex);

    // Mass Entity Combat Functions
    UFUNCTION(BlueprintCallable, Category = "Mass Combat")
    void SpawnCombatEntities(int32 Count, ECrowd_FactionType Faction, FVector SpawnArea);

    UFUNCTION(BlueprintCallable, Category = "Mass Combat")
    void ProcessCombatBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Mass Combat")
    void AssignEntitiesToFormation(const TArray<FMassEntityHandle>& Entities, int32 FormationIndex);

    // Faction Warfare Functions
    UFUNCTION(BlueprintCallable, Category = "Faction Warfare")
    void UpdateFactionStrengths(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Faction Warfare")
    ECrowd_FactionType GetDominantFaction() const;

    UFUNCTION(BlueprintCallable, Category = "Faction Warfare")
    void TriggerFactionWar(ECrowd_FactionType Faction1, ECrowd_FactionType Faction2);

    // System Management
    UFUNCTION(BlueprintCallable, Category = "System Management")
    void InitializeTerritorialCombatSystem();

    UFUNCTION(BlueprintCallable, Category = "System Management")
    void ShutdownTerritorialCombatSystem();

    UFUNCTION(BlueprintCallable, Category = "System Management")
    void ResetAllTerritories();

    // Debug and Visualization
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawTerritorialZones() const;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawCombatFormations() const;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogCombatStatistics() const;

private:
    // Internal Processing
    void ProcessTerritorialConflicts(float DeltaTime);
    void UpdateFormationCohesion(FCrowd_CombatFormation& Formation);
    void HandleZoneCapture(int32 ZoneIndex, ECrowd_FactionType NewController);
    void OptimizeCombatPerformance();
    
    // Utility Functions
    float CalculateZoneInfluence(const FCrowd_TerritorialZone& Zone, ECrowd_FactionType Faction) const;
    FVector CalculateFormationPosition(const FCrowd_CombatFormation& Formation, int32 MemberIndex) const;
    bool IsEntityInCombatRange(FMassEntityHandle Entity, FVector TargetPosition) const;

    // Internal State
    float LastUpdateTime;
    int32 ProcessingEntityIndex;
    bool bSystemInitialized;
};