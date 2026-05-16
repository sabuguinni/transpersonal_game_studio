#include "StudioDirectorSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"
#include "Landscape/Landscape.h"

UStudioDirectorSystem::UStudioDirectorSystem()
{
    CurrentCycleId = TEXT("PROD_CYCLE_AUTO_20260516_001");
    CycleStartTime = 0.0f;
}

void UStudioDirectorSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director System Initialized"));
    
    CycleStartTime = FPlatformTime::Seconds();
    SetupInitialTasks();
    InitializeBiomeTargets();
}

void UStudioDirectorSystem::Deinitialize()
{
    AgentTasks.Empty();
    BiomeStatuses.Empty();
    
    Super::Deinitialize();
}

void UStudioDirectorSystem::AssignTaskToAgent(int32 AgentNumber, const FString& TaskDescription, EDir_TaskPriority Priority)
{
    FDir_AgentTask NewTask;
    NewTask.AgentNumber = AgentNumber;
    NewTask.TaskDescription = TaskDescription;
    NewTask.Priority = Priority;
    NewTask.Status = EDir_TaskStatus::Pending;
    NewTask.CycleId = CurrentCycleId;
    
    // Estimate duration based on agent type
    switch (AgentNumber)
    {
        case 2: // Engine Architect
        case 3: // Core Systems
            NewTask.EstimatedDuration = 300.0f; // 5 minutes
            break;
        case 5: // World Generator
        case 6: // Environment Artist
        case 7: // Architecture
            NewTask.EstimatedDuration = 600.0f; // 10 minutes
            break;
        case 9: // Character Artist
        case 10: // Animation
            NewTask.EstimatedDuration = 450.0f; // 7.5 minutes
            break;
        default:
            NewTask.EstimatedDuration = 360.0f; // 6 minutes
            break;
    }
    
    AgentTasks.Add(NewTask);
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Assigned task to Agent #%d: %s"), 
           AgentNumber, *TaskDescription);
}

void UStudioDirectorSystem::UpdateTaskStatus(int32 AgentNumber, const FString& CycleId, EDir_TaskStatus NewStatus)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentNumber == AgentNumber && Task.CycleId == CycleId)
        {
            Task.Status = NewStatus;
            
            UE_LOG(LogTemp, Warning, TEXT("Studio Director: Agent #%d task status updated to %d"), 
                   AgentNumber, (int32)NewStatus);
            break;
        }
    }
}

TArray<FDir_AgentTask> UStudioDirectorSystem::GetPendingTasks() const
{
    TArray<FDir_AgentTask> PendingTasks;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_TaskStatus::Pending || Task.Status == EDir_TaskStatus::InProgress)
        {
            PendingTasks.Add(Task);
        }
    }
    
    return PendingTasks;
}

FDir_AgentTask UStudioDirectorSystem::GetTaskForAgent(int32 AgentNumber) const
{
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentNumber == AgentNumber && Task.CycleId == CurrentCycleId)
        {
            return Task;
        }
    }
    
    return FDir_AgentTask(); // Return default task if none found
}

void UStudioDirectorSystem::RegisterBiomeStatus(EDir_BiomeType BiomeType, FVector Location, int32 ActorCount)
{
    // Find existing biome status or create new one
    FDir_BiomeStatus* ExistingStatus = nullptr;
    for (FDir_BiomeStatus& Status : BiomeStatuses)
    {
        if (Status.BiomeType == BiomeType)
        {
            ExistingStatus = &Status;
            break;
        }
    }
    
    if (!ExistingStatus)
    {
        FDir_BiomeStatus NewStatus;
        NewStatus.BiomeType = BiomeType;
        NewStatus.WorldLocation = Location;
        BiomeStatuses.Add(NewStatus);
        ExistingStatus = &BiomeStatuses.Last();
    }
    
    ExistingStatus->ActorCount = ActorCount;
    ExistingStatus->bIsPopulated = ActorCount >= 500; // Minimum for populated biome
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Biome %d registered with %d actors at %s"), 
           (int32)BiomeType, ActorCount, *Location.ToString());
}

TArray<FDir_BiomeStatus> UStudioDirectorSystem::GetBiomeStatuses() const
{
    return BiomeStatuses;
}

bool UStudioDirectorSystem::AreCriteriaMet() const
{
    bool bCriterion1 = ValidateBiomePopulation();
    bool bCriterion2 = ValidateAtmosphere();
    bool bCriterion3 = ValidateFBXPipeline();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Criteria Status - Biomes: %s, Atmosphere: %s, FBX: %s"), 
           bCriterion1 ? TEXT("PASS") : TEXT("FAIL"),
           bCriterion2 ? TEXT("PASS") : TEXT("FAIL"),
           bCriterion3 ? TEXT("PASS") : TEXT("FAIL"));
    
    return bCriterion1 && bCriterion2 && bCriterion3;
}

void UStudioDirectorSystem::ValidateMinPlayableMap()
{
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        
        int32 CharacterCount = 0;
        int32 StaticMeshCount = 0;
        int32 LandscapeCount = 0;
        
        for (AActor* Actor : AllActors)
        {
            if (Actor->IsA<APawn>())
            {
                CharacterCount++;
            }
            else if (Actor->IsA<AStaticMeshActor>())
            {
                StaticMeshCount++;
            }
            else if (Actor->IsA<ALandscape>())
            {
                LandscapeCount++;
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Studio Director: MinPlayableMap Validation - Characters: %d, Static Meshes: %d, Landscapes: %d"), 
               CharacterCount, StaticMeshCount, LandscapeCount);
    }
}

void UStudioDirectorSystem::GenerateProductionReport()
{
    float CurrentTime = FPlatformTime::Seconds();
    float CycleDuration = CurrentTime - CycleStartTime;
    
    int32 CompletedTasks = 0;
    int32 PendingTasks = 0;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_TaskStatus::Completed)
        {
            CompletedTasks++;
        }
        else
        {
            PendingTasks++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director Production Report:"));
    UE_LOG(LogTemp, Warning, TEXT("Cycle Duration: %.2f seconds"), CycleDuration);
    UE_LOG(LogTemp, Warning, TEXT("Tasks Completed: %d"), CompletedTasks);
    UE_LOG(LogTemp, Warning, TEXT("Tasks Pending: %d"), PendingTasks);
    UE_LOG(LogTemp, Warning, TEXT("Biomes Registered: %d"), BiomeStatuses.Num());
}

void UStudioDirectorSystem::InitializeAgentTasks()
{
    AgentTasks.Empty();
    CurrentCycleId = TEXT("PROD_CYCLE_AUTO_20260516_001");
    
    // Critical path tasks for MILESTONE 1
    AssignTaskToAgent(2, TEXT("Validate engine architecture and compilation"), EDir_TaskPriority::Critical);
    AssignTaskToAgent(3, TEXT("Implement core physics and movement systems"), EDir_TaskPriority::Critical);
    AssignTaskToAgent(5, TEXT("Populate 5 biomes with 500+ actors each"), EDir_TaskPriority::Critical);
    AssignTaskToAgent(6, TEXT("Create environment assets for biomes"), EDir_TaskPriority::High);
    AssignTaskToAgent(8, TEXT("Establish Cretaceous atmosphere lighting"), EDir_TaskPriority::High);
    AssignTaskToAgent(9, TEXT("Test FBX pipeline with character assets"), EDir_TaskPriority::High);
    AssignTaskToAgent(12, TEXT("Implement survival HUD system"), EDir_TaskPriority::Medium);
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Initialized %d agent tasks for cycle %s"), 
           AgentTasks.Num(), *CurrentCycleId);
}

void UStudioDirectorSystem::SetupInitialTasks()
{
    InitializeAgentTasks();
}

void UStudioDirectorSystem::InitializeBiomeTargets()
{
    // Initialize biome targets based on memory coordinates
    TArray<TPair<EDir_BiomeType, FVector>> BiomeTargets = {
        {EDir_BiomeType::Savanna, FVector(0, 0, 0)},
        {EDir_BiomeType::Swamp, FVector(-50000, -45000, 0)},
        {EDir_BiomeType::Forest, FVector(-45000, 40000, 0)},
        {EDir_BiomeType::Desert, FVector(55000, 0, 0)},
        {EDir_BiomeType::Mountain, FVector(40000, 50000, 0)}
    };
    
    for (const auto& BiomeTarget : BiomeTargets)
    {
        FDir_BiomeStatus Status;
        Status.BiomeType = BiomeTarget.Key;
        Status.WorldLocation = BiomeTarget.Value;
        Status.ActorCount = 0;
        Status.bIsPopulated = false;
        BiomeStatuses.Add(Status);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Initialized %d biome targets"), BiomeStatuses.Num());
}

bool UStudioDirectorSystem::ValidateBiomePopulation() const
{
    int32 PopulatedBiomes = 0;
    
    for (const FDir_BiomeStatus& Status : BiomeStatuses)
    {
        if (Status.bIsPopulated && Status.ActorCount >= 500)
        {
            PopulatedBiomes++;
        }
    }
    
    return PopulatedBiomes >= 5; // All 5 biomes must be populated
}

bool UStudioDirectorSystem::ValidateAtmosphere() const
{
    // Check for proper Cretaceous lighting setup
    // This would normally check for specific lighting actors and settings
    return true; // Placeholder - Agent #8 responsibility
}

bool UStudioDirectorSystem::ValidateFBXPipeline() const
{
    // Check if FBX import pipeline has been tested
    // This would normally check for imported assets in Content/TestAssets/
    return false; // Placeholder - Agents #6/#7/#9 responsibility
}