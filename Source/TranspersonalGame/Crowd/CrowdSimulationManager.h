#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "Engine/DataTable.h"
#include "CrowdSimulationManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurSpeciesData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SpeciesName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GroupSize = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TerritoryRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsHerbivore = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsNocturnal = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AggressionLevel = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FlockingStrength = 0.8f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowdDensityZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Center;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Radius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxEntities = 200;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> AllowedSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpawnProbability = 0.7f;
};

UCLASS()
class TRANSPERSONALGAME_API UCrowdSimulationManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void InitializeCrowdSystem();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnDinosaurHerd(const FString& SpeciesName, const FVector& Location, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void RegisterDensityZone(const FCrowdDensityZone& Zone);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateCrowdDensity(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetEntityCountInRadius(const FVector& Location, float Radius) const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    class UDataTable* DinosaurSpeciesTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 MaxGlobalEntities = 50000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float UpdateFrequency = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float PlayerInfluenceRadius = 1000.0f;

    UPROPERTY()
    TArray<FCrowdDensityZone> DensityZones;

    UPROPERTY()
    class UMassEntitySubsystem* MassEntitySubsystem;

    UPROPERTY()
    class UMassSpawnerSubsystem* MassSpawnerSubsystem;

private:
    FTimerHandle UpdateTimer;
    int32 CurrentEntityCount = 0;

    void UpdatePlayerProximity();
    void ManageEntityLOD();
    void ProcessEmergentBehaviors();
};