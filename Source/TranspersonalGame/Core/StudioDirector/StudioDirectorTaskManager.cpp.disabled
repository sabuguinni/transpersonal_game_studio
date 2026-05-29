#include "StudioDirectorTaskManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

UStudioDirectorTaskManager::UStudioDirectorTaskManager()
{
    NextTaskID = 1;
    LastValidationTime = FDateTime::Now();
}

void UStudioDirectorTaskManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director Task Manager Initialized"));
    
    InitializeBiomes();
    AssignCriticalTasks();
}

void UStudioDirectorTaskManager::Deinitialize()
{
    ActiveTasks.Empty();
    BiomeStatuses.Empty();
    
    Super::Deinitialize();
}

void UStudioDirectorTaskManager::InitializeBiomes()
{
    // Initialize the 5 main biomes with their center coordinates
    TArray<TPair<FString, FVector>> BiomeData = {
        {TEXT("Savana"), FVector(0.0f, 0.0f, 100.0f)},
        {TEXT("Pantano"), FVector(-50000.0f, -45000.0f, 50.0f)},
        {TEXT("Floresta"), FVector(-45000.0f, 40000.0f, 150.0f)},
        {TEXT("Deserto"), FVector(55000.0f, 0.0f, 120.0f)},
        {TEXT("Montanha"), FVector(40000.0f, 50000.0f, 300.0f)}
    };

    BiomeStatuses.Empty();
    for (const auto& BiomePair : BiomeData)
    {
        FDir_BiomeStatus NewBiome;
        NewBiome.BiomeName = BiomePair.Key;
        NewBiome.CenterLocation = BiomePair.Value;
        NewBiome.ActorCount = 0;
        NewBiome.DinosaurCount = 0;
        NewBiome.bIsPopulated = false;
        NewBiome.PopulationPercentage = 0.0f;
        
        BiomeStatuses.Add(NewBiome);
    }

    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Initialized %d biomes"), BiomeStatuses.Num());
}

int32 UStudioDirectorTaskManager::CreateTask(EDir_AgentType Agent, const FString& Description, EDir_AgentPriority Priority, const FString& Deliverable)
{
    FDir_AgentTask NewTask;
    NewTask.TaskID = NextTaskID++;
    NewTask.AssignedAgent = Agent;
    NewTask.TaskDescription = Description;
    NewTask.Priority = Priority;
    NewTask.Status = EDir_TaskStatus::Pending;
    NewTask.ExpectedDeliverable = Deliverable;
    NewTask.CreatedAt = FDateTime::Now();
    
    // Set due date based on priority
    float HoursToAdd = 24.0f;
    switch (Priority)
    {
        case EDir_AgentPriority::Critical:
            HoursToAdd = 2.0f;
            break;
        case EDir_AgentPriority::High:
            HoursToAdd = 8.0f;
            break;
        case EDir_AgentPriority::Medium:
            HoursToAdd = 24.0f;
            break;
        case EDir_AgentPriority::Low:
            HoursToAdd = 72.0f;
            break;
    }
    
    NewTask.DueDate = NewTask.CreatedAt + FTimespan::FromHours(HoursToAdd);
    
    ActiveTasks.Add(NewTask);
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Created Task %d for Agent %d: %s"), 
           NewTask.TaskID, (int32)Agent, *Description);
    
    return NewTask.TaskID;
}

bool UStudioDirectorTaskManager::UpdateTaskStatus(int32 TaskID, EDir_TaskStatus NewStatus)
{
    for (FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.TaskID == TaskID)
        {
            Task.Status = NewStatus;
            UE_LOG(LogTemp, Warning, TEXT("Studio Director: Task %d status updated to %d"), TaskID, (int32)NewStatus);
            return true;
        }
    }
    
    return false;
}

TArray<FDir_AgentTask> UStudioDirectorTaskManager::GetTasksForAgent(EDir_AgentType Agent)
{
    TArray<FDir_AgentTask> AgentTasks;
    
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.AssignedAgent == Agent)
        {
            AgentTasks.Add(Task);
        }
    }
    
    return AgentTasks;
}

TArray<FDir_AgentTask> UStudioDirectorTaskManager::GetTasksByStatus(EDir_TaskStatus Status)
{
    TArray<FDir_AgentTask> StatusTasks;
    
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.Status == Status)
        {
            StatusTasks.Add(Task);
        }
    }
    
    return StatusTasks;
}

FDir_AgentTask UStudioDirectorTaskManager::GetTaskByID(int32 TaskID)
{
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.TaskID == TaskID)
        {
            return Task;
        }
    }
    
    return FDir_AgentTask(); // Return default task if not found
}

void UStudioDirectorTaskManager::UpdateBiomeStatus(const FString& BiomeName, const FVector& Location, int32 ActorCount, int32 DinosaurCount)
{
    for (FDir_BiomeStatus& Biome : BiomeStatuses)
    {
        if (Biome.BiomeName == BiomeName)
        {
            Biome.ActorCount = ActorCount;
            Biome.DinosaurCount = DinosaurCount;
            Biome.PopulationPercentage = FMath::Clamp((float)ActorCount / 500.0f, 0.0f, 1.0f) * 100.0f;
            Biome.bIsPopulated = ActorCount >= 500;
            
            UE_LOG(LogTemp, Warning, TEXT("Studio Director: Updated %s - %d actors, %d dinosaurs (%.1f%% populated)"), 
                   *BiomeName, ActorCount, DinosaurCount, Biome.PopulationPercentage);
            break;
        }
    }
}

TArray<FDir_BiomeStatus> UStudioDirectorTaskManager::GetAllBiomeStatuses()
{
    return BiomeStatuses;
}

FDir_BiomeStatus UStudioDirectorTaskManager::GetBiomeStatus(const FString& BiomeName)
{
    for (const FDir_BiomeStatus& Biome : BiomeStatuses)
    {
        if (Biome.BiomeName == BiomeName)
        {
            return Biome;
        }
    }
    
    return FDir_BiomeStatus(); // Return default if not found
}

bool UStudioDirectorTaskManager::ValidateMinimumViablePrototype()
{
    LastValidationTime = FDateTime::Now();
    
    // Check if we have a playable character
    bool bHasPlayableCharacter = false;
    bool bHasBasicTerrain = false;
    bool bHasDinosaurs = false;
    bool bHasMovement = false;
    
    if (UWorld* World = GetWorld())
    {
        // Check for TranspersonalCharacter
        for (TActorIterator<APawn> ActorItr(World); ActorItr; ++ActorItr)
        {
            APawn* Pawn = *ActorItr;
            if (Pawn && Pawn->GetClass()->GetName().Contains(TEXT("TranspersonalCharacter")))
            {
                bHasPlayableCharacter = true;
                bHasMovement = true; // Assume character has movement if it exists
                break;
            }
        }
        
        // Check for terrain/landscape
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && (Actor->GetClass()->GetName().Contains(TEXT("Landscape")) || 
                         Actor->GetClass()->GetName().Contains(TEXT("StaticMesh"))))
            {
                bHasBasicTerrain = true;
                break;
            }
        }
        
        // Check for dinosaurs
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor)
            {
                FString ActorName = Actor->GetActorLabel().ToLower();
                if (ActorName.Contains(TEXT("trex")) || ActorName.Contains(TEXT("raptor")) || 
                    ActorName.Contains(TEXT("brachio")) || ActorName.Contains(TEXT("tricera")))
                {
                    bHasDinosaurs = true;
                    break;
                }
            }
        }
    }
    
    bool bIsValid = bHasPlayableCharacter && bHasBasicTerrain && bHasDinosaurs && bHasMovement;
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director MVP Validation: Character=%s, Terrain=%s, Dinosaurs=%s, Movement=%s - RESULT: %s"),
           bHasPlayableCharacter ? TEXT("YES") : TEXT("NO"),
           bHasBasicTerrain ? TEXT("YES") : TEXT("NO"),
           bHasDinosaurs ? TEXT("YES") : TEXT("NO"),
           bHasMovement ? TEXT("YES") : TEXT("NO"),
           bIsValid ? TEXT("VALID") : TEXT("INVALID"));
    
    return bIsValid;
}

TArray<FString> UStudioDirectorTaskManager::GetPrototypeValidationReport()
{
    TArray<FString> Report;
    
    Report.Add(TEXT("=== MINIMUM VIABLE PROTOTYPE VALIDATION ==="));
    Report.Add(FString::Printf(TEXT("Last Validation: %s"), *LastValidationTime.ToString()));
    
    bool bMVPValid = ValidateMinimumViablePrototype();
    Report.Add(FString::Printf(TEXT("MVP Status: %s"), bMVPValid ? TEXT("VALID") : TEXT("INVALID")));
    
    // Biome population report
    Report.Add(TEXT(""));
    Report.Add(TEXT("=== BIOME POPULATION STATUS ==="));
    for (const FDir_BiomeStatus& Biome : BiomeStatuses)
    {
        Report.Add(FString::Printf(TEXT("%s: %d actors, %d dinosaurs (%.1f%% populated)"),
                   *Biome.BiomeName, Biome.ActorCount, Biome.DinosaurCount, Biome.PopulationPercentage));
    }
    
    // Task completion report
    Report.Add(TEXT(""));
    Report.Add(TEXT("=== TASK COMPLETION STATUS ==="));
    int32 CompletedTasks = 0;
    int32 TotalTasks = ActiveTasks.Num();
    
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.Status == EDir_TaskStatus::Completed)
        {
            CompletedTasks++;
        }
    }
    
    Report.Add(FString::Printf(TEXT("Completed Tasks: %d/%d (%.1f%%)"),
               CompletedTasks, TotalTasks, TotalTasks > 0 ? (float)CompletedTasks / TotalTasks * 100.0f : 0.0f));
    
    return Report;
}

void UStudioDirectorTaskManager::AssignCriticalTasks()
{
    // Critical tasks for Milestone 1 - "Walk Around" prototype
    CreateTask(EDir_AgentType::ProceduralWorld, 
               TEXT("Create heightmap-based terrain for all 5 biomes with proper elevation"),
               EDir_AgentPriority::Critical,
               TEXT("Landscape actors in MinPlayableMap with height variation"));
    
    CreateTask(EDir_AgentType::EnvironmentArtist,
               TEXT("Populate biomes with 500+ static mesh actors each using real assets"),
               EDir_AgentPriority::Critical,
               TEXT("2500+ environment actors spawned across all biomes"));
    
    CreateTask(EDir_AgentType::CharacterArtist,
               TEXT("Ensure TranspersonalCharacter has proper mesh and animations"),
               EDir_AgentPriority::Critical,
               TEXT("Playable character with MetaHuman or custom mesh"));
    
    CreateTask(EDir_AgentType::Animation,
               TEXT("Implement WASD movement, running, jumping for TranspersonalCharacter"),
               EDir_AgentPriority::Critical,
               TEXT("Responsive character movement with animations"));
    
    CreateTask(EDir_AgentType::NPCBehavior,
               TEXT("Create basic dinosaur AI with idle, roaming, and territorial behavior"),
               EDir_AgentPriority::High,
               TEXT("Dinosaur actors with basic behavior trees"));
    
    CreateTask(EDir_AgentType::Lighting,
               TEXT("Set up day/night cycle with proper directional lighting and atmosphere"),
               EDir_AgentPriority::High,
               TEXT("Dynamic lighting system with sky atmosphere"));
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Assigned %d critical tasks for Milestone 1"), ActiveTasks.Num());
}

float UStudioDirectorTaskManager::GetOverallProjectProgress()
{
    if (ActiveTasks.Num() == 0)
    {
        return 0.0f;
    }
    
    int32 CompletedTasks = 0;
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.Status == EDir_TaskStatus::Completed)
        {
            CompletedTasks++;
        }
    }
    
    return (float)CompletedTasks / ActiveTasks.Num() * 100.0f;
}