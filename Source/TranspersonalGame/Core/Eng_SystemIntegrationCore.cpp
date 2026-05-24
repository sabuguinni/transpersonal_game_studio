#include "Eng_SystemIntegrationCore.h"
#include "Eng_ArchitectureCore.h"
#include "Core_PhysicsManager.h"
#include "BiomeManager.h"
#include "DinosaurBase.h"
#include "Engine/World.h"
#include "TimerManager.h"

UEng_SystemIntegrationCore::UEng_SystemIntegrationCore()
{
    ArchitectureCore = nullptr;
    PhysicsManager = nullptr;
    BiomeManager = nullptr;
    CurrentPhase = EEng_InitializationPhase::PreInit;
    InitializationStartTime = 0.0;
}

void UEng_SystemIntegrationCore::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("SystemIntegrationCore: Starting initialization"));
    InitializationStartTime = FPlatformTime::Seconds();
    
    // Initialize system dependencies mapping
    InitializeSystemDependencies();
    
    // Start the initialization sequence
    InitializeAllSystems();
}

void UEng_SystemIntegrationCore::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("SystemIntegrationCore: Shutting down all systems"));
    
    // Clean shutdown in reverse order
    BiomeManager = nullptr;
    PhysicsManager = nullptr;
    ArchitectureCore = nullptr;
    
    SystemInitializationStatus.Empty();
    FailedSystems.Empty();
    
    Super::Deinitialize();
}

void UEng_SystemIntegrationCore::InitializeAllSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("SystemIntegrationCore: Beginning full system initialization"));
    CurrentPhase = EEng_InitializationPhase::CoreSystems;
    
    // Phase 1: Core Architecture
    if (!InitializeArchitectureCore())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to initialize Architecture Core"));
        FailedSystems.Add(EEng_SystemType::Architecture);
        return;
    }
    
    // Phase 2: Physics Systems
    CurrentPhase = EEng_InitializationPhase::PhysicsSystems;
    if (!InitializePhysicsCore())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to initialize Physics Core"));
        FailedSystems.Add(EEng_SystemType::Physics);
        return;
    }
    
    // Phase 3: World Generation
    CurrentPhase = EEng_InitializationPhase::WorldGeneration;
    if (!InitializeWorldGeneration())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to initialize World Generation"));
        FailedSystems.Add(EEng_SystemType::WorldGeneration);
        return;
    }
    
    // Phase 4: Character Systems
    CurrentPhase = EEng_InitializationPhase::CharacterSystems;
    if (!InitializeCharacterSystems())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to initialize Character Systems"));
        FailedSystems.Add(EEng_SystemType::Character);
        return;
    }
    
    CurrentPhase = EEng_InitializationPhase::Complete;
    
    double InitTime = FPlatformTime::Seconds() - InitializationStartTime;
    UE_LOG(LogTemp, Warning, TEXT("SystemIntegrationCore: All systems initialized in %.2f seconds"), InitTime);
    
    LogSystemStatus();
}

bool UEng_SystemIntegrationCore::ValidateSystemIntegration()
{
    UE_LOG(LogTemp, Warning, TEXT("SystemIntegrationCore: Validating system integration"));
    
    bool bAllSystemsValid = true;
    
    // Validate each system
    for (const auto& SystemPair : SystemInitializationStatus)
    {
        EEng_SystemType SystemType = SystemPair.Key;
        bool bInitialized = SystemPair.Value;
        
        if (!bInitialized)
        {
            UE_LOG(LogTemp, Error, TEXT("System %d not initialized"), (int32)SystemType);
            bAllSystemsValid = false;
            continue;
        }
        
        // Validate dependency chain
        if (!ValidateDependencyChain(SystemType))
        {
            UE_LOG(LogTemp, Error, TEXT("System %d dependency chain invalid"), (int32)SystemType);
            bAllSystemsValid = false;
        }
    }
    
    return bAllSystemsValid;
}

FString UEng_SystemIntegrationCore::GetSystemStatus() const
{
    FString StatusString = TEXT("=== SYSTEM INTEGRATION STATUS ===\n");
    
    StatusString += FString::Printf(TEXT("Current Phase: %d\n"), (int32)CurrentPhase);
    StatusString += FString::Printf(TEXT("Initialized Systems: %d\n"), SystemInitializationStatus.Num());
    StatusString += FString::Printf(TEXT("Failed Systems: %d\n"), FailedSystems.Num());
    
    StatusString += TEXT("\nSystem Details:\n");
    for (const auto& SystemPair : SystemInitializationStatus)
    {
        EEng_SystemType SystemType = SystemPair.Key;
        bool bInitialized = SystemPair.Value;
        StatusString += FString::Printf(TEXT("- System %d: %s\n"), 
            (int32)SystemType, 
            bInitialized ? TEXT("INITIALIZED") : TEXT("FAILED"));
    }
    
    if (FailedSystems.Num() > 0)
    {
        StatusString += TEXT("\nFailed Systems:\n");
        for (EEng_SystemType FailedSystem : FailedSystems)
        {
            StatusString += FString::Printf(TEXT("- System %d\n"), (int32)FailedSystem);
        }
    }
    
    return StatusString;
}

bool UEng_SystemIntegrationCore::ReinitializeSystem(EEng_SystemType SystemType)
{
    UE_LOG(LogTemp, Warning, TEXT("SystemIntegrationCore: Reinitializing system %d"), (int32)SystemType);
    
    // Check dependencies first
    if (!AreDependenciesSatisfied(SystemType))
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot reinitialize system %d - dependencies not satisfied"), (int32)SystemType);
        return false;
    }
    
    bool bSuccess = false;
    
    switch (SystemType)
    {
        case EEng_SystemType::Architecture:
            bSuccess = InitializeArchitectureCore();
            break;
        case EEng_SystemType::Physics:
            bSuccess = InitializePhysicsCore();
            break;
        case EEng_SystemType::WorldGeneration:
            bSuccess = InitializeWorldGeneration();
            break;
        case EEng_SystemType::Character:
            bSuccess = InitializeCharacterSystems();
            break;
        default:
            UE_LOG(LogTemp, Error, TEXT("Unknown system type %d"), (int32)SystemType);
            return false;
    }
    
    SystemInitializationStatus.Add(SystemType, bSuccess);
    
    if (!bSuccess)
    {
        FailedSystems.AddUnique(SystemType);
    }
    else
    {
        FailedSystems.Remove(SystemType);
    }
    
    return bSuccess;
}

void UEng_SystemIntegrationCore::InitializeSystemDependencies()
{
    SystemDependencies.Empty();
    
    // Architecture has no dependencies
    SystemDependencies.Add(EEng_SystemType::Architecture, TArray<EEng_SystemType>());
    
    // Physics depends on Architecture
    TArray<EEng_SystemType> PhysicsDeps;
    PhysicsDeps.Add(EEng_SystemType::Architecture);
    SystemDependencies.Add(EEng_SystemType::Physics, PhysicsDeps);
    
    // World Generation depends on Architecture and Physics
    TArray<EEng_SystemType> WorldGenDeps;
    WorldGenDeps.Add(EEng_SystemType::Architecture);
    WorldGenDeps.Add(EEng_SystemType::Physics);
    SystemDependencies.Add(EEng_SystemType::WorldGeneration, WorldGenDeps);
    
    // Character systems depend on all previous systems
    TArray<EEng_SystemType> CharacterDeps;
    CharacterDeps.Add(EEng_SystemType::Architecture);
    CharacterDeps.Add(EEng_SystemType::Physics);
    CharacterDeps.Add(EEng_SystemType::WorldGeneration);
    SystemDependencies.Add(EEng_SystemType::Character, CharacterDeps);
}

bool UEng_SystemIntegrationCore::InitializeArchitectureCore()
{
    UE_LOG(LogTemp, Warning, TEXT("Initializing Architecture Core"));
    
    if (!ArchitectureCore)
    {
        ArchitectureCore = NewObject<UEng_ArchitectureCore>(this);
    }
    
    if (ArchitectureCore)
    {
        SystemInitializationStatus.Add(EEng_SystemType::Architecture, true);
        UE_LOG(LogTemp, Warning, TEXT("Architecture Core initialized successfully"));
        return true;
    }
    
    UE_LOG(LogTemp, Error, TEXT("Failed to create Architecture Core"));
    SystemInitializationStatus.Add(EEng_SystemType::Architecture, false);
    return false;
}

bool UEng_SystemIntegrationCore::InitializePhysicsCore()
{
    UE_LOG(LogTemp, Warning, TEXT("Initializing Physics Core"));
    
    if (!PhysicsManager)
    {
        PhysicsManager = NewObject<UCore_PhysicsManager>(this);
    }
    
    if (PhysicsManager)
    {
        SystemInitializationStatus.Add(EEng_SystemType::Physics, true);
        UE_LOG(LogTemp, Warning, TEXT("Physics Core initialized successfully"));
        return true;
    }
    
    UE_LOG(LogTemp, Error, TEXT("Failed to create Physics Manager"));
    SystemInitializationStatus.Add(EEng_SystemType::Physics, false);
    return false;
}

bool UEng_SystemIntegrationCore::InitializeWorldGeneration()
{
    UE_LOG(LogTemp, Warning, TEXT("Initializing World Generation"));
    
    if (!BiomeManager)
    {
        BiomeManager = NewObject<UBiomeManager>(this);
    }
    
    if (BiomeManager)
    {
        SystemInitializationStatus.Add(EEng_SystemType::WorldGeneration, true);
        UE_LOG(LogTemp, Warning, TEXT("World Generation initialized successfully"));
        return true;
    }
    
    UE_LOG(LogTemp, Error, TEXT("Failed to create Biome Manager"));
    SystemInitializationStatus.Add(EEng_SystemType::WorldGeneration, false);
    return false;
}

bool UEng_SystemIntegrationCore::InitializeCharacterSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Initializing Character Systems"));
    
    // Character systems are primarily actor-based, so we validate the class exists
    UClass* DinosaurClass = UClass::TryFindTypeSlow<UClass>(TEXT("DinosaurBase"));
    
    if (DinosaurClass)
    {
        SystemInitializationStatus.Add(EEng_SystemType::Character, true);
        UE_LOG(LogTemp, Warning, TEXT("Character Systems initialized successfully"));
        return true;
    }
    
    UE_LOG(LogTemp, Error, TEXT("Failed to find DinosaurBase class"));
    SystemInitializationStatus.Add(EEng_SystemType::Character, false);
    return false;
}

bool UEng_SystemIntegrationCore::ValidateDependencyChain(EEng_SystemType SystemType)
{
    const TArray<EEng_SystemType>* Dependencies = SystemDependencies.Find(SystemType);
    if (!Dependencies)
    {
        return true; // No dependencies
    }
    
    for (EEng_SystemType Dependency : *Dependencies)
    {
        const bool* bInitialized = SystemInitializationStatus.Find(Dependency);
        if (!bInitialized || !(*bInitialized))
        {
            return false;
        }
    }
    
    return true;
}

bool UEng_SystemIntegrationCore::AreDependenciesSatisfied(EEng_SystemType SystemType)
{
    return ValidateDependencyChain(SystemType);
}

void UEng_SystemIntegrationCore::LogSystemStatus() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== SYSTEM INTEGRATION STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Current Phase: %d"), (int32)CurrentPhase);
    UE_LOG(LogTemp, Warning, TEXT("Initialized Systems: %d"), SystemInitializationStatus.Num());
    UE_LOG(LogTemp, Warning, TEXT("Failed Systems: %d"), FailedSystems.Num());
    
    for (const auto& SystemPair : SystemInitializationStatus)
    {
        EEng_SystemType SystemType = SystemPair.Key;
        bool bInitialized = SystemPair.Value;
        UE_LOG(LogTemp, Warning, TEXT("System %d: %s"), 
            (int32)SystemType, 
            bInitialized ? TEXT("INITIALIZED") : TEXT("FAILED"));
    }
}