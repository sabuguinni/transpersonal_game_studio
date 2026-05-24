#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "Subsystems/WorldSubsystem.h"
#include "Build_FinalSystemIntegrator.generated.h"

UENUM(BlueprintType)
enum class EBuild_IntegrationPhase : uint8
{
    PreValidation,
    SystemDiscovery,
    DependencyMapping,
    CrossSystemValidation,
    PerformanceValidation,
    FinalIntegration,
    PostIntegrationTest,
    Complete
};

USTRUCT(BlueprintType)
struct FBuild_SystemIntegrationMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration Metrics")
    int32 TotalSystemsDiscovered = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Metrics")
    int32 SystemsValidated = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Metrics")
    int32 CrossSystemConnections = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Metrics")
    int32 PerformanceIssuesFound = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Metrics")
    float TotalIntegrationTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Metrics")
    bool bIntegrationComplete = false;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Metrics")
    FString LastIntegrationError;
};

USTRUCT(BlueprintType)
struct FBuild_SystemDependency
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "System Dependency")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "System Dependency")
    TArray<FString> Dependencies;

    UPROPERTY(BlueprintReadOnly, Category = "System Dependency")
    int32 Priority = 0;

    UPROPERTY(BlueprintReadOnly, Category = "System Dependency")
    bool bIsCore = false;

    UPROPERTY(BlueprintReadOnly, Category = "System Dependency")
    bool bValidated = false;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_FinalSystemIntegrator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_FinalSystemIntegrator();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void StartFinalIntegration();

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void PerformCrossSystemValidation();

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void RunPerformanceValidation();

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void CompleteIntegration();

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    FBuild_SystemIntegrationMetrics GetIntegrationMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    bool IsIntegrationComplete() const;

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void ResetIntegration();

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void DiscoverAllSystems();

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void MapSystemDependencies();

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    TArray<FBuild_SystemDependency> GetSystemDependencies() const;

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void ValidateSystemDependency(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void LogIntegrationStatus();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Integration State")
    EBuild_IntegrationPhase CurrentPhase;

    UPROPERTY(BlueprintReadOnly, Category = "Integration State")
    FBuild_SystemIntegrationMetrics IntegrationMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Integration State")
    TArray<FBuild_SystemDependency> SystemDependencies;

    UPROPERTY(BlueprintReadOnly, Category = "Integration State")
    TArray<FString> DiscoveredSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Integration State")
    TArray<FString> ValidatedSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Integration State")
    TArray<FString> FailedSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Integration State")
    float IntegrationStartTime;

    UPROPERTY(BlueprintReadOnly, Category = "Integration State")
    bool bIntegrationInProgress;

private:
    void AdvanceToNextPhase();
    void ProcessCurrentPhase();
    void ValidateIndividualSystem(const FString& SystemName);
    void CheckSystemPerformance(const FString& SystemName);
    void LogSystemStatus(const FString& SystemName, bool bSuccess);
    void UpdateIntegrationMetrics();
    void HandleIntegrationError(const FString& ErrorMessage);
    void FinalizeIntegration();
};