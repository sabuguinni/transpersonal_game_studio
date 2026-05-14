#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/TextRenderComponent.h"
#include "Engine/Engine.h"
#include "StudioDirectorSystem.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle,
    Working,
    Completed,
    Failed,
    Waiting
};

USTRUCT(BlueprintType)
struct FDir_AgentInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Agent")
    FString AgentName;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Agent")
    int32 AgentID;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Agent")
    EDir_AgentStatus Status;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Agent")
    FString CurrentTask;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Agent")
    float ProgressPercentage;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Agent")
    FString LastOutput;

    FDir_AgentInfo()
    {
        AgentName = TEXT("");
        AgentID = 0;
        Status = EDir_AgentStatus::Idle;
        CurrentTask = TEXT("");
        ProgressPercentage = 0.0f;
        LastOutput = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FDir_BiomeStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Biome")
    FString BiomeName;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Biome")
    FVector Location;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Biome")
    bool bIsReady;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Biome")
    int32 ActorCount;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Biome")
    FString ResponsibleAgent;

    FDir_BiomeStatus()
    {
        BiomeName = TEXT("");
        Location = FVector::ZeroVector;
        bIsReady = false;
        ActorCount = 0;
        ResponsibleAgent = TEXT("");
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

    // Agent Management
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Studio Director")
    TArray<FDir_AgentInfo> AgentList;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Studio Director")
    TArray<FDir_BiomeStatus> BiomeList;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Studio Director")
    FString CurrentCycleID;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Studio Director")
    int32 TotalCycles;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Studio Director")
    float BudgetUsed;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Studio Director")
    float BudgetLimit;

    // Display Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UTextRenderComponent* MainStatusDisplay;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UTextRenderComponent* AgentStatusDisplay;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UTextRenderComponent* BiomeStatusDisplay;

    // Core Functions
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeAgents();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus, const FString& Task);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateBiomeStatus(const FString& BiomeName, bool bReady, int32 ActorCount);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void RefreshDisplays();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void StartNewCycle(const FString& CycleID);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FString GenerateStatusReport();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool ValidateMinPlayableMap();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void LogDirectorMessage(const FString& Message);

private:
    void SetupDisplayComponents();
    void UpdateMainDisplay();
    void UpdateAgentDisplay();
    void UpdateBiomeDisplay();
    
    float LastUpdateTime;
    bool bSystemInitialized;
};

#include "StudioDirectorSystem.generated.h"