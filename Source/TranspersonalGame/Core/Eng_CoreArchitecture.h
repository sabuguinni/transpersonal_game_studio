#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "SharedTypes.h"
#include "Eng_CoreArchitecture.generated.h"

class UEng_SystemArchitect;
class UEng_GameModeArchitect;
class UEng_MasterArchitect;

/**
 * Core Architecture Registry - Central hub for all architectural systems
 * Manages initialization order, dependency resolution, and system validation
 * This is the foundation that ensures all other systems follow architectural standards
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Core Architecture Registry"))
class TRANSPERSONALGAME_API UEng_CoreArchitecture : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_CoreArchitecture();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core Architecture Management
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool InitializeArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateArchitecturalCompliance();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RegisterArchitecturalSystem(const FString& SystemName, UObject* SystemInstance);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool IsSystemRegistered(const FString& SystemName) const;

    // System Dependencies
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ResolveDependencies();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void AddSystemDependency(const FString& SystemName, const FString& DependsOn);

    // Architecture Validation
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FString> GetArchitecturalViolations() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool EnforceArchitecturalStandards();

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    float GetSystemInitializationTime(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FString GetArchitecturalHealthReport() const;

protected:
    // Core architectural components
    UPROPERTY(BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UEng_MasterArchitect> MasterArchitect;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UEng_SystemArchitect> SystemArchitect;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UEng_GameModeArchitect> GameModeArchitect;

    // System registry
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TMap<FString, TObjectPtr<UObject>> RegisteredSystems;

    // Dependency graph
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TMap<FString, TArray<FString>> SystemDependencies;

    // Initialization tracking
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TMap<FString, float> SystemInitTimes;

    // Architecture state
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    bool bArchitectureInitialized;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    bool bDependenciesResolved;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TArray<FString> ArchitecturalViolations;

private:
    // Internal initialization methods
    void InitializeMasterArchitect();
    void InitializeSystemArchitect();
    void InitializeGameModeArchitect();
    
    // Dependency resolution
    bool ValidateDependencyGraph();
    TArray<FString> GetInitializationOrder();
    
    // Validation helpers
    void ValidateSystemCompliance(const FString& SystemName, UObject* System);
    void CheckArchitecturalStandards();
    
    // Performance tracking
    void StartSystemTimer(const FString& SystemName);
    void EndSystemTimer(const FString& SystemName);
    
    FDateTime LastValidationTime;
    float TotalInitializationTime;
};

/**
 * Architectural System Interface - All major systems must implement this
 */
UINTERFACE(BlueprintType)
class TRANSPERSONALGAME_API UEng_ArchitecturalSystem : public UInterface
{
    GENERATED_BODY()
};

class TRANSPERSONALGAME_API IEng_ArchitecturalSystem
{
    GENERATED_BODY()

public:
    // System lifecycle
    UFUNCTION(BlueprintImplementableEvent, Category = "Architecture")
    bool InitializeSystem();

    UFUNCTION(BlueprintImplementableEvent, Category = "Architecture")
    bool ValidateSystemIntegrity();

    UFUNCTION(BlueprintImplementableEvent, Category = "Architecture")
    void ShutdownSystem();

    // System information
    UFUNCTION(BlueprintImplementableEvent, Category = "Architecture")
    FString GetSystemName() const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Architecture")
    TArray<FString> GetSystemDependencies() const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Architecture")
    float GetSystemVersion() const;

    // Performance metrics
    UFUNCTION(BlueprintImplementableEvent, Category = "Architecture")
    FString GetPerformanceMetrics() const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Architecture")
    bool IsSystemHealthy() const;
};

/**
 * Architecture Event System - For system communication
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ArchitecturalEvent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Event")
    FString EventType;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Event")
    FString SourceSystem;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Event")
    FString TargetSystem;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Event")
    FString EventData;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Event")
    float Timestamp;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Event")
    int32 Priority;

    FEng_ArchitecturalEvent()
    {
        EventType = TEXT("");
        SourceSystem = TEXT("");
        TargetSystem = TEXT("");
        EventData = TEXT("");
        Timestamp = 0.0f;
        Priority = 0;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEng_OnArchitecturalEvent, const FEng_ArchitecturalEvent&, Event);

#include "Eng_CoreArchitecture.generated.h"