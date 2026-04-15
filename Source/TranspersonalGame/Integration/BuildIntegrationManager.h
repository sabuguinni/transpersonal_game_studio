#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "BuildIntegrationManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBuildValidationComplete, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnModuleStatusChanged, FString, ModuleName, EBuild_ModuleStatus, Status);

/**
 * Build Integration Manager
 * Orchestrates integration between all game systems and validates build health
 * Ensures all modules work together cohesively
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuildIntegrationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuildIntegrationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Build validation
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ValidateBuildHealth();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool IsModuleHealthy(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    float GetOverallBuildHealth() const;

    // Module registration
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void RegisterModule(const FString& ModuleName, EBuild_ModuleStatus Status);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void UpdateModuleStatus(const FString& ModuleName, EBuild_ModuleStatus NewStatus);

    // Integration testing
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void RunIntegrationTests();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool TestModuleInteraction(const FString& ModuleA, const FString& ModuleB);

    // Build reporting
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FBuild_HealthReport GenerateHealthReport() const;

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FString> GetFailedModules() const;

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FString> GetHealthyModules() const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Build Integration")
    FOnBuildValidationComplete OnBuildValidationComplete;

    UPROPERTY(BlueprintAssignable, Category = "Build Integration")
    FOnModuleStatusChanged OnModuleStatusChanged;

protected:
    // Module tracking
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration", meta = (AllowPrivateAccess = "true"))
    TMap<FString, EBuild_ModuleStatus> ModuleStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration", meta = (AllowPrivateAccess = "true"))
    TMap<FString, float> ModuleHealthScores;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration", meta = (AllowPrivateAccess = "true"))
    TArray<FString> CriticalModules;

    // Integration state
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration", meta = (AllowPrivateAccess = "true"))
    bool bIntegrationTestsRunning;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration", meta = (AllowPrivateAccess = "true"))
    float LastValidationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration", meta = (AllowPrivateAccess = "true"))
    int32 ValidationCycleCount;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Integration")
    float HealthValidationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Integration")
    float MinimumHealthThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Integration")
    bool bAutoRunValidation;

private:
    // Internal validation methods
    void ValidateModuleHealth(const FString& ModuleName);
    void ValidateModuleInteractions();
    void ValidateSystemIntegration();
    
    // Health calculation
    float CalculateModuleHealth(const FString& ModuleName) const;
    float CalculateOverallHealth() const;
    
    // Integration testing
    bool TestWorldGeneration();
    bool TestCharacterSystems();
    bool TestAISystems();
    bool TestCombatSystems();
    bool TestAudioSystems();
    bool TestVFXSystems();
    
    // Reporting
    void LogHealthReport(const FBuild_HealthReport& Report) const;
    void NotifyHealthChange(const FString& ModuleName, float OldHealth, float NewHealth);

    // Timer handles
    FTimerHandle ValidationTimerHandle;
};