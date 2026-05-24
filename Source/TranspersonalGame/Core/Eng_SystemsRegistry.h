#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Eng_SystemsRegistry.generated.h"

/**
 * Central Systems Registry - Engine Architect's master coordination system
 * 
 * This is the architectural foundation that enforces system dependencies,
 * initialization order, and cross-system communication protocols.
 * 
 * ARCHITECTURAL PRINCIPLES:
 * - Single source of truth for all system states
 * - Dependency-aware initialization ordering
 * - Performance budget enforcement
 * - Cross-agent coordination protocols
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemRegistration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Registration")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Registration")
    int32 InitializationPriority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Registration")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Registration")
    ESystemState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Registration")
    float PerformanceBudgetMs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Registration")
    bool bIsCriticalSystem;

    FEng_SystemRegistration()
    {
        SystemName = TEXT("");
        InitializationPriority = 100;
        CurrentState = ESystemState::Uninitialized;
        PerformanceBudgetMs = 16.67f; // 60 FPS budget
        bIsCriticalSystem = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float AverageFrameTimeMs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PeakFrameTimeMs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 ActiveSystemsCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TotalMemoryUsageMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerformanceProfile CurrentProfile;

    FEng_PerformanceMetrics()
    {
        AverageFrameTimeMs = 16.67f;
        PeakFrameTimeMs = 16.67f;
        ActiveSystemsCount = 0;
        TotalMemoryUsageMB = 0.0f;
        CurrentProfile = EPerformanceProfile::Development;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_SystemsRegistry : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_SystemsRegistry();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /**
     * SYSTEM REGISTRATION INTERFACE
     * Used by all agent systems to register themselves with the architecture
     */
    UFUNCTION(BlueprintCallable, Category = "Systems Registry")
    bool RegisterSystem(const FString& SystemName, int32 Priority, const TArray<FString>& Dependencies, bool bIsCritical = false);

    UFUNCTION(BlueprintCallable, Category = "Systems Registry")
    bool UnregisterSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Systems Registry")
    bool SetSystemState(const FString& SystemName, ESystemState NewState);

    UFUNCTION(BlueprintCallable, Category = "Systems Registry")
    ESystemState GetSystemState(const FString& SystemName) const;

    /**
     * DEPENDENCY MANAGEMENT
     * Ensures systems initialize in correct order based on dependencies
     */
    UFUNCTION(BlueprintCallable, Category = "Dependencies")
    TArray<FString> GetInitializationOrder() const;

    UFUNCTION(BlueprintCallable, Category = "Dependencies")
    bool ValidateDependencies() const;

    UFUNCTION(BlueprintCallable, Category = "Dependencies")
    TArray<FString> GetSystemDependents(const FString& SystemName) const;

    /**
     * PERFORMANCE MONITORING
     * Tracks system performance and enforces architectural budgets
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FEng_PerformanceMetrics GetPerformanceMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceBudgetExceeded() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceProfile(EPerformanceProfile NewProfile);

    /**
     * ARCHITECTURAL VALIDATION
     * Ensures all systems comply with architectural standards
     */
    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateSystemArchitecture(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Validation")
    TArray<FString> GetArchitecturalViolations() const;

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool EnforceArchitecturalCompliance();

    /**
     * CROSS-SYSTEM COMMUNICATION
     * Provides secure messaging between registered systems
     */
    UFUNCTION(BlueprintCallable, Category = "Communication")
    bool SendSystemMessage(const FString& FromSystem, const FString& ToSystem, const FString& Message);

    UFUNCTION(BlueprintCallable, Category = "Communication")
    TArray<FString> GetPendingMessages(const FString& SystemName);

    /**
     * DEBUG AND MONITORING
     */
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DumpSystemsStatus() const;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    bool IsSystemRegistered(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    int32 GetRegisteredSystemsCount() const;

protected:
    // Core registry data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Registry", meta = (AllowPrivateAccess = "true"))
    TMap<FString, FEng_SystemRegistration> RegisteredSystems;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    FEng_PerformanceMetrics CurrentMetrics;

    // Cross-system messaging
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Communication", meta = (AllowPrivateAccess = "true"))
    TMap<FString, TArray<FString>> SystemMessages;

    // Architectural compliance tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compliance", meta = (AllowPrivateAccess = "true"))
    TArray<FString> ArchitecturalViolations;

    // Performance monitoring
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float PerformanceUpdateTimer;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    TArray<float> FrameTimeHistory;

private:
    // Internal helper methods
    bool ResolveDependencyOrder(TArray<FString>& OutOrder) const;
    bool HasCircularDependency(const FString& SystemName, TSet<FString>& VisitedSystems) const;
    void UpdateFrameTimeHistory(float FrameTime);
    float CalculateAverageFrameTime() const;
    bool ValidateSystemCompliance(const FEng_SystemRegistration& System) const;
};