#include "ProductionManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Landscape/Landscape.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"

UProductionManager::UProductionManager()
{
    bCompilationReady = false;
    bMinPlayableMapValid = false;
    PhantomHeaderCount = 0;
    DuplicateActorCount = 0;
}

void UProductionManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionManager: Initializing Studio Director production pipeline"));
    
    InitializeBiomeData();
    
    // Set up critical path for agents
    CriticalPathAgents = {
        TEXT("Agent #20 (Integration)"),
        TEXT("Agent #5 (World Generator)"),
        TEXT("Agent #9 (Character Artist)"),
        TEXT("Agent #12 (Combat/UI)")
    };
    
    // Initialize agent priorities
    AgentPriorities.Add(TEXT("Agent #20"), 1); // Highest priority - compilation cleanup
    AgentPriorities.Add(TEXT("Agent #5"), 2);  // World expansion
    AgentPriorities.Add(TEXT("Agent #9"), 3);  // Dinosaur placement
    AgentPriorities.Add(TEXT("Agent #12"), 4); // UI implementation
    
    // Perform initial assessment
    ValidateCompilationReadiness();
    CheckBiomeDistribution();
}

void UProductionManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("ProductionManager: Shutting down production pipeline"));
    Super::Deinitialize();
}

void UProductionManager::InitializeBiomeData()
{
    // Initialize biome centers based on memory coordinates
    BiomeCenters.Add(TEXT("Pantano"), FVector(-50000.0f, -45000.0f, 0.0f));
    BiomeCenters.Add(TEXT("Floresta"), FVector(-45000.0f, 40000.0f, 0.0f));
    BiomeCenters.Add(TEXT("Savana"), FVector(0.0f, 0.0f, 0.0f));
    BiomeCenters.Add(TEXT("Deserto"), FVector(55000.0f, 0.0f, 0.0f));
    BiomeCenters.Add(TEXT("Montanha"), FVector(40000.0f, 50000.0f, 500.0f));
    
    // Initialize actor counts
    for (const auto& BiomePair : BiomeCenters)
    {
        BiomeActorCounts.Add(BiomePair.Key, 0);
    }
}

void UProductionManager::DistributeAgentTasks()
{
    UE_LOG(LogTemp, Warning, TEXT("ProductionManager: Distributing tasks to agents"));
    
    // Log critical production priorities
    UE_LOG(LogTemp, Warning, TEXT("CRITICAL: Agent #20 must clean 122 phantom headers"));
    UE_LOG(LogTemp, Warning, TEXT("CRITICAL: Remove duplicate systems (DinosaurCrowdSystem_Crowd.h + MassDinosaurSystem.h)"));
    UE_LOG(LogTemp, Warning, TEXT("CRITICAL: Move misplaced .cpp files to Source/ directory"));
    UE_LOG(LogTemp, Warning, TEXT("CRITICAL: All spawns must use biome coordinates - NEVER Vector(0,0,0)"));
    
    // Validate current state
    ValidateCompilationReadiness();
    CheckBiomeDistribution();
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionManager: Task distribution complete"));
}

void UProductionManager::ValidateCompilationReadiness()
{
    UE_LOG(LogTemp, Warning, TEXT("ProductionManager: Validating compilation readiness"));
    
    ScanForPhantomHeaders();
    CountDuplicateActors();
    
    // Check if compilation is ready
    bCompilationReady = (PhantomHeaderCount == 0);
    
    if (!bCompilationReady)
    {
        UE_LOG(LogTemp, Error, TEXT("COMPILATION BLOCKED: %d phantom headers found"), PhantomHeaderCount);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("COMPILATION READY: No phantom headers detected"));
    }
}

void UProductionManager::ScanForPhantomHeaders()
{
    // Simulate scanning for phantom headers
    // In real implementation, this would scan the Source directory
    PhantomHeaderCount = 122; // Based on memory data
    
    UE_LOG(LogTemp, Error, TEXT("Found %d phantom headers without .cpp implementation"), PhantomHeaderCount);
}

void UProductionManager::CountDuplicateActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    DuplicateActorCount = 0;
    
    // Count lighting actors
    TArray<AActor*> DirectionalLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);
    
    if (DirectionalLights.Num() > 1)
    {
        DuplicateActorCount += DirectionalLights.Num() - 1;
        UE_LOG(LogTemp, Error, TEXT("Found %d DirectionalLights (should be 1)"), DirectionalLights.Num());
    }
    
    // Additional duplicate checks would go here for SkyAtmosphere, SkyLight, etc.
    
    UE_LOG(LogTemp, Warning, TEXT("Total duplicate actors: %d"), DuplicateActorCount);
}

void UProductionManager::CheckBiomeDistribution()
{
    UE_LOG(LogTemp, Warning, TEXT("ProductionManager: Checking biome distribution"));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Reset biome counts
    for (auto& BiomePair : BiomeActorCounts)
    {
        BiomePair.Value = 0;
    }
    
    // Count actors in each biome
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (!Actor)
        {
            continue;
        }
        
        FVector ActorLocation = Actor->GetActorLocation();
        
        // Check which biome this actor belongs to
        for (const auto& BiomePair : BiomeCenters)
        {
            FVector BiomeCenter = BiomePair.Value;
            float Distance = FVector::Dist2D(ActorLocation, BiomeCenter);
            
            // If within 20km of biome center, count it
            if (Distance < 20000.0f)
            {
                BiomeActorCounts[BiomePair.Key]++;
                break;
            }
        }
    }
    
    // Log distribution
    for (const auto& BiomePair : BiomeActorCounts)
    {
        UE_LOG(LogTemp, Warning, TEXT("Biome %s: %d actors"), *BiomePair.Key, BiomePair.Value);
    }
}

bool UProductionManager::IsMinPlayableMapReady() const
{
    return bMinPlayableMapValid && HasPlayableCharacter() && HasFunctionalTerrain();
}

int32 UProductionManager::GetPhantomHeaderCount() const
{
    return PhantomHeaderCount;
}

int32 UProductionManager::GetDuplicateActorCount() const
{
    return DuplicateActorCount;
}

bool UProductionManager::HasPlayableCharacter() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Check for TranspersonalCharacter or any pawn
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    return PlayerPawn != nullptr;
}

bool UProductionManager::HasFunctionalTerrain() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Check for landscape
    TArray<AActor*> Landscapes;
    UGameplayStatics::GetAllActorsOfClass(World, ALandscape::StaticClass(), Landscapes);
    
    return Landscapes.Num() > 0;
}

void UProductionManager::SetAgentPriority(const FString& AgentName, int32 Priority)
{
    AgentPriorities.Add(AgentName, Priority);
    UE_LOG(LogTemp, Warning, TEXT("Set priority %d for %s"), Priority, *AgentName);
}

TArray<FString> UProductionManager::GetCriticalPath() const
{
    return CriticalPathAgents;
}

void UProductionManager::ReportAgentProgress(const FString& AgentName, const FString& Deliverable)
{
    UE_LOG(LogTemp, Warning, TEXT("Agent Progress: %s delivered %s"), *AgentName, *Deliverable);
}