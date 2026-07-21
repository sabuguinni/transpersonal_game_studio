#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Eng_CoreArchitectureManager.generated.h"

/**
 * Engine Architect - Core Architecture Manager
 * Central system that orchestrates all architectural standards and enforces
 * compilation integrity across the entire TranspersonalGame module.
 * 
 * CYCLE 005 PRIORITY: Fix orphaned headers and establish compilation baseline
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_CoreArchitectureManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_CoreArchitectureManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core Architecture Management
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void InitializeArchitecturalStandards();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateModuleIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void FixOrphanedHeaders();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void EnforceCompilationStandards();

    // Module Health Monitoring
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    int32 GetOrphanedHeaderCount() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FString> GetCriticalMissingFiles() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsModuleCompilationHealthy() const;

    // Architectural Standards Enforcement
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void EnforceNamingConventions();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void ValidateIncludeStructure();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void CheckForDuplicateDefinitions();

protected:
    // Core architectural state
    UPROPERTY(BlueprintReadOnly, Category = "Architecture State")
    bool bIsInitialized;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture State")
    bool bCompilationHealthy;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture State")
    int32 OrphanedHeaderCount;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture State")
    int32 TotalHeaderFiles;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture State")
    int32 TotalCppFiles;

    // Critical missing files tracking
    UPROPERTY(BlueprintReadOnly, Category = "Architecture State")
    TArray<FString> CriticalMissingFiles;

    // Architectural standards
    UPROPERTY(BlueprintReadOnly, Category = "Architecture Standards")
    TArray<FString> RequiredModules;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture Standards")
    TArray<FString> CoreSystemClasses;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture Standards")
    TArray<FString> MandatoryHeaders;

private:
    // Internal validation methods
    void ScanModuleStructure();
    void IdentifyOrphanedHeaders();
    void ValidateCoreSystemsIntegrity();
    void CheckModuleDependencies();
    
    // Compilation fixing methods
    void CreateMissingCppFiles();
    void FixIncludeErrors();
    void ResolveCircularDependencies();
    
    // Architectural enforcement
    void ApplyNamingStandards();
    void ValidateClassHierarchy();
    void CheckUPropertyUsage();
};