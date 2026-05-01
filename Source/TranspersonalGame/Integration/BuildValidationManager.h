#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "BuildValidationManager.generated.h"

/**
 * Build Validation Manager - Integration Agent #19
 * Validates compilation, tests all agent systems, manages build integrity
 * Ensures all 18 agent outputs integrate correctly into a playable game
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuildValidationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuildValidationManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Build validation functions
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool ValidateCompilation();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool ValidateAgentOutputs();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool CleanDuplicateActors();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    FString GenerateBuildReport();

    // System validation functions
    UFUNCTION(BlueprintCallable, Category = "System Validation")
    bool ValidateCharacterSystem();

    UFUNCTION(BlueprintCallable, Category = "System Validation")
    bool ValidateDinosaurSystem();

    UFUNCTION(BlueprintCallable, Category = "System Validation")
    bool ValidateEnvironmentSystem();

    UFUNCTION(BlueprintCallable, Category = "System Validation")
    bool ValidateAudioSystem();

    UFUNCTION(BlueprintCallable, Category = "System Validation")
    bool ValidateVFXSystem();

    UFUNCTION(BlueprintCallable, Category = "System Validation")
    bool ValidateAISystem();

    UFUNCTION(BlueprintCallable, Category = "System Validation")
    bool ValidateQuestSystem();

    UFUNCTION(BlueprintCallable, Category = "System Validation")
    bool ValidateWorldGeneration();

    // Actor management
    UFUNCTION(BlueprintCallable, Category = "Actor Management")
    int32 CountActorsByType(const FString& ActorTypeName);

    UFUNCTION(BlueprintCallable, Category = "Actor Management")
    TArray<AActor*> GetActorsByType(const FString& ActorTypeName);

    UFUNCTION(BlueprintCallable, Category = "Actor Management")
    bool RemoveDuplicateActors(const FString& ActorTypeName, int32 MaxAllowed = 1);

    // Build status properties
    UPROPERTY(BlueprintReadOnly, Category = "Build Status")
    bool bCompilationValid;

    UPROPERTY(BlueprintReadOnly, Category = "Build Status")
    bool bAllSystemsValid;

    UPROPERTY(BlueprintReadOnly, Category = "Build Status")
    int32 TotalActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Build Status")
    int32 DuplicateActorsRemoved;

    UPROPERTY(BlueprintReadOnly, Category = "Build Status")
    FString LastValidationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Build Status")
    TArray<FString> ValidationErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Build Status")
    TArray<FString> ValidationWarnings;

    // System status tracking
    UPROPERTY(BlueprintReadOnly, Category = "System Status")
    bool bCharacterSystemValid;

    UPROPERTY(BlueprintReadOnly, Category = "System Status")
    bool bDinosaurSystemValid;

    UPROPERTY(BlueprintReadOnly, Category = "System Status")
    bool bEnvironmentSystemValid;

    UPROPERTY(BlueprintReadOnly, Category = "System Status")
    bool bAudioSystemValid;

    UPROPERTY(BlueprintReadOnly, Category = "System Status")
    bool bVFXSystemValid;

    UPROPERTY(BlueprintReadOnly, Category = "System Status")
    bool bAISystemValid;

    UPROPERTY(BlueprintReadOnly, Category = "System Status")
    bool bQuestSystemValid;

    UPROPERTY(BlueprintReadOnly, Category = "System Status")
    bool bWorldGenerationValid;

private:
    // Internal validation helpers
    void LogValidationResult(const FString& SystemName, bool bValid);
    void AddValidationError(const FString& ErrorMessage);
    void AddValidationWarning(const FString& WarningMessage);
    void ClearValidationMessages();

    // Actor counting and management
    TMap<FString, int32> ActorCounts;
    void UpdateActorCounts();
    bool IsActorTypeAllowed(const FString& ActorTypeName);

    // Build integrity checks
    bool CheckModuleDependencies();
    bool CheckClassRegistration();
    bool CheckAssetIntegrity();

    // Validation timer
    FTimerHandle ValidationTimerHandle;
    void PeriodicValidation();

    UPROPERTY()
    float ValidationInterval;
};