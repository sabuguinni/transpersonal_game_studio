#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Subsystems/WorldSubsystem.h"
#include "../../SharedTypes.h"
#include "SystemIntegrationManager.generated.h"

/**
 * System Integration Manager
 * Coordinates communication between all game systems
 * Ensures proper initialization order and dependency management
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API USystemIntegrationManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    USystemIntegrationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    // System Registration and Management
    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void RegisterSystem(const FString& SystemName, UObject* SystemInstance, int32 Priority = 0);

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void UnregisterSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    UObject* GetSystem(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    bool IsSystemRegistered(const FString& SystemName) const;

    // System Lifecycle Management
    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void InitializeAllSystems();

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void ShutdownAllSystems();

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void UpdateAllSystems(float DeltaTime);

    // Dependency Management
    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void AddSystemDependency(const FString& SystemName, const FString& DependencyName);

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void RemoveSystemDependency(const FString& SystemName, const FString& DependencyName);

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    bool ValidateDependencies() const;

    // Communication Hub
    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void BroadcastSystemEvent(const FString& EventName, const FString& EventData);

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void SendSystemMessage(const FString& TargetSystem, const FString& MessageType, const FString& MessageData);

    // System Status
    UFUNCTION(BlueprintCallable, Category = "System Integration")
    TArray<FString> GetRegisteredSystemNames() const;

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    int32 GetSystemCount() const { return RegisteredSystems.Num(); }

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    bool AreAllSystemsHealthy() const;

protected:
    // System registry structure
    USTRUCT(BlueprintType)
    struct FEng_SystemInfo
    {
        GENERATED_BODY()

        UPROPERTY(BlueprintReadOnly)
        TObjectPtr<UObject> SystemInstance;

        UPROPERTY(BlueprintReadOnly)
        int32 Priority;

        UPROPERTY(BlueprintReadOnly)
        bool bInitialized;

        UPROPERTY(BlueprintReadOnly)
        bool bHealthy;

        UPROPERTY(BlueprintReadOnly)
        TArray<FString> Dependencies;

        UPROPERTY(BlueprintReadOnly)
        FDateTime LastUpdate;

        FEng_SystemInfo()
        {
            SystemInstance = nullptr;
            Priority = 0;
            bInitialized = false;
            bHealthy = true;
            LastUpdate = FDateTime::Now();
        }
    };

    // System registry
    UPROPERTY(BlueprintReadOnly, Category = "System Integration")
    TMap<FString, FEng_SystemInfo> RegisteredSystems;

    // System initialization order (sorted by priority)
    UPROPERTY(BlueprintReadOnly, Category = "System Integration")
    TArray<FString> InitializationOrder;

    // Event system
    UPROPERTY(BlueprintReadOnly, Category = "System Integration")
    TMap<FString, TArray<FString>> SystemEventListeners;

    // Integration status
    UPROPERTY(BlueprintReadOnly, Category = "System Integration")
    bool bAllSystemsInitialized;

    UPROPERTY(BlueprintReadOnly, Category = "System Integration")
    bool bIntegrationHealthy;

    UPROPERTY(BlueprintReadOnly, Category = "System Integration")
    TArray<FString> IntegrationErrors;

    UPROPERTY(BlueprintReadOnly, Category = "System Integration")
    TArray<FString> IntegrationWarnings;

private:
    // Internal methods
    void UpdateInitializationOrder();
    bool ValidateSystemDependencies(const FString& SystemName, TArray<FString>& VisitedSystems) const;
    void InitializeSystemInOrder(const FString& SystemName);
    void NotifySystemEvent(const FString& SystemName, const FString& EventName, const FString& EventData);

    // Core system names (always initialized first)
    static const TArray<FString> CoreSystemNames;
};