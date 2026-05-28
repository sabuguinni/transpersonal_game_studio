#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Containers/Map.h"
#include "UObject/WeakObjectPtr.h"
#include "EngArch_CoreSystemRegistry.generated.h"

class UObject;
class AActor;
class UActorComponent;

/**
 * Core System Registry - Central hub for all game systems
 * Manages registration, discovery, and lifecycle of all major game systems
 * Enforces architectural compliance and system dependencies
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEngArch_CoreSystemRegistry : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEngArch_CoreSystemRegistry();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // System Registration
    UFUNCTION(BlueprintCallable, Category = "System Registry")
    void RegisterSystem(const FString& SystemName, UObject* SystemObject);

    UFUNCTION(BlueprintCallable, Category = "System Registry")
    void UnregisterSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "System Registry")
    UObject* GetSystem(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "System Registry")
    bool IsSystemRegistered(const FString& SystemName) const;

    // Component Registration
    UFUNCTION(BlueprintCallable, Category = "System Registry")
    void RegisterComponent(const FString& ComponentType, UActorComponent* Component);

    UFUNCTION(BlueprintCallable, Category = "System Registry")
    TArray<UActorComponent*> GetComponentsOfType(const FString& ComponentType) const;

    // Actor Registration
    UFUNCTION(BlueprintCallable, Category = "System Registry")
    void RegisterActor(const FString& ActorType, AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "System Registry")
    TArray<AActor*> GetActorsOfType(const FString& ActorType) const;

    // System Dependencies
    UFUNCTION(BlueprintCallable, Category = "System Registry")
    void AddSystemDependency(const FString& SystemName, const FString& DependsOnSystem);

    UFUNCTION(BlueprintCallable, Category = "System Registry")
    bool ValidateSystemDependencies() const;

    // Debug and Validation
    UFUNCTION(BlueprintCallable, Category = "System Registry")
    void LogRegisteredSystems() const;

    UFUNCTION(BlueprintCallable, Category = "System Registry")
    int32 GetSystemCount() const;

    UFUNCTION(BlueprintCallable, Category = "System Registry")
    TArray<FString> GetAllSystemNames() const;

protected:
    // Core system storage
    UPROPERTY()
    TMap<FString, TWeakObjectPtr<UObject>> RegisteredSystems;

    // Component tracking
    UPROPERTY()
    TMap<FString, TArray<TWeakObjectPtr<UActorComponent>>> RegisteredComponents;

    // Actor tracking
    UPROPERTY()
    TMap<FString, TArray<TWeakObjectPtr<AActor>>> RegisteredActors;

    // System dependencies
    UPROPERTY()
    TMap<FString, TArray<FString>> SystemDependencies;

private:
    void InitializeCoreArchitecture();
    void ValidateArchitecturalCompliance();
    void CleanupInvalidReferences();
};