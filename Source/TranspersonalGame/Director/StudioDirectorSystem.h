#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "StudioDirectorSystem.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentPriority : uint8
{
    LOW = 0,
    MEDIUM = 1,
    HIGH = 2,
    CRITICAL = 3
};

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    IDLE = 0,
    ASSIGNED = 1,
    WORKING = 2,
    COMPLETED = 3,
    BLOCKED = 4,
    ERROR = 5
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_AgentPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FDateTime AssignedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FDateTime CompletedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FVector CoordinationZone;

    FDir_AgentTask()
    {
        AgentID = TEXT("");
        TaskDescription = TEXT("");
        Priority = EDir_AgentPriority::MEDIUM;
        Status = EDir_AgentStatus::IDLE;
        AssignedTime = FDateTime::Now();
        CompletedTime = FDateTime();
        CoordinationZone = FVector::ZeroVector;
    }
};

USTRUCT(BlueprintType)
struct FDir_BiomeCoordinates
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 RequiredActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 CurrentActorCount;

    FDir_BiomeCoordinates()
    {
        BiomeName = TEXT("");
        CenterLocation = FVector::ZeroVector;
        Radius = 10000.0f;
        RequiredActorCount = 500;
        CurrentActorCount = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AStudioDirectorSystem : public AActor
{
    GENERATED_BODY()

public:
    AStudioDirectorSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core coordination functions
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeCoordinationSystem();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AssignTaskToAgent(const FString& AgentID, const FString& TaskDescription, EDir_AgentPriority Priority);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentStatus(const FString& AgentID, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CreateCoordinationZones();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ValidateBiomePopulation();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTask> GetHighPriorityTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FString GenerateStatusReport() const;

    // Biome management
    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void InitializeBiomeCoordinates();

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    FDir_BiomeCoordinates GetBiomeByName(const FString& BiomeName) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void UpdateBiomeActorCount(const FString& BiomeName, int32 ActorCount);

    // Asset pipeline management
    UFUNCTION(BlueprintCallable, Category = "Asset Pipeline")
    void ValidateAssetPipeline();

    UFUNCTION(BlueprintCallable, Category = "Asset Pipeline")
    void RequestAssetFromMeshy(const FString& AssetName, const FString& Prompt, const FString& BiomeName);

protected:
    // Agent task management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    TArray<FDir_BiomeCoordinates> BiomeCoordinates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    FString CurrentCycleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    int32 TotalActiveAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    float CoordinationUpdateInterval;

    // Coordination zone actors
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coordination")
    TArray<AActor*> CoordinationZoneActors;

    // Internal functions
    void UpdateCoordinationSystem();
    void CheckTaskCompletion();
    void GenerateAgentAssignments();
    void ValidateProjectMilestones();

private:
    FTimerHandle CoordinationTimerHandle;
    float LastUpdateTime;
};