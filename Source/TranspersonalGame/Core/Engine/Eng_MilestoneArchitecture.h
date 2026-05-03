#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "../SharedTypes.h"
#include "Eng_MilestoneArchitecture.generated.h"

/**
 * Engine Architect's Milestone Architecture Manager
 * Defines and enforces technical rules for Milestone 1 completion
 * Validates that each agent produces functional, visible gameplay elements
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_AgentTechnicalRules
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Technical Rules")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Technical Rules")
    TArray<FString> MandatoryClasses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Technical Rules")
    TArray<FString> ForbiddenPatterns;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Technical Rules")
    TArray<FString> RequiredFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Technical Rules")
    TArray<FString> PerformanceLimits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Technical Rules")
    FString RequiredDeliverable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Technical Rules")
    bool bRulesCompleted;

    FEng_AgentTechnicalRules()
    {
        AgentName = TEXT("Unknown");
        bRulesCompleted = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemValidationResult
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    bool bCompilationSuccess;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    bool bRuntimeSuccess;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    bool bVisibilityTest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    FString ValidationNotes;

    FEng_SystemValidationResult()
    {
        SystemName = TEXT("Unknown");
        bCompilationSuccess = false;
        bRuntimeSuccess = false;
        bVisibilityTest = false;
        ValidationNotes = TEXT("");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEng_MilestoneArchitecture : public AActor
{
    GENERATED_BODY()

public:
    AEng_MilestoneArchitecture();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Technical Rules Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture Rules")
    TArray<FEng_AgentTechnicalRules> MilestoneAgentRules;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture Rules")
    TArray<FString> GlobalValidationCheckpoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture Rules")
    float PerformanceTargetFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture Rules")
    int32 MaxActiveAICount;

    // System Validation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    TArray<FEng_SystemValidationResult> SystemValidationResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    bool bMilestone1Complete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    float ValidationCheckInterval;

    // Visual Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ArchitectureMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UTextRenderComponent* StatusDisplay;

    // Core Architecture Functions
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void InitializeMilestoneRules();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateAgentDeliverable(const FString& AgentName, const FString& DeliverableType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RunSystemValidation();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool CheckCriticalComponentsPresent();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void UpdateMilestoneProgress();

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFPS();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool ValidatePerformanceTargets();

    // Agent Coordination
    UFUNCTION(BlueprintCallable, Category = "Coordination")
    TArray<FString> GetPendingAgentTasks();

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    void MarkAgentTaskComplete(const FString& AgentName, const FString& TaskName);

    // Critical System Checks
    UFUNCTION(BlueprintCallable, Category = "Critical Systems")
    bool ValidateCharacterSystem();

    UFUNCTION(BlueprintCallable, Category = "Critical Systems")
    bool ValidateWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "Critical Systems")
    bool ValidateDinosaurAI();

    UFUNCTION(BlueprintCallable, Category = "Critical Systems")
    bool ValidateSurvivalHUD();

    UFUNCTION(BlueprintCallable, Category = "Critical Systems")
    bool ValidateLightingSystem();

private:
    // Internal validation timer
    float ValidationTimer;
    
    // Milestone completion tracking
    int32 CompletedAgentTasks;
    int32 TotalRequiredTasks;

    // Performance monitoring
    TArray<float> RecentFPSReadings;
    float LastFPSCheck;

    // Helper functions
    void SetupArchitectureDisplay();
    void UpdateStatusDisplay();
    FString GetMilestoneProgressText();
};