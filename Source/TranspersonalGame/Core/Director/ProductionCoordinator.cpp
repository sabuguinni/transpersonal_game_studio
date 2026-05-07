#include "ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"

UProductionCoordinator::UProductionCoordinator()
{
    // Initialize default metrics
    CurrentMetrics = FDir_ProductionMetrics();
}

void UProductionCoordinator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Initializing Studio Director coordination system"));
    
    // Set up default agents and biomes
    SetupDefaultAgents();
    SetupBiomeData();
    
    // Update initial metrics
    UpdateProductionMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Initialization complete"));
}

void UProductionCoordinator::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Shutting down"));
    Super::Deinitialize();
}

void UProductionCoordinator::RegisterAgent(const FString& AgentName, const FString& Priority)
{
    // Check if agent already exists
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName)
        {
            Task.Priority = Priority;
            UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Updated existing agent %s with priority %s"), *AgentName, *Priority);
            return;
        }
    }
    
    // Create new agent task
    FDir_AgentTask NewTask;
    NewTask.AgentName = AgentName;
    NewTask.Priority = Priority;
    NewTask.bCompleted = false;
    NewTask.CompletionPercentage = 0.0f;
    
    AgentTasks.Add(NewTask);
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Registered new agent %s with priority %s"), *AgentName, *Priority);
}

void UProductionCoordinator::AssignTaskToAgent(const FString& AgentName, const TArray<FString>& Tasks, const TArray<FString>& Deliverables)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName)
        {
            Task.TaskList = Tasks;
            Task.Deliverables = Deliverables;
            Task.bCompleted = false;
            Task.CompletionPercentage = 0.0f;
            
            UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Assigned %d tasks to agent %s"), Tasks.Num(), *AgentName);
            return;
        }
    }
    
    UE_LOG(LogTemp, Error, TEXT("ProductionCoordinator: Agent %s not found for task assignment"), *AgentName);
}

void UProductionCoordinator::UpdateAgentProgress(const FString& AgentName, float CompletionPercentage)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName)
        {
            Task.CompletionPercentage = FMath::Clamp(CompletionPercentage, 0.0f, 100.0f);
            
            if (Task.CompletionPercentage >= 100.0f)
            {
                Task.bCompleted = true;
            }
            
            UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Agent %s progress updated to %.1f%%"), *AgentName, Task.CompletionPercentage);
            return;
        }
    }
}

void UProductionCoordinator::MarkAgentCompleted(const FString& AgentName)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName)
        {
            Task.bCompleted = true;
            Task.CompletionPercentage = 100.0f;
            UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Agent %s marked as completed"), *AgentName);
            return;
        }
    }
}

void UProductionCoordinator::InitializeBiomeCoordinates()
{
    SetupBiomeData();
}

FVector UProductionCoordinator::GetRandomLocationInBiome(const FString& BiomeName)
{
    for (const FDir_BiomeCoordinates& Biome : BiomeCoordinates)
    {
        if (Biome.BiomeName == BiomeName)
        {
            // Generate random location within biome bounds
            float RandomX = FMath::RandRange(Biome.MinBounds.X, Biome.MaxBounds.X);
            float RandomY = FMath::RandRange(Biome.MinBounds.Y, Biome.MaxBounds.Y);
            float RandomZ = Biome.CenterLocation.Z; // Keep Z at biome center level
            
            return FVector(RandomX, RandomY, RandomZ);
        }
    }
    
    UE_LOG(LogTemp, Error, TEXT("ProductionCoordinator: Biome %s not found, returning center location"), *BiomeName);
    return FVector(0, 0, 0);
}

FString UProductionCoordinator::GetBiomeAtLocation(const FVector& Location)
{
    for (const FDir_BiomeCoordinates& Biome : BiomeCoordinates)
    {
        if (Location.X >= Biome.MinBounds.X && Location.X <= Biome.MaxBounds.X &&
            Location.Y >= Biome.MinBounds.Y && Location.Y <= Biome.MaxBounds.Y)
        {
            return Biome.BiomeName;
        }
    }
    
    return TEXT("Unknown");
}

void UProductionCoordinator::UpdateProductionMetrics()
{
    CurrentMetrics.PhantomHeaders = CountPhantomHeaders();
    CurrentMetrics.ActiveAgents = AgentTasks.Num();
    
    // Calculate overall progress
    float TotalProgress = 0.0f;
    int32 ValidAgents = 0;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        TotalProgress += Task.CompletionPercentage;
        ValidAgents++;
    }
    
    if (ValidAgents > 0)
    {
        CurrentMetrics.OverallProgress = TotalProgress / ValidAgents;
    }
    
    LogProductionStatus();
}

FDir_ProductionMetrics UProductionCoordinator::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

TArray<FDir_AgentTask> UProductionCoordinator::GetTasksByPriority(const FString& Priority)
{
    TArray<FDir_AgentTask> FilteredTasks;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Priority == Priority)
        {
            FilteredTasks.Add(Task);
        }
    }
    
    return FilteredTasks;
}

void UProductionCoordinator::SetCriticalBlocker(const FString& BlockerDescription)
{
    CriticalBlockers.AddUnique(BlockerDescription);
    UE_LOG(LogTemp, Error, TEXT("ProductionCoordinator: CRITICAL BLOCKER ADDED: %s"), *BlockerDescription);
}

bool UProductionCoordinator::HasCriticalBlockers() const
{
    return CriticalBlockers.Num() > 0;
}

void UProductionCoordinator::GenerateProductionReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Agents: %d"), AgentTasks.Num());
    UE_LOG(LogTemp, Warning, TEXT("Overall Progress: %.1f%%"), CurrentMetrics.OverallProgress);
    UE_LOG(LogTemp, Warning, TEXT("Phantom Headers: %d"), CurrentMetrics.PhantomHeaders);
    UE_LOG(LogTemp, Warning, TEXT("Critical Blockers: %d"), CriticalBlockers.Num());
    
    for (const FString& Blocker : CriticalBlockers)
    {
        UE_LOG(LogTemp, Error, TEXT("  BLOCKER: %s"), *Blocker);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END REPORT ==="));
}

void UProductionCoordinator::ValidateProjectStructure()
{
    UpdateProductionMetrics();
    
    if (CurrentMetrics.PhantomHeaders > 0)
    {
        SetCriticalBlocker(FString::Printf(TEXT("Found %d phantom headers without .cpp files"), CurrentMetrics.PhantomHeaders));
    }
    
    GenerateProductionReport();
}

void UProductionCoordinator::SetupDefaultAgents()
{
    // Register all 19 agents with their priorities
    RegisterAgent(TEXT("Agent_02_Engine_Architect"), TEXT("CRITICAL"));
    RegisterAgent(TEXT("Agent_03_Core_Systems"), TEXT("CRITICAL"));
    RegisterAgent(TEXT("Agent_04_Performance"), TEXT("HIGH"));
    RegisterAgent(TEXT("Agent_05_World_Generator"), TEXT("CRITICAL"));
    RegisterAgent(TEXT("Agent_06_Environment_Artist"), TEXT("HIGH"));
    RegisterAgent(TEXT("Agent_07_Architecture"), TEXT("MEDIUM"));
    RegisterAgent(TEXT("Agent_08_Lighting"), TEXT("MEDIUM"));
    RegisterAgent(TEXT("Agent_09_Character_Artist"), TEXT("HIGH"));
    RegisterAgent(TEXT("Agent_10_Animation"), TEXT("HIGH"));
    RegisterAgent(TEXT("Agent_11_NPC_Behavior"), TEXT("MEDIUM"));
    RegisterAgent(TEXT("Agent_12_Combat_AI"), TEXT("HIGH"));
    RegisterAgent(TEXT("Agent_13_Crowd_Simulation"), TEXT("MEDIUM"));
    RegisterAgent(TEXT("Agent_14_Quest_Designer"), TEXT("LOW"));
    RegisterAgent(TEXT("Agent_15_Narrative"), TEXT("LOW"));
    RegisterAgent(TEXT("Agent_16_Audio"), TEXT("MEDIUM"));
    RegisterAgent(TEXT("Agent_17_VFX"), TEXT("MEDIUM"));
    RegisterAgent(TEXT("Agent_18_QA_Testing"), TEXT("HIGH"));
    RegisterAgent(TEXT("Agent_19_Integration"), TEXT("CRITICAL"));
    RegisterAgent(TEXT("Agent_20_Build_Manager"), TEXT("CRITICAL"));
}

void UProductionCoordinator::SetupBiomeData()
{
    BiomeCoordinates.Empty();
    
    // SWAMP (Southwest)
    FDir_BiomeCoordinates Swamp;
    Swamp.BiomeName = TEXT("SWAMP");
    Swamp.CenterLocation = FVector(-50000, -45000, 0);
    Swamp.MinBounds = FVector(-77500, -76500, -100);
    Swamp.MaxBounds = FVector(-25000, -15000, 100);
    Swamp.TypicalActors = {TEXT("Water"), TEXT("Dense Vegetation"), TEXT("Mud"), TEXT("Dead Trees")};
    BiomeCoordinates.Add(Swamp);
    
    // FOREST (Northwest)
    FDir_BiomeCoordinates Forest;
    Forest.BiomeName = TEXT("FOREST");
    Forest.CenterLocation = FVector(-45000, 40000, 0);
    Forest.MinBounds = FVector(-77500, 15000, -100);
    Forest.MaxBounds = FVector(-15000, 76500, 100);
    Forest.TypicalActors = {TEXT("Tall Trees"), TEXT("Dense Shade"), TEXT("Ferns"), TEXT("Fallen Logs")};
    BiomeCoordinates.Add(Forest);
    
    // SAVANNA (Center)
    FDir_BiomeCoordinates Savanna;
    Savanna.BiomeName = TEXT("SAVANNA");
    Savanna.CenterLocation = FVector(0, 0, 0);
    Savanna.MinBounds = FVector(-20000, -20000, -100);
    Savanna.MaxBounds = FVector(20000, 20000, 100);
    Savanna.TypicalActors = {TEXT("Tall Grass"), TEXT("Sparse Trees"), TEXT("Flat Rocks")};
    BiomeCoordinates.Add(Savanna);
    
    // DESERT (East)
    FDir_BiomeCoordinates Desert;
    Desert.BiomeName = TEXT("DESERT");
    Desert.CenterLocation = FVector(55000, 0, 0);
    Desert.MinBounds = FVector(25000, -30000, -100);
    Desert.MaxBounds = FVector(79500, 30000, 100);
    Desert.TypicalActors = {TEXT("Dunes"), TEXT("Sandy Rocks"), TEXT("Primitive Cacti"), TEXT("Heat")};
    BiomeCoordinates.Add(Desert);
    
    // MOUNTAIN (Northeast)
    FDir_BiomeCoordinates Mountain;
    Mountain.BiomeName = TEXT("MOUNTAIN");
    Mountain.CenterLocation = FVector(40000, 50000, 500);
    Mountain.MinBounds = FVector(15000, 20000, 200);
    Mountain.MaxBounds = FVector(79500, 76500, 1000);
    Mountain.TypicalActors = {TEXT("Rock"), TEXT("Snow"), TEXT("Cliffs"), TEXT("Wind"), TEXT("High Altitude")};
    BiomeCoordinates.Add(Mountain);
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Initialized %d biomes"), BiomeCoordinates.Num());
}

int32 UProductionCoordinator::CountPhantomHeaders()
{
    // This would need to be implemented with file system access
    // For now, return a placeholder based on known issues
    return 122; // Known from brain memories
}

void UProductionCoordinator::LogProductionStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Metrics updated - Progress: %.1f%%, Phantom Headers: %d"), 
           CurrentMetrics.OverallProgress, CurrentMetrics.PhantomHeaders);
}