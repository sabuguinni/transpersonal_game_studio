#include "DirectorSystemManager.h"
#include "Dir_AgentCoordinator.h"
#include "Dir_ProductionPipeline.h"
#include "Dir_QualityAssurance.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"

UDirectorSystemManager::UDirectorSystemManager()
{
    CurrentStatus = EDir_ProductionStatus::Initializing;
    ActiveAgent = EDir_AgentType::StudioDirector;
    MaxBuildHistoryCount = 10;
    AgentTimeoutSeconds = 300.0f; // 5 minutes default timeout
    bEnableAutomaticQualityChecks = true;
}

void UDirectorSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    LogDirectorActivity("Director System Manager initializing...", EDir_LogLevel::Info);
    
    // Create core components
    AgentCoordinator = NewObject<UDir_AgentCoordinator>(this);
    ProductionPipeline = NewObject<UDir_ProductionPipeline>(this);
    QualityAssurance = NewObject<UDir_QualityAssurance>(this);
    
    if (AgentCoordinator && ProductionPipeline && QualityAssurance)
    {
        // Initialize components
        AgentCoordinator->Initialize(this);
        ProductionPipeline->Initialize(this);
        QualityAssurance->Initialize(this);
        
        // Bind events
        AgentCoordinator->OnAgentCompleted.AddDynamic(this, &UDirectorSystemManager::OnAgentCompleted);
        QualityAssurance->OnQualityCheckFailed.AddDynamic(this, &UDirectorSystemManager::OnQualityCheckFailed);
        ProductionPipeline->OnBuildCreated.AddDynamic(this, &UDirectorSystemManager::OnBuildCreated);
        
        CurrentStatus = EDir_ProductionStatus::Ready;
        LogDirectorActivity("Director System Manager initialized successfully", EDir_LogLevel::Success);
    }
    else
    {
        CurrentStatus = EDir_ProductionStatus::Error;
        LogDirectorActivity("Failed to initialize Director System components", EDir_LogLevel::Error);
    }
}

void UDirectorSystemManager::Deinitialize()
{
    LogDirectorActivity("Director System Manager shutting down...", EDir_LogLevel::Info);
    
    // Cleanup components
    if (AgentCoordinator)
    {
        AgentCoordinator->Shutdown();
        AgentCoordinator = nullptr;
    }
    
    if (ProductionPipeline)
    {
        ProductionPipeline->Shutdown();
        ProductionPipeline = nullptr;
    }
    
    if (QualityAssurance)
    {
        QualityAssurance->Shutdown();
        QualityAssurance = nullptr;
    }
    
    // Clear state
    AgentStatusList.Empty();
    BuildHistory.Empty();
    CurrentStatus = EDir_ProductionStatus::Shutdown;
    
    Super::Deinitialize();
}

bool UDirectorSystemManager::ShouldCreateSubsystem(UObject* Outer) const
{
    // Only create in game worlds, not in editor preview worlds
    if (UWorld* World = Cast<UWorld>(Outer))
    {
        return World->IsGameWorld();
    }
    return false;
}

void UDirectorSystemManager::InitializeProductionPipeline()
{
    if (CurrentStatus != EDir_ProductionStatus::Ready)
    {
        LogDirectorActivity("Cannot initialize production pipeline - system not ready", EDir_LogLevel::Warning);
        return;
    }
    
    LogDirectorActivity("Initializing production pipeline for 18-agent coordination", EDir_LogLevel::Info);
    
    if (ProductionPipeline)
    {
        ProductionPipeline->SetupAgentChain();
        CurrentStatus = EDir_ProductionStatus::PipelineReady;
        
        // Register all 18 agents in the correct order
        RegisterAgent(EDir_AgentType::StudioDirector, "Studio Director - Creative Vision Coordinator");
        RegisterAgent(EDir_AgentType::EngineArchitect, "Engine Architect - Technical Foundation");
        RegisterAgent(EDir_AgentType::CoreSystems, "Core Systems - Physics & Collision");
        RegisterAgent(EDir_AgentType::PerformanceOptimizer, "Performance Optimizer - 60fps Guarantee");
        RegisterAgent(EDir_AgentType::WorldGenerator, "Procedural World Generator - PCG Terrain");
        RegisterAgent(EDir_AgentType::EnvironmentArtist, "Environment Artist - World Population");
        RegisterAgent(EDir_AgentType::ArchitectureAgent, "Architecture Agent - Prehistoric Buildings");
        RegisterAgent(EDir_AgentType::LightingAgent, "Lighting Agent - Lumen & Atmosphere");
        RegisterAgent(EDir_AgentType::CharacterArtist, "Character Artist - MetaHuman Creation");
        RegisterAgent(EDir_AgentType::AnimationAgent, "Animation Agent - Motion Matching & IK");
        RegisterAgent(EDir_AgentType::NPCBehavior, "NPC Behavior - AI Routines & Memory");
        RegisterAgent(EDir_AgentType::CombatAI, "Combat AI - Tactical Dinosaur Behavior");
        RegisterAgent(EDir_AgentType::CrowdSimulation, "Crowd Simulation - Mass AI 50k Agents");
        RegisterAgent(EDir_AgentType::QuestDesigner, "Quest Designer - Mission Implementation");
        RegisterAgent(EDir_AgentType::NarrativeAgent, "Narrative Agent - Story & Dialogue");
        RegisterAgent(EDir_AgentType::AudioAgent, "Audio Agent - MetaSounds & ElevenLabs");
        RegisterAgent(EDir_AgentType::VFXAgent, "VFX Agent - Niagara Effects");
        RegisterAgent(EDir_AgentType::QAAgent, "QA Agent - Quality Assurance & Testing");
        RegisterAgent(EDir_AgentType::IntegrationAgent, "Integration Agent - Final Build Assembly");
        
        LogDirectorActivity("Production pipeline initialized with all 18 agents registered", EDir_LogLevel::Success);
    }
}

void UDirectorSystemManager::ProcessCreativeVision(const FString& VisionDescription)
{
    LogDirectorActivity(FString::Printf(TEXT("Processing creative vision: %s"), *VisionDescription), EDir_LogLevel::Info);
    
    if (ProductionPipeline)
    {
        // Convert creative vision into technical specifications
        FDir_CreativeVision Vision;
        Vision.Description = VisionDescription;
        Vision.Timestamp = FDateTime::Now();
        Vision.Priority = EDir_Priority::High;
        
        ProductionPipeline->ProcessCreativeInput(Vision);
        
        // Start the agent chain
        TriggerAgentSequence(EDir_AgentType::EngineArchitect);
    }
}

bool UDirectorSystemManager::ValidateAgentChain()
{
    if (!AgentCoordinator)
    {
        LogDirectorActivity("Cannot validate agent chain - AgentCoordinator is null", EDir_LogLevel::Error);
        return false;
    }
    
    bool bChainValid = AgentCoordinator->ValidateAgentSequence();
    
    if (bChainValid)
    {
        LogDirectorActivity("Agent chain validation successful - all 18 agents properly configured", EDir_LogLevel::Success);
    }
    else
    {
        LogDirectorActivity("Agent chain validation failed - missing dependencies detected", EDir_LogLevel::Error);
    }
    
    return bChainValid;
}

void UDirectorSystemManager::TriggerAgentSequence(EDir_AgentType StartingAgent)
{
    if (CurrentStatus != EDir_ProductionStatus::PipelineReady)
    {
        LogDirectorActivity("Cannot trigger agent sequence - pipeline not ready", EDir_LogLevel::Warning);
        return;
    }
    
    LogDirectorActivity(FString::Printf(TEXT("Triggering agent sequence starting with: %s"), 
        *UEnum::GetValueAsString(StartingAgent)), EDir_LogLevel::Info);
    
    ActiveAgent = StartingAgent;
    CurrentStatus = EDir_ProductionStatus::AgentActive;
    
    if (AgentCoordinator)
    {
        AgentCoordinator->ActivateAgent(StartingAgent);
        
        // Set timeout for agent completion
        if (UWorld* World = GetWorld())
        {
            FTimerHandle TimeoutHandle;
            World->GetTimerManager().SetTimer(TimeoutHandle, [this, StartingAgent]()
            {
                LogDirectorActivity(FString::Printf(TEXT("Agent timeout: %s"), 
                    *UEnum::GetValueAsString(StartingAgent)), EDir_LogLevel::Error);
                DeactivateAgent(StartingAgent, "Timeout exceeded");
            }, AgentTimeoutSeconds, false);
        }
    }
}

EDir_ProductionStatus UDirectorSystemManager::GetCurrentProductionStatus() const
{
    return CurrentStatus;
}

void UDirectorSystemManager::RegisterAgent(EDir_AgentType AgentType, const FString& AgentIdentifier)
{
    FDir_AgentStatus NewAgent;
    NewAgent.AgentType = AgentType;
    NewAgent.Identifier = AgentIdentifier;
    NewAgent.Status = EDir_AgentStatus::Registered;
    NewAgent.LastActivity = FDateTime::Now();
    
    // Remove existing entry if present
    AgentStatusList.RemoveAll([AgentType](const FDir_AgentStatus& Status)
    {
        return Status.AgentType == AgentType;
    });
    
    AgentStatusList.Add(NewAgent);
    
    LogDirectorActivity(FString::Printf(TEXT("Registered agent: %s"), *AgentIdentifier), EDir_LogLevel::Info);
}

void UDirectorSystemManager::DeactivateAgent(EDir_AgentType AgentType, const FString& Reason)
{
    for (FDir_AgentStatus& Status : AgentStatusList)
    {
        if (Status.AgentType == AgentType)
        {
            Status.Status = EDir_AgentStatus::Inactive;
            Status.LastActivity = FDateTime::Now();
            Status.StatusMessage = Reason;
            break;
        }
    }
    
    LogDirectorActivity(FString::Printf(TEXT("Deactivated agent %s: %s"), 
        *UEnum::GetValueAsString(AgentType), *Reason), EDir_LogLevel::Warning);
}

TArray<FDir_AgentStatus> UDirectorSystemManager::GetActiveAgents() const
{
    TArray<FDir_AgentStatus> ActiveAgents;
    
    for (const FDir_AgentStatus& Status : AgentStatusList)
    {
        if (Status.Status == EDir_AgentStatus::Active || Status.Status == EDir_AgentStatus::Working)
        {
            ActiveAgents.Add(Status);
        }
    }
    
    return ActiveAgents;
}

bool UDirectorSystemManager::CanProceedToNextAgent(EDir_AgentType CurrentAgent) const
{
    if (!AgentCoordinator)
    {
        return false;
    }
    
    // Check if current agent has completed successfully
    for (const FDir_AgentStatus& Status : AgentStatusList)
    {
        if (Status.AgentType == CurrentAgent)
        {
            if (Status.Status != EDir_AgentStatus::Completed)
            {
                return false;
            }
            break;
        }
    }
    
    // Check dependencies for next agent
    return AgentCoordinator->CheckAgentDependencies(CurrentAgent);
}

void UDirectorSystemManager::ReportAgentCompletion(EDir_AgentType CompletedAgent, bool bSuccess, const FString& Output)
{
    for (FDir_AgentStatus& Status : AgentStatusList)
    {
        if (Status.AgentType == CompletedAgent)
        {
            Status.Status = bSuccess ? EDir_AgentStatus::Completed : EDir_AgentStatus::Failed;
            Status.LastActivity = FDateTime::Now();
            Status.StatusMessage = Output;
            break;
        }
    }
    
    if (bSuccess)
    {
        LogDirectorActivity(FString::Printf(TEXT("Agent completed successfully: %s"), 
            *UEnum::GetValueAsString(CompletedAgent)), EDir_LogLevel::Success);
        
        // Trigger quality assessment if enabled
        if (bEnableAutomaticQualityChecks)
        {
            TriggerQualityAssessment();
        }
        
        // Proceed to next agent in sequence
        if (AgentCoordinator)
        {
            EDir_AgentType NextAgent = AgentCoordinator->GetNextAgent(CompletedAgent);
            if (NextAgent != EDir_AgentType::None)
            {
                TriggerAgentSequence(NextAgent);
            }
            else
            {
                // All agents completed - finalize build
                CurrentStatus = EDir_ProductionStatus::BuildReady;
                LogDirectorActivity("All agents completed - build ready for finalization", EDir_LogLevel::Success);
            }
        }
    }
    else
    {
        LogDirectorActivity(FString::Printf(TEXT("Agent failed: %s - %s"), 
            *UEnum::GetValueAsString(CompletedAgent), *Output), EDir_LogLevel::Error);
        CurrentStatus = EDir_ProductionStatus::Error;
    }
}

void UDirectorSystemManager::TriggerQualityAssessment()
{
    if (QualityAssurance)
    {
        LogDirectorActivity("Triggering quality assessment", EDir_LogLevel::Info);
        QualityAssurance->RunQualityCheck(ActiveAgent);
    }
}

void UDirectorSystemManager::CreateBuildSnapshot(const FString& BuildDescription)
{
    FDir_BuildInfo NewBuild;
    NewBuild.BuildId = GenerateBuildIdentifier();
    NewBuild.Description = BuildDescription;
    NewBuild.Timestamp = FDateTime::Now();
    NewBuild.Status = EDir_BuildStatus::InProgress;
    NewBuild.AgentContributions = AgentStatusList;
    
    BuildHistory.Add(NewBuild);
    
    // Cleanup old builds if we exceed the limit
    CleanupOldBuilds();
    
    LogDirectorActivity(FString::Printf(TEXT("Created build snapshot: %s"), *NewBuild.BuildId), EDir_LogLevel::Info);
    
    if (ProductionPipeline)
    {
        ProductionPipeline->CreateBuildSnapshot(NewBuild);
    }
}

bool UDirectorSystemManager::RollbackToPreviousBuild(int32 BuildsBack)
{
    if (BuildHistory.Num() <= BuildsBack)
    {
        LogDirectorActivity("Cannot rollback - insufficient build history", EDir_LogLevel::Error);
        return false;
    }
    
    int32 TargetIndex = BuildHistory.Num() - 1 - BuildsBack;
    const FDir_BuildInfo& TargetBuild = BuildHistory[TargetIndex];
    
    LogDirectorActivity(FString::Printf(TEXT("Rolling back to build: %s"), *TargetBuild.BuildId), EDir_LogLevel::Warning);
    
    if (ProductionPipeline)
    {
        return ProductionPipeline->RollbackToBuild(TargetBuild);
    }
    
    return false;
}

TArray<FDir_BuildInfo> UDirectorSystemManager::GetBuildHistory() const
{
    return BuildHistory;
}

void UDirectorSystemManager::UpdateProductionStatus()
{
    // Update status based on current agent states
    int32 ActiveCount = 0;
    int32 CompletedCount = 0;
    int32 FailedCount = 0;
    
    for (const FDir_AgentStatus& Status : AgentStatusList)
    {
        switch (Status.Status)
        {
            case EDir_AgentStatus::Active:
            case EDir_AgentStatus::Working:
                ActiveCount++;
                break;
            case EDir_AgentStatus::Completed:
                CompletedCount++;
                break;
            case EDir_AgentStatus::Failed:
                FailedCount++;
                break;
        }
    }
    
    if (FailedCount > 0)
    {
        CurrentStatus = EDir_ProductionStatus::Error;
    }
    else if (ActiveCount > 0)
    {
        CurrentStatus = EDir_ProductionStatus::AgentActive;
    }
    else if (CompletedCount == AgentStatusList.Num())
    {
        CurrentStatus = EDir_ProductionStatus::BuildReady;
    }
}

void UDirectorSystemManager::ValidateAgentDependencies()
{
    if (AgentCoordinator)
    {
        AgentCoordinator->ValidateAllDependencies();
    }
}

void UDirectorSystemManager::CleanupOldBuilds()
{
    while (BuildHistory.Num() > MaxBuildHistoryCount)
    {
        BuildHistory.RemoveAt(0);
    }
}

void UDirectorSystemManager::OnAgentCompleted(EDir_AgentType AgentType, bool bSuccess)
{
    // This is called by the AgentCoordinator when an agent finishes
    UpdateProductionStatus();
}

void UDirectorSystemManager::OnQualityCheckFailed(const FString& FailureReason)
{
    LogDirectorActivity(FString::Printf(TEXT("Quality check failed: %s"), *FailureReason), EDir_LogLevel::Error);
    CurrentStatus = EDir_ProductionStatus::QualityCheckFailed;
}

void UDirectorSystemManager::OnBuildCreated(const FDir_BuildInfo& NewBuild)
{
    LogDirectorActivity(FString::Printf(TEXT("Build created: %s"), *NewBuild.BuildId), EDir_LogLevel::Success);
}

bool UDirectorSystemManager::IsAgentSequenceValid() const
{
    return AgentCoordinator ? AgentCoordinator->ValidateAgentSequence() : false;
}

FString UDirectorSystemManager::GenerateBuildIdentifier() const
{
    FDateTime Now = FDateTime::Now();
    return FString::Printf(TEXT("BUILD_%04d%02d%02d_%02d%02d%02d"), 
        Now.GetYear(), Now.GetMonth(), Now.GetDay(),
        Now.GetHour(), Now.GetMinute(), Now.GetSecond());
}

void UDirectorSystemManager::LogDirectorActivity(const FString& Activity, EDir_LogLevel LogLevel)
{
    FString LogPrefix;
    switch (LogLevel)
    {
        case EDir_LogLevel::Info:
            LogPrefix = TEXT("[DIRECTOR-INFO]");
            break;
        case EDir_LogLevel::Warning:
            LogPrefix = TEXT("[DIRECTOR-WARN]");
            break;
        case EDir_LogLevel::Error:
            LogPrefix = TEXT("[DIRECTOR-ERROR]");
            break;
        case EDir_LogLevel::Success:
            LogPrefix = TEXT("[DIRECTOR-SUCCESS]");
            break;
    }
    
    FString FullMessage = FString::Printf(TEXT("%s %s"), *LogPrefix, *Activity);
    
    // Log to UE5 console
    UE_LOG(LogTemp, Log, TEXT("%s"), *FullMessage);
    
    // Also log to screen if in development
    if (GEngine && LogLevel == EDir_LogLevel::Error)
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FullMessage);
    }
    else if (GEngine && LogLevel == EDir_LogLevel::Success)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FullMessage);
    }
}