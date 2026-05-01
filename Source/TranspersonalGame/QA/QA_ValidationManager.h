#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "../SharedTypes.h"
#include "QA_ValidationManager.generated.h"

/**
 * QA Validation Manager - Comprehensive testing and validation system
 * Validates all game systems, integration points, and performance metrics
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQA_ValidationManager : public AActor
{
    GENERATED_BODY()

public:
    AQA_ValidationManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateMinPlayableMap();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateCharacterSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateAudioSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateNarrativeSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateIntegrationPoints();

    // Performance validation
    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    float GetCurrentFPS();

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    int32 GetActorCount();

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    float GetMemoryUsage();

    // Test creation functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing", CallInEditor)
    void CreateTestMarkers();

    UFUNCTION(BlueprintCallable, Category = "QA Testing", CallInEditor)
    void RunComprehensiveTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing", CallInEditor)
    void GenerateValidationReport();

protected:
    // Validation state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA State")
    bool bAllSystemsValid;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA State")
    bool bMinPlayableMapValid;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA State")
    bool bCharacterSystemsValid;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA State")
    bool bVFXSystemsValid;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA State")
    bool bAudioSystemsValid;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA State")
    bool bNarrativeSystemsValid;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA State")
    bool bIntegrationValid;

    // Performance metrics
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Performance")
    float CurrentFPS;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Performance")
    int32 TotalActorCount;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Performance")
    float MemoryUsageMB;

    // Test configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    bool bAutoRunTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    float TestInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    bool bCreateTestMarkers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    bool bLogVerboseResults;

    // Test markers
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Markers")
    TArray<AActor*> TestMarkers;

    // Validation mesh component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Components")
    UStaticMeshComponent* ValidationMeshComponent;

private:
    // Internal validation helpers
    bool ValidateActorClass(const FString& ClassName);
    bool ValidateComponentIntegration();
    bool ValidateMapAssets();
    bool ValidatePlayerSystems();
    bool ValidateEnemySystems();
    bool ValidateEnvironmentSystems();
    
    // Performance monitoring
    void UpdatePerformanceMetrics();
    void LogValidationResults();
    
    // Test timer
    float TestTimer;
    
    // Validation results storage
    TMap<FString, bool> ValidationResults;
    TArray<FString> ValidationErrors;
    TArray<FString> ValidationWarnings;
};