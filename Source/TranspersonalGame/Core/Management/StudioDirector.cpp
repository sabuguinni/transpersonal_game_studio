#include "StudioDirector.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

AStudioDirector::AStudioDirector()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    CurrentCycleNumber = 0;
    CurrentCycleID = "";
    CurrentMilestone = EDir_ProductionMilestone::WalkAround;
    TotalFilesCreated = 0;
    TotalUE5CommandsExecuted = 0;
    ProductionEfficiency = 0.0f;
}

void AStudioDirector::BeginPlay()
{
    Super::BeginPlay();

    InitializeAgentPipeline();
    InitializeMilestones();

    UE_LOG(LogTemp, Warning, TEXT("Studio Director initialized - Production pipeline ready"));
}

void AStudioDirector::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update production metrics every second
    static float MetricsTimer = 0.0f;
    MetricsTimer += DeltaTime;
    
    if (MetricsTimer >= 1.0f)
    {
        // Calculate production efficiency based on active agents
        int32 ActiveAgents = 0;
        int32 CompletedAgents = 0;
        
        for (const FDir_AgentTask& Task : AgentTasks)
        {
            if (Task.Status == EDir_AgentStatus::Working)
                ActiveAgents++;
            else if (Task.Status == EDir_AgentStatus::Completed)
                CompletedAgents++;
        }
        
        ProductionEfficiency = AgentTasks.Num() > 0 ? 
            (float)(ActiveAgents + CompletedAgents) / AgentTasks.Num() * 100.0f : 0.0f;
        
        MetricsTimer = 0.0f;
    }
}

void AStudioDirector::InitializeAgentPipeline()
{
    AgentTasks.Empty();
    SetupDefaultAgents();
    
    UE_LOG(LogTemp, Warning, TEXT("Agent pipeline initialized with %d agents"), AgentTasks.Num());
}

void AStudioDirector::SetupDefaultAgents()
{
    // Define all 19 agents in the production pipeline
    TArray<TPair<int32, FString>> AgentDefinitions = {
        {1, "Studio Director"},
        {2, "Engine Architect"},
        {3, "Core Systems Programmer"},
        {4, "Performance Optimizer"},
        {5, "Procedural World Generator"},
        {6, "Environment Artist"},
        {7, "Architecture & Interior Agent"},
        {8, "Lighting & Atmosphere Agent"},
        {9, "Character Artist Agent"},
        {10, "Animation Agent"},
        {11, "NPC Behavior Agent"},
        {12, "Combat & Enemy AI Agent"},
        {13, "Crowd & Traffic Simulation"},
        {14, "Quest & Mission Designer"},
        {15, "Narrative & Dialogue Agent"},
        {16, "Audio Agent"},
        {17, "VFX Agent"},
        {18, "QA & Testing Agent"},
        {19, "Integration & Build Agent"}
    };

    for (const auto& AgentDef : AgentDefinitions)
    {
        FDir_AgentTask NewTask;
        NewTask.AgentNumber = AgentDef.Key;
        NewTask.AgentName = AgentDef.Value;
        NewTask.Status = EDir_AgentStatus::Idle;
        NewTask.ProgressPercentage = 0.0f;
        
        // Set initial tasks based on current milestone
        if (CurrentMilestone == EDir_ProductionMilestone::WalkAround)
        {
            switch (AgentDef.Key)
            {
                case 2: NewTask.CurrentTask = "Finalize core architecture and compilation rules"; break;
                case 3: NewTask.CurrentTask = "Implement physics and collision systems"; break;
                case 5: NewTask.CurrentTask = "Create varied terrain with height variation"; break;
                case 9: NewTask.CurrentTask = "Enhance player character model and materials"; break;
                case 10: NewTask.CurrentTask = "Add basic movement animations"; break;
                case 12: NewTask.CurrentTask = "Implement basic dinosaur AI behavior"; break;
                default: NewTask.CurrentTask = "Support milestone 1 completion"; break;
            }
        }
        
        AgentTasks.Add(NewTask);
    }
}

void AStudioDirector::InitializeMilestones()
{
    ProductionMilestones.Empty();
    SetupDefaultMilestones();
    
    UE_LOG(LogTemp, Warning, TEXT("Production milestones initialized"));
}

void AStudioDirector::SetupDefaultMilestones()
{
    // Milestone 1: Walk Around
    FDir_MilestoneData Milestone1;
    Milestone1.MilestoneType = EDir_ProductionMilestone::WalkAround;
    Milestone1.MilestoneName = "Walk Around";
    Milestone1.Requirements = {
        "ThirdPersonCharacter with WASD movement",
        "Camera boom + follow camera",
        "Landscape with basic terrain variation",
        "Player can walk, run, jump",
        "3-5 static dinosaur meshes in world",
        "Directional light + sky atmosphere + fog"
    };
    Milestone1.RequiredAgents = {2, 3, 5, 9, 10, 12};
    Milestone1.bIsCompleted = false;
    Milestone1.CompletionPercentage = 0.0f;
    ProductionMilestones.Add(Milestone1);

    // Milestone 2: Basic Survival
    FDir_MilestoneData Milestone2;
    Milestone2.MilestoneType = EDir_ProductionMilestone::BasicSurvival;
    Milestone2.MilestoneName = "Basic Survival";
    Milestone2.Requirements = {
        "Health, hunger, thirst, stamina systems",
        "Basic crafting system",
        "Resource gathering mechanics",
        "Day/night cycle",
        "Temperature system"
    };
    Milestone2.RequiredAgents = {3, 8, 11, 14, 15};
    Milestone2.bIsCompleted = false;
    Milestone2.CompletionPercentage = 0.0f;
    ProductionMilestones.Add(Milestone2);

    // Milestone 3: Dinosaur Encounters
    FDir_MilestoneData Milestone3;
    Milestone3.MilestoneType = EDir_ProductionMilestone::DinosaurEncounters;
    Milestone3.MilestoneName = "Dinosaur Encounters";
    Milestone3.Requirements = {
        "Dinosaur AI with territorial behavior",
        "Combat system with primitive weapons",
        "Stealth and evasion mechanics",
        "Dinosaur animation and sound",
        "Fear and stress systems"
    };
    Milestone3.RequiredAgents = {10, 11, 12, 16, 17};
    Milestone3.bIsCompleted = false;
    Milestone3.CompletionPercentage = 0.0f;
    ProductionMilestones.Add(Milestone3);
}

void AStudioDirector::UpdateAgentStatus(int32 AgentNumber, EDir_AgentStatus NewStatus, float Progress)
{
    FDir_AgentTask* Task = FindAgentTask(AgentNumber);
    if (Task)
    {
        Task->Status = NewStatus;
        Task->ProgressPercentage = FMath::Clamp(Progress, 0.0f, 100.0f);
        
        UE_LOG(LogTemp, Log, TEXT("Agent %d (%s) status updated: %d, Progress: %.1f%%"), 
               AgentNumber, *Task->AgentName, (int32)NewStatus, Progress);
    }
}

void AStudioDirector::AssignTaskToAgent(int32 AgentNumber, const FString& TaskDescription, const TArray<FString>& Dependencies)
{
    FDir_AgentTask* Task = FindAgentTask(AgentNumber);
    if (Task)
    {
        Task->CurrentTask = TaskDescription;
        Task->Dependencies = Dependencies;
        Task->Status = EDir_AgentStatus::Working;
        Task->ProgressPercentage = 0.0f;
        
        UE_LOG(LogTemp, Warning, TEXT("Task assigned to Agent %d: %s"), AgentNumber, *TaskDescription);
    }
}

bool AStudioDirector::CanAgentProceed(int32 AgentNumber) const
{
    const FDir_AgentTask* Task = nullptr;
    for (const FDir_AgentTask& AgentTask : AgentTasks)
    {
        if (AgentTask.AgentNumber == AgentNumber)
        {
            Task = &AgentTask;
            break;
        }
    }
    
    if (!Task)
        return false;
    
    // Check if all dependencies are completed
    for (const FString& Dependency : Task->Dependencies)
    {
        // Parse dependency (format: "Agent_X_Completed" or similar)
        // For now, simplified logic
        bool DependencyMet = true; // Implement proper dependency checking
        if (!DependencyMet)
            return false;
    }
    
    return true;
}

TArray<int32> AStudioDirector::GetBlockedAgents() const
{
    TArray<int32> BlockedAgents;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Blocked)
        {
            BlockedAgents.Add(Task.AgentNumber);
        }
    }
    
    return BlockedAgents;
}

void AStudioDirector::UpdateMilestoneProgress(EDir_ProductionMilestone Milestone, float Progress)
{
    FDir_MilestoneData* MilestoneData = FindMilestone(Milestone);
    if (MilestoneData)
    {
        MilestoneData->CompletionPercentage = FMath::Clamp(Progress, 0.0f, 100.0f);
        
        if (Progress >= 100.0f && !MilestoneData->bIsCompleted)
        {
            MilestoneData->bIsCompleted = true;
            UE_LOG(LogTemp, Warning, TEXT("Milestone completed: %s"), *MilestoneData->MilestoneName);
        }
    }
}

bool AStudioDirector::IsMilestoneCompleted(EDir_ProductionMilestone Milestone) const
{
    for (const FDir_MilestoneData& MilestoneData : ProductionMilestones)
    {
        if (MilestoneData.MilestoneType == Milestone)
        {
            return MilestoneData.bIsCompleted;
        }
    }
    return false;
}

void AStudioDirector::AdvanceToNextMilestone()
{
    if (IsMilestoneCompleted(CurrentMilestone))
    {
        switch (CurrentMilestone)
        {
            case EDir_ProductionMilestone::WalkAround:
                CurrentMilestone = EDir_ProductionMilestone::BasicSurvival;
                break;
            case EDir_ProductionMilestone::BasicSurvival:
                CurrentMilestone = EDir_ProductionMilestone::DinosaurEncounters;
                break;
            case EDir_ProductionMilestone::DinosaurEncounters:
                CurrentMilestone = EDir_ProductionMilestone::FullGameplay;
                break;
            default:
                break;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Advanced to next milestone: %d"), (int32)CurrentMilestone);
    }
}

void AStudioDirector::StartProductionCycle(const FString& CycleID)
{
    CurrentCycleID = CycleID;
    CurrentCycleNumber++;
    
    UE_LOG(LogTemp, Warning, TEXT("Production cycle started: %s (Cycle #%d)"), *CycleID, CurrentCycleNumber);
}

void AStudioDirector::CompleteProductionCycle()
{
    // Update metrics
    TotalFilesCreated += 8; // Estimate based on agent output
    TotalUE5CommandsExecuted += 2; // Minimum UE5 commands per cycle
    
    UE_LOG(LogTemp, Warning, TEXT("Production cycle completed: %s"), *CurrentCycleID);
}

FString AStudioDirector::GenerateProductionReport() const
{
    FString Report = FString::Printf(TEXT("=== PRODUCTION REPORT ===\n"));
    Report += FString::Printf(TEXT("Cycle: %s (#%d)\n"), *CurrentCycleID, CurrentCycleNumber);
    Report += FString::Printf(TEXT("Current Milestone: %d\n"), (int32)CurrentMilestone);
    Report += FString::Printf(TEXT("Production Efficiency: %.1f%%\n"), ProductionEfficiency);
    Report += FString::Printf(TEXT("Total Files Created: %d\n"), TotalFilesCreated);
    Report += FString::Printf(TEXT("Total UE5 Commands: %d\n"), TotalUE5CommandsExecuted);
    
    Report += TEXT("\n=== AGENT STATUS ===\n");
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        Report += FString::Printf(TEXT("Agent %d (%s): %s - %.1f%%\n"), 
                                  Task.AgentNumber, *Task.AgentName, 
                                  *UEnum::GetValueAsString(Task.Status), Task.ProgressPercentage);
    }
    
    return Report;
}

void AStudioDirector::ValidateWorldState()
{
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    // Count actors by type
    int32 CharacterCount = 0;
    int32 LandscapeCount = 0;
    int32 DinosaurCount = 0;
    int32 LightCount = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        FString ActorName = Actor->GetName().ToLower();
        
        if (ActorName.Contains(TEXT("character")))
            CharacterCount++;
        else if (ActorName.Contains(TEXT("landscape")) || ActorName.Contains(TEXT("terrain")))
            LandscapeCount++;
        else if (ActorName.Contains(TEXT("rex")) || ActorName.Contains(TEXT("raptor")) || ActorName.Contains(TEXT("dino")))
            DinosaurCount++;
        else if (ActorName.Contains(TEXT("light")))
            LightCount++;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("World State - Characters: %d, Landscapes: %d, Dinosaurs: %d, Lights: %d"), 
           CharacterCount, LandscapeCount, DinosaurCount, LightCount);
}

void AStudioDirector::DebugPrintAgentStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== AGENT STATUS DEBUG ==="));
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        UE_LOG(LogTemp, Warning, TEXT("Agent %d (%s): %s - %.1f%% - Task: %s"), 
               Task.AgentNumber, *Task.AgentName, 
               *UEnum::GetValueAsString(Task.Status), Task.ProgressPercentage, *Task.CurrentTask);
    }
}

void AStudioDirector::DebugPrintMilestoneStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== MILESTONE STATUS DEBUG ==="));
    for (const FDir_MilestoneData& Milestone : ProductionMilestones)
    {
        UE_LOG(LogTemp, Warning, TEXT("Milestone: %s - %.1f%% - Completed: %s"), 
               *Milestone.MilestoneName, Milestone.CompletionPercentage, 
               Milestone.bIsCompleted ? TEXT("YES") : TEXT("NO"));
    }
}

void AStudioDirector::ResetProductionPipeline()
{
    AgentTasks.Empty();
    ProductionMilestones.Empty();
    CurrentCycleNumber = 0;
    CurrentCycleID = "";
    CurrentMilestone = EDir_ProductionMilestone::WalkAround;
    TotalFilesCreated = 0;
    TotalUE5CommandsExecuted = 0;
    ProductionEfficiency = 0.0f;
    
    InitializeAgentPipeline();
    InitializeMilestones();
    
    UE_LOG(LogTemp, Warning, TEXT("Production pipeline reset"));
}

FDir_AgentTask* AStudioDirector::FindAgentTask(int32 AgentNumber)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentNumber == AgentNumber)
        {
            return &Task;
        }
    }
    return nullptr;
}

FDir_MilestoneData* AStudioDirector::FindMilestone(EDir_ProductionMilestone Milestone)
{
    for (FDir_MilestoneData& MilestoneData : ProductionMilestones)
    {
        if (MilestoneData.MilestoneType == Milestone)
        {
            return &MilestoneData;
        }
    }
    return nullptr;
}