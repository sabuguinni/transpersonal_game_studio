#include "EngArch_CoreSystemRegistry.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"

UEngArch_CoreSystemRegistry::UEngArch_CoreSystemRegistry()
{
    // Initialize with default capacity
    RegisteredSystems.Reserve(32);
    RegisteredComponents.Reserve(16);
    RegisteredActors.Reserve(64);
    SystemDependencies.Reserve(16);
}

void UEngArch_CoreSystemRegistry::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("EngArch_CoreSystemRegistry: Initializing core system registry"));
    
    InitializeCoreArchitecture();
    ValidateArchitecturalCompliance();
}

void UEngArch_CoreSystemRegistry::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("EngArch_CoreSystemRegistry: Shutting down system registry"));
    
    // Clear all registrations
    RegisteredSystems.Empty();
    RegisteredComponents.Empty();
    RegisteredActors.Empty();
    SystemDependencies.Empty();
    
    Super::Deinitialize();
}

void UEngArch_CoreSystemRegistry::RegisterSystem(const FString& SystemName, UObject* SystemObject)
{
    if (!SystemObject)
    {
        UE_LOG(LogTemp, Error, TEXT("EngArch_CoreSystemRegistry: Cannot register null system: %s"), *SystemName);
        return;
    }
    
    if (RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("EngArch_CoreSystemRegistry: System already registered, replacing: %s"), *SystemName);
    }
    
    RegisteredSystems.Add(SystemName, SystemObject);
    UE_LOG(LogTemp, Log, TEXT("EngArch_CoreSystemRegistry: Registered system: %s"), *SystemName);
}

void UEngArch_CoreSystemRegistry::UnregisterSystem(const FString& SystemName)
{
    if (RegisteredSystems.Remove(SystemName) > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("EngArch_CoreSystemRegistry: Unregistered system: %s"), *SystemName);
        
        // Also remove dependencies
        SystemDependencies.Remove(SystemName);
        
        // Remove this system from other systems' dependencies
        for (auto& Dependency : SystemDependencies)
        {
            Dependency.Value.Remove(SystemName);
        }
    }
}

UObject* UEngArch_CoreSystemRegistry::GetSystem(const FString& SystemName) const
{
    if (const TWeakObjectPtr<UObject>* SystemPtr = RegisteredSystems.Find(SystemName))
    {
        if (SystemPtr->IsValid())
        {
            return SystemPtr->Get();
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("EngArch_CoreSystemRegistry: System reference is stale: %s"), *SystemName);
        }
    }
    
    return nullptr;
}

bool UEngArch_CoreSystemRegistry::IsSystemRegistered(const FString& SystemName) const
{
    const TWeakObjectPtr<UObject>* SystemPtr = RegisteredSystems.Find(SystemName);
    return SystemPtr && SystemPtr->IsValid();
}

void UEngArch_CoreSystemRegistry::RegisterComponent(const FString& ComponentType, UActorComponent* Component)
{
    if (!Component)
    {
        UE_LOG(LogTemp, Error, TEXT("EngArch_CoreSystemRegistry: Cannot register null component: %s"), *ComponentType);
        return;
    }
    
    TArray<TWeakObjectPtr<UActorComponent>>& ComponentArray = RegisteredComponents.FindOrAdd(ComponentType);
    ComponentArray.AddUnique(Component);
    
    UE_LOG(LogTemp, Log, TEXT("EngArch_CoreSystemRegistry: Registered component: %s"), *ComponentType);
}

TArray<UActorComponent*> UEngArch_CoreSystemRegistry::GetComponentsOfType(const FString& ComponentType) const
{
    TArray<UActorComponent*> ValidComponents;
    
    if (const TArray<TWeakObjectPtr<UActorComponent>>* ComponentArray = RegisteredComponents.Find(ComponentType))
    {
        for (const TWeakObjectPtr<UActorComponent>& ComponentPtr : *ComponentArray)
        {
            if (ComponentPtr.IsValid())
            {
                ValidComponents.Add(ComponentPtr.Get());
            }
        }
    }
    
    return ValidComponents;
}

void UEngArch_CoreSystemRegistry::RegisterActor(const FString& ActorType, AActor* Actor)
{
    if (!Actor)
    {
        UE_LOG(LogTemp, Error, TEXT("EngArch_CoreSystemRegistry: Cannot register null actor: %s"), *ActorType);
        return;
    }
    
    TArray<TWeakObjectPtr<AActor>>& ActorArray = RegisteredActors.FindOrAdd(ActorType);
    ActorArray.AddUnique(Actor);
    
    UE_LOG(LogTemp, Log, TEXT("EngArch_CoreSystemRegistry: Registered actor: %s"), *ActorType);
}

TArray<AActor*> UEngArch_CoreSystemRegistry::GetActorsOfType(const FString& ActorType) const
{
    TArray<AActor*> ValidActors;
    
    if (const TArray<TWeakObjectPtr<AActor>>* ActorArray = RegisteredActors.Find(ActorType))
    {
        for (const TWeakObjectPtr<AActor>& ActorPtr : *ActorArray)
        {
            if (ActorPtr.IsValid())
            {
                ValidActors.Add(ActorPtr.Get());
            }
        }
    }
    
    return ValidActors;
}

void UEngArch_CoreSystemRegistry::AddSystemDependency(const FString& SystemName, const FString& DependsOnSystem)
{
    TArray<FString>& Dependencies = SystemDependencies.FindOrAdd(SystemName);
    Dependencies.AddUnique(DependsOnSystem);
    
    UE_LOG(LogTemp, Log, TEXT("EngArch_CoreSystemRegistry: Added dependency: %s depends on %s"), *SystemName, *DependsOnSystem);
}

bool UEngArch_CoreSystemRegistry::ValidateSystemDependencies() const
{
    bool bAllDependenciesValid = true;
    
    for (const auto& SystemDep : SystemDependencies)
    {
        const FString& SystemName = SystemDep.Key;
        const TArray<FString>& Dependencies = SystemDep.Value;
        
        for (const FString& Dependency : Dependencies)
        {
            if (!IsSystemRegistered(Dependency))
            {
                UE_LOG(LogTemp, Error, TEXT("EngArch_CoreSystemRegistry: Missing dependency: %s requires %s"), *SystemName, *Dependency);
                bAllDependenciesValid = false;
            }
        }
    }
    
    return bAllDependenciesValid;
}

void UEngArch_CoreSystemRegistry::LogRegisteredSystems() const
{
    UE_LOG(LogTemp, Warning, TEXT("EngArch_CoreSystemRegistry: === REGISTERED SYSTEMS ==="));
    
    for (const auto& System : RegisteredSystems)
    {
        const FString& SystemName = System.Key;
        const bool bIsValid = System.Value.IsValid();
        
        UE_LOG(LogTemp, Warning, TEXT("  %s: %s"), *SystemName, bIsValid ? TEXT("VALID") : TEXT("INVALID"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("EngArch_CoreSystemRegistry: Total systems: %d"), RegisteredSystems.Num());
}

int32 UEngArch_CoreSystemRegistry::GetSystemCount() const
{
    return RegisteredSystems.Num();
}

TArray<FString> UEngArch_CoreSystemRegistry::GetAllSystemNames() const
{
    TArray<FString> SystemNames;
    RegisteredSystems.GetKeys(SystemNames);
    return SystemNames;
}

void UEngArch_CoreSystemRegistry::InitializeCoreArchitecture()
{
    // Register core architectural systems
    UE_LOG(LogTemp, Warning, TEXT("EngArch_CoreSystemRegistry: Initializing core architecture"));
    
    // Self-register the registry
    RegisterSystem(TEXT("CoreSystemRegistry"), this);
    
    // Set up core system dependencies
    AddSystemDependency(TEXT("PhysicsSystem"), TEXT("CoreSystemRegistry"));
    AddSystemDependency(TEXT("BiomeSystem"), TEXT("CoreSystemRegistry"));
    AddSystemDependency(TEXT("CharacterSystem"), TEXT("PhysicsSystem"));
    AddSystemDependency(TEXT("DinosaurAI"), TEXT("BiomeSystem"));
    AddSystemDependency(TEXT("CombatSystem"), TEXT("CharacterSystem"));
}

void UEngArch_CoreSystemRegistry::ValidateArchitecturalCompliance()
{
    UE_LOG(LogTemp, Warning, TEXT("EngArch_CoreSystemRegistry: Validating architectural compliance"));
    
    CleanupInvalidReferences();
    
    const bool bDependenciesValid = ValidateSystemDependencies();
    if (bDependenciesValid)
    {
        UE_LOG(LogTemp, Warning, TEXT("EngArch_CoreSystemRegistry: All system dependencies are valid"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("EngArch_CoreSystemRegistry: Some system dependencies are missing"));
    }
}

void UEngArch_CoreSystemRegistry::CleanupInvalidReferences()
{
    // Clean up invalid system references
    for (auto It = RegisteredSystems.CreateIterator(); It; ++It)
    {
        if (!It->Value.IsValid())
        {
            UE_LOG(LogTemp, Warning, TEXT("EngArch_CoreSystemRegistry: Removing invalid system: %s"), *It->Key);
            It.RemoveCurrent();
        }
    }
    
    // Clean up invalid component references
    for (auto& ComponentEntry : RegisteredComponents)
    {
        ComponentEntry.Value.RemoveAll([](const TWeakObjectPtr<UActorComponent>& Ptr) {
            return !Ptr.IsValid();
        });
    }
    
    // Clean up invalid actor references
    for (auto& ActorEntry : RegisteredActors)
    {
        ActorEntry.Value.RemoveAll([](const TWeakObjectPtr<AActor>& Ptr) {
            return !Ptr.IsValid();
        });
    }
}