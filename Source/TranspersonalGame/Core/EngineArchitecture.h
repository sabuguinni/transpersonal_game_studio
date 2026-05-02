#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "EngineArchitecture.generated.h"

/**
 * Engine Architecture System - Core architectural validation and management
 * Ensures all game systems follow established architectural patterns
 * Validates system dependencies and prevents architectural violations
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEngineArchitecture : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEngineArchitecture();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Architecture validation
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateSystemDependencies();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateModuleIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void RegisterSystemModule(const FString& ModuleName, int32 Priority);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool CheckCompilationStatus();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    float GetCurrentFrameTime() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    int32 GetActiveActorCount() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void LogSystemStatus();

protected:
    // System registry
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TMap<FString, int32> RegisteredSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    bool bArchitectureValid;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    float LastValidationTime;

    // Performance metrics
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 PeakActorCount;

private:
    // Internal validation methods
    bool ValidateWorldGeneration();
    bool ValidateCharacterSystems();
    bool ValidateDinosaurSystems();
    bool ValidateRenderingPipeline();

    // Performance tracking
    void UpdatePerformanceMetrics();
    
    // System dependencies
    TArray<FString> CoreSystemModules;
    TArray<FString> GameplaySystemModules;
    TArray<FString> RenderingSystemModules;
};

/**
 * Architecture validation result structure
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ArchitectureValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bIsValid;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString ValidationMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> FailedSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float ValidationTime;

    FEng_ArchitectureValidationResult()
    {
        bIsValid = false;
        ValidationTime = 0.0f;
    }
};

/**
 * System module registration info
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemModuleInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    int32 Priority;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    bool bIsActive;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    float LastUpdateTime;

    FEng_SystemModuleInfo()
    {
        Priority = 0;
        bIsActive = false;
        LastUpdateTime = 0.0f;
    }
};