#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "../SharedTypes.h"
#include "EngineArchitectureManager.generated.h"

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
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ArchitectureVisualization;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UTextRenderComponent* StatusDisplay;

    // Architecture Validation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bPerformArchitectureValidation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float ValidationInterval;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    bool bArchitectureValid;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    int32 TotalActorsInLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    int32 DuplicateActorsFound;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    int32 CompilationErrors;

    // Module System Tracking
    UPROPERTY(BlueprintReadOnly, Category = "Modules")
    TArray<FString> LoadedModules;

    UPROPERTY(BlueprintReadOnly, Category = "Modules")
    TArray<FString> FailedModules;

    UPROPERTY(BlueprintReadOnly, Category = "Modules")
    int32 ActiveCppClasses;

    // Performance Metrics
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFrameRate;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls;

    // Milestone 1 Architecture Requirements
    UPROPERTY(BlueprintReadOnly, Category = "Milestone1")
    bool bCharacterMovementValid;

    UPROPERTY(BlueprintReadOnly, Category = "Milestone1")
    bool bCameraSystemValid;

    UPROPERTY(BlueprintReadOnly, Category = "Milestone1")
    bool bTerrainValid;

    UPROPERTY(BlueprintReadOnly, Category = "Milestone1")
    bool bLightingValid;

    UPROPERTY(BlueprintReadOnly, Category = "Milestone1")
    bool bDinosaurActorsValid;

    UPROPERTY(BlueprintReadOnly, Category = "Milestone1")
    int32 DinosaurCount;

public:
    // Architecture Validation Functions
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ValidateArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void CleanupDuplicateActors();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ValidateModuleIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void CheckMilestone1Requirements();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FString GetArchitectureReport();

    // Module Management
    UFUNCTION(BlueprintCallable, Category = "Modules")
    bool IsModuleLoaded(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Modules")
    void RefreshModuleList();

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    void LogArchitectureStatus();

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void SetArchitectureVisualizationVisible(bool bVisible);

private:
    // Internal validation helpers
    void ValidateActorCounts();
    void ValidateSystemIntegrity();
    void UpdateStatusDisplay();
    
    float LastValidationTime;
    FString CurrentStatusText;
};