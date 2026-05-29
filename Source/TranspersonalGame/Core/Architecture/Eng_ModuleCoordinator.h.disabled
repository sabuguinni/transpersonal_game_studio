#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "Eng_SystemsRegistry.h"
#include "Eng_ModuleCoordinator.generated.h"

/**
 * Engine Architect - CYCLE 002 Module Coordination System
 * 
 * Module Coordinator - Orchestrates communication between all game modules
 * This system ensures proper inter-module communication, prevents circular dependencies,
 * and provides a unified interface for module lifecycle management.
 * 
 * CRITICAL FUNCTIONS:
 * - Module registration and discovery
 * - Inter-module message passing
 * - Dependency resolution and validation
 * - Performance monitoring across modules
 * - Hot-reload support for development
 */

UENUM(BlueprintType)
enum class EEng_ModuleType : uint8
{
    Core            UMETA(DisplayName = "Core"),
    Physics         UMETA(DisplayName = "Physics"),
    WorldGeneration UMETA(DisplayName = "World Generation"),
    Character       UMETA(DisplayName = "Character"),
    AI              UMETA(DisplayName = "AI"),
    Combat          UMETA(DisplayName = "Combat"),
    Survival        UMETA(DisplayName = "Survival"),
    Quest           UMETA(DisplayName = "Quest"),
    Audio           UMETA(DisplayName = "Audio"),
    VFX             UMETA(DisplayName = "VFX"),
    UI              UMETA(DisplayName = "UI"),
    Performance     UMETA(DisplayName = "Performance"),
    Integration     UMETA(DisplayName = "Integration")
};

UENUM(BlueprintType)
enum class EEng_MessagePriority : uint8
{
    Immediate       UMETA(DisplayName = "Immediate"),
    High            UMETA(DisplayName = "High"),
    Normal          UMETA(DisplayName = "Normal"),
    Low             UMETA(DisplayName = "Low"),
    Background      UMETA(DisplayName = "Background")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ModuleInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    FString ModuleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    EEng_ModuleType ModuleType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    FString Version;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    TArray<FString> ProvidedInterfaces;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    float LoadTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    int32 MessagesSent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    int32 MessagesReceived;

    FEng_ModuleInfo()
    {
        ModuleName = "";
        ModuleType = EEng_ModuleType::Core;
        Version = "1.0.0";
        bIsActive = false;
        LoadTime = 0.0f;
        MessagesSent = 0;
        MessagesReceived = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_InterModuleMessage
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Message")
    FString MessageID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Message")
    FString SourceModule;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Message")
    FString TargetModule;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Message")
    FString MessageType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Message")
    EEng_MessagePriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Message")
    FString PayloadData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Message")
    float Timestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Message")
    bool bRequiresResponse;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Message")
    float TimeoutSeconds;

    FEng_InterModuleMessage()
    {
        MessageID = "";
        SourceModule = "";
        TargetModule = "";
        MessageType = "";
        Priority = EEng_MessagePriority::Normal;
        PayloadData = "";
        Timestamp = 0.0f;
        bRequiresResponse = false;
        TimeoutSeconds = 30.0f;
    }
};

DECLARE_DYNAMIC_DELEGATE_OneParam(FEng_MessageHandler, const FEng_InterModuleMessage&, Message);

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_ModuleCoordinator : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UEng_ModuleCoordinator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;

    // Module Registration
    UFUNCTION(BlueprintCallable, Category = "Module Coordinator")
    bool RegisterModule(const FEng_ModuleInfo& ModuleInfo);

    UFUNCTION(BlueprintCallable, Category = "Module Coordinator")
    bool UnregisterModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Module Coordinator")
    bool IsModuleRegistered(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, Category = "Module Coordinator")
    FEng_ModuleInfo GetModuleInfo(const FString& ModuleName) const;

    // Module Discovery
    UFUNCTION(BlueprintCallable, Category = "Module Coordinator")
    TArray<FString> GetModulesByType(EEng_ModuleType ModuleType) const;

    UFUNCTION(BlueprintCallable, Category = "Module Coordinator")
    TArray<FString> GetActiveModules() const;

    UFUNCTION(BlueprintCallable, Category = "Module Coordinator")
    TArray<FString> GetModulesProvidingInterface(const FString& InterfaceName) const;

    // Message Passing
    UFUNCTION(BlueprintCallable, Category = "Module Coordinator")
    bool SendMessage(const FEng_InterModuleMessage& Message);

    UFUNCTION(BlueprintCallable, Category = "Module Coordinator")
    bool SendSimpleMessage(const FString& SourceModule, const FString& TargetModule, 
                          const FString& MessageType, const FString& Data);

    UFUNCTION(BlueprintCallable, Category = "Module Coordinator")
    bool BroadcastMessage(const FString& SourceModule, EEng_ModuleType TargetType, 
                         const FString& MessageType, const FString& Data);

    // Message Handling
    UFUNCTION(BlueprintCallable, Category = "Module Coordinator")
    bool RegisterMessageHandler(const FString& ModuleName, const FString& MessageType, 
                               const FEng_MessageHandler& Handler);

    UFUNCTION(BlueprintCallable, Category = "Module Coordinator")
    bool UnregisterMessageHandler(const FString& ModuleName, const FString& MessageType);

    // Dependency Management
    UFUNCTION(BlueprintCallable, Category = "Module Coordinator")
    bool ValidateModuleDependencies() const;

    UFUNCTION(BlueprintCallable, Category = "Module Coordinator")
    TArray<FString> GetModuleDependencies(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, Category = "Module Coordinator")
    bool AreModuleDependenciesLoaded(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, Category = "Module Coordinator")
    TArray<FString> GetLoadOrder() const;

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Module Coordinator")
    void UpdateModulePerformance(const FString& ModuleName, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Module Coordinator")
    float GetModuleAveragePerformance(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, Category = "Module Coordinator")
    int32 GetModuleMessageCount(const FString& ModuleName, bool bSent = true) const;

    // Debug and Diagnostics
    UFUNCTION(BlueprintCallable, Category = "Module Coordinator")
    void LogModuleStatus() const;

    UFUNCTION(BlueprintCallable, Category = "Module Coordinator")
    void LogMessageQueue() const;

    UFUNCTION(BlueprintCallable, Category = "Module Coordinator")
    TArray<FString> GetModulesWithErrors() const;

    // Hot Reload Support
    UFUNCTION(BlueprintCallable, Category = "Module Coordinator")
    void PrepareForHotReload();

    UFUNCTION(BlueprintCallable, Category = "Module Coordinator")
    void CompleteHotReload();

    // Event Delegates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnModuleRegistered, FString, ModuleName);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnModuleUnregistered, FString, ModuleName);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMessageSent, FString, SourceModule, FString, TargetModule);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllModulesLoaded);

    UPROPERTY(BlueprintAssignable, Category = "Module Coordinator Events")
    FOnModuleRegistered OnModuleRegistered;

    UPROPERTY(BlueprintAssignable, Category = "Module Coordinator Events")
    FOnModuleUnregistered OnModuleUnregistered;

    UPROPERTY(BlueprintAssignable, Category = "Module Coordinator Events")
    FOnMessageSent OnMessageSent;

    UPROPERTY(BlueprintAssignable, Category = "Module Coordinator Events")
    FOnAllModulesLoaded OnAllModulesLoaded;

protected:
    // Core data structures
    UPROPERTY()
    TMap<FString, FEng_ModuleInfo> RegisteredModules;

    UPROPERTY()
    TQueue<FEng_InterModuleMessage> MessageQueue;

    UPROPERTY()
    TMap<FString, TMap<FString, FEng_MessageHandler>> MessageHandlers;

    UPROPERTY()
    TMap<FString, TArray<float>> ModulePerformanceData;

    // Systems Registry reference
    UPROPERTY()
    UEng_SystemsRegistry* SystemsRegistry;

    // Internal processing
    void ProcessMessageQueue();
    void ProcessMessage(const FEng_InterModuleMessage& Message);
    bool ValidateMessage(const FEng_InterModuleMessage& Message) const;
    FString GenerateMessageID() const;
    void UpdateMessageStatistics(const FString& SourceModule, const FString& TargetModule);

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration", meta = (AllowPrivateAccess = "true"))
    int32 MaxMessagesPerFrame;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration", meta = (AllowPrivateAccess = "true"))
    float MessageTimeoutSeconds;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration", meta = (AllowPrivateAccess = "true"))
    bool bEnablePerformanceTracking;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration", meta = (AllowPrivateAccess = "true"))
    bool bLogModuleEvents;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration", meta = (AllowPrivateAccess = "true"))
    bool bLogMessageTraffic;
};