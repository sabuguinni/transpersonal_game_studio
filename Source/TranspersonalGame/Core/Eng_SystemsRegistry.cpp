#include "Eng_SystemsRegistry.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "TimerManager.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Core_PhysicsManager.h"
#include "BiomeManager.h"
#include "StudioDirector.h"

UEng_SystemsRegistry::UEng_SystemsRegistry()
    : PhysicsManager(nullptr)
    , BiomeManager(nullptr)
    , StudioDirector(nullptr)
    , bAllSystemsInitialized(false)
    , TotalInitializationTime(0.0f)
{
    UE_LOG(LogTemp, Log, TEXT("Engine Systems Registry constructor called"));
}

void UEng_SystemsRegistry::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Engine Systems Registry initializing..."));
    
    // Initialize core systems in dependency order
    InitializeCorePhysics();
    InitializeWorldSystems();
    InitializeGameplaySystems();
    
    bAllSystemsInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("Engine Systems Registry initialization complete"));
}

void UEng_SystemsRegistry::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("Engine Systems Registry shutting down..."));
    
    ShutdownAllSystems();
    RegisteredSystems.Empty();
    
    Super::Deinitialize();
}

void UEng_SystemsRegistry::RegisterSystem(const FString& SystemName, UObject* SystemInstance, int32 Priority)
{
    if (!SystemInstance)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot register null system: %s"), *SystemName);
        return;
    }
    
    FEng_SystemRegistration Registration;
    Registration.SystemName = SystemName;
    Registration.SystemInstance = SystemInstance;
    Registration.Priority = Priority;
    Registration.bIsInitialized = false;
    Registration.InitializationTime = 0.0f;
    Registration.RegistrationTime = FDateTime::Now();
    
    RegisteredSystems.Add(SystemName, Registration);
    
    UE_LOG(LogTemp, Log, TEXT("Registered system: %s (Priority: %d)"), *SystemName, Priority);
}

void UEng_SystemsRegistry::UnregisterSystem(const FString& SystemName)
{
    if (RegisteredSystems.Contains(SystemName))
    {
        RegisteredSystems.Remove(SystemName);
        UE_LOG(LogTemp, Log, TEXT("Unregistered system: %s"), *SystemName);
    }
}

UObject* UEng_SystemsRegistry::GetSystem(const FString& SystemName)
{
    if (FEng_SystemRegistration* Registration = RegisteredSystems.Find(SystemName))
    {
        return Registration->SystemInstance;
    }
    return nullptr;
}

bool UEng_SystemsRegistry::ValidateAllSystems()
{
    bool bAllValid = true;
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        if (!ValidateSystem(SystemPair.Key))
        {
            bAllValid = false;
            UE_LOG(LogTemp, Error, TEXT("System validation failed: %s"), *SystemPair.Key);
        }
    }
    
    // Validate core integrations
    if (!ValidatePhysicsIntegration())
    {
        bAllValid = false;
        UE_LOG(LogTemp, Error, TEXT("Physics integration validation failed"));
    }
    
    if (!ValidateWorldGeneration())
    {
        bAllValid = false;
        UE_LOG(LogTemp, Error, TEXT("World generation validation failed"));
    }
    
    return bAllValid;
}

bool UEng_SystemsRegistry::ValidateSystem(const FString& SystemName)
{
    FEng_SystemRegistration* Registration = RegisteredSystems.Find(SystemName);
    if (!Registration)
    {
        return false;
    }
    
    // Check if system instance is valid
    if (!IsValid(Registration->SystemInstance))
    {
        return false;
    }
    
    // Check dependencies
    if (!AreDependenciesSatisfied(SystemName))
    {
        return false;
    }
    
    return true;
}

void UEng_SystemsRegistry::InitializeAllSystems()
{
    UE_LOG(LogTemp, Log, TEXT("Initializing all registered systems..."));
    
    double StartTime = FPlatformTime::Seconds();
    
    // Get systems sorted by priority
    TArray<FEng_SystemRegistration> SortedSystems;
    RegisteredSystems.GenerateValueArray(SortedSystems);
    SortSystemsByPriority(SortedSystems);
    
    // Initialize each system
    for (FEng_SystemRegistration& Registration : SortedSystems)
    {
        if (!Registration.bIsInitialized)
        {
            StartSystemTimer(Registration.SystemName);
            
            // System-specific initialization logic would go here
            // For now, just mark as initialized
            Registration.bIsInitialized = true;
            
            EndSystemTimer(Registration.SystemName);
            
            UE_LOG(LogTemp, Log, TEXT("Initialized system: %s"), *Registration.SystemName);
        }
    }
    
    TotalInitializationTime = FPlatformTime::Seconds() - StartTime;
    bAllSystemsInitialized = true;
    
    UE_LOG(LogTemp, Log, TEXT("All systems initialized in %.3f seconds"), TotalInitializationTime);
}

void UEng_SystemsRegistry::ShutdownAllSystems()
{
    UE_LOG(LogTemp, Log, TEXT("Shutting down all systems..."));
    
    for (auto& SystemPair : RegisteredSystems)
    {
        SystemPair.Value.bIsInitialized = false;
        UE_LOG(LogTemp, Log, TEXT("Shutdown system: %s"), *SystemPair.Key);
    }
    
    bAllSystemsInitialized = false;
}

TArray<FString> UEng_SystemsRegistry::GetRegisteredSystems() const
{
    TArray<FString> SystemNames;
    RegisteredSystems.GetKeys(SystemNames);
    return SystemNames;
}

bool UEng_SystemsRegistry::IsSystemRegistered(const FString& SystemName) const
{
    return RegisteredSystems.Contains(SystemName);
}

bool UEng_SystemsRegistry::IsSystemInitialized(const FString& SystemName) const
{
    if (const FEng_SystemRegistration* Registration = RegisteredSystems.Find(SystemName))
    {
        return Registration->bIsInitialized;
    }
    return false;
}

float UEng_SystemsRegistry::GetSystemInitializationTime(const FString& SystemName) const
{
    if (const FEng_SystemRegistration* Registration = RegisteredSystems.Find(SystemName))
    {
        return Registration->InitializationTime;
    }
    return 0.0f;
}

void UEng_SystemsRegistry::LogSystemsStatus() const
{
    UE_LOG(LogTemp, Log, TEXT("=== SYSTEMS REGISTRY STATUS ==="));
    UE_LOG(LogTemp, Log, TEXT("Total registered systems: %d"), RegisteredSystems.Num());
    UE_LOG(LogTemp, Log, TEXT("All systems initialized: %s"), bAllSystemsInitialized ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Log, TEXT("Total initialization time: %.3f seconds"), TotalInitializationTime);
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        const FEng_SystemRegistration& Registration = SystemPair.Value;
        UE_LOG(LogTemp, Log, TEXT("System: %s | Priority: %d | Initialized: %s | Init Time: %.3f"),
            *Registration.SystemName,
            Registration.Priority,
            Registration.bIsInitialized ? TEXT("YES") : TEXT("NO"),
            Registration.InitializationTime);
    }
    
    UE_LOG(LogTemp, Log, TEXT("=== END SYSTEMS STATUS ==="));
}

void UEng_SystemsRegistry::InitializeCorePhysics()
{
    UE_LOG(LogTemp, Log, TEXT("Initializing core physics systems..."));
    
    // Register physics manager
    if (!PhysicsManager)
    {
        // In a real implementation, we would create or find the physics manager
        RegisterSystem(TEXT("PhysicsManager"), nullptr, 10);
    }
}

void UEng_SystemsRegistry::InitializeWorldSystems()
{
    UE_LOG(LogTemp, Log, TEXT("Initializing world systems..."));
    
    // Register biome manager
    if (!BiomeManager)
    {
        // In a real implementation, we would create or find the biome manager
        RegisterSystem(TEXT("BiomeManager"), nullptr, 20);
    }
}

void UEng_SystemsRegistry::InitializeGameplaySystems()
{
    UE_LOG(LogTemp, Log, TEXT("Initializing gameplay systems..."));
    
    // Register studio director
    if (!StudioDirector)
    {
        // In a real implementation, we would find the studio director
        RegisterSystem(TEXT("StudioDirector"), nullptr, 30);
    }
}

void UEng_SystemsRegistry::InitializeAudioSystems()
{
    UE_LOG(LogTemp, Log, TEXT("Initializing audio systems..."));
    RegisterSystem(TEXT("AudioManager"), nullptr, 40);
}

void UEng_SystemsRegistry::InitializeVFXSystems()
{
    UE_LOG(LogTemp, Log, TEXT("Initializing VFX systems..."));
    RegisterSystem(TEXT("VFXManager"), nullptr, 50);
}

bool UEng_SystemsRegistry::ValidatePhysicsIntegration()
{
    // Check if physics systems are properly integrated
    return IsSystemRegistered(TEXT("PhysicsManager"));
}

bool UEng_SystemsRegistry::ValidateWorldGeneration()
{
    // Check if world generation systems are ready
    return IsSystemRegistered(TEXT("BiomeManager"));
}

bool UEng_SystemsRegistry::ValidateGameplayIntegration()
{
    // Check if gameplay systems are integrated
    return IsSystemRegistered(TEXT("StudioDirector"));
}

void UEng_SystemsRegistry::StartSystemTimer(const FString& SystemName)
{
    // Timer implementation would go here
}

void UEng_SystemsRegistry::EndSystemTimer(const FString& SystemName)
{
    // Timer implementation would go here
    if (FEng_SystemRegistration* Registration = RegisteredSystems.Find(SystemName))
    {
        Registration->InitializationTime = 0.1f; // Placeholder
    }
}

TArray<FString> UEng_SystemsRegistry::GetSystemDependencies(const FString& SystemName)
{
    TArray<FString> Dependencies;
    
    // Define system dependencies
    if (SystemName == TEXT("BiomeManager"))
    {
        Dependencies.Add(TEXT("PhysicsManager"));
    }
    else if (SystemName == TEXT("StudioDirector"))
    {
        Dependencies.Add(TEXT("PhysicsManager"));
        Dependencies.Add(TEXT("BiomeManager"));
    }
    
    return Dependencies;
}

bool UEng_SystemsRegistry::AreDependenciesSatisfied(const FString& SystemName)
{
    TArray<FString> Dependencies = GetSystemDependencies(SystemName);
    
    for (const FString& Dependency : Dependencies)
    {
        if (!IsSystemInitialized(Dependency))
        {
            return false;
        }
    }
    
    return true;
}

void UEng_SystemsRegistry::SortSystemsByPriority(TArray<FEng_SystemRegistration>& Systems)
{
    Systems.Sort([](const FEng_SystemRegistration& A, const FEng_SystemRegistration& B)
    {
        return A.Priority < B.Priority;
    });
}