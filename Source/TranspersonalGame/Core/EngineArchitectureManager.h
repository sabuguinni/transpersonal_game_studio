#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "../SharedTypes.h"
#include "EngineArchitectureManager.generated.h"

/**
 * Engine Architecture Manager - Central system that enforces architectural rules
 * and coordinates technical systems for Milestone 1 implementation.
 * 
 * This actor ensures all technical systems follow UE5 best practices and
 * maintains performance standards for the dinosaur survival game.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEngineArchitectureManager : public AActor
{
    GENERATED_BODY()

public:
    AEngineArchitectureManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture")
    UStaticMeshComponent* VisualizationMesh;

    // Architecture Validation System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bEnableArchitectureValidation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float ValidationInterval;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    float LastValidationTime;

    // Performance Monitoring
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameRate;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFrameRate;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 CurrentActorCount;

    // Milestone 1 Technical Requirements
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone1")
    bool bEnforceTerrainLimits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone1")
    float MaxTerrainSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone1")
    int32 MaxDinosaurCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone1")
    bool bRequireNavMesh;

    // System Status Tracking
    UPROPERTY(BlueprintReadOnly, Category = "Status")
    TArray<FString> ValidationErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    TArray<FString> PerformanceWarnings;

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    bool bArchitectureHealthy;

public:
    // Architecture Validation Functions
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ValidateArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool CheckLightingSetup();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool CheckTerrainCompliance();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool CheckCharacterSystem();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool CheckDinosaurSystem();

    // Performance Monitoring Functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CheckActorCount();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CheckFrameRate();

    // Milestone 1 Enforcement Functions
    UFUNCTION(BlueprintCallable, Category = "Milestone1")
    void EnforceMilestone1Rules();

    UFUNCTION(BlueprintCallable, Category = "Milestone1")
    bool ValidateMinimumViablePrototype();

    UFUNCTION(BlueprintCallable, Category = "Milestone1")
    void ReportArchitectureStatus();

    // System Coordination Functions
    UFUNCTION(BlueprintCallable, Category = "Coordination")
    void NotifySystemReady(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    void NotifySystemError(const FString& SystemName, const FString& ErrorMessage);

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    TArray<FString> GetSystemReadinessReport();

    // Debug and Visualization
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void LogArchitectureReport();

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void VisualizeSystemBounds();

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void ClearValidationErrors();

private:
    // Internal validation helpers
    void ValidateLightingActors();
    void ValidateTerrainActors();
    void ValidateCharacterActors();
    void ValidateDinosaurActors();
    void ValidateNavigationSystem();
    
    // Performance helpers
    void CalculateFrameRate(float DeltaTime);
    void CountWorldActors();
    
    // System status tracking
    TMap<FString, bool> SystemReadiness;
    TArray<float> FrameTimeHistory;
    float FrameTimeAccumulator;
    int32 FrameTimeCount;
};