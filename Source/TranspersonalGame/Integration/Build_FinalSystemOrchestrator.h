#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "Build_FinalSystemOrchestrator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemIntegrationMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalActorsInWorld = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 VFXSystemsActive = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 QAValidationsPassed = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float SystemIntegrationScore = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bAllCriticalSystemsOnline = false;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString LastIntegrationTimestamp;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> ActiveSystemNames;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> FailedSystemNames;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_CycleCompletionData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Cycle")
    int32 CycleNumber = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle")
    float CycleExecutionTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle")
    int32 SystemsIntegrated = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle")
    int32 ValidationTestsPassed = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle")
    bool bCycleSuccessful = false;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle")
    FString CycleCompletionReport;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_FinalSystemOrchestrator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_FinalSystemOrchestrator();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void InitializeSystemOrchestration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ExecuteFinalCycleIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateAllSystemsIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void GenerateIntegrationReport();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void CompleteCycleOrchestration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_SystemIntegrationMetrics GetCurrentIntegrationMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_CycleCompletionData GetCycleCompletionData() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateSystemDependencies();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void OrchestrateCrossSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void FinalizeSystemDeployment();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FBuild_SystemIntegrationMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FBuild_CycleCompletionData CycleData;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<AActor*> ManagedActors;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TMap<FString, bool> SystemValidationResults;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float IntegrationStartTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bOrchestrationActive = false;

private:
    void UpdateIntegrationMetrics();
    void ValidateVFXSystems();
    void ValidateQASystems();
    void ValidateWorldSystems();
    void GenerateSystemReport();
    void LogIntegrationStatus();
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABuild_IntegrationOrchestratorActor : public AActor
{
    GENERATED_BODY()

public:
    ABuild_IntegrationOrchestratorActor();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Integration")
    class UStaticMeshComponent* OrchestratorMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Integration")
    class USceneComponent* IntegrationRoot;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    UBuild_FinalSystemOrchestrator* OrchestratorSubsystem;

public:
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void TriggerSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void DisplayIntegrationStatus();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ExecuteEmergencyIntegration();
};