#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Eng_MasterArchitecture.generated.h"

/**
 * Master Architecture System - Unified architecture management for TranspersonalGame
 * Consolidates all architecture validation, performance monitoring, and system coordination
 * Created by Engine Architect Agent #2 - Cycle 009
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    bool bIsActive;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    bool bIsValidated;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    float PerformanceScore;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    FString LastError;

    FEng_SystemStatus()
    {
        SystemName = TEXT("");
        bIsActive = false;
        bIsValidated = false;
        PerformanceScore = 0.0f;
        LastError = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ArchitectureMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsage;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bPerformanceTarget60FPS;

    FEng_ArchitectureMetrics()
    {
        FrameTime = 0.0f;
        MemoryUsage = 0.0f;
        ActiveActors = 0;
        DrawCalls = 0;
        bPerformanceTarget60FPS = true;
    }
};

UENUM(BlueprintType)
enum class EEng_ArchitectureValidationLevel : uint8
{
    None = 0,
    Basic = 1,
    Standard = 2,
    Comprehensive = 3,
    Critical = 4
};

/**
 * Master Architecture Subsystem
 * Single source of truth for all architecture decisions and validation
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_MasterArchitecture : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_MasterArchitecture();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Architecture validation
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateSystemArchitecture(EEng_ArchitectureValidationLevel ValidationLevel = EEng_ArchitectureValidationLevel::Standard);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool RegisterSystem(const FString& SystemName, UObject* SystemObject);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool UnregisterSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FEng_SystemStatus GetSystemStatus(const FString& SystemName) const;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FEng_ArchitectureMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceTargetMet() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceTarget(float TargetFrameTime, float TargetMemoryMB);

    // Module dependency management
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateModuleDependencies() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FString> GetModuleLoadOrder() const;

    // Compilation validation
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateCompilation() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FString> GetCompilationErrors() const;

protected:
    // System registry
    UPROPERTY()
    TMap<FString, UObject*> RegisteredSystems;

    UPROPERTY()
    TMap<FString, FEng_SystemStatus> SystemStatuses;

    // Performance targets
    UPROPERTY()
    float TargetFrameTime;

    UPROPERTY()
    float TargetMemoryMB;

    // Architecture rules
    UPROPERTY()
    TArray<FString> RequiredModules;

    UPROPERTY()
    TArray<FString> ModuleLoadOrder;

private:
    // Internal validation methods
    bool ValidateUE5Integration() const;
    bool ValidateSharedTypes() const;
    bool ValidateCoreClasses() const;
    void UpdateSystemMetrics();
    void LogArchitectureStatus() const;
};