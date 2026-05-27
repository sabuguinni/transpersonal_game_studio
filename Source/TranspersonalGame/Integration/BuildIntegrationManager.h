#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "BuildIntegrationManager.generated.h"

/**
 * Build Integration Manager - Handles cross-system integration and build validation
 * Ensures all game systems work together correctly and maintains build stability
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuildIntegrationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuildIntegrationManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Integration validation
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateActorCounts();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidatePerformance();

    // Build management
    UFUNCTION(BlueprintCallable, Category = "Build")
    void SaveCurrentBuild();

    UFUNCTION(BlueprintCallable, Category = "Build")
    bool LoadBuild(const FString& BuildName);

    UFUNCTION(BlueprintCallable, Category = "Build")
    TArray<FString> GetAvailableBuilds();

    // System monitoring
    UFUNCTION(BlueprintCallable, Category = "Monitoring")
    FBuild_SystemStatus GetSystemStatus();

    UFUNCTION(BlueprintCallable, Category = "Monitoring")
    void LogSystemMetrics();

    UFUNCTION(BlueprintCallable, Category = "Monitoring")
    void OptimizeMemory();

protected:
    // Integration state
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bSystemsValidated;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float LastValidationTime;

    // Performance metrics
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls;

    // Build tracking
    UPROPERTY(BlueprintReadOnly, Category = "Build")
    FString CurrentBuildVersion;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    TArray<FString> BuildHistory;

private:
    // Internal validation methods
    bool ValidateCharacterSystems();
    bool ValidateDinosaurSystems();
    bool ValidateEnvironmentSystems();
    bool ValidateAudioSystems();
    bool ValidateVFXSystems();

    // Performance monitoring
    void UpdatePerformanceMetrics();
    void CheckMemoryUsage();
    void OptimizeActorCounts();

    // Build utilities
    void CreateBuildSnapshot();
    void CleanupOldBuilds();
    void ValidateBuildIntegrity();

    // Timer handles
    FTimerHandle ValidationTimerHandle;
    FTimerHandle PerformanceTimerHandle;
    FTimerHandle BuildTimerHandle;
};