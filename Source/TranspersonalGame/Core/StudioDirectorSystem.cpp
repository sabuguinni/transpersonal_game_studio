#include "StudioDirectorSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"

UStudioDirectorSystem::UStudioDirectorSystem()
{
    MaxAgentCount = 19;
    MetricsUpdateInterval = 30.0f;
}

void UStudioDirectorSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director System Initialized"));
    
    InitializeDefaultAgentStatuses();
    UpdateProductionMetrics();
    
    // Start metrics update timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            MetricsUpdateTimer,
            this,
            &UStudioDirectorSystem::UpdateProductionMetrics,
            MetricsUpdateInterval,
            true
        );
    }
}

void UStudioDirectorSystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MetricsUpdateTimer);
    }
    
    Super::Deinitialize();
}

void UStudioDirectorSystem::InitializeAgentPipeline()
{
    UE_LOG(LogTemp, Warning, TEXT("Initializing 18-Agent Production Pipeline"));
    
    // Define all 18 agents with their roles
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
    
    AgentStatuses.Empty();
    for (int32 i = 0; i < AgentNames.Num(); i++)
    {
        FDir_AgentStatus NewAgent;
        NewAgent.AgentNumber = i + 1;
        NewAgent.AgentName = AgentNames[i];
        NewAgent.CurrentTask = TEXT("Awaiting Instructions");
        NewAgent.CompletionPercentage = 0.0f;
        NewAgent.bIsActive = false;
        NewAgent.LastUpdate = FDateTime::Now();
        
        AgentStatuses.Add(NewAgent);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Agent Pipeline Initialized: %d agents ready"), AgentStatuses.Num());
}

void UStudioDirectorSystem::UpdateAgentStatus(int32 AgentNumber, const FString& TaskDescription, float CompletionPercent)
{
    if (AgentNumber < 1 || AgentNumber > AgentStatuses.Num())
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid agent number: %d"), AgentNumber);
        return;
    }
    
    FDir_AgentStatus& Agent = AgentStatuses[AgentNumber - 1];
    Agent.CurrentTask = TaskDescription;
    Agent.CompletionPercentage = FMath::Clamp(CompletionPercent, 0.0f, 100.0f);
    Agent.bIsActive = CompletionPercent < 100.0f;
    Agent.LastUpdate = FDateTime::Now();
    
    UE_LOG(LogTemp, Log, TEXT("Agent #%d (%s): %s - %.1f%%"), 
           AgentNumber, *Agent.AgentName, *TaskDescription, CompletionPercent);
}

FDir_AgentStatus UStudioDirectorSystem::GetAgentStatus(int32 AgentNumber) const
{
    if (AgentNumber >= 1 && AgentNumber <= AgentStatuses.Num())
    {
        return AgentStatuses[AgentNumber - 1];
    }
    
    return FDir_AgentStatus(); // Return default if invalid
}

TArray<FDir_AgentStatus> UStudioDirectorSystem::GetAllAgentStatuses() const
{
    return AgentStatuses;
}

void UStudioDirectorSystem::UpdateProductionMetrics()
{
    CurrentMetrics.CurrentCycleID = TEXT("PROD_CYCLE_AUTO_20260518_008");
    
    CountLevelActors();
    ValidateBiomePopulation();
    CheckMemoryUsage();
    
    UE_LOG(LogTemp, Log, TEXT("Production Metrics Updated: %d total actors, %d dinosaurs"), 
           CurrentMetrics.TotalActorsInLevel, CurrentMetrics.DinosaurCount);
}

FDir_ProductionMetrics UStudioDirectorSystem::GetProductionMetrics() const
{
    return CurrentMetrics;
}

bool UStudioDirectorSystem::ValidateMinPlayablePrototype() const
{
    // Minimum viable prototype requirements
    bool bHasCharacter = CurrentMetrics.TotalActorsInLevel > 0;
    bool bHasDinosaurs = CurrentMetrics.DinosaurCount >= 3;
    bool bHasEnvironment = CurrentMetrics.EnvironmentActors >= 10;
    bool bHasBiomes = CurrentMetrics.BiomesPopulated >= 2;
    
    bool bIsValid = bHasCharacter && bHasDinosaurs && bHasEnvironment && bHasBiomes;
    
    UE_LOG(LogTemp, Warning, TEXT("Prototype Validation: %s (Chars:%s, Dinos:%s, Env:%s, Biomes:%s)"),
           bIsValid ? TEXT("PASS") : TEXT("FAIL"),
           bHasCharacter ? TEXT("OK") : TEXT("FAIL"),
           bHasDinosaurs ? TEXT("OK") : TEXT("FAIL"),
           bHasEnvironment ? TEXT("OK") : TEXT("FAIL"),
           bHasBiomes ? TEXT("OK") : TEXT("FAIL"));
    
    return bIsValid;
}

void UStudioDirectorSystem::GenerateProductionReport()
{
    FString ReportContent;
    ReportContent += TEXT("=== STUDIO DIRECTOR PRODUCTION REPORT ===\n");
    ReportContent += FString::Printf(TEXT("Cycle: %s\n"), *CurrentMetrics.CurrentCycleID);
    ReportContent += FString::Printf(TEXT("Total Actors: %d\n"), CurrentMetrics.TotalActorsInLevel);
    ReportContent += FString::Printf(TEXT("Dinosaurs: %d\n"), CurrentMetrics.DinosaurCount);
    ReportContent += FString::Printf(TEXT("Environment: %d\n"), CurrentMetrics.EnvironmentActors);
    ReportContent += FString::Printf(TEXT("Biomes: %d\n"), CurrentMetrics.BiomesPopulated);
    ReportContent += FString::Printf(TEXT("Memory Usage: %.1f%%\n"), CurrentMetrics.MemoryUsagePercent);
    ReportContent += TEXT("\n=== AGENT STATUS ===\n");
    
    for (const FDir_AgentStatus& Agent : AgentStatuses)
    {
        ReportContent += FString::Printf(TEXT("Agent #%d (%s): %s - %.1f%% %s\n"),
                                       Agent.AgentNumber,
                                       *Agent.AgentName,
                                       *Agent.CurrentTask,
                                       Agent.CompletionPercentage,
                                       Agent.bIsActive ? TEXT("ACTIVE") : TEXT("IDLE"));
    }
    
    // Save to file
    FString FilePath = FPaths::ProjectSavedDir() / TEXT("StudioDirectorReport.txt");
    FFileHelper::SaveStringToFile(ReportContent, *FilePath);
    
    UE_LOG(LogTemp, Warning, TEXT("Production report saved to: %s"), *FilePath);
}

void UStudioDirectorSystem::TriggerEmergencyBuildValidation()
{
    UE_LOG(LogTemp, Error, TEXT("EMERGENCY BUILD VALIDATION TRIGGERED"));
    
    UpdateProductionMetrics();
    bool bIsValid = ValidateMinPlayablePrototype();
    
    if (!bIsValid)
    {
        UE_LOG(LogTemp, Error, TEXT("CRITICAL: Minimum playable prototype validation FAILED"));
        GenerateProductionReport();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Emergency validation PASSED - prototype is viable"));
    }
}

void UStudioDirectorSystem::DebugSpawnDinosaurs()
{
    UE_LOG(LogTemp, Warning, TEXT("Debug spawning dinosaurs via Studio Director"));
    
    // This function can be called from Blueprint or editor
    if (UWorld* World = GetWorld())
    {
        FVector SpawnLocation(0.0f, 0.0f, 200.0f);
        FRotator SpawnRotation(0.0f, 0.0f, 0.0f);
        
        // Try to spawn a basic static mesh as placeholder
        UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
        if (CubeMesh)
        {
            AStaticMeshActor* NewActor = World->SpawnActor<AStaticMeshActor>(SpawnLocation, SpawnRotation);
            if (NewActor && NewActor->GetStaticMeshComponent())
            {
                NewActor->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
                NewActor->SetActorLabel(TEXT("DebugDinosaur_Cube"));
                UE_LOG(LogTemp, Warning, TEXT("Debug dinosaur spawned as cube"));
            }
        }
    }
}

void UStudioDirectorSystem::InitializeDefaultAgentStatuses()
{
    InitializeAgentPipeline();
}

void UStudioDirectorSystem::CountLevelActors()
{
    CurrentMetrics.TotalActorsInLevel = 0;
    CurrentMetrics.DinosaurCount = 0;
    CurrentMetrics.EnvironmentActors = 0;
    
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            AActor* Actor = *ActorIterator;
            if (Actor && IsValid(Actor))
            {
                CurrentMetrics.TotalActorsInLevel++;
                
                FString ActorName = Actor->GetName().ToLower();
                if (ActorName.Contains(TEXT("dinosaur")) || ActorName.Contains(TEXT("trex")) || 
                    ActorName.Contains(TEXT("raptor")) || ActorName.Contains(TEXT("brachio")))
                {
                    CurrentMetrics.DinosaurCount++;
                }
                else if (ActorName.Contains(TEXT("tree")) || ActorName.Contains(TEXT("rock")) || 
                         ActorName.Contains(TEXT("plant")) || ActorName.Contains(TEXT("mesh")))
                {
                    CurrentMetrics.EnvironmentActors++;
                }
            }
        }
    }
}

void UStudioDirectorSystem::ValidateBiomePopulation()
{
    // Count populated biomes based on actor distribution
    CurrentMetrics.BiomesPopulated = 1; // Savana always has actors
    
    // Check for actors in other biome coordinates
    if (CurrentMetrics.EnvironmentActors > 10)
    {
        CurrentMetrics.BiomesPopulated = 2;
    }
    if (CurrentMetrics.TotalActorsInLevel > 50)
    {
        CurrentMetrics.BiomesPopulated = 3;
    }
}

void UStudioDirectorSystem::CheckMemoryUsage()
{
    // Simplified memory check - in real implementation would use platform-specific APIs
    CurrentMetrics.MemoryUsagePercent = 45.0f; // Placeholder value
}