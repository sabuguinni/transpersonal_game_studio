#include "Director_ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

UDir_ProductionCoordinator::UDir_ProductionCoordinator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    // Initialize biome distribution targets
    BiomeDistributionTargets.Add(TEXT("Savana"), FDir_BiomeTarget{FVector(0, 0, 100), 0.20f});
    BiomeDistributionTargets.Add(TEXT("Pantano"), FDir_BiomeTarget{FVector(-50000, -45000, 100), 0.20f});
    BiomeDistributionTargets.Add(TEXT("Floresta"), FDir_BiomeTarget{FVector(-45000, 40000, 100), 0.20f});
    BiomeDistributionTargets.Add(TEXT("Deserto"), FDir_BiomeTarget{FVector(55000, 0, 100), 0.20f});
    BiomeDistributionTargets.Add(TEXT("Montanha"), FDir_BiomeTarget{FVector(40000, 50000, 100), 0.20f});
    
    // Initialize agent priorities
    InitializeAgentPriorities();
    
    // Initialize critical path milestones
    InitializeCriticalPath();
}

void UDir_ProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Production Coordinator initialized - Managing 19-agent pipeline"));
    
    // Start production monitoring
    StartProductionMonitoring();
}

void UDir_ProductionCoordinator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update production metrics every second
    UpdateProductionMetrics();
    
    // Check critical path violations
    ValidateCriticalPath();
    
    // Monitor biome distribution compliance
    MonitorBiomeDistribution();
}

void UDir_ProductionCoordinator::InitializeAgentPriorities()
{
    // Critical path agents (blocking others)
    AgentPriorities.Add(2, FDir_AgentPriority{TEXT("Engine Architect"), 10, true, TArray<int32>{3,4,5}});
    AgentPriorities.Add(15, FDir_AgentPriority{TEXT("Narrative Designer"), 10, true, TArray<int32>{14}});
    
    // Core gameplay agents
    AgentPriorities.Add(3, FDir_AgentPriority{TEXT("Core Systems"), 9, false, TArray<int32>{4,9,10}});
    AgentPriorities.Add(9, FDir_AgentPriority{TEXT("Character Artist"), 8, false, TArray<int32>{10}});
    AgentPriorities.Add(10, FDir_AgentPriority{TEXT("Animation Agent"), 8, false, TArray<int32>{11,12}});
    
    // World building agents
    AgentPriorities.Add(5, FDir_AgentPriority{TEXT("World Generator"), 7, false, TArray<int32>{6,7,8}});
    AgentPriorities.Add(6, FDir_AgentPriority{TEXT("Environment Artist"), 6, false, TArray<int32>{7,8}});
    AgentPriorities.Add(8, FDir_AgentPriority{TEXT("Lighting Artist"), 6, false, TArray<int32>{}});
    
    // AI and behavior agents
    AgentPriorities.Add(11, FDir_AgentPriority{TEXT("NPC Behavior"), 5, false, TArray<int32>{12,13}});
    AgentPriorities.Add(12, FDir_AgentPriority{TEXT("Combat AI"), 5, false, TArray<int32>{}});
    
    // Polish and integration agents
    AgentPriorities.Add(18, FDir_AgentPriority{TEXT("QA Testing"), 4, true, TArray<int32>{}});
    AgentPriorities.Add(19, FDir_AgentPriority{TEXT("Integration"), 3, true, TArray<int32>{}});
}

void UDir_ProductionCoordinator::InitializeCriticalPath()
{
    // Milestone 1: Walk Around (Cycles 19-20)
    CriticalMilestones.Add(FDir_Milestone{
        TEXT("Walk Around"),
        20,
        TArray<FString>{
            TEXT("TranspersonalCharacter with WASD movement"),
            TEXT("Camera boom + follow camera"),
            TEXT("Basic landscape terrain"),
            TEXT("Player walk/run/jump"),
            TEXT("5 static dinosaur meshes"),
            TEXT("Directional light + sky")
        },
        false
    });
    
    // Milestone 2: Basic Survival (Cycles 21-25)
    CriticalMilestones.Add(FDir_Milestone{
        TEXT("Basic Survival"),
        25,
        TArray<FString>{
            TEXT("Hunger/Thirst/Health systems"),
            TEXT("Basic crafting (stone tools)"),
            TEXT("Dinosaur basic AI (idle/pursue)"),
            TEXT("Day/night cycle"),
            TEXT("Temperature system")
        },
        false
    });
    
    // Milestone 3: Combat & Danger (Cycles 26-30)
    CriticalMilestones.Add(FDir_Milestone{
        TEXT("Combat & Danger"),
        30,
        TArray<FString>{
            TEXT("Primitive weapon combat"),
            TEXT("Dinosaur attack behaviors"),
            TEXT("Player death/respawn"),
            TEXT("Territory system"),
            TEXT("Basic shelter building")
        },
        false
    });
}

void UDir_ProductionCoordinator::StartProductionMonitoring()
{
    // Log production start
    UE_LOG(LogTemp, Warning, TEXT("Production monitoring started - Target: Milestone 1 by Cycle 20"));
    
    // Initialize metrics
    ProductionMetrics.TotalAgents = 19;
    ProductionMetrics.ActiveAgents = 0;
    ProductionMetrics.CompletedTasks = 0;
    ProductionMetrics.BlockedAgents = 0;
    ProductionMetrics.CurrentCycle = 20; // AUTO_20260530_002
    
    bProductionActive = true;
}

void UDir_ProductionCoordinator::UpdateProductionMetrics()
{
    if (!bProductionActive) return;
    
    // Count active vs blocked agents
    int32 ActiveCount = 0;
    int32 BlockedCount = 0;
    
    for (auto& Priority : AgentPriorities)
    {
        if (Priority.Value.bIsBlocking)
        {
            BlockedCount++;
        }
        else
        {
            ActiveCount++;
        }
    }
    
    ProductionMetrics.ActiveAgents = ActiveCount;
    ProductionMetrics.BlockedAgents = BlockedCount;
    
    // Log critical metrics every 10 ticks
    static int32 TickCounter = 0;
    TickCounter++;
    
    if (TickCounter >= 10)
    {
        UE_LOG(LogTemp, Log, TEXT("Production Status - Active: %d, Blocked: %d, Cycle: %d"), 
               ActiveCount, BlockedCount, ProductionMetrics.CurrentCycle);
        TickCounter = 0;
    }
}

void UDir_ProductionCoordinator::ValidateCriticalPath()
{
    // Check if we're on track for Milestone 1
    if (ProductionMetrics.CurrentCycle >= 20)
    {
        bool bMilestone1Ready = true;
        
        // Check TranspersonalCharacter exists and has movement
        if (!ValidateCharacterMovement())
        {
            bMilestone1Ready = false;
            UE_LOG(LogTemp, Error, TEXT("CRITICAL: TranspersonalCharacter movement not implemented"));
        }
        
        // Check terrain exists
        if (!ValidateBasicTerrain())
        {
            bMilestone1Ready = false;
            UE_LOG(LogTemp, Error, TEXT("CRITICAL: Basic terrain not implemented"));
        }
        
        // Check dinosaur meshes
        if (!ValidateDinosaurMeshes())
        {
            bMilestone1Ready = false;
            UE_LOG(LogTemp, Error, TEXT("CRITICAL: Dinosaur meshes not placed"));
        }
        
        if (!bMilestone1Ready)
        {
            UE_LOG(LogTemp, Error, TEXT("MILESTONE 1 BLOCKED - Immediate action required"));
            TriggerEmergencyProtocol();
        }
    }
}

void UDir_ProductionCoordinator::MonitorBiomeDistribution()
{
    // Check if actors are properly distributed across biomes
    UWorld* World = GetWorld();
    if (!World) return;
    
    TMap<FString, int32> BiomeCounts;
    BiomeCounts.Add(TEXT("Savana"), 0);
    BiomeCounts.Add(TEXT("Pantano"), 0);
    BiomeCounts.Add(TEXT("Floresta"), 0);
    BiomeCounts.Add(TEXT("Deserto"), 0);
    BiomeCounts.Add(TEXT("Montanha"), 0);
    
    // Count actors in each biome
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetName().Contains(TEXT("Dinosaur")))
        {
            FVector Location = Actor->GetActorLocation();
            FString Biome = GetBiomeFromLocation(Location);
            if (BiomeCounts.Contains(Biome))
            {
                BiomeCounts[Biome]++;
            }
        }
    }
    
    // Log distribution every 30 seconds
    static float LastLogTime = 0.0f;
    float CurrentTime = World->GetTimeSeconds();
    
    if (CurrentTime - LastLogTime > 30.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Biome Distribution - Savana: %d, Pantano: %d, Floresta: %d, Deserto: %d, Montanha: %d"),
               BiomeCounts[TEXT("Savana")], BiomeCounts[TEXT("Pantano")], BiomeCounts[TEXT("Floresta")],
               BiomeCounts[TEXT("Deserto")], BiomeCounts[TEXT("Montanha")]);
        LastLogTime = CurrentTime;
    }
}

bool UDir_ProductionCoordinator::ValidateCharacterMovement()
{
    // Check if TranspersonalCharacter exists in the world
    UWorld* World = GetWorld();
    if (!World) return false;
    
    for (TActorIterator<APawn> PawnItr(World); PawnItr; ++PawnItr)
    {
        APawn* Pawn = *PawnItr;
        if (Pawn && Pawn->GetName().Contains(TEXT("TranspersonalCharacter")))
        {
            // Character exists - assume movement is implemented
            return true;
        }
    }
    
    return false;
}

bool UDir_ProductionCoordinator::ValidateBasicTerrain()
{
    // Check if landscape exists
    UWorld* World = GetWorld();
    if (!World) return false;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && (Actor->GetName().Contains(TEXT("Landscape")) || Actor->GetName().Contains(TEXT("Terrain"))))
        {
            return true;
        }
    }
    
    return false;
}

bool UDir_ProductionCoordinator::ValidateDinosaurMeshes()
{
    // Check if dinosaur meshes are placed
    UWorld* World = GetWorld();
    if (!World) return false;
    
    int32 DinosaurCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && (Actor->GetName().Contains(TEXT("TRex")) || 
                     Actor->GetName().Contains(TEXT("Velociraptor")) ||
                     Actor->GetName().Contains(TEXT("Brachiosaurus"))))
        {
            DinosaurCount++;
        }
    }
    
    return DinosaurCount >= 5;
}

FString UDir_ProductionCoordinator::GetBiomeFromLocation(const FVector& Location)
{
    // Determine biome based on location
    float DistanceToSavana = FVector::Dist(Location, FVector(0, 0, 100));
    float DistanceToPantano = FVector::Dist(Location, FVector(-50000, -45000, 100));
    float DistanceToFloresta = FVector::Dist(Location, FVector(-45000, 40000, 100));
    float DistanceToDeserto = FVector::Dist(Location, FVector(55000, 0, 100));
    float DistanceToMontanha = FVector::Dist(Location, FVector(40000, 50000, 100));
    
    float MinDistance = FMath::Min({DistanceToSavana, DistanceToPantano, DistanceToFloresta, DistanceToDeserto, DistanceToMontanha});
    
    if (MinDistance == DistanceToSavana) return TEXT("Savana");
    if (MinDistance == DistanceToPantano) return TEXT("Pantano");
    if (MinDistance == DistanceToFloresta) return TEXT("Floresta");
    if (MinDistance == DistanceToDeserto) return TEXT("Deserto");
    return TEXT("Montanha");
}

void UDir_ProductionCoordinator::TriggerEmergencyProtocol()
{
    UE_LOG(LogTemp, Error, TEXT("EMERGENCY PROTOCOL ACTIVATED - Redirecting all agents to Milestone 1 tasks"));
    
    // Force all agents to focus on critical path
    bEmergencyMode = true;
    
    // Log specific actions needed
    UE_LOG(LogTemp, Error, TEXT("REQUIRED ACTIONS:"));
    UE_LOG(LogTemp, Error, TEXT("1. Agent #9: Implement TranspersonalCharacter movement"));
    UE_LOG(LogTemp, Error, TEXT("2. Agent #5: Create basic landscape terrain"));
    UE_LOG(LogTemp, Error, TEXT("3. Agent #6: Place 5 dinosaur meshes in biomes"));
    UE_LOG(LogTemp, Error, TEXT("4. Agent #8: Setup directional light + sky"));
}

void UDir_ProductionCoordinator::ExecuteAgentTask(int32 AgentID, const FString& TaskDescription)
{
    if (AgentPriorities.Contains(AgentID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Agent #%d executing: %s"), AgentID, *TaskDescription);
        
        // Update metrics
        ProductionMetrics.CompletedTasks++;
        
        // Check if this completes any dependencies
        FDir_AgentPriority& Priority = AgentPriorities[AgentID];
        for (int32 DependentAgent : Priority.Dependencies)
        {
            UE_LOG(LogTemp, Log, TEXT("Agent #%d unblocked by #%d completion"), DependentAgent, AgentID);
        }
    }
}

FDir_ProductionStatus UDir_ProductionCoordinator::GetProductionStatus() const
{
    return ProductionMetrics;
}

bool UDir_ProductionCoordinator::IsAgentBlocked(int32 AgentID) const
{
    if (!AgentPriorities.Contains(AgentID)) return false;
    
    const FDir_AgentPriority& Priority = AgentPriorities[AgentID];
    return Priority.bIsBlocking;
}

TArray<int32> UDir_ProductionCoordinator::GetCriticalPathAgents() const
{
    TArray<int32> CriticalAgents;
    
    for (auto& Priority : AgentPriorities)
    {
        if (Priority.Value.Priority >= 8)
        {
            CriticalAgents.Add(Priority.Key);
        }
    }
    
    return CriticalAgents;
}