#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "StudioDirectorV43.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogStudioDirectorV43, Log, All);

UENUM(BlueprintType)
enum class EProductionPhase : uint8
{
    Initialization  UMETA(DisplayName = "Initialization"),
    Active         UMETA(DisplayName = "Active Production"),
    Handoff        UMETA(DisplayName = "Agent Handoff"),
    Complete       UMETA(DisplayName = "Complete")
};

USTRUCT(BlueprintType)
struct FAgentInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AgentID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bActive = false;

    FAgentInfo()
    {
        AgentID = 0;
        Name = TEXT("");
        Description = TEXT("");
        bActive = false;
    }
};

USTRUCT(BlueprintType)
struct FProductionAsset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString AssetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString AssetType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime CreationTime;

    FProductionAsset()
    {
        AssetName = TEXT("");
        AssetType = TEXT("");
        Status = TEXT("Pending");
        CreationTime = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FProjectHealthStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bModulesCompiled = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAssetPipelineReady = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSubsystemsActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bProjectHealthy = false;

    void Reset()
    {
        bModulesCompiled = false;
        bAssetPipelineReady = false;
        bSubsystemsActive = false;
        bProjectHealthy = false;
    }
};

USTRUCT(BlueprintType)
struct FAgentHandoffData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString FromAgent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ToAgent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CycleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FProjectHealthStatus ProjectHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FProductionAsset> ProductionAssets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime HandoffTime;

    FAgentHandoffData()
    {
        FromAgent = TEXT("");
        ToAgent = TEXT("");
        CycleID = TEXT("");
        HandoffTime = FDateTime::Now();
    }
};

/**
 * Studio Director V43 - Coordinates the 19-agent production pipeline
 * Responsible for vision translation, production asset creation, and agent chain management
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AStudioDirectorV43 : public AActor
{
    GENERATED_BODY()

public:
    AStudioDirectorV43();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Production Cycle Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString CurrentCycleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EProductionPhase ProductionPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDateTime CycleStartTime;

    // Agent Chain Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Chain")
    TArray<FAgentInfo> AgentChain;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Chain")
    int32 CurrentAgentIndex;

    // Production Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    TArray<FProductionAsset> ProductionAssets;

    // Project Health
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
    FProjectHealthStatus ProjectHealthStatus;

    // Handoff Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Handoff")
    FAgentHandoffData HandoffData;

public:
    // Core Functions
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void StartProductionCycle();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeAgentChain();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void SignalNextAgent();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FString GetProductionReport() const;

    // Production Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void CreateProductionAssets();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void ValidateProjectHealth();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void MonitorProductionPipeline(float DeltaTime);

    // Subsystem Integration
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void InitializeCoreSubsystems();

    // Getters
    UFUNCTION(BlueprintPure, Category = "Studio Director")
    EProductionPhase GetCurrentPhase() const { return ProductionPhase; }

    UFUNCTION(BlueprintPure, Category = "Studio Director")
    FString GetCurrentCycleID() const { return CurrentCycleID; }

    UFUNCTION(BlueprintPure, Category = "Studio Director")
    bool IsProjectHealthy() const { return ProjectHealthStatus.bProjectHealthy; }

    UFUNCTION(BlueprintPure, Category = "Studio Director")
    int32 GetProductionAssetCount() const { return ProductionAssets.Num(); }
};