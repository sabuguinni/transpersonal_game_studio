#include "StudioDirectorCoordinationSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

UStudioDirectorCoordinationSystem::UStudioDirectorCoordinationSystem()
{
    bEmergencyMode = false;
    InitializeAgentStatuses();
}

void UStudioDirectorCoordinationSystem::InitializeProductionPipeline()
{
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Initializing 19-agent production pipeline"));
    
    // Initialize all agent statuses
    InitializeAgentStatuses();
    
    // Setup dependency chain
    SetupAgentDependencies();
    
    // Start with Architecture phase
    CurrentMetrics.CurrentPhase = EDir_ProductionPhase::Architecture;
    CurrentMetrics.TotalAgents = 19;
    
    LogProductionEvent("Production pipeline initialized - Architecture phase started");
}

void UStudioDirectorCoordinationSystem::AssignTaskToAgent(int32 AgentID, const FDir_AgentTask& Task)
{
    if (AgentID < 1 || AgentID > 19)
    {
        UE_LOG(LogTemp, Error, TEXT("Studio Director: Invalid Agent ID %d"), AgentID);
        return;
    }

    // Validate dependencies before assignment
    if (!ValidateAgentDependencies(AgentID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Studio Director: Agent %d dependencies not met, task queued"), AgentID);
        AgentTaskQueues.FindOrAdd(AgentID).Add(Task);
        return;
    }

    // Assign task and update status
    AgentTaskQueues.FindOrAdd(AgentID).Add(Task);
    AgentStatusMap.Add(AgentID, EDir_AgentStatus::Active);
    
    FString LogMessage = FString::Printf(TEXT("Task assigned to Agent %d: %s"), AgentID, *Task.TaskName);
    LogProductionEvent(LogMessage);
    
    CalculateProductionMetrics();
}

void UStudioDirectorCoordinationSystem::UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus)
{
    if (AgentStatusMap.Contains(AgentID))
    {
        EDir_AgentStatus OldStatus = AgentStatusMap[AgentID];
        AgentStatusMap[AgentID] = NewStatus;
        
        FString StatusStr = "";
        switch (NewStatus)
        {
            case EDir_AgentStatus::Idle: StatusStr = "Idle"; break;
            case EDir_AgentStatus::Active: StatusStr = "Active"; break;
            case EDir_AgentStatus::Blocked: StatusStr = "Blocked"; break;
            case EDir_AgentStatus::Complete: StatusStr = "Complete"; break;
            case EDir_AgentStatus::Error: StatusStr = "Error"; break;
        }
        
        FString LogMessage = FString::Printf(TEXT("Agent %d status changed to %s"), AgentID, *StatusStr);
        LogProductionEvent(LogMessage);
        
        // Handle error states
        if (NewStatus == EDir_AgentStatus::Error)
        {
            HandleAgentTimeout(AgentID);
        }
        
        CalculateProductionMetrics();
    }
}

bool UStudioDirectorCoordinationSystem::ValidateAgentDependencies(int32 AgentID)
{
    // Agent dependency chain validation
    switch (AgentID)
    {
        case 1: // Studio Director - no dependencies
            return true;
        case 2: // Engine Architect - depends on Studio Director
            return AgentStatusMap.Contains(1) && AgentStatusMap[1] == EDir_AgentStatus::Complete;
        case 3: // Core Systems - depends on Engine Architect
            return AgentStatusMap.Contains(2) && AgentStatusMap[2] == EDir_AgentStatus::Complete;
        case 4: // Performance Optimizer - depends on Core Systems
            return AgentStatusMap.Contains(3) && AgentStatusMap[3] == EDir_AgentStatus::Complete;
        case 5: // World Generator - depends on Core Systems
            return AgentStatusMap.Contains(3) && AgentStatusMap[3] == EDir_AgentStatus::Complete;
        case 6: // Environment Artist - depends on World Generator
            return AgentStatusMap.Contains(5) && AgentStatusMap[5] == EDir_AgentStatus::Complete;
        case 7: // Architecture Agent - depends on Environment Artist
            return AgentStatusMap.Contains(6) && AgentStatusMap[6] == EDir_AgentStatus::Complete;
        case 8: // Lighting Agent - depends on Architecture
            return AgentStatusMap.Contains(7) && AgentStatusMap[7] == EDir_AgentStatus::Complete;
        case 9: // Character Artist - depends on Core Systems
            return AgentStatusMap.Contains(3) && AgentStatusMap[3] == EDir_AgentStatus::Complete;
        case 10: // Animation Agent - depends on Character Artist
            return AgentStatusMap.Contains(9) && AgentStatusMap[9] == EDir_AgentStatus::Complete;
        case 11: // NPC Behavior - depends on Animation
            return AgentStatusMap.Contains(10) && AgentStatusMap[10] == EDir_AgentStatus::Complete;
        case 12: // Combat AI - depends on NPC Behavior
            return AgentStatusMap.Contains(11) && AgentStatusMap[11] == EDir_AgentStatus::Complete;
        case 13: // Crowd Simulation - depends on Combat AI
            return AgentStatusMap.Contains(12) && AgentStatusMap[12] == EDir_AgentStatus::Complete;
        case 14: // Quest Designer - depends on NPC Behavior
            return AgentStatusMap.Contains(11) && AgentStatusMap[11] == EDir_AgentStatus::Complete;
        case 15: // Narrative Agent - no dependencies (runs parallel)
            return true;
        case 16: // Audio Agent - depends on Environment
            return AgentStatusMap.Contains(6) && AgentStatusMap[6] == EDir_AgentStatus::Complete;
        case 17: // VFX Agent - depends on Lighting
            return AgentStatusMap.Contains(8) && AgentStatusMap[8] == EDir_AgentStatus::Complete;
        case 18: // QA Agent - depends on all core systems
            return AgentStatusMap.Contains(3) && AgentStatusMap[3] == EDir_AgentStatus::Complete &&
                   AgentStatusMap.Contains(9) && AgentStatusMap[9] == EDir_AgentStatus::Complete;
        case 19: // Integration Agent - depends on QA
            return AgentStatusMap.Contains(18) && AgentStatusMap[18] == EDir_AgentStatus::Complete;
        default:
            return false;
    }
}

void UStudioDirectorCoordinationSystem::TriggerEmergencyProtocol(const FString& Reason)
{
    bEmergencyMode = true;
    FString LogMessage = FString::Printf(TEXT("EMERGENCY PROTOCOL TRIGGERED: %s"), *Reason);
    LogProductionEvent(LogMessage);
    
    UE_LOG(LogTemp, Error, TEXT("Studio Director Emergency: %s"), *Reason);
    
    // Pause all non-critical agents
    for (auto& StatusPair : AgentStatusMap)
    {
        if (StatusPair.Value == EDir_AgentStatus::Active && StatusPair.Key > 3)
        {
            AgentStatusMap[StatusPair.Key] = EDir_AgentStatus::Blocked;
        }
    }
}

FDir_ProductionMetrics UStudioDirectorCoordinationSystem::GetProductionMetrics() const
{
    return CurrentMetrics;
}

TArray<FDir_AgentTask> UStudioDirectorCoordinationSystem::GetAgentTasks(int32 AgentID) const
{
    if (AgentTaskQueues.Contains(AgentID))
    {
        return AgentTaskQueues[AgentID];
    }
    return TArray<FDir_AgentTask>();
}

void UStudioDirectorCoordinationSystem::AdvanceProductionPhase()
{
    switch (CurrentMetrics.CurrentPhase)
    {
        case EDir_ProductionPhase::Architecture:
            CurrentMetrics.CurrentPhase = EDir_ProductionPhase::CoreSystems;
            LogProductionEvent("Advanced to Core Systems phase");
            break;
        case EDir_ProductionPhase::CoreSystems:
            CurrentMetrics.CurrentPhase = EDir_ProductionPhase::WorldGeneration;
            LogProductionEvent("Advanced to World Generation phase");
            break;
        case EDir_ProductionPhase::WorldGeneration:
            CurrentMetrics.CurrentPhase = EDir_ProductionPhase::Characters;
            LogProductionEvent("Advanced to Characters phase");
            break;
        case EDir_ProductionPhase::Characters:
            CurrentMetrics.CurrentPhase = EDir_ProductionPhase::AI;
            LogProductionEvent("Advanced to AI phase");
            break;
        case EDir_ProductionPhase::AI:
            CurrentMetrics.CurrentPhase = EDir_ProductionPhase::Audio;
            LogProductionEvent("Advanced to Audio phase");
            break;
        case EDir_ProductionPhase::Audio:
            CurrentMetrics.CurrentPhase = EDir_ProductionPhase::Integration;
            LogProductionEvent("Advanced to Integration phase");
            break;
        case EDir_ProductionPhase::Integration:
            CurrentMetrics.CurrentPhase = EDir_ProductionPhase::Testing;
            LogProductionEvent("Advanced to Testing phase");
            break;
        case EDir_ProductionPhase::Testing:
            LogProductionEvent("Production complete - all phases finished");
            break;
    }
}

bool UStudioDirectorCoordinationSystem::ValidatePlayablePrototype()
{
    return CheckMilestone1Requirements();
}

void UStudioDirectorCoordinationSystem::GenerateProductionReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== STUDIO DIRECTOR PRODUCTION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Agents: %d"), CurrentMetrics.TotalAgents);
    UE_LOG(LogTemp, Warning, TEXT("Active Agents: %d"), CurrentMetrics.ActiveAgents);
    UE_LOG(LogTemp, Warning, TEXT("Completed Tasks: %d"), CurrentMetrics.CompletedTasks);
    UE_LOG(LogTemp, Warning, TEXT("Blocked Tasks: %d"), CurrentMetrics.BlockedTasks);
    UE_LOG(LogTemp, Warning, TEXT("Overall Progress: %.2f%%"), CurrentMetrics.OverallProgress);
    
    for (const FString& LogEntry : ProductionLog)
    {
        UE_LOG(LogTemp, Log, TEXT("LOG: %s"), *LogEntry);
    }
}

void UStudioDirectorCoordinationSystem::HandleAgentTimeout(int32 AgentID)
{
    FString LogMessage = FString::Printf(TEXT("Agent %d timeout - implementing recovery protocol"), AgentID);
    LogProductionEvent(LogMessage);
    
    // Reset agent to idle and reassign tasks
    AgentStatusMap[AgentID] = EDir_AgentStatus::Idle;
    
    // If critical agent (1-5), trigger emergency protocol
    if (AgentID <= 5)
    {
        TriggerEmergencyProtocol(FString::Printf(TEXT("Critical Agent %d timeout"), AgentID));
    }
}

void UStudioDirectorCoordinationSystem::HandleBuildFailure()
{
    LogProductionEvent("Build failure detected - halting production");
    TriggerEmergencyProtocol("Build compilation failed");
}

void UStudioDirectorCoordinationSystem::HandleQABlocker(const FString& BlockerDescription)
{
    FString LogMessage = FString::Printf(TEXT("QA Blocker: %s"), *BlockerDescription);
    LogProductionEvent(LogMessage);
    TriggerEmergencyProtocol(FString::Printf(TEXT("QA Blocker: %s"), *BlockerDescription));
}

void UStudioDirectorCoordinationSystem::InitializeAgentStatuses()
{
    for (int32 i = 1; i <= 19; i++)
    {
        AgentStatusMap.Add(i, EDir_AgentStatus::Idle);
    }
    
    // Studio Director starts as active
    AgentStatusMap[1] = EDir_AgentStatus::Active;
}

void UStudioDirectorCoordinationSystem::CalculateProductionMetrics()
{
    CurrentMetrics.ActiveAgents = 0;
    CurrentMetrics.CompletedTasks = 0;
    CurrentMetrics.BlockedTasks = 0;
    
    for (const auto& StatusPair : AgentStatusMap)
    {
        switch (StatusPair.Value)
        {
            case EDir_AgentStatus::Active:
                CurrentMetrics.ActiveAgents++;
                break;
            case EDir_AgentStatus::Complete:
                CurrentMetrics.CompletedTasks++;
                break;
            case EDir_AgentStatus::Blocked:
                CurrentMetrics.BlockedTasks++;
                break;
        }
    }
    
    CurrentMetrics.OverallProgress = (float)CurrentMetrics.CompletedTasks / CurrentMetrics.TotalAgents * 100.0f;
}

void UStudioDirectorCoordinationSystem::LogProductionEvent(const FString& Event)
{
    FString TimeStamp = FDateTime::Now().ToString();
    FString LogEntry = FString::Printf(TEXT("[%s] %s"), *TimeStamp, *Event);
    ProductionLog.Add(LogEntry);
    
    UE_LOG(LogTemp, Log, TEXT("Studio Director: %s"), *LogEntry);
}

bool UStudioDirectorCoordinationSystem::CheckMilestone1Requirements()
{
    // Check for playable character
    UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);
    if (!World) return false;
    
    // Check for TranspersonalCharacter in world
    bool bHasCharacter = false;
    for (TActorIterator<APawn> ActorItr(World); ActorItr; ++ActorItr)
    {
        APawn* Pawn = *ActorItr;
        if (Pawn && Pawn->GetClass()->GetName().Contains("TranspersonalCharacter"))
        {
            bHasCharacter = true;
            break;
        }
    }
    
    // Check for terrain (landscape)
    bool bHasTerrain = false;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetClass()->GetName().Contains("Landscape"))
        {
            bHasTerrain = true;
            break;
        }
    }
    
    // Check for dinosaurs
    bool bHasDinosaurs = false;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && (Actor->GetActorNameOrLabel().Contains("TRex") || 
                     Actor->GetActorNameOrLabel().Contains("Raptor") ||
                     Actor->GetActorNameOrLabel().Contains("Brachio")))
        {
            bHasDinosaurs = true;
            break;
        }
    }
    
    bool bMilestone1Complete = bHasCharacter && bHasTerrain && bHasDinosaurs;
    
    if (bMilestone1Complete)
    {
        LogProductionEvent("MILESTONE 1 COMPLETE: Playable prototype validated");
    }
    else
    {
        FString MissingElements = "";
        if (!bHasCharacter) MissingElements += "Character ";
        if (!bHasTerrain) MissingElements += "Terrain ";
        if (!bHasDinosaurs) MissingElements += "Dinosaurs ";
        
        LogProductionEvent(FString::Printf(TEXT("Milestone 1 incomplete - Missing: %s"), *MissingElements));
    }
    
    return bMilestone1Complete;
}

void UStudioDirectorCoordinationSystem::SetupAgentDependencies()
{
    // Define the dependency chain for the 19-agent pipeline
    LogProductionEvent("Agent dependency chain established");
    LogProductionEvent("Phase 1: Architecture (Agents 1-2)");
    LogProductionEvent("Phase 2: Core Systems (Agents 3-4)");
    LogProductionEvent("Phase 3: World & Characters (Agents 5-10)");
    LogProductionEvent("Phase 4: AI & Behavior (Agents 11-13)");
    LogProductionEvent("Phase 5: Content & Polish (Agents 14-17)");
    LogProductionEvent("Phase 6: QA & Integration (Agents 18-19)");
}