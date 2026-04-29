#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "EngineArchitectureManager.generated.h"

/**
 * Engine Architecture Manager - Central system for managing technical architecture
 * Enforces coding standards, module dependencies, and compilation rules
 * Validates system integrity and performance constraints
 */

UENUM(BlueprintType)
enum class EEng_SystemStatus : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    Initializing    UMETA(DisplayName = "Initializing"),
    Active          UMETA(DisplayName = "Active"),
    Warning         UMETA(DisplayName = "Warning"),
    Error           UMETA(DisplayName = "Error"),
    Disabled        UMETA(DisplayName = "Disabled")
};

UENUM(BlueprintType)
enum class EEng_ModuleType : uint8
{
    Core            UMETA(DisplayName = "Core"),
    Gameplay        UMETA(DisplayName = "Gameplay"),
    Rendering       UMETA(DisplayName = "Rendering"),
    Audio           UMETA(DisplayName = "Audio"),
    AI              UMETA(DisplayName = "AI"),
    Physics         UMETA(DisplayName = "Physics"),
    UI              UMETA(DisplayName = "UI"),
    Tools           UMETA(DisplayName = "Tools")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "System")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    EEng_SystemStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    EEng_ModuleType ModuleType;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    float PerformanceImpact;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    int32 Priority;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    FDateTime LastUpdate;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    FString ErrorMessage;

    FEng_SystemInfo()
    {
        SystemName = TEXT("");
        Status = EEng_SystemStatus::Unknown;
        ModuleType = EEng_ModuleType::Core;
        PerformanceImpact = 0.0f;
        Priority = 0;
        LastUpdate = FDateTime::Now();
        ErrorMessage = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_CompilationRule
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FString RuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FString Description;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    bool bMandatory;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FString ViolationMessage;

    FEng_CompilationRule()
    {
        RuleName = TEXT("");
        Description = TEXT("");
        bMandatory = true;
        ViolationMessage = TEXT("");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEngineArchitectureManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEngineArchitectureManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // System Registration
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void RegisterSystem(const FString& SystemName, EEng_ModuleType ModuleType, int32 Priority = 0);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void UnregisterSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void UpdateSystemStatus(const FString& SystemName, EEng_SystemStatus NewStatus, const FString& ErrorMessage = TEXT(""));

    // System Queries
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FEng_SystemInfo> GetAllSystems() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FEng_SystemInfo GetSystemInfo(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FEng_SystemInfo> GetSystemsByType(EEng_ModuleType ModuleType) const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FEng_SystemInfo> GetSystemsByStatus(EEng_SystemStatus Status) const;

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void UpdateSystemPerformance(const FString& SystemName, float PerformanceImpact);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    float GetTotalPerformanceImpact() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FEng_SystemInfo> GetHighPerformanceImpactSystems(float Threshold = 10.0f) const;

    // Compilation Rules
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void AddCompilationRule(const FString& RuleName, const FString& Description, bool bMandatory = true);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FEng_CompilationRule> GetCompilationRules() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateCompilationRules(TArray<FString>& OutViolations) const;

    // System Health
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsSystemHealthy() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    int32 GetErrorCount() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    int32 GetWarningCount() const;

    // Debug and Reporting
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FString GenerateSystemReport() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void LogSystemStatus() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Engine Architecture")
    void ValidateArchitecture();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    TMap<FString, FEng_SystemInfo> RegisteredSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    TArray<FEng_CompilationRule> CompilationRules;

    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    bool bInitialized;

    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    FDateTime LastValidation;

private:
    void SetupDefaultCompilationRules();
    void ValidateSystemDependencies();
    void CheckPerformanceConstraints();
};