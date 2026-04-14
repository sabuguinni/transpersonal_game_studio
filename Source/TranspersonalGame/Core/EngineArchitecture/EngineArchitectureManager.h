#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "../../SharedTypes.h"
#include "EngineArchitectureManager.generated.h"

// Forward declarations
class USystemValidationManager;
class UPerformanceProfiler;
class UModuleManager;
class UDependencyResolver;

/**
 * Central Engine Architecture Manager
 * Defines and enforces the technical architecture rules for the entire project
 * All other systems must register with and follow the rules defined here
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEngineArchitectureManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEngineArchitectureManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core Architecture Rules
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool RegisterSystem(const FString& SystemName, EEng_SystemType SystemType, int32 Priority);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateSystemDependencies(const FString& SystemName, const TArray<FString>& Dependencies);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void EnforcePerformanceConstraints(const FEng_PerformanceConstraints& Constraints);

    // System Management
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void InitializeAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void ShutdownAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsSystemActive(const FString& SystemName) const;

    // Architecture Validation
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateArchitecturalIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FString> GetArchitectureViolations() const;

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FEng_PerformanceMetrics GetCurrentPerformanceMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void SetPerformanceTarget(EEng_PerformanceTarget Target);

    // Module Dependencies
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool CheckModuleDependencies(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void ResolveCircularDependencies();

protected:
    // Core subsystem references
    UPROPERTY(BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USystemValidationManager> ValidationManager;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UPerformanceProfiler> PerformanceProfiler;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UModuleManager> ModuleManager;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UDependencyResolver> DependencyResolver;

    // Architecture state
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TMap<FString, FEng_SystemInfo> RegisteredSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TArray<FString> ArchitectureViolations;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    FEng_PerformanceConstraints CurrentConstraints;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    EEng_PerformanceTarget PerformanceTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    bool bArchitectureInitialized;

private:
    // Internal validation methods
    bool ValidateSystemRegistration(const FString& SystemName, EEng_SystemType SystemType);
    void UpdateSystemPriorities();
    void CheckPerformanceCompliance();
    void LogArchitectureState();

    // System lifecycle management
    void InitializeSystemInOrder(const FString& SystemName);
    void ShutdownSystemInOrder(const FString& SystemName);

    // Performance tracking
    float LastPerformanceCheck;
    static constexpr float PerformanceCheckInterval = 1.0f;
};