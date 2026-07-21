#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Eng_CompilationCleaner.generated.h"

/**
 * Engine Architect's Compilation Cleaner
 * Responsible for detecting and fixing orphaned headers, duplicate systems, and compilation blockers
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_CompilationCleaner : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_CompilationCleaner();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core cleaning functions
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Engine Architecture")
    void ScanForOrphanedHeaders();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Engine Architecture")
    void DetectDuplicateSystems();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Engine Architecture")
    void ValidateCompilationStatus();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Engine Architecture")
    void GenerateCleanupReport();

    // Compilation health monitoring
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Engine Architecture")
    bool CheckCriticalSystemsLoaded();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Engine Architecture")
    void FixCommonCompilationErrors();

protected:
    // Internal validation functions
    void ScanSourceDirectory(const FString& Directory);
    void AnalyzeHeaderDependencies();
    void CheckModuleDependencies();
    void ValidateUClassDeclarations();

    // Cleanup utilities
    void RemoveOrphanedHeaders();
    void ConsolidateDuplicateSystems();
    void FixIncludePaths();

private:
    // Tracking data
    UPROPERTY()
    TArray<FString> OrphanedHeaders;

    UPROPERTY()
    TArray<FString> DuplicateSystems;

    UPROPERTY()
    TArray<FString> CompilationErrors;

    UPROPERTY()
    bool bCleanupInProgress;

    UPROPERTY()
    float LastCleanupTime;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cleanup Settings", meta = (AllowPrivateAccess = "true"))
    bool bAutoFixOrphans;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cleanup Settings", meta = (AllowPrivateAccess = "true"))
    bool bRemoveDuplicates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cleanup Settings", meta = (AllowPrivateAccess = "true"))
    bool bValidateOnStartup;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cleanup Settings", meta = (AllowPrivateAccess = "true"))
    float CleanupIntervalSeconds;
};