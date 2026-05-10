#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "BuildIntegrationManager.generated.h"

/**
 * Build Integration Manager - Agent #19
 * Manages build validation, module integration, and compilation status
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
    bool ValidateModuleIntegration();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool CheckOrphanedHeaders();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateActorSpawning();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool TestBiomeCoordinates();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateSharedTypes();

    // Compilation status
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    EBuild_CompilationStatus GetLastCompilationStatus() const;

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void TriggerBuildValidation();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FString> GetCompilationErrors() const;

    // Integration reporting
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FBuild_IntegrationReport GenerateIntegrationReport();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void LogIntegrationStatus();

protected:
    // Build status tracking
    UPROPERTY(BlueprintReadOnly, Category = "Build Status")
    EBuild_CompilationStatus LastCompilationStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Build Status")
    TArray<FString> CompilationErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Build Status")
    TArray<FString> OrphanedHeaders;

    UPROPERTY(BlueprintReadOnly, Category = "Build Status")
    int32 TotalActorsInLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Build Status")
    float LastValidationTime;

    // Integration validation
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TMap<FString, bool> ModuleValidationStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_ModuleInfo> LoadedModules;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FBuild_BiomeValidation BiomeValidation;

private:
    // Internal validation functions
    bool ValidateSourceStructure();
    bool ValidateBinaryFiles();
    bool ValidateActorDistribution();
    void UpdateModuleStatus();
    void ScanForOrphanedHeaders();
    void ValidateBiomeActorPlacement();
    
    // Compilation helpers
    void ParseCompilationOutput(const FString& BuildOutput);
    void SaveValidationResults();
    void NotifyValidationComplete();
};