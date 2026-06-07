#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Eng_ArchitecturalCore.generated.h"

// Core architectural layer definitions
UENUM(BlueprintType)
enum class EEng_ArchitecturalLayer : uint8
{
    Foundation      UMETA(DisplayName = "Foundation Layer"),
    Core           UMETA(DisplayName = "Core Systems Layer"),
    Gameplay       UMETA(DisplayName = "Gameplay Layer"),
    Interface      UMETA(DisplayName = "Interface Layer"),
    Performance    UMETA(DisplayName = "Performance Layer")
};

// System priority levels for architectural governance
UENUM(BlueprintType)
enum class EEng_SystemPriority : uint8
{
    Critical       UMETA(DisplayName = "Critical Priority"),
    High          UMETA(DisplayName = "High Priority"),
    Medium        UMETA(DisplayName = "Medium Priority"),
    Low           UMETA(DisplayName = "Low Priority"),
    Background    UMETA(DisplayName = "Background Priority")
};

// Architectural compliance status
UENUM(BlueprintType)
enum class EEng_ComplianceStatus : uint8
{
    Compliant     UMETA(DisplayName = "Fully Compliant"),
    Warning       UMETA(DisplayName = "Warning Level"),
    Violation     UMETA(DisplayName = "Violation Detected"),
    Critical      UMETA(DisplayName = "Critical Failure"),
    Unknown       UMETA(DisplayName = "Unknown Status")
};

// System registration data structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemRegistration
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "System Registration")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "System Registration")
    EEng_ArchitecturalLayer Layer;

    UPROPERTY(BlueprintReadOnly, Category = "System Registration")
    EEng_SystemPriority Priority;

    UPROPERTY(BlueprintReadOnly, Category = "System Registration")
    TArray<FString> Dependencies;

    UPROPERTY(BlueprintReadOnly, Category = "System Registration")
    float PerformanceBudget;

    UPROPERTY(BlueprintReadOnly, Category = "System Registration")
    bool bIsActive;

    FEng_SystemRegistration()
    {
        SystemName = TEXT("");
        Layer = EEng_ArchitecturalLayer::Core;
        Priority = EEng_SystemPriority::Medium;
        PerformanceBudget = 1.0f;
        bIsActive = false;
    }
};

// Architectural validation result
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    EEng_ComplianceStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString ValidationMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float Timestamp;

    FEng_ValidationResult()
    {
        Status = EEng_ComplianceStatus::Unknown;
        SystemName = TEXT("");
        ValidationMessage = TEXT("");
        Timestamp = 0.0f;
    }
};

/**
 * Supreme architectural authority system that governs all technical decisions
 * and enforces architectural standards across the entire game project.
 * This is the foundational system that all other systems must comply with.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_ArchitecturalCore : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_ArchitecturalCore();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // System registration and management
    UFUNCTION(BlueprintCallable, Category = "Architectural Core")
    bool RegisterSystem(const FEng_SystemRegistration& SystemData);

    UFUNCTION(BlueprintCallable, Category = "Architectural Core")
    bool UnregisterSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Architectural Core")
    bool IsSystemRegistered(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Architectural Core")
    TArray<FEng_SystemRegistration> GetRegisteredSystems() const;

    // Architectural validation
    UFUNCTION(BlueprintCallable, Category = "Architectural Core")
    FEng_ValidationResult ValidateSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Architectural Core")
    TArray<FEng_ValidationResult> ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Architectural Core")
    bool EnforceArchitecturalCompliance();

    // Performance governance
    UFUNCTION(BlueprintCallable, Category = "Architectural Core")
    bool SetSystemPerformanceBudget(const FString& SystemName, float Budget);

    UFUNCTION(BlueprintCallable, Category = "Architectural Core")
    float GetSystemPerformanceBudget(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Architectural Core")
    bool ValidatePerformanceBudgets();

    // Layer management
    UFUNCTION(BlueprintCallable, Category = "Architectural Core")
    TArray<FString> GetSystemsInLayer(EEng_ArchitecturalLayer Layer) const;

    UFUNCTION(BlueprintCallable, Category = "Architectural Core")
    bool ValidateLayerDependencies();

    // System priority management
    UFUNCTION(BlueprintCallable, Category = "Architectural Core")
    bool SetSystemPriority(const FString& SystemName, EEng_SystemPriority Priority);

    UFUNCTION(BlueprintCallable, Category = "Architectural Core")
    EEng_SystemPriority GetSystemPriority(const FString& SystemName) const;

    // Dependency management
    UFUNCTION(BlueprintCallable, Category = "Architectural Core")
    bool AddSystemDependency(const FString& SystemName, const FString& DependencyName);

    UFUNCTION(BlueprintCallable, Category = "Architectural Core")
    bool RemoveSystemDependency(const FString& SystemName, const FString& DependencyName);

    UFUNCTION(BlueprintCallable, Category = "Architectural Core")
    TArray<FString> GetSystemDependencies(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Architectural Core")
    bool ValidateDependencyChain(const FString& SystemName);

    // Architectural reporting
    UFUNCTION(BlueprintCallable, Category = "Architectural Core")
    FString GenerateArchitecturalReport();

    UFUNCTION(BlueprintCallable, Category = "Architectural Core")
    FString GenerateComplianceReport();

    UFUNCTION(BlueprintCallable, Category = "Architectural Core")
    FString GeneratePerformanceReport();

protected:
    // System registry
    UPROPERTY(BlueprintReadOnly, Category = "Architectural Core")
    TMap<FString, FEng_SystemRegistration> RegisteredSystems;

    // Validation results cache
    UPROPERTY(BlueprintReadOnly, Category = "Architectural Core")
    TArray<FEng_ValidationResult> ValidationResults;

    // Performance tracking
    UPROPERTY(BlueprintReadOnly, Category = "Architectural Core")
    TMap<FString, float> SystemPerformanceMetrics;

    // Architectural rules
    UPROPERTY(BlueprintReadOnly, Category = "Architectural Core")
    bool bEnforceStrictCompliance;

    UPROPERTY(BlueprintReadOnly, Category = "Architectural Core")
    float MaxPerformanceBudgetTotal;

    UPROPERTY(BlueprintReadOnly, Category = "Architectural Core")
    int32 MaxSystemsPerLayer;

private:
    // Internal validation methods
    bool ValidateSystemInternal(const FEng_SystemRegistration& System);
    bool CheckLayerCompliance(const FEng_SystemRegistration& System);
    bool CheckDependencyCompliance(const FEng_SystemRegistration& System);
    bool CheckPerformanceCompliance(const FEng_SystemRegistration& System);
    
    // Internal utility methods
    void LogArchitecturalEvent(const FString& Message, EEng_ComplianceStatus Severity);
    void UpdateValidationCache(const FEng_ValidationResult& Result);
    void CleanupValidationCache();
};