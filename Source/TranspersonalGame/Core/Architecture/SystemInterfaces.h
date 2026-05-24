#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Engine/DataAsset.h"
#include "SystemInterfaces.generated.h"

/**
 * SYSTEM INTERFACES - TRANSPERSONAL GAME STUDIO
 * Agent #02 - Engine Architect
 * 
 * Defines the communication protocols between all major game systems.
 * These interfaces ensure loose coupling and maintainable architecture.
 */

// Forward declarations
class UTranspersonalSystemBase;
struct FSystemResourceBudget;

UENUM(BlueprintType)
enum class ESystemState : uint8
{
    Uninitialized = 0,
    Initializing = 1,
    Active = 2,
    Paused = 3,
    Error = 4,
    ShuttingDown = 5
};

UENUM(BlueprintType)
enum class ESystemMessageType : uint8
{
    Initialize = 0,
    Shutdown = 1,
    Pause = 2,
    Resume = 3,
    PerformanceWarning = 4,
    ResourceExhausted = 5,
    SystemError = 6
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FSystemMessage
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ESystemMessageType MessageType = ESystemMessageType::Initialize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SourceSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TargetSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString MessageData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Timestamp = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresResponse = false;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FSystemDependency
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DependentSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString RequiredSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHardDependency = true; // System cannot start without this dependency

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeoutSeconds = 30.0f;
};

/**
 * Interface for systems that can be initialized and managed
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UManagedSystem : public UInterface
{
    GENERATED_BODY()
};

class TRANSPERSONALGAME_API IManagedSystem
{
    GENERATED_BODY()

public:
    // System lifecycle
    virtual bool InitializeSystem(const FString& ConfigData = TEXT("")) = 0;
    virtual void ShutdownSystem() = 0;
    virtual void PauseSystem() = 0;
    virtual void ResumeSystem() = 0;

    // System state
    virtual ESystemState GetSystemState() const = 0;
    virtual FString GetSystemName() const = 0;
    virtual float GetSystemVersion() const { return 1.0f; }

    // Dependencies
    virtual TArray<FSystemDependency> GetSystemDependencies() const { return TArray<FSystemDependency>(); }
    virtual bool AreDependenciesSatisfied() const { return true; }

    // Health monitoring
    virtual bool IsSystemHealthy() const { return true; }
    virtual FString GetSystemStatus() const { return TEXT("OK"); }
};

/**
 * Interface for systems that need regular updates
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UTickableSystem : public UInterface
{
    GENERATED_BODY()
};

class TRANSPERSONALGAME_API ITickableSystem
{
    GENERATED_BODY()

public:
    virtual void TickSystem(float DeltaTime) = 0;
    virtual bool ShouldTickSystem() const { return true; }
    virtual float GetTickInterval() const { return 0.0f; } // 0 = every frame
    virtual int32 GetTickPriority() const { return 0; } // Higher = earlier
};

/**
 * Interface for systems that can receive messages
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UMessageReceiver : public UInterface
{
    GENERATED_BODY()
};

class TRANSPERSONALGAME_API IMessageReceiver
{
    GENERATED_BODY()

public:
    virtual bool HandleSystemMessage(const FSystemMessage& Message) = 0;
    virtual TArray<ESystemMessageType> GetSupportedMessageTypes() const = 0;
    virtual bool CanReceiveMessage(const FSystemMessage& Message) const { return true; }
};

/**
 * Interface for systems that monitor performance
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UPerformanceMonitored : public UInterface
{
    GENERATED_BODY()
};

class TRANSPERSONALGAME_API IPerformanceMonitored
{
    GENERATED_BODY()

public:
    // Performance metrics
    virtual float GetCPUUsageMs() const = 0;
    virtual int32 GetMemoryUsageMB() const = 0;
    virtual int32 GetDrawCallCount() const { return 0; }
    virtual int32 GetTriangleCount() const { return 0; }

    // Performance limits
    virtual FSystemResourceBudget GetResourceBudget() const = 0;
    virtual bool IsWithinPerformanceBudget() const = 0;
    virtual void OptimizePerformance() {}

    // Performance history
    virtual float GetAverageCPUUsage(float TimeWindowSeconds = 5.0f) const { return GetCPUUsageMs(); }
    virtual float GetPeakCPUUsage(float TimeWindowSeconds = 5.0f) const { return GetCPUUsageMs(); }
};

/**
 * Interface for systems that can be configured
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UConfigurableSystem : public UInterface
{
    GENERATED_BODY()
};

class TRANSPERSONALGAME_API IConfigurableSystem
{
    GENERATED_BODY()

public:
    virtual bool LoadConfiguration(const FString& ConfigPath) = 0;
    virtual bool SaveConfiguration(const FString& ConfigPath) const = 0;
    virtual void ApplyConfiguration() = 0;
    virtual void ResetToDefaults() = 0;
    virtual FString GetConfigurationSummary() const { return TEXT("No configuration"); }
};

/**
 * Data asset for system configuration
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API USystemConfigurationAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    float SystemVersion = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FSystemResourceBudget ResourceBudget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dependencies")
    TArray<FSystemDependency> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TMap<FString, FString> ConfigurationParameters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bAutoInitialize = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bCriticalSystem = false; // Cannot be disabled
};

/**
 * System registry for managing all game systems
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API USystemRegistry : public UObject
{
    GENERATED_BODY()

public:
    // System registration
    UFUNCTION(BlueprintCallable, Category = "System Registry")
    bool RegisterSystem(UObject* System, const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "System Registry")
    bool UnregisterSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "System Registry")
    UObject* GetSystem(const FString& SystemName);

    // System discovery
    UFUNCTION(BlueprintCallable, Category = "System Registry")
    TArray<FString> GetRegisteredSystemNames() const;

    UFUNCTION(BlueprintCallable, Category = "System Registry")
    TArray<UObject*> GetSystemsByInterface(TSubclassOf<UInterface> InterfaceClass);

    // System lifecycle management
    UFUNCTION(BlueprintCallable, Category = "System Registry")
    bool InitializeAllSystems();

    UFUNCTION(BlueprintCallable, Category = "System Registry")
    void ShutdownAllSystems();

    UFUNCTION(BlueprintCallable, Category = "System Registry")
    bool InitializeSystem(const FString& SystemName);

    // Message broadcasting
    UFUNCTION(BlueprintCallable, Category = "System Registry")
    bool BroadcastMessage(const FSystemMessage& Message);

    UFUNCTION(BlueprintCallable, Category = "System Registry")
    bool SendMessageToSystem(const FSystemMessage& Message, const FString& TargetSystem);

    // System health monitoring
    UFUNCTION(BlueprintCallable, Category = "System Registry")
    bool AreAllSystemsHealthy() const;

    UFUNCTION(BlueprintCallable, Category = "System Registry")
    TArray<FString> GetUnhealthySystems() const;

protected:
    UPROPERTY()
    TMap<FString, TObjectPtr<UObject>> RegisteredSystems;

    UPROPERTY()
    TArray<FSystemMessage> MessageQueue;

    bool ValidateSystemDependencies(const FString& SystemName);
    void ProcessMessageQueue();
    void LogSystemStatus();
};