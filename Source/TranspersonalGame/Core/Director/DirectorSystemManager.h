#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "../../SharedTypes.h"
#include "DirectorSystemManager.generated.h"

// Forward declarations
class UDirectorConsciousnessSubsystem;
class UDirectorPerformanceSubsystem;
class UDirectorIntegrationSubsystem;

/**
 * Studio Director's System Manager - Central coordination hub for all game systems
 * Manages the 18-agent chain workflow and ensures proper system integration
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(TranspersonalGame))
class TRANSPERSONALGAME_API UDirectorSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UDirectorSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === SYSTEM COORDINATION ===
    
    /** Initialize all subsystems in proper dependency order */
    UFUNCTION(BlueprintCallable, Category = "Director System")
    void InitializeSystemChain();
    
    /** Validate system health and report status */
    UFUNCTION(BlueprintCallable, Category = "Director System")
    FDir_SystemHealthReport GetSystemHealthReport();
    
    /** Emergency system shutdown for critical errors */
    UFUNCTION(BlueprintCallable, Category = "Director System")
    void EmergencyShutdown(const FString& Reason);

    // === AGENT CHAIN MANAGEMENT ===
    
    /** Current active agent in the 18-agent chain */
    UPROPERTY(BlueprintReadOnly, Category = "Agent Chain", meta = (AllowPrivateAccess = "true"))
    int32 CurrentActiveAgent;
    
    /** Progress through the agent chain (0.0 to 1.0) */
    UPROPERTY(BlueprintReadOnly, Category = "Agent Chain", meta = (AllowPrivateAccess = "true"))
    float ChainProgress;
    
    /** Advance to next agent in chain */
    UFUNCTION(BlueprintCallable, Category = "Agent Chain")
    bool AdvanceToNextAgent();
    
    /** Get current agent name */
    UFUNCTION(BlueprintCallable, Category = "Agent Chain")
    FString GetCurrentAgentName();

    // === PERFORMANCE MONITORING ===
    
    /** Target FPS for current platform */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 TargetFPS;
    
    /** Current frame time in milliseconds */
    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float CurrentFrameTime;
    
    /** Performance warning threshold */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceWarningThreshold;

    // === SYSTEM INTEGRATION ===
    
    /** Register a new system component */
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void RegisterSystemComponent(UActorComponent* Component, const FString& SystemName);
    
    /** Unregister a system component */
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void UnregisterSystemComponent(const FString& SystemName);
    
    /** Get registered system by name */
    UFUNCTION(BlueprintCallable, Category = "Integration")
    UActorComponent* GetSystemComponent(const FString& SystemName);

protected:
    // === INTERNAL STATE ===
    
    /** Map of registered system components */
    UPROPERTY()
    TMap<FString, UActorComponent*> RegisteredSystems;
    
    /** Agent chain names for reference */
    UPROPERTY()
    TArray<FString> AgentChainNames;
    
    /** System initialization status */
    UPROPERTY()
    bool bSystemsInitialized;
    
    /** Last performance check time */
    UPROPERTY()
    float LastPerformanceCheck;

private:
    // === INTERNAL METHODS ===
    
    /** Initialize agent chain names */
    void InitializeAgentChain();
    
    /** Update performance metrics */
    void UpdatePerformanceMetrics();
    
    /** Validate system dependencies */
    bool ValidateSystemDependencies();
    
    /** Log system status */
    void LogSystemStatus();
};

/**
 * Director System Actor - Spawnable actor containing the DirectorSystemManager
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADirectorSystemActor : public AActor
{
    GENERATED_BODY()

public:
    ADirectorSystemActor();

protected:
    /** The main director system manager component */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Director System")
    UDirectorSystemManager* DirectorSystemManager;

    virtual void BeginPlay() override;

public:
    /** Get the director system manager */
    UFUNCTION(BlueprintCallable, Category = "Director System")
    UDirectorSystemManager* GetDirectorSystemManager() const { return DirectorSystemManager; }
};