#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "../SharedTypes.h"
#include "BuildIntegrationManager.generated.h"

/**
 * Build Integration Manager - Handles cross-module integration and validation
 * Ensures all game systems work together correctly
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

    // Integration validation
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateWorldState();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateCharacterSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateAISystems();

    // Build management
    UFUNCTION(BlueprintCallable, Category = "Build")
    void GenerateBuildReport();

    UFUNCTION(BlueprintCallable, Category = "Build")
    bool CheckModuleDependencies();

    UFUNCTION(BlueprintCallable, Category = "Build")
    void CleanupDuplicateActors();

    // System status
    UPROPERTY(BlueprintReadOnly, Category = "Status")
    bool bAllSystemsValid;

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    bool bWorldStateValid;

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    bool bCharacterSystemsValid;

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    bool bAISystemsValid;

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    int32 TotalActorsInLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    int32 DuplicateActorsFound;

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    FString LastValidationResult;

    // Integration events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnValidationComplete, bool, bSuccess);
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnValidationComplete OnValidationComplete;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBuildReportGenerated, const FString&, ReportPath);
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnBuildReportGenerated OnBuildReportGenerated;

private:
    // Internal validation helpers
    bool ValidateActorCounts();
    bool ValidateLightingSetup();
    bool ValidateTerrainSetup();
    bool ValidatePlayerSetup();
    bool ValidateDinosaurSetup();

    // Cleanup helpers
    void RemoveDuplicateLightingActors();
    void RemoveDuplicateAtmosphereActors();

    // Validation state
    TArray<FString> ValidationErrors;
    TArray<FString> ValidationWarnings;
    
    // Actor tracking
    TMap<FString, int32> ActorCountsByType;
    TArray<AActor*> DuplicateActors;
};