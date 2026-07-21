#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Eng_SystemsRegistry.generated.h"

/**
 * Central registry for all game systems
 * Manages system lifecycle, dependencies, and communication
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_SystemsRegistry : public UObject
{
    GENERATED_BODY()

public:
    UEng_SystemsRegistry();

    // System registration
    UFUNCTION(BlueprintCallable, Category = "Systems Registry")
    void RegisterSystem(const FString& SystemName, UObject* SystemInstance, int32 Priority = 100);

    UFUNCTION(BlueprintCallable, Category = "Systems Registry")
    void UnregisterSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Systems Registry")
    UObject* GetSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Systems Registry")
    bool IsSystemRegistered(const FString& SystemName);

    // System lifecycle management
    UFUNCTION(BlueprintCallable, Category = "Systems Registry")
    void InitializeAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Systems Registry")
    void ShutdownAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Systems Registry")
    void UpdateSystemPriorities();

    // System dependency management
    UFUNCTION(BlueprintCallable, Category = "Systems Registry")
    void AddSystemDependency(const FString& SystemName, const FString& DependencyName);

    UFUNCTION(BlueprintCallable, Category = "Systems Registry")
    bool ValidateSystemDependencies();

    UFUNCTION(BlueprintCallable, Category = "Systems Registry")
    TArray<FString> GetSystemInitializationOrder();

    // System communication
    UFUNCTION(BlueprintCallable, Category = "Systems Registry")
    void BroadcastSystemMessage(const FString& MessageType, const FString& MessageData);

    UFUNCTION(BlueprintCallable, Category = "Systems Registry")
    void SendSystemMessage(const FString& TargetSystem, const FString& MessageType, const FString& MessageData);

protected:
    // System storage
    UPROPERTY(BlueprintReadOnly, Category = "Registry")
    TMap<FString, UObject*> RegisteredSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Registry")
    TMap<FString, int32> SystemPriorities;

    UPROPERTY(BlueprintReadOnly, Category = "Registry")
    TMap<FString, TArray<FString>> SystemDependencies;

    UPROPERTY(BlueprintReadOnly, Category = "Registry")
    TArray<FString> SystemInitializationOrder;

    // Registry state
    UPROPERTY(BlueprintReadOnly, Category = "Registry")
    bool bSystemsInitialized;

    UPROPERTY(BlueprintReadOnly, Category = "Registry")
    int32 TotalRegisteredSystems;

private:
    void CalculateInitializationOrder();
    bool HasCircularDependencies();
    void TopologicalSort(TArray<FString>& SortedSystems);
};