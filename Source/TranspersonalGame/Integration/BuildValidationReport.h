#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SharedTypes.h"
#include "BuildValidationReport.generated.h"

/**
 * Build Validation Report - Integration Agent #19
 * Tracks compilation status, system health, and integration metrics
 * Used for automated build validation and quality assurance
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuildValidationReport : public UDataAsset
{
    GENERATED_BODY()

public:
    UBuildValidationReport();

    // === BUILD STATUS ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Status")
    EBuild_CompilationStatus CompilationStatus;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Status")
    int32 TotalHeaderFiles;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Status")
    int32 TotalCppFiles;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Status")
    int32 OrphanedHeaders;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Status")
    float ImplementationRatio;
    
    // === SYSTEM HEALTH ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Health")
    int32 SystemsLoaded;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Health")
    int32 SystemsFailed;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Health")
    TArray<FString> FailedSystems;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Health")
    float SystemLoadRatio;
    
    // === MAP STATUS ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Status")
    int32 TotalActors;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Status")
    TMap<FString, int32> ActorTypeCounts;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Status")
    bool bMinPlayableMapReady;
    
    // === INTEGRATION METRICS ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    float IntegrationScore;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    FDateTime LastValidationTime;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    TArray<FString> Recommendations;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    EBuild_ValidationResult ValidationResult;
    
    // === BINARY STATUS ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Binary Status")
    int32 CompiledBinaries;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Binary Status")
    int32 ObjectFiles;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Binary Status")
    bool bBuildToolAvailable;
    
    // === METHODS ===
    
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void UpdateValidationData();
    
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool IsSystemHealthy() const;
    
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool IsBuildReady() const;
    
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    FString GetStatusSummary() const;
    
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    TArray<FString> GetCriticalIssues() const;
    
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void GenerateReport();
    
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void SaveReportToFile(const FString& FilePath);
    
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void LoadReportFromFile(const FString& FilePath);

protected:
    // Internal validation methods
    void ValidateFileStructure();
    void ValidateSystemLoading();
    void ValidateMapStatus();
    void CalculateIntegrationScore();
    void GenerateRecommendations();
    
private:
    // Validation state
    bool bValidationInProgress;
    FDateTime ValidationStartTime;
};