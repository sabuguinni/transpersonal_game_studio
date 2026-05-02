#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "Eng_TechnicalArchitecture.generated.h"

/**
 * Technical Architecture Manager - Engine Architect Agent #2
 * Defines and enforces the core technical architecture for the entire game
 * Ensures all 19 agents follow consistent patterns and standards
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemRequirements
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Requirements")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Requirements")
    int32 MinMemoryMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Requirements")
    float MaxCPUUsagePercent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Requirements")
    int32 MaxDrawCalls;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Requirements")
    bool bRequiresWorldPartition;

    FEng_SystemRequirements()
    {
        SystemName = TEXT("Unknown");
        MinMemoryMB = 512;
        MaxCPUUsagePercent = 10.0f;
        MaxDrawCalls = 1000;
        bRequiresWorldPartition = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ArchitectureRule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture Rule")
    FString RuleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture Rule")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture Rule")
    EDir_Priority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture Rule")
    bool bMandatory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture Rule")
    TArray<FString> AffectedAgents;

    FEng_ArchitectureRule()
    {
        RuleName = TEXT("Default Rule");
        Description = TEXT("No description");
        Priority = EDir_Priority::Medium;
        bMandatory = false;
    }
};

UENUM(BlueprintType)
enum class EEng_ArchitectureLayer : uint8
{
    Core            UMETA(DisplayName = "Core Systems"),
    Gameplay        UMETA(DisplayName = "Gameplay Systems"),
    Presentation    UMETA(DisplayName = "Presentation Layer"),
    Platform        UMETA(DisplayName = "Platform Layer"),
    Tools           UMETA(DisplayName = "Tools & Pipeline")
};

UENUM(BlueprintType)
enum class EEng_SystemCategory : uint8
{
    Physics         UMETA(DisplayName = "Physics & Collision"),
    Rendering       UMETA(DisplayName = "Rendering & Graphics"),
    Audio           UMETA(DisplayName = "Audio & Sound"),
    AI              UMETA(DisplayName = "AI & Behavior"),
    Networking      UMETA(DisplayName = "Networking"),
    Input           UMETA(DisplayName = "Input & Controls"),
    UI              UMETA(DisplayName = "User Interface"),
    World           UMETA(DisplayName = "World Generation"),
    Character       UMETA(DisplayName = "Character Systems"),
    Combat          UMETA(DisplayName = "Combat & Interaction")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_TechnicalArchitecture : public UActorComponent
{
    GENERATED_BODY()

public:
    UEng_TechnicalArchitecture();

protected:
    virtual void BeginPlay() override;

    // Core Architecture Rules
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture Rules")
    TArray<FEng_ArchitectureRule> CoreRules;

    // System Requirements
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Requirements")
    TArray<FEng_SystemRequirements> SystemRequirements;

    // Performance Targets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 TargetFPS_PC;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 TargetFPS_Console;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxMemoryUsageMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxFrameTimeMS;

    // World Partition Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Partition")
    bool bEnforceWorldPartition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Partition")
    float WorldPartitionCellSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Partition")
    int32 MaxLoadedCells;

    // Lumen Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
    bool bEnableLumen;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
    float LumenSceneDetailLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
    int32 LumenMaxTraceDistance;

    // Nanite Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nanite")
    bool bEnableNanite;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nanite")
    int32 NaniteMaxTriangles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nanite")
    float NaniteLODBias;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Architecture Validation Functions
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateSystemCompliance(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void EnforceArchitectureRules();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FEng_SystemRequirements GetSystemRequirements(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RegisterNewSystem(const FString& SystemName, EEng_SystemCategory Category, const FEng_SystemRequirements& Requirements);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FString> GetNonCompliantSystems();

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool CheckPerformanceTargets();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFrameTime();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetCurrentMemoryUsage();

    // Milestone 1 Validation
    UFUNCTION(BlueprintCallable, Category = "Milestone")
    bool ValidateMilestone1Requirements();

    UFUNCTION(BlueprintCallable, Category = "Milestone")
    int32 GetMilestone1Progress();

    UFUNCTION(BlueprintCallable, Category = "Milestone")
    TArray<FString> GetMissingMilestone1Requirements();

    // Agent Coordination
    UFUNCTION(BlueprintCallable, Category = "Coordination")
    void NotifyAgentProgress(int32 AgentID, const FString& TaskCompleted);

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    bool IsAgentReadyForWork(int32 AgentID);

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    TArray<FString> GetBlockedAgents();

private:
    // Internal tracking
    TMap<FString, bool> SystemComplianceStatus;
    TMap<int32, FString> AgentLastTask;
    TMap<int32, float> AgentLastUpdateTime;

    // Architecture enforcement
    void ValidateWorldPartitionSetup();
    void ValidateLumenConfiguration();
    void ValidateNaniteSettings();
    void CheckSystemDependencies();
    
    // Performance monitoring
    float LastFrameTime;
    int32 LastMemoryUsage;
    TArray<float> FrameTimeHistory;
    
    // Milestone tracking
    bool bMilestone1CharacterMovement;
    bool bMilestone1TerrainVariation;
    bool bMilestone1DinosaurMeshes;
    bool bMilestone1LightingSystem;
    bool bMilestone1PlayerInteraction;
};

/**
 * Technical Architecture Actor - Spawnable in levels for architecture management
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEng_TechnicalArchitectureActor : public AActor
{
    GENERATED_BODY()

public:
    AEng_TechnicalArchitectureActor();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UEng_TechnicalArchitecture* ArchitectureComponent;

    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    UEng_TechnicalArchitecture* GetArchitectureComponent() const { return ArchitectureComponent; }
};