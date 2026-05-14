#include "StudioDirectorSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"

UStudioDirectorSystem::UStudioDirectorSystem()
{
    bSystemInitialized = false;
}

void UStudioDirectorSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director System initializing..."));
    
    InitializeAgentChain();
    bSystemInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director System initialized successfully"));
}

void UStudioDirectorSystem::Deinitialize()
{
    RegisteredAgents.Empty();
    bSystemInitialized = false;
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director System deinitialized"));
    
    Super::Deinitialize();
}

void UStudioDirectorSystem::RegisterAgent(int32 AgentID, const FString& AgentName)
{
    FDir_AgentInfo NewAgent;
    NewAgent.AgentID = AgentID;
    NewAgent.AgentName = AgentName;
    NewAgent.Status = EDir_AgentStatus::Idle;
    NewAgent.CurrentTask = TEXT("Awaiting instructions");
    NewAgent.ProgressPercent = 0.0f;
    NewAgent.LastUpdate = FDateTime::Now();
    
    RegisteredAgents.Add(AgentID, NewAgent);
    
    UE_LOG(LogTemp, Log, TEXT("Registered Agent #%d: %s"), AgentID, *AgentName);
}

void UStudioDirectorSystem::UpdateAgentStatus(int32 AgentID, EDir_AgentStatus Status, const FString& Task, float Progress)
{
    if (FDir_AgentInfo* Agent = RegisteredAgents.Find(AgentID))
    {
        Agent->Status = Status;
        Agent->CurrentTask = Task;
        Agent->ProgressPercent = FMath::Clamp(Progress, 0.0f, 100.0f);
        Agent->LastUpdate = FDateTime::Now();
        
        UE_LOG(LogTemp, Log, TEXT("Agent #%d status updated: %s - %.1f%%"), 
               AgentID, *Task, Progress);
    }
}

FDir_AgentInfo UStudioDirectorSystem::GetAgentInfo(int32 AgentID) const
{
    if (const FDir_AgentInfo* Agent = RegisteredAgents.Find(AgentID))
    {
        return *Agent;
    }
    
    return FDir_AgentInfo();
}

TArray<FDir_AgentInfo> UStudioDirectorSystem::GetAllAgents() const
{
    TArray<FDir_AgentInfo> AllAgents;
    
    for (const auto& AgentPair : RegisteredAgents)
    {
        AllAgents.Add(AgentPair.Value);
    }
    
    // Sort by Agent ID
    AllAgents.Sort([](const FDir_AgentInfo& A, const FDir_AgentInfo& B) {
        return A.AgentID < B.AgentID;
    });
    
    return AllAgents;
}

void UStudioDirectorSystem::UpdateSystemMetrics()
{
    if (UWorld* World = GetWorld())
    {
        // Count actors by type
        CurrentMetrics.TotalActors = 0;
        CurrentMetrics.CharacterActors = 0;
        CurrentMetrics.DinosaurActors = 0;
        CurrentMetrics.EnvironmentActors = 0;
        
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (IsValid(Actor))
            {
                CurrentMetrics.TotalActors++;
                
                FString ActorName = Actor->GetClass()->GetName();
                if (ActorName.Contains(TEXT("Character")))
                {
                    CurrentMetrics.CharacterActors++;
                }
                else if (ActorName.Contains(TEXT("Dinosaur")) || ActorName.Contains(TEXT("TRex")) || 
                         ActorName.Contains(TEXT("Raptor")) || ActorName.Contains(TEXT("Brach")))
                {
                    CurrentMetrics.DinosaurActors++;
                }
                else if (ActorName.Contains(TEXT("Tree")) || ActorName.Contains(TEXT("Rock")) || 
                         ActorName.Contains(TEXT("Landscape")))
                {
                    CurrentMetrics.EnvironmentActors++;
                }
            }
        }
        
        // Get performance metrics
        CurrentMetrics.FrameRate = 1.0f / FApp::GetDeltaTime();
        CurrentMetrics.MemoryUsageMB = FPlatformMemory::GetStats().UsedPhysical / (1024.0f * 1024.0f);
    }
}

FDir_SystemMetrics UStudioDirectorSystem::GetSystemMetrics() const
{
    return CurrentMetrics;
}

void UStudioDirectorSystem::QueueTaskForAgent(int32 AgentID, const FString& TaskDescription, EDir_SystemPriority Priority)
{
    UpdateAgentStatus(AgentID, EDir_AgentStatus::Working, TaskDescription, 0.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("Task queued for Agent #%d: %s (Priority: %d)"), 
           AgentID, *TaskDescription, (int32)Priority);
}

void UStudioDirectorSystem::CompleteAgentTask(int32 AgentID, bool bSuccess)
{
    EDir_AgentStatus NewStatus = bSuccess ? EDir_AgentStatus::Completed : EDir_AgentStatus::Failed;
    FString StatusText = bSuccess ? TEXT("Task completed successfully") : TEXT("Task failed");
    
    UpdateAgentStatus(AgentID, NewStatus, StatusText, 100.0f);
}

void UStudioDirectorSystem::InitializeMinPlayableMap()
{
    UE_LOG(LogTemp, Warning, TEXT("Initializing MinPlayableMap for playable prototype..."));
    
    // Queue tasks for critical agents
    QueueTaskForAgent(2, TEXT("Validate engine architecture and core systems"), EDir_SystemPriority::Critical);
    QueueTaskForAgent(3, TEXT("Implement physics and collision systems"), EDir_SystemPriority::Critical);
    QueueTaskForAgent(5, TEXT("Generate 10km2 landscape with 5 biomes"), EDir_SystemPriority::High);
    QueueTaskForAgent(9, TEXT("Create character system with MetaHuman"), EDir_SystemPriority::High);
    QueueTaskForAgent(10, TEXT("Implement character animations and movement"), EDir_SystemPriority::High);
    QueueTaskForAgent(12, TEXT("Create dinosaur AI and combat systems"), EDir_SystemPriority::Medium);
    
    UE_LOG(LogTemp, Warning, TEXT("MinPlayableMap initialization tasks queued"));
}

bool UStudioDirectorSystem::ValidateGameplayReadiness()
{
    UpdateSystemMetrics();
    
    bool bHasCharacter = CurrentMetrics.CharacterActors > 0;
    bool bHasDinosaurs = CurrentMetrics.DinosaurActors > 0;
    bool bHasEnvironment = CurrentMetrics.EnvironmentActors > 10; // At least trees and rocks
    bool bGoodFrameRate = CurrentMetrics.FrameRate > 30.0f;
    
    bool bReady = bHasCharacter && bHasDinosaurs && bHasEnvironment && bGoodFrameRate;
    
    UE_LOG(LogTemp, Warning, TEXT("Gameplay Readiness Check:"));
    UE_LOG(LogTemp, Warning, TEXT("- Character: %s"), bHasCharacter ? TEXT("PASS") : TEXT("FAIL"));
    UE_LOG(LogTemp, Warning, TEXT("- Dinosaurs: %s"), bHasDinosaurs ? TEXT("PASS") : TEXT("FAIL"));
    UE_LOG(LogTemp, Warning, TEXT("- Environment: %s"), bHasEnvironment ? TEXT("PASS") : TEXT("FAIL"));
    UE_LOG(LogTemp, Warning, TEXT("- Performance: %s (%.1f fps)"), bGoodFrameRate ? TEXT("PASS") : TEXT("FAIL"), CurrentMetrics.FrameRate);
    UE_LOG(LogTemp, Warning, TEXT("Overall: %s"), bReady ? TEXT("READY") : TEXT("NOT READY"));
    
    return bReady;
}

void UStudioDirectorSystem::RunSystemDiagnostics()
{
    UE_LOG(LogTemp, Warning, TEXT("=== STUDIO DIRECTOR SYSTEM DIAGNOSTICS ==="));
    
    UpdateSystemMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("System Status: %s"), bSystemInitialized ? TEXT("INITIALIZED") : TEXT("NOT INITIALIZED"));
    UE_LOG(LogTemp, Warning, TEXT("Registered Agents: %d"), RegisteredAgents.Num());
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), CurrentMetrics.TotalActors);
    UE_LOG(LogTemp, Warning, TEXT("Character Actors: %d"), CurrentMetrics.CharacterActors);
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur Actors: %d"), CurrentMetrics.DinosaurActors);
    UE_LOG(LogTemp, Warning, TEXT("Environment Actors: %d"), CurrentMetrics.EnvironmentActors);
    UE_LOG(LogTemp, Warning, TEXT("Frame Rate: %.1f fps"), CurrentMetrics.FrameRate);
    UE_LOG(LogTemp, Warning, TEXT("Memory Usage: %.1f MB"), CurrentMetrics.MemoryUsageMB);
    
    // Check critical systems
    CheckCriticalSystems();
    
    UE_LOG(LogTemp, Warning, TEXT("=== END DIAGNOSTICS ==="));
}

void UStudioDirectorSystem::InitializeAgentChain()
{
    // Register all 19 agents in the production chain
    RegisterAgent(1, TEXT("Studio Director"));
    RegisterAgent(2, TEXT("Engine Architect"));
    RegisterAgent(3, TEXT("Core Systems Programmer"));
    RegisterAgent(4, TEXT("Performance Optimizer"));
    RegisterAgent(5, TEXT("Procedural World Generator"));
    RegisterAgent(6, TEXT("Environment Artist"));
    RegisterAgent(7, TEXT("Architecture & Interior Agent"));
    RegisterAgent(8, TEXT("Lighting & Atmosphere Agent"));
    RegisterAgent(9, TEXT("Character Artist Agent"));
    RegisterAgent(10, TEXT("Animation Agent"));
    RegisterAgent(11, TEXT("NPC Behavior Agent"));
    RegisterAgent(12, TEXT("Combat & Enemy AI Agent"));
    RegisterAgent(13, TEXT("Crowd & Traffic Simulation"));
    RegisterAgent(14, TEXT("Quest & Mission Designer"));
    RegisterAgent(15, TEXT("Narrative & Dialogue Agent"));
    RegisterAgent(16, TEXT("Audio Agent"));
    RegisterAgent(17, TEXT("VFX Agent"));
    RegisterAgent(18, TEXT("QA & Testing Agent"));
    RegisterAgent(19, TEXT("Integration & Build Agent"));
    
    UE_LOG(LogTemp, Warning, TEXT("Agent chain initialized with %d agents"), RegisteredAgents.Num());
}

void UStudioDirectorSystem::ValidateWorldState()
{
    if (UWorld* World = GetWorld())
    {
        UE_LOG(LogTemp, Log, TEXT("World validation: %s"), *World->GetName());
        
        // Check for essential game objects
        bool bHasPlayerStart = false;
        bool bHasGameMode = false;
        
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (IsValid(Actor))
            {
                FString ActorName = Actor->GetClass()->GetName();
                if (ActorName.Contains(TEXT("PlayerStart")))
                {
                    bHasPlayerStart = true;
                }
                if (ActorName.Contains(TEXT("GameMode")))
                {
                    bHasGameMode = true;
                }
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("World State - PlayerStart: %s, GameMode: %s"), 
               bHasPlayerStart ? TEXT("Found") : TEXT("Missing"),
               bHasGameMode ? TEXT("Found") : TEXT("Missing"));
    }
}

void UStudioDirectorSystem::CheckCriticalSystems()
{
    ValidateWorldState();
    
    // Check if MinPlayableMap requirements are met
    bool bMeetsRequirements = ValidateGameplayReadiness();
    
    if (!bMeetsRequirements)
    {
        UE_LOG(LogTemp, Error, TEXT("CRITICAL: MinPlayableMap does not meet playable prototype requirements"));
        
        // Identify missing components
        if (CurrentMetrics.CharacterActors == 0)
        {
            UE_LOG(LogTemp, Error, TEXT("Missing: Playable character with movement"));
        }
        if (CurrentMetrics.DinosaurActors == 0)
        {
            UE_LOG(LogTemp, Error, TEXT("Missing: Dinosaur actors in the world"));
        }
        if (CurrentMetrics.EnvironmentActors < 10)
        {
            UE_LOG(LogTemp, Error, TEXT("Missing: Sufficient environment actors (trees, rocks, terrain)"));
        }
        if (CurrentMetrics.FrameRate < 30.0f)
        {
            UE_LOG(LogTemp, Error, TEXT("Performance issue: Frame rate below 30 fps"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("SUCCESS: MinPlayableMap meets playable prototype requirements"));
    }
}