#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "BuildIntegrationManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBuildStatusChanged, EBuild_BuildStatus, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnModuleStatusChanged, FString, ModuleName, EBuild_ModuleStatus, Status);

/**
 * Build Integration Manager - Coordinates compilation, module loading, and system integration
 * Manages the build pipeline and ensures all systems work together correctly
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

    // Build Management
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void StartBuildProcess();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    EBuild_BuildStatus GetCurrentBuildStatus() const { return CurrentBuildStatus; }

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FString> GetFailedModules() const;

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    float GetBuildProgress() const { return BuildProgress; }

    // Module Integration
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void RegisterModuleForValidation(const FString& ModuleName, const TArray<FString>& RequiredClasses);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    EBuild_ModuleStatus GetModuleStatus(const FString& ModuleName) const;

    // System Integration
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void RunIntegrationTests();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FString> GetIntegrationErrors() const { return IntegrationErrors; }

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Build Integration")
    FOnBuildStatusChanged OnBuildStatusChanged;

    UPROPERTY(BlueprintAssignable, Category = "Build Integration")
    FOnModuleStatusChanged OnModuleStatusChanged;

protected:
    // Build Status
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration", meta = (AllowPrivateAccess = "true"))
    EBuild_BuildStatus CurrentBuildStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration", meta = (AllowPrivateAccess = "true"))
    float BuildProgress;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration", meta = (AllowPrivateAccess = "true"))
    TArray<FString> IntegrationErrors;

    // Module Management
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration", meta = (AllowPrivateAccess = "true"))
    TMap<FString, EBuild_ModuleStatus> ModuleStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration", meta = (AllowPrivateAccess = "true"))
    TMap<FString, TArray<FString>> ModuleRequiredClasses;

    // System Validation
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration", meta = (AllowPrivateAccess = "true"))
    TArray<FString> CoreSystemClasses;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration", meta = (AllowPrivateAccess = "true"))
    TArray<FString> ValidatedSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration", meta = (AllowPrivateAccess = "true"))
    TArray<FString> FailedSystems;

private:
    // Internal validation methods
    bool ValidateClassLoading(const FString& ClassName);
    bool ValidateCoreGameSystems();
    bool ValidateWorldGeneration();
    bool ValidateCharacterSystems();
    bool ValidateAISystems();
    bool ValidateEnvironmentSystems();

    void UpdateBuildStatus(EBuild_BuildStatus NewStatus);
    void UpdateModuleStatus(const FString& ModuleName, EBuild_ModuleStatus NewStatus);
    void LogIntegrationError(const FString& Error);

    // Timer handles for async operations
    FTimerHandle BuildValidationTimer;
    FTimerHandle SystemCheckTimer;
};