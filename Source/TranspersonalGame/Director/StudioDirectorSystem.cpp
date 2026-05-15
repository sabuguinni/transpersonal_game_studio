#include "StudioDirectorSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

AStudioDirectorSystem::AStudioDirectorSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;
    
    CurrentCycleID = TEXT("PROD_CYCLE_AUTO_20260515_005");
    bIsProductionActive = true;
    LastUpdateTime = 0.0f;
}

void AStudioDirectorSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAgentSystem();
    InitializeBiomeData();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director System initialized for cycle: %s"), *CurrentCycleID);
}

void AStudioDirectorSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastUpdateTime += DeltaTime;
    
    if (LastUpdateTime >= UpdateInterval)
    {
        UpdateWorldMetrics();
        LastUpdateTime = 0.0f;
    }
}

void AStudioDirectorSystem::InitializeAgentSystem()
{
    AgentList.Empty();
    
    // Initialize all 19 agents
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
        FDir_AgentInfo NewAgent;
        NewAgent.AgentNumber = i + 1;
        NewAgent.AgentName = AgentNames[i];
        NewAgent.Status = EDir_AgentStatus::Idle;
        NewAgent.CurrentTask = TEXT("Awaiting assignment");
        NewAgent.ProgressPercentage = 0.0f;
        NewAgent.LastUpdate = FDateTime::Now();
        
        AgentList.Add(NewAgent);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Initialized %d agents in the studio system"), AgentList.Num());
}

void AStudioDirectorSystem::InitializeBiomeData()
{
    BiomeList.Empty();
    
    // Initialize the 5 biomes with their correct coordinates
    TArray<TPair<EDir_BiomeType, FVector>> BiomeData = {
        {EDir_BiomeType::Savanna, FVector(0, 0, 0)},
        {EDir_BiomeType::Swamp, FVector(-50000, -45000, 0)},
        {EDir_BiomeType::Forest, FVector(-45000, 40000, 0)},
        {EDir_BiomeType::Desert, FVector(55000, 0, 0)},
        {EDir_BiomeType::Mountain, FVector(40000, 50000, 0)}
    };
    
    for (const auto& BiomePair : BiomeData)
    {
        FDir_BiomeInfo NewBiome;
        NewBiome.BiomeType = BiomePair.Key;
        NewBiome.CenterLocation = BiomePair.Value;
        NewBiome.ActorCount = 0;
        NewBiome.bIsPopulated = false;
        
        // Set required assets for each biome
        switch (BiomePair.Key)
        {
            case EDir_BiomeType::Savanna:
                NewBiome.RequiredAssets = {TEXT("Acacia Trees"), TEXT("Grass Patches"), TEXT("Rocks"), TEXT("Dinosaurs")};
                break;
            case EDir_BiomeType::Swamp:
                NewBiome.RequiredAssets = {TEXT("Cypress Trees"), TEXT("Swamp Vegetation"), TEXT("Water Plants"), TEXT("Amphibians")};
                break;
            case EDir_BiomeType::Forest:
                NewBiome.RequiredAssets = {TEXT("Conifer Trees"), TEXT("Ferns"), TEXT("Fallen Logs"), TEXT("Forest Dinosaurs")};
                break;
            case EDir_BiomeType::Desert:
                NewBiome.RequiredAssets = {TEXT("Cacti"), TEXT("Sand Dunes"), TEXT("Desert Rocks"), TEXT("Desert Creatures")};
                break;
            case EDir_BiomeType::Mountain:
                NewBiome.RequiredAssets = {TEXT("Pine Trees"), TEXT("Rocky Outcrops"), TEXT("Mountain Vegetation"), TEXT("Flying Dinosaurs")};
                break;
        }
        
        BiomeList.Add(NewBiome);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Initialized %d biomes in the world system"), BiomeList.Num());
}

void AStudioDirectorSystem::UpdateAgentStatus(int32 AgentNumber, EDir_AgentStatus NewStatus, const FString& TaskDescription)
{
    for (FDir_AgentInfo& Agent : AgentList)
    {
        if (Agent.AgentNumber == AgentNumber)
        {
            Agent.Status = NewStatus;
            Agent.CurrentTask = TaskDescription;
            Agent.LastUpdate = FDateTime::Now();
            
            UE_LOG(LogTemp, Log, TEXT("Agent %d (%s) status updated: %s"), 
                   AgentNumber, *Agent.AgentName, *TaskDescription);
            break;
        }
    }
}

void AStudioDirectorSystem::UpdateBiomeInfo(EDir_BiomeType BiomeType, int32 NewActorCount, bool bPopulated)
{
    for (FDir_BiomeInfo& Biome : BiomeList)
    {
        if (Biome.BiomeType == BiomeType)
        {
            Biome.ActorCount = NewActorCount;
            Biome.bIsPopulated = bPopulated;
            
            UE_LOG(LogTemp, Log, TEXT("Biome updated: Type %d, Actors: %d, Populated: %s"), 
                   (int32)BiomeType, NewActorCount, bPopulated ? TEXT("Yes") : TEXT("No"));
            break;
        }
    }
}

FDir_ProductionMetrics AStudioDirectorSystem::CalculateProductionMetrics()
{
    UpdateWorldMetrics();
    return ProductionMetrics;
}

void AStudioDirectorSystem::UpdateWorldMetrics()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Count total actors in world
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    ProductionMetrics.TotalActorsInWorld = AllActors.Num();
    
    // Count active agents
    ProductionMetrics.ActiveAgents = 0;
    for (const FDir_AgentInfo& Agent : AgentList)
    {
        if (Agent.Status == EDir_AgentStatus::Working)
        {
            ProductionMetrics.ActiveAgents++;
        }
    }
    
    // Count completed systems (simplified metric)
    ProductionMetrics.CompletedSystems = 0;
    for (const FDir_AgentInfo& Agent : AgentList)
    {
        if (Agent.Status == EDir_AgentStatus::Completed)
        {
            ProductionMetrics.CompletedSystems++;
        }
    }
    
    // Calculate overall progress
    if (AgentList.Num() > 0)
    {
        ProductionMetrics.OverallProgress = (float)ProductionMetrics.CompletedSystems / (float)AgentList.Num() * 100.0f;
    }
    
    // Update biome actor counts
    for (FDir_BiomeInfo& Biome : BiomeList)
    {
        int32 BiomeActorCount = 0;
        FVector BiomeCenter = Biome.CenterLocation;
        
        for (AActor* Actor : AllActors)
        {
            if (Actor && Actor != this)
            {
                FVector ActorLocation = Actor->GetActorLocation();
                float Distance = FVector::Dist(ActorLocation, BiomeCenter);
                
                if (Distance < 25000.0f) // 25km radius for biome
                {
                    BiomeActorCount++;
                }
            }
        }
        
        Biome.ActorCount = BiomeActorCount;
        Biome.bIsPopulated = BiomeActorCount >= 500; // Criterion from memory
    }
}

void AStudioDirectorSystem::LogProductionStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== STUDIO DIRECTOR PRODUCTION STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Cycle ID: %s"), *CurrentCycleID);
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), ProductionMetrics.TotalActorsInWorld);
    UE_LOG(LogTemp, Warning, TEXT("Active Agents: %d"), ProductionMetrics.ActiveAgents);
    UE_LOG(LogTemp, Warning, TEXT("Completed Systems: %d"), ProductionMetrics.CompletedSystems);
    UE_LOG(LogTemp, Warning, TEXT("Overall Progress: %.1f%%"), ProductionMetrics.OverallProgress);
    UE_LOG(LogTemp, Warning, TEXT("Current Milestone: %s"), *ProductionMetrics.CurrentMilestone);
    
    UE_LOG(LogTemp, Warning, TEXT("=== BIOME STATUS ==="));
    for (const FDir_BiomeInfo& Biome : BiomeList)
    {
        FString BiomeName;
        switch (Biome.BiomeType)
        {
            case EDir_BiomeType::Savanna: BiomeName = TEXT("Savanna"); break;
            case EDir_BiomeType::Swamp: BiomeName = TEXT("Swamp"); break;
            case EDir_BiomeType::Forest: BiomeName = TEXT("Forest"); break;
            case EDir_BiomeType::Desert: BiomeName = TEXT("Desert"); break;
            case EDir_BiomeType::Mountain: BiomeName = TEXT("Mountain"); break;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("%s: %d actors, Populated: %s"), 
               *BiomeName, Biome.ActorCount, Biome.bIsPopulated ? TEXT("YES") : TEXT("NO"));
    }
}

TArray<FDir_AgentInfo> AStudioDirectorSystem::GetActiveAgents()
{
    TArray<FDir_AgentInfo> ActiveAgents;
    
    for (const FDir_AgentInfo& Agent : AgentList)
    {
        if (Agent.Status == EDir_AgentStatus::Working || Agent.Status == EDir_AgentStatus::Waiting)
        {
            ActiveAgents.Add(Agent);
        }
    }
    
    return ActiveAgents;
}

bool AStudioDirectorSystem::ValidateMinimumViablePrototype()
{
    // Check for basic requirements of Milestone 1
    bool bHasCharacter = false;
    bool bHasTerrain = false;
    bool bHasDinosaurs = false;
    bool bHasLighting = false;
    
    if (!GetWorld())
    {
        return false;
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (!Actor) continue;
        
        FString ActorName = Actor->GetName().ToLower();
        
        if (ActorName.Contains(TEXT("character")) || ActorName.Contains(TEXT("player")))
        {
            bHasCharacter = true;
        }
        else if (ActorName.Contains(TEXT("landscape")) || ActorName.Contains(TEXT("terrain")))
        {
            bHasTerrain = true;
        }
        else if (ActorName.Contains(TEXT("dino")) || ActorName.Contains(TEXT("rex")) || ActorName.Contains(TEXT("raptor")))
        {
            bHasDinosaurs = true;
        }
        else if (ActorName.Contains(TEXT("light")) || ActorName.Contains(TEXT("sun")))
        {
            bHasLighting = true;
        }
    }
    
    bool bIsValid = bHasCharacter && bHasTerrain && bHasDinosaurs && bHasLighting;
    
    UE_LOG(LogTemp, Warning, TEXT("MVP Validation - Character: %s, Terrain: %s, Dinosaurs: %s, Lighting: %s, Overall: %s"),
           bHasCharacter ? TEXT("OK") : TEXT("MISSING"),
           bHasTerrain ? TEXT("OK") : TEXT("MISSING"),
           bHasDinosaurs ? TEXT("OK") : TEXT("MISSING"),
           bHasLighting ? TEXT("OK") : TEXT("MISSING"),
           bIsValid ? TEXT("VALID") : TEXT("INVALID"));
    
    return bIsValid;
}

void AStudioDirectorSystem::RefreshWorldState()
{
    UpdateWorldMetrics();
    LogProductionStatus();
    ValidateMinimumViablePrototype();
}

void AStudioDirectorSystem::SetCurrentCycle(const FString& CycleID)
{
    CurrentCycleID = CycleID;
    UE_LOG(LogTemp, Warning, TEXT("Studio Director cycle updated to: %s"), *CurrentCycleID);
}