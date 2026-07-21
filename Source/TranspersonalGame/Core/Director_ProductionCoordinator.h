#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Director_ProductionCoordinator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_BiomeCoordinates
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 SpawnRadius;

    FDir_BiomeCoordinates()
    {
        BiomeName = TEXT("Unknown");
        CenterLocation = FVector::ZeroVector;
        SpawnRadius = 15000;
    }

    FDir_BiomeCoordinates(const FString& Name, const FVector& Center, int32 Radius = 15000)
        : BiomeName(Name), CenterLocation(Center), SpawnRadius(Radius)
    {
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_AgentTaskStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    bool bTaskCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString CurrentTask;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 FilesCreated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 UE5CommandsExecuted;

    FDir_AgentTaskStatus()
    {
        AgentName = TEXT("Unknown");
        bTaskCompleted = false;
        CurrentTask = TEXT("Idle");
        FilesCreated = 0;
        UE5CommandsExecuted = 0;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDir_ProductionCoordinator : public UActorComponent
{
    GENERATED_BODY()

public:
    UDir_ProductionCoordinator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Biome Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production|Biomes")
    TArray<FDir_BiomeCoordinates> BiomeCoordinates;

    UFUNCTION(BlueprintCallable, Category = "Production|Biomes")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Production|Biomes")
    FVector GetRandomLocationInBiome(const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "Production|Biomes")
    FString GetBiomeForDistribution(int32 SpawnIndex);

    // Agent Task Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production|Agents")
    TArray<FDir_AgentTaskStatus> AgentStatuses;

    UFUNCTION(BlueprintCallable, Category = "Production|Agents")
    void InitializeAgentStatuses();

    UFUNCTION(BlueprintCallable, Category = "Production|Agents")
    void UpdateAgentStatus(const FString& AgentName, const FString& Task, int32 Files, int32 Commands);

    UFUNCTION(BlueprintCallable, Category = "Production|Agents")
    bool IsAgentReady(const FString& AgentName);

    // Critical Path Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production|CriticalPath")
    TArray<FString> CriticalPathTasks;

    UFUNCTION(BlueprintCallable, Category = "Production|CriticalPath")
    void InitializeCriticalPath();

    UFUNCTION(BlueprintCallable, Category = "Production|CriticalPath")
    FString GetNextCriticalTask();

    UFUNCTION(BlueprintCallable, Category = "Production|CriticalPath")
    void MarkTaskCompleted(const FString& TaskName);

    // Production Metrics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production|Metrics")
    int32 TotalActorsInWorld;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production|Metrics")
    int32 TotalFilesCreated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production|Metrics")
    float ProductionEfficiency;

    UFUNCTION(BlueprintCallable, Category = "Production|Metrics")
    void UpdateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Production|Metrics")
    void LogProductionStatus();

private:
    float LastUpdateTime;
    int32 CurrentCriticalTaskIndex;
};