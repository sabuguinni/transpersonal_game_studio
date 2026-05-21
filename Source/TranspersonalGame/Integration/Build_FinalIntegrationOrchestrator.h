#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Build_FinalIntegrationOrchestrator.generated.h"

/**
 * Final Integration Orchestrator - Manages cross-system integration and build validation
 * Created by Integration & Build Agent #19 for comprehensive system coordination
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABuild_FinalIntegrationOrchestrator : public AActor
{
    GENERATED_BODY()

public:
    ABuild_FinalIntegrationOrchestrator();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Visual component for editor visibility
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Integration")
    UStaticMeshComponent* OrchestratorMesh;

    // Integration status tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Status")
    bool bCharacterSystemActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Status")
    bool bPhysicsSystemActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Status")
    bool bVFXSystemActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Status")
    bool bQASystemActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Status")
    int32 DinosaurAssetsLoaded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Status")
    int32 TotalActorsInWorld;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Status")
    float IntegrationScore;

    // System validation timer
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    float ValidationInterval;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float LastValidationTime;

public:
    // Integration validation functions
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateCharacterSystem();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidatePhysicsSystem();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateVFXSystem();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateQASystem();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    int32 ValidateDinosaurAssets();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void UpdateIntegrationScore();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void LogIntegrationStatus();

    // Cross-system communication
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void NotifySystemChange(const FString& SystemName, bool bIsActive);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FString GetIntegrationReport();

    // Build validation
    UFUNCTION(BlueprintCallable, Category = "Build")
    bool ValidateBuildIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Build")
    void TriggerBuildValidation();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void MonitorPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetTotalActorCount();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetMemoryUsage();

private:
    // Internal validation helpers
    void PerformPeriodicValidation();
    void UpdateSystemStatus();
    void BroadcastIntegrationStatus();
};