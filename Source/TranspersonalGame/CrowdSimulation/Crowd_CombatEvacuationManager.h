#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Components/ActorComponent.h"
#include "MassEntityTypes.h"
#include "MassSpawnerTypes.h"
#include "MassCommonTypes.h"
#include "MassEntitySubsystem.h"
#include "MassEntityManager.h"
#include "MassProcessingTypes.h"
#include "MassProcessor.h"
#include "MassEntityView.h"
#include "MassObserverRegistry.h"
#include "MassArchetypeTypes.h"
#include "MassExecutionContext.h"
#include "SharedTypes.h"
#include "Crowd_CombatEvacuationManager.generated.h"

class UCombat_CombatStateManager;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EvacuationZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evacuation")
    FVector SafeZoneCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evacuation")
    float SafeZoneRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evacuation")
    int32 MaxCapacity = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evacuation")
    int32 CurrentOccupancy = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evacuation")
    bool bIsActive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evacuation")
    float PanicThreshold = 0.7f;

    FCrowd_EvacuationZone()
    {
        SafeZoneCenter = FVector::ZeroVector;
        SafeZoneRadius = 2000.0f;
        MaxCapacity = 500;
        CurrentOccupancy = 0;
        bIsActive = true;
        PanicThreshold = 0.7f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_PanicBehavior
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Panic")
    float PanicLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Panic")
    FVector FleeDirection = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Panic")
    float FleeSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Panic")
    float PanicDecayRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Panic")
    bool bInPanic = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Panic")
    float GroupCohesion = 0.8f;

    FCrowd_PanicBehavior()
    {
        PanicLevel = 0.0f;
        FleeDirection = FVector::ZeroVector;
        FleeSpeed = 600.0f;
        PanicDecayRate = 0.1f;
        bInPanic = false;
        GroupCohesion = 0.8f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_CombatEvacuationFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    FCrowd_PanicBehavior PanicBehavior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    int32 AssignedEvacuationZone = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    float ThreatAwareness = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    FVector LastKnownThreatLocation = FVector::ZeroVector;

    FCrowd_CombatEvacuationFragment()
    {
        AssignedEvacuationZone = -1;
        ThreatAwareness = 0.0f;
        LastKnownThreatLocation = FVector::ZeroVector;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowd_CombatEvacuationProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UCrowd_CombatEvacuationProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery EntityQuery;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evacuation", meta = (AllowPrivateAccess = "true"))
    float PanicSpreadRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evacuation", meta = (AllowPrivateAccess = "true"))
    float PanicSpreadRate = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evacuation", meta = (AllowPrivateAccess = "true"))
    float MaxEvacuationDistance = 5000.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowd_CombatEvacuationManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowd_CombatEvacuationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Evacuation zone management
    UFUNCTION(BlueprintCallable, Category = "Crowd Evacuation")
    int32 CreateEvacuationZone(const FVector& Center, float Radius, int32 MaxCapacity);

    UFUNCTION(BlueprintCallable, Category = "Crowd Evacuation")
    void RemoveEvacuationZone(int32 ZoneIndex);

    UFUNCTION(BlueprintCallable, Category = "Crowd Evacuation")
    void ActivateEvacuationZone(int32 ZoneIndex, bool bActivate);

    UFUNCTION(BlueprintCallable, Category = "Crowd Evacuation")
    FVector GetNearestSafeZone(const FVector& Location, float& OutDistance);

    // Combat evacuation triggers
    UFUNCTION(BlueprintCallable, Category = "Crowd Evacuation")
    void TriggerMassEvacuation(const FVector& ThreatLocation, float ThreatRadius, float PanicLevel);

    UFUNCTION(BlueprintCallable, Category = "Crowd Evacuation")
    void ProcessCombatThreat(const FVector& ThreatLocation, float ThreatLevel, float AffectedRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Evacuation")
    void UpdateEvacuationBehavior(float DeltaTime);

    // Mass Entity integration
    UFUNCTION(BlueprintCallable, Category = "Crowd Evacuation")
    void RegisterMassEntity(FMassEntityHandle EntityHandle, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Crowd Evacuation")
    void UnregisterMassEntity(FMassEntityHandle EntityHandle);

    UFUNCTION(BlueprintCallable, Category = "Crowd Evacuation")
    void AssignEntityToEvacuationZone(FMassEntityHandle EntityHandle, int32 ZoneIndex);

    // Panic behavior management
    UFUNCTION(BlueprintCallable, Category = "Crowd Evacuation")
    void SpreadPanic(const FVector& PanicSource, float PanicRadius, float PanicIntensity);

    UFUNCTION(BlueprintCallable, Category = "Crowd Evacuation")
    void CalculateFleeDirection(const FVector& EntityLocation, const FVector& ThreatLocation, FVector& OutFleeDirection);

    UFUNCTION(BlueprintCallable, Category = "Crowd Evacuation")
    bool IsLocationSafe(const FVector& Location, float SafetyRadius = 1000.0f);

    // Statistics and monitoring
    UFUNCTION(BlueprintCallable, Category = "Crowd Evacuation")
    int32 GetTotalEvacuatingEntities() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Evacuation")
    float GetAveragePanicLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Evacuation")
    void GetEvacuationStatistics(int32& OutActiveZones, int32& OutEvacuatingEntities, float& OutAveragePanic);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Evacuation Zones")
    TArray<FCrowd_EvacuationZone> EvacuationZones;

    UPROPERTY(BlueprintReadOnly, Category = "Mass Entities")
    TMap<FMassEntityHandle, FCrowd_CombatEvacuationFragment> RegisteredEntities;

    UPROPERTY(BlueprintReadOnly, Category = "Combat Integration")
    UCombat_CombatStateManager* CombatStateManager;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evacuation Settings")
    float GlobalPanicThreshold = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evacuation Settings")
    float PanicDecayRate = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evacuation Settings")
    float MaxEvacuationSpeed = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evacuation Settings")
    int32 MaxSimultaneousEvacuees = 1000;

private:
    void InitializeDefaultEvacuationZones();
    void UpdatePanicSpread(float DeltaTime);
    void ProcessEvacuationMovement(float DeltaTime);
    int32 FindBestEvacuationZone(const FVector& Location);
    void ValidateEvacuationZones();

    float LastUpdateTime;
    bool bSystemInitialized;
};