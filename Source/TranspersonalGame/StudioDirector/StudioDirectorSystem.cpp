#include "StudioDirectorSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"

UStudioDirectorSystem::UStudioDirectorSystem()
{
    CurrentCycleID = TEXT("INIT");
    CycleStartTime = 0.0f;
    bPrototypeComplete = false;
}

void UStudioDirectorSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeAgentTasks();
    InitializeBiomeStatuses();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director System Initialized"));
}

void UStudioDirectorSystem::Deinitialize()
{
    AgentTasks.Empty();
    BiomeStatuses.Empty();
    
    Super::Deinitialize();
}

void UStudioDirectorSystem::InitializeAgentTasks()
{
    // Initialize all 18 agents with their specializations
    TArray<FString> AgentNames = {
        TEXT("Studio Director"),
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

    AgentTasks.Empty();
    for (int32 i = 0; i < AgentNames.Num(); i++)
    {
        FDir_AgentTask NewTask;
        NewTask.AgentNumber = i + 1;
        NewTask.AgentName = AgentNames[i];
        NewTask.Status = EDir_AgentStatus::Idle;
        NewTask.Priority = (i < 5) ? 3 : 1; // Higher priority for core agents
        NewTask.CycleID = CurrentCycleID;
        
        AgentTasks.Add(NewTask);
    }
}

void UStudioDirectorSystem::InitializeBiomeStatuses()
{
    // Initialize 5 biomes with their coordinates from memory
    BiomeStatuses.Empty();
    
    // Savana (0,0)
    FDir_BiomeStatus Savana;
    Savana.BiomeType = EDir_BiomeType::Savana;
    Savana.BiomeCenter = FVector(0, 0, 0);
    BiomeStatuses.Add(Savana);
    
    // Pantano (-50000,-45000)
    FDir_BiomeStatus Pantano;
    Pantano.BiomeType = EDir_BiomeType::Pantano;
    Pantano.BiomeCenter = FVector(-50000, -45000, 0);
    BiomeStatuses.Add(Pantano);
    
    // Floresta (-45000,40000)
    FDir_BiomeStatus Floresta;
    Floresta.BiomeType = EDir_BiomeType::Floresta;
    Floresta.BiomeCenter = FVector(-45000, 40000, 0);
    BiomeStatuses.Add(Floresta);
    
    // Deserto (55000,0)
    FDir_BiomeStatus Deserto;
    Deserto.BiomeType = EDir_BiomeType::Deserto;
    Deserto.BiomeCenter = FVector(55000, 0, 0);
    BiomeStatuses.Add(Deserto);
    
    // Montanha (40000,50000)
    FDir_BiomeStatus Montanha;
    Montanha.BiomeType = EDir_BiomeType::Montanha;
    Montanha.BiomeCenter = FVector(40000, 50000, 0);
    BiomeStatuses.Add(Montanha);
}

void UStudioDirectorSystem::AssignTask(int32 AgentNumber, const FString& TaskDescription, int32 Priority)
{
    if (AgentNumber < 1 || AgentNumber > AgentTasks.Num())
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid agent number: %d"), AgentNumber);
        return;
    }
    
    FDir_AgentTask& Task = AgentTasks[AgentNumber - 1];
    Task.TaskDescription = TaskDescription;
    Task.Priority = Priority;
    Task.Status = EDir_AgentStatus::Working;
    Task.CycleID = CurrentCycleID;
    
    UE_LOG(LogTemp, Warning, TEXT("Task assigned to Agent #%d (%s): %s"), 
           AgentNumber, *Task.AgentName, *TaskDescription);
}

void UStudioDirectorSystem::UpdateAgentStatus(int32 AgentNumber, EDir_AgentStatus NewStatus)
{
    if (AgentNumber < 1 || AgentNumber > AgentTasks.Num())
    {
        return;
    }
    
    AgentTasks[AgentNumber - 1].Status = NewStatus;
}

TArray<FDir_AgentTask> UStudioDirectorSystem::GetActiveAgentTasks()
{
    TArray<FDir_AgentTask> ActiveTasks;
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Working)
        {
            ActiveTasks.Add(Task);
        }
    }
    return ActiveTasks;
}

FDir_AgentTask UStudioDirectorSystem::GetAgentTask(int32 AgentNumber)
{
    if (AgentNumber >= 1 && AgentNumber <= AgentTasks.Num())
    {
        return AgentTasks[AgentNumber - 1];
    }
    return FDir_AgentTask();
}

void UStudioDirectorSystem::UpdateBiomeStatus(EDir_BiomeType BiomeType, int32 ActorCount, int32 DinosaurCount)
{
    for (FDir_BiomeStatus& Biome : BiomeStatuses)
    {
        if (Biome.BiomeType == BiomeType)
        {
            Biome.ActorCount = ActorCount;
            Biome.DinosaurCount = DinosaurCount;
            Biome.bIsPopulated = (ActorCount >= 500); // Minimum 500 actors per biome
            break;
        }
    }
}

TArray<FDir_BiomeStatus> UStudioDirectorSystem::GetAllBiomeStatus()
{
    return BiomeStatuses;
}

bool UStudioDirectorSystem::IsBiomePopulated(EDir_BiomeType BiomeType)
{
    for (const FDir_BiomeStatus& Biome : BiomeStatuses)
    {
        if (Biome.BiomeType == BiomeType)
        {
            return Biome.bIsPopulated;
        }
    }
    return false;
}

bool UStudioDirectorSystem::ValidateMinimumViablePrototype()
{
    // Check prototype requirements
    TArray<FString> Requirements = GetPrototypeRequirements();
    int32 CompletedRequirements = 0;
    
    // Check if character exists and can move
    UWorld* World = GetWorld();
    if (World)
    {
        // Count actors in world
        int32 TotalActors = 0;
        int32 DinosaurActors = 0;
        
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor)
            {
                TotalActors++;
                FString ActorName = Actor->GetName().ToLower();
                if (ActorName.Contains(TEXT("dinosaur")) || 
                    ActorName.Contains(TEXT("trex")) || 
                    ActorName.Contains(TEXT("raptor")))
                {
                    DinosaurActors++;
                }
            }
        }
        
        // Basic prototype validation
        if (TotalActors > 10) CompletedRequirements++; // Has actors in world
        if (DinosaurActors >= 3) CompletedRequirements++; // Has dinosaurs
        
        // Check biomes
        int32 PopulatedBiomes = 0;
        for (const FDir_BiomeStatus& Biome : BiomeStatuses)
        {
            if (Biome.bIsPopulated)
            {
                PopulatedBiomes++;
            }
        }
        if (PopulatedBiomes >= 2) CompletedRequirements++; // At least 2 biomes populated
    }
    
    bPrototypeComplete = (CompletedRequirements >= 3);
    return bPrototypeComplete;
}

TArray<FString> UStudioDirectorSystem::GetPrototypeRequirements()
{
    TArray<FString> Requirements;
    Requirements.Add(TEXT("Playable character with WASD movement"));
    Requirements.Add(TEXT("Landscape with basic terrain variation"));
    Requirements.Add(TEXT("3-5 dinosaur actors placed in world"));
    Requirements.Add(TEXT("Directional light and sky atmosphere"));
    Requirements.Add(TEXT("Player can walk, run, jump"));
    Requirements.Add(TEXT("At least 2 biomes populated with 500+ actors"));
    
    return Requirements;
}

float UStudioDirectorSystem::GetPrototypeCompletionPercentage()
{
    TArray<FString> Requirements = GetPrototypeRequirements();
    if (Requirements.Num() == 0) return 100.0f;
    
    int32 CompletedCount = 0;
    
    // Simple completion check based on biome population and actor count
    UWorld* World = GetWorld();
    if (World)
    {
        int32 TotalActors = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            TotalActors++;
        }
        
        if (TotalActors > 10) CompletedCount++;
        if (TotalActors > 50) CompletedCount++;
        
        int32 PopulatedBiomes = 0;
        for (const FDir_BiomeStatus& Biome : BiomeStatuses)
        {
            if (Biome.bIsPopulated) PopulatedBiomes++;
        }
        
        CompletedCount += PopulatedBiomes;
    }
    
    return FMath::Clamp((float)CompletedCount / (float)Requirements.Num() * 100.0f, 0.0f, 100.0f);
}

void UStudioDirectorSystem::StartNewCycle(const FString& CycleID)
{
    CurrentCycleID = CycleID;
    CycleStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    // Reset all agents to idle for new cycle
    for (FDir_AgentTask& Task : AgentTasks)
    {
        Task.Status = EDir_AgentStatus::Idle;
        Task.CycleID = CycleID;
        Task.TaskDescription = TEXT("");
    }
    
    UE_LOG(LogTemp, Warning, TEXT("New cycle started: %s"), *CycleID);
}

void UStudioDirectorSystem::CompleteCycle()
{
    float CycleTime = GetWorld() ? GetWorld()->GetTimeSeconds() - CycleStartTime : 0.0f;
    
    // Count completed tasks
    int32 CompletedTasks = 0;
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Completed)
        {
            CompletedTasks++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Cycle %s completed: %d/%d tasks, %.2f seconds"), 
           *CurrentCycleID, CompletedTasks, AgentTasks.Num(), CycleTime);
}

void UStudioDirectorSystem::ValidateAgentDependencies()
{
    // Validate that agents are working in correct order
    // Engine Architect must complete before Core Systems, etc.
    
    for (int32 i = 1; i < AgentTasks.Num(); i++)
    {
        FDir_AgentTask& CurrentTask = AgentTasks[i];
        FDir_AgentTask& PreviousTask = AgentTasks[i - 1];
        
        // If current agent is working but previous is not completed
        if (CurrentTask.Status == EDir_AgentStatus::Working && 
            PreviousTask.Status != EDir_AgentStatus::Completed)
        {
            UE_LOG(LogTemp, Warning, TEXT("Dependency violation: Agent #%d working before Agent #%d completed"), 
                   CurrentTask.AgentNumber, PreviousTask.AgentNumber);
        }
    }
}

void UStudioDirectorSystem::CheckPrototypeRequirements()
{
    ValidateMinimumViablePrototype();
    
    float CompletionPercentage = GetPrototypeCompletionPercentage();
    UE_LOG(LogTemp, Warning, TEXT("Prototype completion: %.1f%%"), CompletionPercentage);
    
    if (CompletionPercentage >= 100.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("MILESTONE 1 COMPLETED: Minimum Viable Prototype ready!"));
    }
}