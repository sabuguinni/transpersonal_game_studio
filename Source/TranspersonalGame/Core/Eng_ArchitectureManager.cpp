#include "Eng_ArchitectureManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/PlayerStart.h"
#include "Landscape.h"
#include "GameFramework/Character.h"
#include "Engine/StaticMeshActor.h"

void UEng_ArchitectureManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: ArchitectureManager initialized"));
    
    bArchitectureValid = false;
    bMilestone1Ready = false;
    
    // Initialize biome configurations
    InitializeBiomeConfigurations();
    
    // Setup performance monitoring
    SetupPerformanceMonitoring();
    
    // Register required systems
    RequiredSystems.Add(TEXT("CharacterSystem"));
    RequiredSystems.Add(TEXT("DinosaurSystem"));
    RequiredSystems.Add(TEXT("BiomeSystem"));
    RequiredSystems.Add(TEXT("WorldGeneration"));
    RequiredSystems.Add(TEXT("LightingSystem"));
    
    // Optional systems
    OptionalSystems.Add(TEXT("QuestSystem"));
    OptionalSystems.Add(TEXT("AudioSystem"));
    OptionalSystems.Add(TEXT("VFXSystem"));
    
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Core systems registered"));
}

void UEng_ArchitectureManager::Deinitialize()
{
    RegisteredSystems.Empty();
    SystemPerformanceMetrics.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: ArchitectureManager deinitialized"));
    
    Super::Deinitialize();
}

bool UEng_ArchitectureManager::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

bool UEng_ArchitectureManager::ValidateSystemArchitecture()
{
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Validating system architecture..."));
    
    bool bValid = true;
    
    // Validate system dependencies
    if (!ValidateSystemDependencies())
    {
        UE_LOG(LogTemp, Error, TEXT("ENGINE ARCHITECT: System dependencies validation failed"));
        bValid = false;
    }
    
    // Validate memory usage
    if (!ValidateMemoryUsage())
    {
        UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Memory usage validation failed"));
        // Don't fail architecture for memory warnings
    }
    
    // Validate rendering pipeline
    if (!ValidateRenderingPipeline())
    {
        UE_LOG(LogTemp, Error, TEXT("ENGINE ARCHITECT: Rendering pipeline validation failed"));
        bValid = false;
    }
    
    bArchitectureValid = bValid;
    return bValid;
}

bool UEng_ArchitectureManager::ValidateMilestone1Requirements()
{
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Validating Milestone 1 requirements..."));
    
    bool bReady = true;
    
    // Check character system
    if (!ValidateCharacterSystem())
    {
        UE_LOG(LogTemp, Error, TEXT("MILESTONE 1: Character system not ready"));
        bReady = false;
    }
    
    // Check dinosaur system
    if (!ValidateDinosaurSystem())
    {
        UE_LOG(LogTemp, Error, TEXT("MILESTONE 1: Dinosaur system not ready"));
        bReady = false;
    }
    
    // Check world generation
    if (!ValidateWorldGenerationSystem())
    {
        UE_LOG(LogTemp, Error, TEXT("MILESTONE 1: World generation not ready"));
        bReady = false;
    }
    
    // Check biome distribution
    if (!ValidateBiomeDistribution())
    {
        UE_LOG(LogTemp, Error, TEXT("MILESTONE 1: Biome distribution not ready"));
        bReady = false;
    }
    
    bMilestone1Ready = bReady;
    
    if (bReady)
    {
        UE_LOG(LogTemp, Warning, TEXT("MILESTONE 1: All requirements validated successfully!"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("MILESTONE 1: Requirements validation failed"));
    }
    
    return bReady;
}

void UEng_ArchitectureManager::EnforceArchitecturalRules()
{
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Enforcing architectural rules..."));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Ensure single instances of critical actors
    TArray<AActor*> DirectionalLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);
    
    if (DirectionalLights.Num() > 1)
    {
        UE_LOG(LogTemp, Warning, TEXT("ARCHITECTURE: Multiple DirectionalLights detected, keeping only one"));
        for (int32 i = 1; i < DirectionalLights.Num(); i++)
        {
            DirectionalLights[i]->Destroy();
        }
    }
    
    // Validate actor distribution
    ValidateActorDistribution();
}

void UEng_ArchitectureManager::RegisterCoreSystem(const FString& SystemName, UObject* SystemInstance)
{
    if (SystemInstance)
    {
        RegisteredSystems.Add(SystemName, SystemInstance);
        UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Registered system '%s'"), *SystemName);
    }
}

UObject* UEng_ArchitectureManager::GetCoreSystem(const FString& SystemName)
{
    if (UObject** FoundSystem = RegisteredSystems.Find(SystemName))
    {
        return *FoundSystem;
    }
    return nullptr;
}

void UEng_ArchitectureManager::MonitorSystemPerformance()
{
    // Basic performance monitoring
    SystemPerformanceMetrics.Add(TEXT("FrameRate"), 1.0f / GetWorld()->GetDeltaSeconds());
    SystemPerformanceMetrics.Add(TEXT("ActorCount"), static_cast<float>(GetWorld()->GetCurrentLevel()->Actors.Num()));
    
    // Log performance metrics
    for (const auto& Metric : SystemPerformanceMetrics)
    {
        UE_LOG(LogTemp, Log, TEXT("PERFORMANCE: %s = %.2f"), *Metric.Key, Metric.Value);
    }
}

float UEng_ArchitectureManager::GetSystemPerformanceMetric(const FString& SystemName)
{
    if (float* FoundMetric = SystemPerformanceMetrics.Find(SystemName))
    {
        return *FoundMetric;
    }
    return 0.0f;
}

bool UEng_ArchitectureManager::ValidateBiomeDistribution()
{
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Validating biome distribution..."));
    
    // Check if biome centers are properly configured
    for (const auto& BiomePair : BiomeConfigurations)
    {
        EEng_BiomeType BiomeType = BiomePair.Key;
        FVector BiomeCenter = GetBiomeCenterLocation(BiomeType);
        
        if (BiomeCenter.IsZero())
        {
            UE_LOG(LogTemp, Error, TEXT("BIOME: %s has invalid center location"), 
                   *UEnum::GetValueAsString(BiomeType));
            return false;
        }
    }
    
    return true;
}

FVector UEng_ArchitectureManager::GetBiomeCenterLocation(EEng_BiomeType BiomeType)
{
    // Return biome center coordinates based on the established map layout
    switch (BiomeType)
    {
        case EEng_BiomeType::Swamp:
            return FVector(-50000.0f, -45000.0f, 0.0f);
        case EEng_BiomeType::Forest:
            return FVector(-45000.0f, 40000.0f, 0.0f);
        case EEng_BiomeType::Savanna:
            return FVector(0.0f, 0.0f, 0.0f);
        case EEng_BiomeType::Desert:
            return FVector(55000.0f, 0.0f, 0.0f);
        case EEng_BiomeType::SnowyMountain:
            return FVector(40000.0f, 50000.0f, 500.0f);
        default:
            return FVector::ZeroVector;
    }
}

bool UEng_ArchitectureManager::IsLocationInBiome(const FVector& Location, EEng_BiomeType BiomeType)
{
    FVector BiomeCenter = GetBiomeCenterLocation(BiomeType);
    float Distance = FVector::Dist(Location, BiomeCenter);
    
    // Each biome has approximately 25km radius
    return Distance <= 25000.0f;
}

bool UEng_ArchitectureManager::ValidateCharacterSystem()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Check for PlayerStart
    TArray<AActor*> PlayerStarts;
    UGameplayStatics::GetAllActorsOfClass(World, APlayerStart::StaticClass(), PlayerStarts);
    
    if (PlayerStarts.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("CHARACTER SYSTEM: No PlayerStart found"));
        return false;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("CHARACTER SYSTEM: Validated - %d PlayerStart(s) found"), PlayerStarts.Num());
    return true;
}

void UEng_ArchitectureManager::SetupDefaultCharacterController()
{
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Setting up default character controller"));
    // This will be implemented when TranspersonalCharacter.cpp is created
}

bool UEng_ArchitectureManager::ValidateDinosaurSystem()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Count dinosaur placeholders
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    int32 DinosaurCount = 0;
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetActorLabel().Contains(TEXT("Placeholder")))
        {
            DinosaurCount++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("DINOSAUR SYSTEM: Found %d dinosaur placeholders"), DinosaurCount);
    return DinosaurCount >= 3; // Minimum 3 dinosaurs for Milestone 1
}

void UEng_ArchitectureManager::SpawnDinosaurPlaceholders()
{
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Spawning dinosaur placeholders"));
    // This will create basic placeholders until DinosaurBase.cpp is implemented
}

bool UEng_ArchitectureManager::ValidateWorldGenerationSystem()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Check for landscape
    TArray<AActor*> Landscapes;
    UGameplayStatics::GetAllActorsOfClass(World, ALandscape::StaticClass(), Landscapes);
    
    if (Landscapes.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("WORLD GENERATION: No landscape found"));
        return false;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("WORLD GENERATION: Validated - %d landscape(s) found"), Landscapes.Num());
    return true;
}

void UEng_ArchitectureManager::InitializeWorldPartition()
{
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Initializing World Partition for large world"));
    // World Partition setup for 200km² world
}

bool UEng_ArchitectureManager::ValidateCompilationStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Validating compilation status"));
    
    // Check if critical classes can be loaded
    TArray<FString> CriticalClasses = {
        TEXT("TranspersonalCharacter"),
        TEXT("TranspersonalGameMode"),
        TEXT("DinosaurBase"),
        TEXT("BiomeManager")
    };
    
    bool bAllClassesValid = true;
    for (const FString& ClassName : CriticalClasses)
    {
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName));
        if (!LoadedClass)
        {
            UE_LOG(LogTemp, Error, TEXT("COMPILATION: Class '%s' failed to load"), *ClassName);
            bAllClassesValid = false;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("COMPILATION: Class '%s' loaded successfully"), *ClassName);
        }
    }
    
    return bAllClassesValid;
}

TArray<FString> UEng_ArchitectureManager::GetMissingImplementations()
{
    TArray<FString> MissingImplementations;
    
    // This would scan the source directory for .h files without .cpp
    // For now, return known missing implementations
    MissingImplementations.Add(TEXT("TranspersonalCharacter.cpp"));
    MissingImplementations.Add(TEXT("DinosaurBase.cpp"));
    MissingImplementations.Add(TEXT("BiomeManager.cpp"));
    
    return MissingImplementations;
}

bool UEng_ArchitectureManager::ValidateSystemDependencies()
{
    // Check if all required systems are registered
    for (const FString& RequiredSystem : RequiredSystems)
    {
        if (!RegisteredSystems.Contains(RequiredSystem))
        {
            UE_LOG(LogTemp, Warning, TEXT("DEPENDENCIES: Required system '%s' not registered"), *RequiredSystem);
            return false;
        }
    }
    return true;
}

bool UEng_ArchitectureManager::ValidateMemoryUsage()
{
    // Basic memory validation
    return true; // Placeholder
}

bool UEng_ArchitectureManager::ValidateRenderingPipeline()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Check for basic lighting
    TArray<AActor*> DirectionalLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);
    
    return DirectionalLights.Num() > 0;
}

void UEng_ArchitectureManager::InitializeBiomeConfigurations()
{
    // Initialize biome data
    FEng_BiomeData SwampData;
    SwampData.BiomeType = EEng_BiomeType::Swamp;
    SwampData.BiomeName = TEXT("Prehistoric Swamp");
    SwampData.Temperature = 28.0f;
    SwampData.Humidity = 90.0f;
    SwampData.ThreatLevel = EEng_ThreatLevel::Dangerous;
    SwampData.NativeDinosaurs.Add(EEng_DinosaurSpecies::Dilophosaurus);
    BiomeConfigurations.Add(EEng_BiomeType::Swamp, SwampData);
    
    FEng_BiomeData ForestData;
    ForestData.BiomeType = EEng_BiomeType::Forest;
    ForestData.BiomeName = TEXT("Dense Prehistoric Forest");
    ForestData.Temperature = 22.0f;
    ForestData.Humidity = 75.0f;
    ForestData.ThreatLevel = EEng_ThreatLevel::Cautious;
    ForestData.NativeDinosaurs.Add(EEng_DinosaurSpecies::Raptor);
    BiomeConfigurations.Add(EEng_BiomeType::Forest, ForestData);
    
    FEng_BiomeData SavannaData;
    SavannaData.BiomeType = EEng_BiomeType::Savanna;
    SavannaData.BiomeName = TEXT("Open Savanna Plains");
    SavannaData.Temperature = 30.0f;
    SavannaData.Humidity = 40.0f;
    SavannaData.ThreatLevel = EEng_ThreatLevel::Deadly;
    SavannaData.NativeDinosaurs.Add(EEng_DinosaurSpecies::TRex);
    SavannaData.NativeDinosaurs.Add(EEng_DinosaurSpecies::Triceratops);
    BiomeConfigurations.Add(EEng_BiomeType::Savanna, SavannaData);
    
    FEng_BiomeData DesertData;
    DesertData.BiomeType = EEng_BiomeType::Desert;
    DesertData.BiomeName = TEXT("Arid Desert Wasteland");
    DesertData.Temperature = 40.0f;
    DesertData.Humidity = 15.0f;
    DesertData.ThreatLevel = EEng_ThreatLevel::Extreme;
    DesertData.NativeDinosaurs.Add(EEng_DinosaurSpecies::Ankylosaurus);
    BiomeConfigurations.Add(EEng_BiomeType::Desert, DesertData);
    
    FEng_BiomeData MountainData;
    MountainData.BiomeType = EEng_BiomeType::SnowyMountain;
    MountainData.BiomeName = TEXT("Snowy Mountain Peaks");
    MountainData.Temperature = -5.0f;
    MountainData.Humidity = 60.0f;
    MountainData.ThreatLevel = EEng_ThreatLevel::Dangerous;
    MountainData.NativeDinosaurs.Add(EEng_DinosaurSpecies::Pteranodon);
    BiomeConfigurations.Add(EEng_BiomeType::SnowyMountain, MountainData);
    
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Biome configurations initialized"));
}

void UEng_ArchitectureManager::SetupPerformanceMonitoring()
{
    // Initialize performance metrics
    SystemPerformanceMetrics.Add(TEXT("FrameRate"), 60.0f);
    SystemPerformanceMetrics.Add(TEXT("ActorCount"), 0.0f);
    SystemPerformanceMetrics.Add(TEXT("MemoryUsage"), 0.0f);
    SystemPerformanceMetrics.Add(TEXT("DrawCalls"), 0.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Performance monitoring initialized"));
}

void UEng_ArchitectureManager::ValidateActorDistribution()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    int32 ActorsAtOrigin = 0;
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetActorLocation().IsNearlyZero())
        {
            ActorsAtOrigin++;
        }
    }
    
    if (ActorsAtOrigin > 5)
    {
        UE_LOG(LogTemp, Warning, TEXT("ARCHITECTURE: Too many actors at origin (%d), consider distribution"), ActorsAtOrigin);
    }
}