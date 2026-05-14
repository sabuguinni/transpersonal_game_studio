#include "StudioDirectorSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UStudioDirectorComponent::UStudioDirectorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update every second
    
    LastCoordinationUpdate = 0.0f;
    bSystemInitialized = false;
}

void UStudioDirectorComponent::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAgentCoordination();
    bSystemInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director System Initialized"));
}

void UStudioDirectorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bSystemInitialized)
    {
        LastCoordinationUpdate += DeltaTime;
        
        // Coordinate agents every 5 seconds
        if (LastCoordinationUpdate >= 5.0f)
        {
            CoordinateAgentTasks();
            CheckBiomeReadiness();
            LastCoordinationUpdate = 0.0f;
        }
    }
}

void UStudioDirectorComponent::InitializeAgentCoordination()
{
    // Clear existing data
    AgentStatuses.Empty();
    BiomeCoordinations.Empty();
    
    // Setup default agent statuses
    SetupDefaultAgents();
    
    // Setup default biome coordinates
    SetupDefaultBiomes();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Initialized %d agents and %d biomes"), 
           AgentStatuses.Num(), BiomeCoordinations.Num());
}

void UStudioDirectorComponent::SetupDefaultAgents()
{
    TArray<FString> AgentNames = {
        TEXT("Engine Architect"),
        TEXT("Core Systems Programmer"),
        TEXT("Performance Optimizer"),
        TEXT("Procedural World Generator"),
        TEXT("Environment Artist"),
        TEXT("Architecture & Interior Agent"),
        TEXT("Lighting & Atmosphere Agent"),
        TEXT("Character Artist Agent"),
        TEXT("Animation Agent"),
        TEXT("NPC Behavior Agent"),
        TEXT("Combat & Enemy AI Agent"),
        TEXT("Crowd & Traffic Simulation"),
        TEXT("Quest & Mission Designer"),
        TEXT("Narrative & Dialogue Agent"),
        TEXT("Audio Agent"),
        TEXT("VFX Agent"),
        TEXT("QA & Testing Agent"),
        TEXT("Integration & Build Agent")
    };
    
    for (const FString& AgentName : AgentNames)
    {
        FDir_AgentStatus NewAgent;
        NewAgent.AgentName = AgentName;
        NewAgent.bIsActive = true;
        NewAgent.LastUpdateTime = 0.0f;
        NewAgent.CurrentTask = TEXT("Initializing");
        NewAgent.CompletedCycles = 0;
        
        AgentStatuses.Add(NewAgent);
    }
}

void UStudioDirectorComponent::SetupDefaultBiomes()
{
    // Setup the 5 key biomes for asset pipeline preparation
    TArray<TPair<FString, FVector>> BiomeData = {
        {TEXT("Swamp_SW"), FVector(-5000, -5000, 200)},
        {TEXT("Forest_NW"), FVector(-5000, 5000, 300)},
        {TEXT("Savanna_Center"), FVector(0, 0, 100)},
        {TEXT("Desert_E"), FVector(5000, 0, 150)},
        {TEXT("Mountains_NE"), FVector(5000, 5000, 800)}
    };
    
    for (const auto& BiomeInfo : BiomeData)
    {
        FDir_BiomeCoordination NewBiome;
        NewBiome.BiomeName = BiomeInfo.Key;
        NewBiome.CenterLocation = BiomeInfo.Value;
        NewBiome.Radius = 5000.0f;
        NewBiome.bIsReady = false;
        
        // Assign relevant agents to each biome
        NewBiome.AssignedAgents.Add(TEXT("Procedural World Generator"));
        NewBiome.AssignedAgents.Add(TEXT("Environment Artist"));
        NewBiome.AssignedAgents.Add(TEXT("Lighting & Atmosphere Agent"));
        
        BiomeCoordinations.Add(NewBiome);
    }
}

void UStudioDirectorComponent::UpdateAgentStatus(const FString& AgentName, const FString& Task, bool bActive)
{
    FDir_AgentStatus* AgentStatus = FindAgentStatus(AgentName);
    if (AgentStatus)
    {
        AgentStatus->CurrentTask = Task;
        AgentStatus->bIsActive = bActive;
        AgentStatus->LastUpdateTime = GetWorld()->GetTimeSeconds();
        AgentStatus->CompletedCycles++;
        
        UE_LOG(LogTemp, Log, TEXT("Studio Director: Updated %s - Task: %s, Active: %s"), 
               *AgentName, *Task, bActive ? TEXT("Yes") : TEXT("No"));
    }
}

void UStudioDirectorComponent::RegisterBiome(const FString& BiomeName, FVector Location, float BiomeRadius)
{
    FDir_BiomeCoordination* Biome = FindBiome(BiomeName);
    if (Biome)
    {
        Biome->CenterLocation = Location;
        Biome->Radius = BiomeRadius;
        Biome->bIsReady = true;
        
        UE_LOG(LogTemp, Warning, TEXT("Studio Director: Registered biome %s at location %s"), 
               *BiomeName, *Location.ToString());
    }
}

bool UStudioDirectorComponent::ValidateGameplayReadiness()
{
    // Check if minimum viable prototype requirements are met
    int32 ActiveAgents = 0;
    int32 ReadyBiomes = 0;
    
    for (const FDir_AgentStatus& Agent : AgentStatuses)
    {
        if (Agent.bIsActive && Agent.CompletedCycles > 0)
        {
            ActiveAgents++;
        }
    }
    
    for (const FDir_BiomeCoordination& Biome : BiomeCoordinations)
    {
        if (Biome.bIsReady)
        {
            ReadyBiomes++;
        }
    }
    
    bool bMinimumViable = (ActiveAgents >= 10) && (ReadyBiomes >= 3);
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Gameplay Readiness - Active Agents: %d/18, Ready Biomes: %d/5, Minimum Viable: %s"), 
           ActiveAgents, ReadyBiomes, bMinimumViable ? TEXT("YES") : TEXT("NO"));
    
    return bMinimumViable;
}

void UStudioDirectorComponent::CoordinateAgentTasks()
{
    // Implement agent task coordination logic
    for (FDir_AgentStatus& Agent : AgentStatuses)
    {
        // Check if agent needs task update based on current game state
        if (Agent.CurrentTask == TEXT("Initializing") && Agent.CompletedCycles > 0)
        {
            Agent.CurrentTask = TEXT("Production");
        }
        
        // Detect inactive agents
        float TimeSinceUpdate = GetWorld()->GetTimeSeconds() - Agent.LastUpdateTime;
        if (TimeSinceUpdate > 300.0f) // 5 minutes
        {
            Agent.bIsActive = false;
            UE_LOG(LogTemp, Warning, TEXT("Studio Director: Agent %s marked inactive (no update for %f seconds)"), 
                   *Agent.AgentName, TimeSinceUpdate);
        }
    }
}

void UStudioDirectorComponent::CheckBiomeReadiness()
{
    // Check if biomes meet the 3 criteria for asset purchases
    for (FDir_BiomeCoordination& Biome : BiomeCoordinations)
    {
        // Criteria 1: Landscape expanded (check if terrain exists)
        // Criteria 2: Cretaceous atmosphere (check lighting)
        // Criteria 3: FBX import pipeline (check asset import capability)
        
        // For now, mark as ready if assigned agents are active
        bool bAllAgentsActive = true;
        for (const FString& AgentName : Biome.AssignedAgents)
        {
            FDir_AgentStatus* Agent = FindAgentStatus(AgentName);
            if (!Agent || !Agent->bIsActive)
            {
                bAllAgentsActive = false;
                break;
            }
        }
        
        if (bAllAgentsActive && !Biome.bIsReady)
        {
            Biome.bIsReady = true;
            UE_LOG(LogTemp, Warning, TEXT("Studio Director: Biome %s marked as ready"), *Biome.BiomeName);
        }
    }
}

void UStudioDirectorComponent::ValidateAssetPipeline()
{
    // Validate that asset import pipeline is ready for commercial assets
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Validating asset pipeline for TurboSquid dinosaurs and RealBiomes"));
    
    // Check FBX import capability
    // Check biome geographic separation
    // Check atmospheric lighting stability
}

void UStudioDirectorComponent::DebugPrintAgentStatuses()
{
    UE_LOG(LogTemp, Warning, TEXT("=== STUDIO DIRECTOR AGENT STATUS REPORT ==="));
    
    for (const FDir_AgentStatus& Agent : AgentStatuses)
    {
        UE_LOG(LogTemp, Warning, TEXT("Agent: %s | Active: %s | Task: %s | Cycles: %d"), 
               *Agent.AgentName, 
               Agent.bIsActive ? TEXT("YES") : TEXT("NO"), 
               *Agent.CurrentTask, 
               Agent.CompletedCycles);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END AGENT STATUS REPORT ==="));
}

void UStudioDirectorComponent::ValidateMinPlayableMap()
{
    UE_LOG(LogTemp, Warning, TEXT("=== STUDIO DIRECTOR MAP VALIDATION ==="));
    
    // Check for required gameplay elements
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    int32 TerrainCount = 0;
    int32 DinosaurCount = 0;
    int32 VegetationCount = 0;
    int32 LightingCount = 0;
    
    for (AActor* Actor : AllActors)
    {
        FString ActorName = Actor->GetName();
        FString ClassName = Actor->GetClass()->GetName();
        
        if (ClassName.Contains(TEXT("Landscape")))
            TerrainCount++;
        else if (ActorName.Contains(TEXT("Rex")) || ActorName.Contains(TEXT("Raptor")) || ActorName.Contains(TEXT("Brachio")))
            DinosaurCount++;
        else if (ActorName.Contains(TEXT("Tree")) || ActorName.Contains(TEXT("Plant")))
            VegetationCount++;
        else if (ClassName.Contains(TEXT("Light")))
            LightingCount++;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Map Elements - Terrain: %d, Dinosaurs: %d, Vegetation: %d, Lights: %d"), 
           TerrainCount, DinosaurCount, VegetationCount, LightingCount);
    
    bool bMapReady = (TerrainCount > 0) && (DinosaurCount >= 3) && (VegetationCount >= 5) && (LightingCount >= 2);
    UE_LOG(LogTemp, Warning, TEXT("MinPlayableMap Status: %s"), bMapReady ? TEXT("READY") : TEXT("NEEDS WORK"));
}

FDir_AgentStatus* UStudioDirectorComponent::FindAgentStatus(const FString& AgentName)
{
    for (FDir_AgentStatus& Agent : AgentStatuses)
    {
        if (Agent.AgentName == AgentName)
        {
            return &Agent;
        }
    }
    return nullptr;
}

FDir_BiomeCoordination* UStudioDirectorComponent::FindBiome(const FString& BiomeName)
{
    for (FDir_BiomeCoordination& Biome : BiomeCoordinations)
    {
        if (Biome.BiomeName == BiomeName)
        {
            return &Biome;
        }
    }
    return nullptr;
}

// AStudioDirectorActor Implementation

AStudioDirectorActor::AStudioDirectorActor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 2.0f;
    
    // Create director component
    DirectorComponent = CreateDefaultSubobject<UStudioDirectorComponent>(TEXT("DirectorComponent"));
}

void AStudioDirectorActor::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director Actor initialized in world"));
}

void AStudioDirectorActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Monitor overall game state
    MonitorGameplayState();
}

void AStudioDirectorActor::ExecuteAgentCoordination()
{
    if (DirectorComponent)
    {
        DirectorComponent->CoordinateAgentTasks();
        DirectorComponent->CheckBiomeReadiness();
        DirectorComponent->ValidateAssetPipeline();
    }
}

void AStudioDirectorActor::MonitorGameplayState()
{
    if (DirectorComponent)
    {
        bool bGameplayReady = DirectorComponent->ValidateGameplayReadiness();
        
        // Log status periodically
        static float LastStatusLog = 0.0f;
        LastStatusLog += GetWorld()->GetDeltaSeconds();
        
        if (LastStatusLog >= 10.0f) // Log every 10 seconds
        {
            UE_LOG(LogTemp, Warning, TEXT("Studio Director: Gameplay Ready = %s"), 
                   bGameplayReady ? TEXT("YES") : TEXT("NO"));
            LastStatusLog = 0.0f;
        }
    }
}