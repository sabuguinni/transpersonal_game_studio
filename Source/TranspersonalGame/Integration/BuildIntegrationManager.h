#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "UObject/ObjectMacros.h"
#include "../SharedTypes.h"
#include "BuildIntegrationManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBuildStatusChanged, EBuildStatus, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnModuleValidated, FString, ModuleName, bool, bIsValid);

/**
 * Build Integration Manager - Manages compilation, validation and integration of all game modules
 * Responsible for ensuring all agent outputs work together as a cohesive game
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
    void StartBuildValidation();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ValidateModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool IsModuleValid(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    EBuildStatus GetCurrentBuildStatus() const { return CurrentBuildStatus; }

    // Integration Testing
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void RunIntegrationTests();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ValidateActorSpawning();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ValidateSystemInteractions();

    // Cleanup Operations
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void CleanupOrphanedHeaders();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void RemoveDuplicateActors();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void OptimizeMapPerformance();

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Build Integration")
    FOnBuildStatusChanged OnBuildStatusChanged;

    UPROPERTY(BlueprintAssignable, Category = "Build Integration")
    FOnModuleValidated OnModuleValidated;

protected:
    // Build Status
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    EBuildStatus CurrentBuildStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TMap<FString, bool> ModuleValidationStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TArray<FString> ValidationErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    int32 TotalActorsInMap;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    int32 ValidatedModules;

    // Integration Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Integration")
    bool bAutoValidateOnStartup;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Integration")
    bool bCleanupOrphansOnValidation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Integration")
    float ValidationTimeout;

private:
    // Internal validation methods
    void ValidateClassLoading();
    void ValidateActorIntegrity();
    void ValidateSystemDependencies();
    void UpdateBuildStatus(EBuildStatus NewStatus);
    
    // Cleanup helpers
    void IdentifyOrphanedFiles();
    void RemoveInvalidActors();
    void ConsolidateDuplicateSystems();

    // Timer handles
    FTimerHandle ValidationTimerHandle;
    FTimerHandle CleanupTimerHandle;
};