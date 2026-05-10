#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Components/SceneComponent.h"
#include "Dir_ProductionCoordinator.generated.h"

/**
 * Studio Director's Production Coordination System
 * Tracks the 18-agent pipeline and manages development milestones
 * Used for coordinating agent deliverables and monitoring build status
 */

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Pending     UMETA(DisplayName = "Pending"),
    Active      UMETA(DisplayName = "Active"), 
    Completed   UMETA(DisplayName = "Completed"),
    Blocked     UMETA(DisplayName = "Blocked"),
    Failed      UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EDir_BuildStatus : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    Compiling       UMETA(DisplayName = "Compiling"),
    Success         UMETA(DisplayName = "Success"),
    Failed          UMETA(DisplayName = "Failed"),
    Blocked         UMETA(DisplayName = "Blocked")
};

USTRUCT(BlueprintType)
struct FDir_AgentInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Info")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Info")
    FString Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Info")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Info")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Info")
    TArray<FString> Deliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Info")
    FString LastUpdate;

    FDir_AgentInfo()
    {
        AgentName = TEXT("");
        Role = TEXT("");
        Status = EDir_AgentStatus::Pending;
        LastUpdate = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalHeaderFiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalImplementationFiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 OrphanedHeaders;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CompilationErrors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    EDir_BuildStatus BuildStatus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 MapActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    bool bHasPlayableCharacter;

    FDir_ProductionMetrics()
    {
        TotalHeaderFiles = 0;
        TotalImplementationFiles = 0;
        OrphanedHeaders = 0;
        CompilationErrors = 0;
        BuildStatus = EDir_BuildStatus::Unknown;
        MapActorCount = 0;
        bHasPlayableCharacter = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADir_ProductionCoordinator : public AActor
{
    GENERATED_BODY()

public:
    ADir_ProductionCoordinator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core coordination system
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Agent pipeline tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Pipeline")
    TArray<FDir_AgentInfo> AgentPipeline;

    // Current production metrics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    FDir_ProductionMetrics CurrentMetrics;

    // Cycle management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cycle Management")
    FString CurrentCycleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cycle Management")
    int32 CurrentAgentIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cycle Management")
    TArray<FString> CriticalBlockers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cycle Management")
    TArray<FString> ImmediatePriorities;

    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    void InitializeAgentPipeline();

    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    void UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    void RefreshProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    FDir_AgentInfo GetCurrentActiveAgent() const;

    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    TArray<FString> GetCriticalBlockers() const;

    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    bool CanAgentProceed(const FString& AgentName) const;

    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    void LogProductionStatus() const;

    // Editor-only functions
    UFUNCTION(CallInEditor, Category = "Development Tools")
    void DebugPrintAgentPipeline();

    UFUNCTION(CallInEditor, Category = "Development Tools")
    void ForceRefreshMetrics();

private:
    // Internal tracking
    float LastMetricsUpdate;
    static constexpr float MetricsUpdateInterval = 30.0f; // Update every 30 seconds

    // Helper functions
    void SetupDefaultAgentPipeline();
    void UpdateBuildStatus();
    bool CheckAgentDependencies(const FString& AgentName) const;
};