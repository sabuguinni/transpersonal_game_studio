#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "BuildIntegrationManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ModuleStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    bool bIsLoaded;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    bool bHasErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    int32 ClassCount;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    FString LastError;

    FBuild_ModuleStatus()
    {
        ModuleName = TEXT("");
        bIsLoaded = false;
        bHasErrors = false;
        ClassCount = 0;
        LastError = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    bool bCompilationSuccess;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    int32 ErrorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    int32 WarningCount;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    TArray<FString> ErrorMessages;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    TArray<FBuild_ModuleStatus> ModuleStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    float ValidationTime;

    FBuild_ValidationResult()
    {
        bCompilationSuccess = false;
        ErrorCount = 0;
        WarningCount = 0;
        ValidationTime = 0.0f;
    }
};

UCLASS(BlueprintType)
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
    FBuild_ValidationResult ValidateCurrentBuild();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool CheckModuleIntegrity(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FString> GetOrphanedHeaders();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateClassDependencies();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void GenerateIntegrationReport();

    // Module management
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FBuild_ModuleStatus> GetAllModuleStatuses();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ReloadModule(const FString& ModuleName);

    // Error tracking
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void LogBuildError(const FString& ErrorMessage, const FString& SourceFile);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FString> GetRecentErrors();

    // Integration testing
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool TestCrossModuleIntegration();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateGameplayFlow();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TArray<FString> RecentErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TArray<FBuild_ModuleStatus> CachedModuleStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    float LastValidationTime;

    // Timer handle for periodic validation
    FTimerHandle ValidationTimerHandle;

    // Internal validation functions
    bool ValidateSourceStructure();
    bool CheckForCircularDependencies();
    bool ValidateUPropertyExposure();
    void UpdateModuleStatuses();
    void CacheValidationResults(const FBuild_ValidationResult& Result);
};

#include "BuildIntegrationManager.generated.h"