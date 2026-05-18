#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "StudioDirectorCoordination.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    int32 AgentNumber;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    bool bCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString CycleID;

    FDir_AgentTask()
    {
        AgentNumber = 0;
        TaskDescription = TEXT("");
        Priority = TEXT("Medium");
        bCompleted = false;
        CycleID = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_BiomeStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 ActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bPopulated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> SpawnedAssets;

    FDir_BiomeStatus()
    {
        BiomeName = TEXT("");
        Location = FVector::ZeroVector;
        ActorCount = 0;
        bPopulated = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UStudioDirectorCoordination : public UActorComponent
{
    GENERATED_BODY()

public:
    UStudioDirectorCoordination();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coordination")
    TArray<FDir_AgentTask> ActiveTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coordination")
    TArray<FDir_BiomeStatus> BiomeStatuses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coordination")
    FString CurrentCycleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coordination")
    int32 TotalActorsInMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coordination")
    bool bMinPlayableMapReady;

    // Core coordination functions
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeCoordination();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AssignTaskToAgent(int32 AgentNumber, const FString& TaskDescription, const FString& Priority);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CompleteAgentTask(int32 AgentNumber, const FString& CycleID);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateBiomeStatus(const FString& BiomeName, const FVector& Location, int32 ActorCount);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool ValidateMinPlayablePrototype();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FString GenerateCoordinationReport();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void SpawnDinosaurInBiome(const FString& BiomeName, const FString& DinosaurType);

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void ScanCurrentMapStatus();

    UFUNCTION()
    void ValidateBiomePopulation();

    UFUNCTION()
    void CheckAgentProgress();

private:
    void SetupDefaultBiomes();
    void LogCoordinationStatus();
};