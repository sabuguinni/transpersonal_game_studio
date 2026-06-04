#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Build_FinalCycleDeliveryOrchestrator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_FinalDeliveryMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Delivery Metrics")
    int32 IntegratedAgentSystems = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Delivery Metrics")
    int32 TotalAgentSystems = 9;

    UPROPERTY(BlueprintReadOnly, Category = "Delivery Metrics")
    int32 BuildHealthComponents = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Delivery Metrics")
    int32 TotalBuildComponents = 4;

    UPROPERTY(BlueprintReadOnly, Category = "Delivery Metrics")
    int32 WorldActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Delivery Metrics")
    FString BuildStatus = TEXT("UNKNOWN");

    UPROPERTY(BlueprintReadOnly, Category = "Delivery Metrics")
    bool bProductionReady = false;

    UPROPERTY(BlueprintReadOnly, Category = "Delivery Metrics")
    float IntegrationScore = 0.0f;

    FBuild_FinalDeliveryMetrics()
    {
        IntegratedAgentSystems = 0;
        TotalAgentSystems = 9;
        BuildHealthComponents = 0;
        TotalBuildComponents = 4;
        WorldActorCount = 0;
        BuildStatus = TEXT("UNKNOWN");
        bProductionReady = false;
        IntegrationScore = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_AgentIntegrationStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Agent Status")
    FString AgentName = TEXT("");

    UPROPERTY(BlueprintReadOnly, Category = "Agent Status")
    int32 LoadedClasses = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Agent Status")
    int32 TotalClasses = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Agent Status")
    bool bFullyIntegrated = false;

    UPROPERTY(BlueprintReadOnly, Category = "Agent Status")
    TArray<FString> MissingClasses;

    FBuild_AgentIntegrationStatus()
    {
        AgentName = TEXT("");
        LoadedClasses = 0;
        TotalClasses = 0;
        bFullyIntegrated = false;
        MissingClasses.Empty();
    }
};

/**
 * Final Cycle Delivery Orchestrator - Coordinates the completion and delivery of Cycle 006
 * Validates all agent integrations, build health, and determines production readiness
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABuild_FinalCycleDeliveryOrchestrator : public AActor
{
    GENERATED_BODY()

public:
    ABuild_FinalCycleDeliveryOrchestrator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Final delivery orchestration
    UFUNCTION(BlueprintCallable, Category = "Final Delivery")
    void ExecuteFinalDeliveryOrchestration();

    UFUNCTION(BlueprintCallable, Category = "Final Delivery")
    FBuild_FinalDeliveryMetrics ValidateAllAgentIntegrations();

    UFUNCTION(BlueprintCallable, Category = "Final Delivery")
    bool ValidateBuildHealth();

    UFUNCTION(BlueprintCallable, Category = "Final Delivery")
    void GenerateFinalDeliveryReport();

    // Agent integration validation
    UFUNCTION(BlueprintCallable, Category = "Agent Integration")
    FBuild_AgentIntegrationStatus ValidateAgentIntegration(const FString& AgentName, const TArray<FString>& RequiredClasses);

    UFUNCTION(BlueprintCallable, Category = "Agent Integration")
    TArray<FBuild_AgentIntegrationStatus> GetAllAgentIntegrationStatus();

    // Build status determination
    UFUNCTION(BlueprintCallable, Category = "Build Status")
    FString DetermineFinalBuildStatus(const FBuild_FinalDeliveryMetrics& Metrics);

    UFUNCTION(BlueprintCallable, Category = "Build Status")
    bool IsProductionReady(const FBuild_FinalDeliveryMetrics& Metrics);

    UFUNCTION(BlueprintCallable, Category = "Build Status")
    float CalculateIntegrationScore(const FBuild_FinalDeliveryMetrics& Metrics);

protected:
    // Final delivery state
    UPROPERTY(BlueprintReadOnly, Category = "Delivery State")
    FBuild_FinalDeliveryMetrics CurrentDeliveryMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Delivery State")
    TArray<FBuild_AgentIntegrationStatus> AgentIntegrationStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Delivery State")
    bool bDeliveryOrchestrationComplete = false;

    UPROPERTY(BlueprintReadOnly, Category = "Delivery State")
    FDateTime DeliveryCompletionTime;

    // Agent system definitions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Systems")
    TMap<FString, TArray<FString>> AgentSystemClasses;

    // Build component definitions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Components")
    TArray<FString> CriticalBuildComponents;

private:
    void InitializeAgentSystemDefinitions();
    void InitializeBuildComponentDefinitions();
    bool ValidateClassExists(const FString& ClassName);
    void LogDeliveryProgress(const FString& Message);
};