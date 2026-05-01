#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "BuildIntegrationManager.generated.h"

/**
 * Build Integration Manager - Agent #19
 * Manages cross-module dependencies and build validation
 * Ensures all systems integrate properly and compile successfully
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
    bool ValidateAllModules();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool TestCrossModuleDependencies();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void GenerateBuildReport();

    // Module registration
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void RegisterModule(const FString& ModuleName, bool bIsActive);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool IsModuleActive(const FString& ModuleName) const;

    // Integration testing
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool TestMinPlayableMapIntegration();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ValidateActorSpawning();

protected:
    // Module tracking
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TMap<FString, bool> RegisteredModules;

    // Build status
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    bool bLastBuildSuccessful;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    FString LastBuildError;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    int32 TotalModulesRegistered;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    int32 ActiveModulesCount;

    // Integration test results
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TArray<FString> IntegrationTestResults;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    float LastIntegrationTestTime;

private:
    // Internal validation functions
    bool ValidateModuleHeaders();
    bool ValidateModuleImplementations();
    bool ValidateSharedTypes();
    bool TestActorClassLoading();
    void LogIntegrationStatus();
};