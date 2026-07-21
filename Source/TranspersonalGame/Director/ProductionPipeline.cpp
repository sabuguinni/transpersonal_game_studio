#include "ProductionPipeline.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

AProductionPipeline::AProductionPipeline()
{
    PrimaryActorTick.bCanEverTick = true;
    
    OverallProgress = 0.0f;
    TotalActorsInWorld = 0;
    bPipelineActive = false;
    LastUpdateTime = 0.0f;
}

void AProductionPipeline::BeginPlay()
{
    Super::BeginPlay();
    
    InitializePipeline();
    
    UE_LOG(LogTemp, Warning, TEXT("Production Pipeline initialized with %d agents"), AgentTasks.Num());
}

void AProductionPipeline::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastUpdateTime += DeltaTime;
    
    // Update every 5 seconds
    if (LastUpdateTime >= 5.0f)
    {
        UpdateWorldMetrics();
        CheckDependencies();
        CalculateOverallProgress();
        LastUpdateTime = 0.0f;
    }
}

void AProductionPipeline::InitializePipeline()
{
    // Initialize 19 agent tasks
    AgentTasks.Empty();
    
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
    
    for (int32 i = 0; i < AgentNames.Num(); i++)
    {
        FDir_AgentTask NewTask;
        NewTask.AgentNumber = i + 1;
        NewTask.AgentName = AgentNames[i];
        NewTask.Status = EDir_AgentStatus::Idle;
        NewTask.CurrentStage = EDir_ProductionStage::EngineArchitecture;
        NewTask.ProgressPercentage = 0.0f;
        AgentTasks.Add(NewTask);
    }
    
    SetupAgentDependencies();
    
    // Initialize biome statuses
    BiomeStatuses.Empty();
    
    RegisterBiome(TEXT("Savana"), FVector(0, 0, 0), 500);
    RegisterBiome(TEXT("Pantano"), FVector(-50000, -45000, 0), 500);
    RegisterBiome(TEXT("Floresta"), FVector(-45000, 40000, 0), 500);
    RegisterBiome(TEXT("Deserto"), FVector(55000, 0, 0), 500);
    RegisterBiome(TEXT("Montanha"), FVector(40000, 50000, 0), 500);
    
    bPipelineActive = true;
}

void AProductionPipeline::SetupAgentDependencies()
{
    // Agent #2 (Engine Architect) - no dependencies
    // Agent #3 (Core Systems) depends on #2
    if (AgentTasks.IsValidIndex(2))
    {
        AgentTasks[2].Dependencies.Add(2);
    }
    
    // Agent #4 (Performance) depends on #3
    if (AgentTasks.IsValidIndex(3))
    {
        AgentTasks[3].Dependencies.Add(3);
    }
    
    // Agent #5 (World Gen) depends on #2, #3
    if (AgentTasks.IsValidIndex(4))
    {
        AgentTasks[4].Dependencies.AddUnique(2);
        AgentTasks[4].Dependencies.AddUnique(3);
    }
    
    // Agent #6 (Environment) depends on #5
    if (AgentTasks.IsValidIndex(5))
    {
        AgentTasks[5].Dependencies.Add(5);
    }
    
    // Continue dependency chain...
    for (int32 i = 6; i < AgentTasks.Num(); i++)
    {
        if (i > 0)
        {
            AgentTasks[i].Dependencies.Add(i);
        }
    }
}

void AProductionPipeline::UpdateAgentStatus(int32 AgentNumber, EDir_AgentStatus NewStatus, const FString& TaskDescription)
{
    int32 AgentIndex = AgentNumber - 1;
    if (AgentTasks.IsValidIndex(AgentIndex))
    {
        AgentTasks[AgentIndex].Status = NewStatus;
        AgentTasks[AgentIndex].CurrentTask = TaskDescription;
        AgentTasks[AgentIndex].LastUpdate = FDateTime::Now();
        
        UE_LOG(LogTemp, Log, TEXT("Agent #%d (%s) status updated: %s - %s"), 
               AgentNumber, 
               *AgentTasks[AgentIndex].AgentName,
               *UEnum::GetValueAsString(NewStatus),
               *TaskDescription);
    }
}

void AProductionPipeline::UpdateBiomeStatus(const FString& BiomeName, int32 NewActorCount)
{
    for (FDir_BiomeStatus& Biome : BiomeStatuses)
    {
        if (Biome.BiomeName == BiomeName)
        {
            Biome.ActorCount = NewActorCount;
            Biome.bIsPopulated = (NewActorCount >= Biome.TargetActorCount);
            
            UE_LOG(LogTemp, Log, TEXT("Biome %s updated: %d/%d actors (Populated: %s)"),
                   *BiomeName, NewActorCount, Biome.TargetActorCount,
                   Biome.bIsPopulated ? TEXT("Yes") : TEXT("No"));
            break;
        }
    }
}

bool AProductionPipeline::CanAgentProceed(int32 AgentNumber)
{
    int32 AgentIndex = AgentNumber - 1;
    if (!AgentTasks.IsValidIndex(AgentIndex))
    {
        return false;
    }
    
    // Check if all dependencies are completed
    for (int32 DepAgentNumber : AgentTasks[AgentIndex].Dependencies)
    {
        int32 DepIndex = DepAgentNumber - 1;
        if (AgentTasks.IsValidIndex(DepIndex))
        {
            if (AgentTasks[DepIndex].Status != EDir_AgentStatus::Completed)
            {
                return false;
            }
        }
    }
    
    return true;
}

void AProductionPipeline::AdvanceToNextStage(int32 AgentNumber)
{
    int32 AgentIndex = AgentNumber - 1;
    if (AgentTasks.IsValidIndex(AgentIndex))
    {
        // Mark current stage as completed
        AgentTasks[AgentIndex].Status = EDir_AgentStatus::Completed;
        AgentTasks[AgentIndex].ProgressPercentage = 100.0f;
        
        UE_LOG(LogTemp, Warning, TEXT("Agent #%d (%s) completed stage: %s"), 
               AgentNumber,
               *AgentTasks[AgentIndex].AgentName,
               *UEnum::GetValueAsString(AgentTasks[AgentIndex].CurrentStage));
    }
}

float AProductionPipeline::CalculateOverallProgress()
{
    if (AgentTasks.Num() == 0)
    {
        return 0.0f;
    }
    
    float TotalProgress = 0.0f;
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        TotalProgress += Task.ProgressPercentage;
    }
    
    OverallProgress = TotalProgress / AgentTasks.Num();
    return OverallProgress;
}

void AProductionPipeline::RegisterBiome(const FString& BiomeName, FVector Location, int32 TargetActors)
{
    FDir_BiomeStatus NewBiome;
    NewBiome.BiomeName = BiomeName;
    NewBiome.Location = Location;
    NewBiome.TargetActorCount = TargetActors;
    NewBiome.ActorCount = 0;
    NewBiome.bIsPopulated = false;
    
    BiomeStatuses.Add(NewBiome);
    
    UE_LOG(LogTemp, Log, TEXT("Registered biome: %s at (%f, %f, %f) - Target: %d actors"),
           *BiomeName, Location.X, Location.Y, Location.Z, TargetActors);
}

bool AProductionPipeline::IsBiomePopulated(const FString& BiomeName)
{
    for (const FDir_BiomeStatus& Biome : BiomeStatuses)
    {
        if (Biome.BiomeName == BiomeName)
        {
            return Biome.bIsPopulated;
        }
    }
    return false;
}

TArray<FString> AProductionPipeline::GetUnpopulatedBiomes()
{
    TArray<FString> UnpopulatedBiomes;
    
    for (const FDir_BiomeStatus& Biome : BiomeStatuses)
    {
        if (!Biome.bIsPopulated)
        {
            UnpopulatedBiomes.Add(Biome.BiomeName);
        }
    }
    
    return UnpopulatedBiomes;
}

void AProductionPipeline::GenerateProductionReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION PIPELINE REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Overall Progress: %.1f%%"), OverallProgress);
    UE_LOG(LogTemp, Warning, TEXT("Total Actors in World: %d"), TotalActorsInWorld);
    UE_LOG(LogTemp, Warning, TEXT("Pipeline Active: %s"), bPipelineActive ? TEXT("Yes") : TEXT("No"));
    
    UE_LOG(LogTemp, Warning, TEXT("\n--- AGENT STATUS ---"));
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        UE_LOG(LogTemp, Warning, TEXT("Agent #%d (%s): %s - %.1f%% - %s"),
               Task.AgentNumber,
               *Task.AgentName,
               *UEnum::GetValueAsString(Task.Status),
               Task.ProgressPercentage,
               *Task.CurrentTask);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("\n--- BIOME STATUS ---"));
    for (const FDir_BiomeStatus& Biome : BiomeStatuses)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: %d/%d actors (%s)"),
               *Biome.BiomeName,
               Biome.ActorCount,
               Biome.TargetActorCount,
               Biome.bIsPopulated ? TEXT("POPULATED") : TEXT("NEEDS WORK"));
    }
}

void AProductionPipeline::LogPipelineStatus()
{
    GenerateProductionReport();
}

bool AProductionPipeline::ValidateMinimumViablePrototype()
{
    // Check if basic requirements are met
    bool bHasCharacter = false;
    bool bHasTerrain = false;
    bool bHasLighting = false;
    
    UWorld* World = GetWorld();
    if (World)
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            FString ClassName = Actor->GetClass()->GetName();
            
            if (ClassName.Contains(TEXT("Character")) || ClassName.Contains(TEXT("Pawn")))
            {
                bHasCharacter = true;
            }
            else if (ClassName.Contains(TEXT("Landscape")) || ClassName.Contains(TEXT("Terrain")))
            {
                bHasTerrain = true;
            }
            else if (ClassName.Contains(TEXT("Light")))
            {
                bHasLighting = true;
            }
        }
    }
    
    bool bIsValid = bHasCharacter && bHasTerrain && bHasLighting;
    
    UE_LOG(LogTemp, Warning, TEXT("MVP Validation: Character=%s, Terrain=%s, Lighting=%s -> %s"),
           bHasCharacter ? TEXT("OK") : TEXT("MISSING"),
           bHasTerrain ? TEXT("OK") : TEXT("MISSING"),
           bHasLighting ? TEXT("OK") : TEXT("MISSING"),
           bIsValid ? TEXT("VALID") : TEXT("INVALID"));
    
    return bIsValid;
}

void AProductionPipeline::CheckDependencies()
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Waiting)
        {
            if (CanAgentProceed(Task.AgentNumber))
            {
                Task.Status = EDir_AgentStatus::Idle;
                UE_LOG(LogTemp, Log, TEXT("Agent #%d dependencies satisfied - ready to proceed"), Task.AgentNumber);
            }
        }
    }
}

void AProductionPipeline::UpdateWorldMetrics()
{
    UWorld* World = GetWorld();
    if (World)
    {
        TotalActorsInWorld = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            TotalActorsInWorld++;
        }
        
        // Update biome actor counts
        for (FDir_BiomeStatus& Biome : BiomeStatuses)
        {
            int32 BiomeActorCount = 0;
            for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
            {
                AActor* Actor = *ActorItr;
                FVector ActorLoc = Actor->GetActorLocation();
                float Distance = FVector::Dist(ActorLoc, Biome.Location);
                
                if (Distance <= 10000.0f) // 10km radius
                {
                    BiomeActorCount++;
                }
            }
            
            if (BiomeActorCount != Biome.ActorCount)
            {
                UpdateBiomeStatus(Biome.BiomeName, BiomeActorCount);
            }
        }
    }
}