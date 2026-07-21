#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Eng_MasterArchitect.generated.h"

UENUM(BlueprintType)
enum class EEng_SystemState : uint8
{
    Uninitialized   UMETA(DisplayName = "Uninitialized"),
    Initializing    UMETA(DisplayName = "Initializing"), 
    Operational     UMETA(DisplayName = "Operational"),
    Error          UMETA(DisplayName = "Error"),
    Shutdown       UMETA(DisplayName = "Shutdown")
};

UENUM(BlueprintType)
enum class EEng_SystemPriority : uint8
{
    Critical       UMETA(DisplayName = "Critical"),
    High          UMETA(DisplayName = "High"),
    Medium        UMETA(DisplayName = "Medium"),
    Low           UMETA(DisplayName = "Low"),
    Background    UMETA(DisplayName = "Background")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "System Info")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "System Info")
    EEng_SystemState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "System Info")
    EEng_SystemPriority Priority;

    UPROPERTY(BlueprintReadOnly, Category = "System Info")
    float InitializationTime;

    UPROPERTY(BlueprintReadOnly, Category = "System Info")
    FString LastError;

    UPROPERTY(BlueprintReadOnly, Category = "System Info")
    int32 DependencyCount;

    FEng_SystemInfo()
    {
        SystemName = TEXT("Unknown");
        CurrentState = EEng_SystemState::Uninitialized;
        Priority = EEng_SystemPriority::Medium;
        InitializationTime = 0.0f;
        LastError = TEXT("");
        DependencyCount = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ArchitectureMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Architecture Metrics")
    int32 TotalSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture Metrics")
    int32 OperationalSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture Metrics")
    int32 ErrorSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture Metrics")
    float OverallHealth;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture Metrics")
    float AverageInitTime;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture Metrics")
    FDateTime LastUpdate;

    FEng_ArchitectureMetrics()
    {
        TotalSystems = 0;
        OperationalSystems = 0;
        ErrorSystems = 0;
        OverallHealth = 0.0f;
        AverageInitTime = 0.0f;
        LastUpdate = FDateTime::Now();
    }
};

/**
 * Master Architecture System - The central nervous system of the game
 * Defines and enforces all architectural rules, system dependencies, and initialization order
 * This is the foundation that all other systems must respect
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_MasterArchitect : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_MasterArchitect();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core Architecture Management
    UFUNCTION(BlueprintCallable, Category = "Engine Architect")
    bool RegisterSystem(const FString& SystemName, EEng_SystemPriority Priority, const TArray<FString>& Dependencies);

    UFUNCTION(BlueprintCallable, Category = "Engine Architect")
    bool InitializeSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architect")
    bool ShutdownSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architect")
    EEng_SystemState GetSystemState(const FString& SystemName) const;

    // Architecture Validation
    UFUNCTION(BlueprintCallable, Category = "Engine Architect")
    bool ValidateArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Engine Architect")
    bool ValidateSystemDependencies();

    UFUNCTION(BlueprintCallable, Category = "Engine Architect")
    TArray<FString> GetSystemErrors() const;

    // Metrics and Monitoring
    UFUNCTION(BlueprintCallable, Category = "Engine Architect")
    FEng_ArchitectureMetrics GetArchitectureMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architect")
    TArray<FEng_SystemInfo> GetAllSystemInfo() const;

    // Critical System Rules
    UFUNCTION(BlueprintCallable, Category = "Engine Architect")
    bool EnforceInitializationOrder();

    UFUNCTION(BlueprintCallable, Category = "Engine Architect")
    void SetCriticalSystemFailureResponse(bool bShutdownOnCriticalFailure);

    // Blueprint Events
    UFUNCTION(BlueprintImplementableEvent, Category = "Engine Architect")
    void OnSystemStateChanged(const FString& SystemName, EEng_SystemState NewState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Engine Architect")
    void OnArchitectureValidationFailed(const TArray<FString>& Errors);

    UFUNCTION(BlueprintImplementableEvent, Category = "Engine Architect")
    void OnCriticalSystemFailure(const FString& SystemName, const FString& Error);

protected:
    // System Registry
    UPROPERTY(BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    TMap<FString, FEng_SystemInfo> RegisteredSystems;

    // Dependency Graph
    UPROPERTY(BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    TMap<FString, TArray<FString>> SystemDependencies;

    // Initialization Order
    UPROPERTY(BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    TArray<FString> InitializationOrder;

    // Architecture Rules
    UPROPERTY(BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    bool bEnforceStrictDependencies;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    bool bShutdownOnCriticalFailure;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    float MaxInitializationTime;

    // Metrics
    UPROPERTY(BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    FEng_ArchitectureMetrics CurrentMetrics;

private:
    // Internal Methods
    void BuildInitializationOrder();
    bool CheckDependenciesResolved(const FString& SystemName) const;
    void UpdateMetrics();
    void LogSystemEvent(const FString& SystemName, const FString& Event) const;
    bool IsSystemCritical(const FString& SystemName) const;
};

/**
 * World-level Architecture Coordinator
 * Manages per-world system instances and coordinates with the master architect
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_WorldArchitectureCoordinator : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UEng_WorldArchitectureCoordinator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // World-specific Architecture Management
    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    bool InitializeWorldSystems();

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    bool ShutdownWorldSystems();

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    bool ValidateWorldArchitecture();

    // System Coordination
    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    UEng_MasterArchitect* GetMasterArchitect() const;

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    bool RegisterWorldSystem(const FString& SystemName, UObject* SystemInstance);

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    UObject* GetWorldSystem(const FString& SystemName) const;

protected:
    // World System Registry
    UPROPERTY(BlueprintReadOnly, Category = "World Architecture", meta = (AllowPrivateAccess = "true"))
    TMap<FString, TObjectPtr<UObject>> WorldSystems;

    // Coordination State
    UPROPERTY(BlueprintReadOnly, Category = "World Architecture", meta = (AllowPrivateAccess = "true"))
    bool bWorldSystemsInitialized;

    UPROPERTY(BlueprintReadOnly, Category = "World Architecture", meta = (AllowPrivateAccess = "true"))
    float WorldInitializationTime;

private:
    void InitializePhysicsSystems();
    void InitializeWorldGenerationSystems();
    void InitializeCharacterSystems();
    void InitializeAISystems();
    void InitializeAudioSystems();
};

#include "Eng_MasterArchitect.generated.h"