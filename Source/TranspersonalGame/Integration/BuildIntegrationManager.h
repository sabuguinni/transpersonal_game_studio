#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "BuildIntegrationManager.generated.h"

/**
 * Build Integration Manager - Agent #19
 * Manages build validation, system integration, and compilation health
 * Ensures all agent outputs integrate correctly into a playable build
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

    // Build validation functions
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool CheckCompilationHealth();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    int32 GetOrphanedHeaderCount();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FString> GetCriticalMissingImplementations();

    // System status checks
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool IsCharacterSystemOperational();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool IsGameModeSystemOperational();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool IsWorldGenSystemOperational();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool IsLevelSystemOperational();

    // Integration reporting
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FString GenerateIntegrationReport();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    float GetSystemHealthPercentage();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    EBuild_IntegrationStatus GetBuildStatus();

    // Build management
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void SaveBuildSnapshot(const FString& SnapshotName);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool RestoreBuildSnapshot(const FString& SnapshotName);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FString> GetAvailableSnapshots();

protected:
    // Internal validation functions
    bool ValidateModuleDependencies();
    bool ValidateClassLoading();
    bool ValidateActorSpawning();
    bool ValidateLevelContent();

    // System health tracking
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TMap<FString, bool> SystemHealthStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    int32 TotalSystemsCount;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    int32 OperationalSystemsCount;

    // Build snapshots
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TArray<FString> BuildSnapshots;

    // Integration metrics
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    float LastValidationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    int32 OrphanedHeadersCount;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TArray<FString> CriticalMissingFiles;

    // Event delegates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnIntegrationStatusChanged, EBuild_IntegrationStatus, NewStatus);
    UPROPERTY(BlueprintAssignable, Category = "Build Integration")
    FOnIntegrationStatusChanged OnIntegrationStatusChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSystemHealthChanged, float, HealthPercentage);
    UPROPERTY(BlueprintAssignable, Category = "Build Integration")
    FOnSystemHealthChanged OnSystemHealthChanged;

private:
    // Internal state
    EBuild_IntegrationStatus CurrentBuildStatus;
    float LastHealthPercentage;
    
    // Validation cache
    bool bValidationCacheValid;
    double LastValidationTimestamp;
    static constexpr double ValidationCacheTimeout = 30.0; // 30 seconds
};