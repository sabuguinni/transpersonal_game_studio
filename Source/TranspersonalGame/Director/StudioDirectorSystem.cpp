#include "StudioDirectorSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/TextRenderComponent.h"
#include "Engine/Font.h"

AStudioDirectorSystem::AStudioDirectorSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize default values
    CurrentCycleID = TEXT("PROD_CYCLE_AUTO_20260514_010");
    TotalCycles = 0;
    BudgetUsed = 90.66f;
    BudgetLimit = 100.0f;
    LastUpdateTime = 0.0f;
    bSystemInitialized = false;
    
    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    // Setup display components
    SetupDisplayComponents();
}

void AStudioDirectorSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize the studio system
    InitializeAgents();
    InitializeBiomes();
    RefreshDisplays();
    bSystemInitialized = true;
    
    LogDirectorMessage(TEXT("Studio Director System Online - Monitoring 19 Agents"));
}

void AStudioDirectorSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastUpdateTime += DeltaTime;
    
    // Update displays every 2 seconds
    if (LastUpdateTime >= 2.0f)
    {
        RefreshDisplays();
        LastUpdateTime = 0.0f;
    }
}

void AStudioDirectorSystem::SetupDisplayComponents()
{
    // Main status display
    MainStatusDisplay = CreateDefaultSubobject<UTextRenderComponent>(TEXT("MainStatusDisplay"));
    MainStatusDisplay->SetupAttachment(RootComponent);
    MainStatusDisplay->SetRelativeLocation(FVector(0, 0, 200));
    MainStatusDisplay->SetWorldSize(300.0f);
    MainStatusDisplay->SetTextRenderColor(FColor::Green);
    MainStatusDisplay->SetHorizontalAlignment(EHTA_Center);
    
    // Agent status display
    AgentStatusDisplay = CreateDefaultSubobject<UTextRenderComponent>(TEXT("AgentStatusDisplay"));
    AgentStatusDisplay->SetupAttachment(RootComponent);
    AgentStatusDisplay->SetRelativeLocation(FVector(-500, 0, 100));
    AgentStatusDisplay->SetWorldSize(150.0f);
    AgentStatusDisplay->SetTextRenderColor(FColor::Cyan);
    
    // Biome status display
    BiomeStatusDisplay = CreateDefaultSubobject<UTextRenderComponent>(TEXT("BiomeStatusDisplay"));
    BiomeStatusDisplay->SetupAttachment(RootComponent);
    BiomeStatusDisplay->SetRelativeLocation(FVector(500, 0, 100));
    BiomeStatusDisplay->SetWorldSize(150.0f);
    BiomeStatusDisplay->SetTextRenderColor(FColor::Yellow);
}

void AStudioDirectorSystem::InitializeAgents()
{
    AgentList.Empty();
    
    // Initialize all 19 agents
    TArray<FString> AgentNames = {
        TEXT("Studio Director"), TEXT("Engine Architect"), TEXT("Core Systems Programmer"),
        TEXT("Performance Optimizer"), TEXT("Procedural World Generator"), TEXT("Environment Artist"),
        TEXT("Architecture & Interior Agent"), TEXT("Lighting & Atmosphere Agent"), TEXT("Character Artist Agent"),
        TEXT("Animation Agent"), TEXT("NPC Behavior Agent"), TEXT("Combat & Enemy AI Agent"),
        TEXT("Crowd & Traffic Simulation"), TEXT("Quest & Mission Designer"), TEXT("Narrative & Dialogue Agent"),
        TEXT("Audio Agent"), TEXT("VFX Agent"), TEXT("QA & Testing Agent"), TEXT("Integration & Build Agent")
    };
    
    for (int32 i = 0; i < AgentNames.Num(); i++)
    {
        FDir_AgentInfo NewAgent;
        NewAgent.AgentID = i + 1;
        NewAgent.AgentName = AgentNames[i];
        NewAgent.Status = EDir_AgentStatus::Idle;
        NewAgent.CurrentTask = TEXT("Awaiting Instructions");
        NewAgent.ProgressPercentage = 0.0f;
        AgentList.Add(NewAgent);
    }
}

void AStudioDirectorSystem::InitializeBiomes()
{
    BiomeList.Empty();
    
    // Initialize the 5 biomes
    FDir_BiomeStatus SwampBiome;
    SwampBiome.BiomeName = TEXT("Swamp SW");
    SwampBiome.Location = FVector(-5000, -5000, 0);
    SwampBiome.bIsReady = true;
    SwampBiome.ResponsibleAgent = TEXT("Environment Artist");
    BiomeList.Add(SwampBiome);
    
    FDir_BiomeStatus ForestBiome;
    ForestBiome.BiomeName = TEXT("Forest NW");
    ForestBiome.Location = FVector(-5000, 5000, 0);
    ForestBiome.bIsReady = true;
    ForestBiome.ResponsibleAgent = TEXT("Environment Artist");
    BiomeList.Add(ForestBiome);
    
    FDir_BiomeStatus SavannaBiome;
    SavannaBiome.BiomeName = TEXT("Savanna Center");
    SavannaBiome.Location = FVector(0, 0, 0);
    SavannaBiome.bIsReady = true;
    SavannaBiome.ResponsibleAgent = TEXT("Procedural World Generator");
    BiomeList.Add(SavannaBiome);
    
    FDir_BiomeStatus DesertBiome;
    DesertBiome.BiomeName = TEXT("Desert E");
    DesertBiome.Location = FVector(5000, 0, 0);
    DesertBiome.bIsReady = false;
    DesertBiome.ResponsibleAgent = TEXT("Environment Artist");
    BiomeList.Add(DesertBiome);
    
    FDir_BiomeStatus MountainBiome;
    MountainBiome.BiomeName = TEXT("Mountains NE");
    MountainBiome.Location = FVector(5000, 5000, 0);
    MountainBiome.bIsReady = false;
    MountainBiome.ResponsibleAgent = TEXT("Procedural World Generator");
    BiomeList.Add(MountainBiome);
}

void AStudioDirectorSystem::UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus, const FString& Task)
{
    for (FDir_AgentInfo& Agent : AgentList)
    {
        if (Agent.AgentID == AgentID)
        {
            Agent.Status = NewStatus;
            Agent.CurrentTask = Task;
            
            // Update progress based on status
            switch (NewStatus)
            {
                case EDir_AgentStatus::Working:
                    Agent.ProgressPercentage = 50.0f;
                    break;
                case EDir_AgentStatus::Completed:
                    Agent.ProgressPercentage = 100.0f;
                    break;
                case EDir_AgentStatus::Failed:
                    Agent.ProgressPercentage = 0.0f;
                    break;
                default:
                    Agent.ProgressPercentage = 0.0f;
                    break;
            }
            
            LogDirectorMessage(FString::Printf(TEXT("Agent #%d (%s) status updated: %s"), 
                AgentID, *Agent.AgentName, *Task));
            break;
        }
    }
}

void AStudioDirectorSystem::UpdateBiomeStatus(const FString& BiomeName, bool bReady, int32 ActorCount)
{
    for (FDir_BiomeStatus& Biome : BiomeList)
    {
        if (Biome.BiomeName == BiomeName)
        {
            Biome.bIsReady = bReady;
            Biome.ActorCount = ActorCount;
            LogDirectorMessage(FString::Printf(TEXT("Biome %s updated: Ready=%s, Actors=%d"), 
                *BiomeName, bReady ? TEXT("Yes") : TEXT("No"), ActorCount));
            break;
        }
    }
}

void AStudioDirectorSystem::RefreshDisplays()
{
    UpdateMainDisplay();
    UpdateAgentDisplay();
    UpdateBiomeDisplay();
}

void AStudioDirectorSystem::UpdateMainDisplay()
{
    if (!MainStatusDisplay) return;
    
    FString MainText = FString::Printf(TEXT("TRANSPERSONAL GAME STUDIO\nSTUDIO DIRECTOR ACTIVE\n\nCYCLE: %s\nBUDGET: $%.2f/$%.2f\nAGENTS: %d TOTAL\nSTATUS: COORDINATING PRODUCTION"),
        *CurrentCycleID, BudgetUsed, BudgetLimit, AgentList.Num());
    
    MainStatusDisplay->SetText(MainText);
}

void AStudioDirectorSystem::UpdateAgentDisplay()
{
    if (!AgentStatusDisplay) return;
    
    FString AgentText = TEXT("=== AGENT STATUS ===\n");
    
    // Show first 10 agents to avoid text overflow
    for (int32 i = 0; i < FMath::Min(10, AgentList.Num()); i++)
    {
        const FDir_AgentInfo& Agent = AgentList[i];
        FString StatusText;
        
        switch (Agent.Status)
        {
            case EDir_AgentStatus::Working:
                StatusText = TEXT("WORK");
                break;
            case EDir_AgentStatus::Completed:
                StatusText = TEXT("DONE");
                break;
            case EDir_AgentStatus::Failed:
                StatusText = TEXT("FAIL");
                break;
            case EDir_AgentStatus::Waiting:
                StatusText = TEXT("WAIT");
                break;
            default:
                StatusText = TEXT("IDLE");
                break;
        }
        
        AgentText += FString::Printf(TEXT("#%02d %s [%s]\n"), 
            Agent.AgentID, *StatusText, *Agent.AgentName.Left(15));
    }
    
    AgentStatusDisplay->SetText(AgentText);
}

void AStudioDirectorSystem::UpdateBiomeDisplay()
{
    if (!BiomeStatusDisplay) return;
    
    FString BiomeText = TEXT("=== BIOME STATUS ===\n");
    
    for (const FDir_BiomeStatus& Biome : BiomeList)
    {
        FString ReadyText = Biome.bIsReady ? TEXT("READY") : TEXT("WORK");
        BiomeText += FString::Printf(TEXT("%s [%s]\nActors: %d\n\n"), 
            *Biome.BiomeName, *ReadyText, Biome.ActorCount);
    }
    
    BiomeStatusDisplay->SetText(BiomeText);
}

void AStudioDirectorSystem::StartNewCycle(const FString& CycleID)
{
    CurrentCycleID = CycleID;
    TotalCycles++;
    
    // Reset all agents to idle for new cycle
    for (FDir_AgentInfo& Agent : AgentList)
    {
        Agent.Status = EDir_AgentStatus::Idle;
        Agent.CurrentTask = TEXT("Awaiting Instructions");
        Agent.ProgressPercentage = 0.0f;
    }
    
    LogDirectorMessage(FString::Printf(TEXT("New cycle started: %s (Total: %d)"), *CycleID, TotalCycles));
}

FString AStudioDirectorSystem::GenerateStatusReport()
{
    FString Report = FString::Printf(TEXT("STUDIO DIRECTOR REPORT\nCycle: %s\nBudget: $%.2f/$%.2f\n\n"), 
        *CurrentCycleID, BudgetUsed, BudgetLimit);
    
    // Agent summary
    int32 WorkingAgents = 0, CompletedAgents = 0, FailedAgents = 0;
    for (const FDir_AgentInfo& Agent : AgentList)
    {
        switch (Agent.Status)
        {
            case EDir_AgentStatus::Working:
                WorkingAgents++;
                break;
            case EDir_AgentStatus::Completed:
                CompletedAgents++;
                break;
            case EDir_AgentStatus::Failed:
                FailedAgents++;
                break;
        }
    }
    
    Report += FString::Printf(TEXT("AGENTS: %d Working, %d Completed, %d Failed\n\n"), 
        WorkingAgents, CompletedAgents, FailedAgents);
    
    // Biome summary
    int32 ReadyBiomes = 0;
    for (const FDir_BiomeStatus& Biome : BiomeList)
    {
        if (Biome.bIsReady) ReadyBiomes++;
    }
    
    Report += FString::Printf(TEXT("BIOMES: %d/%d Ready\n"), ReadyBiomes, BiomeList.Num());
    
    return Report;
}

bool AStudioDirectorSystem::ValidateMinPlayableMap()
{
    // Check if essential components exist
    UWorld* World = GetWorld();
    if (!World) return false;
    
    // Count essential actors
    int32 PlayerStarts = 0;
    int32 Landscapes = 0;
    int32 Lights = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;
        
        FString ClassName = Actor->GetClass()->GetName();
        
        if (ClassName.Contains(TEXT("PlayerStart")))
            PlayerStarts++;
        else if (ClassName.Contains(TEXT("Landscape")))
            Landscapes++;
        else if (ClassName.Contains(TEXT("Light")))
            Lights++;
    }
    
    bool bIsValid = (PlayerStarts > 0) && (Landscapes > 0) && (Lights > 0);
    
    LogDirectorMessage(FString::Printf(TEXT("Map Validation: PlayerStarts=%d, Landscapes=%d, Lights=%d, Valid=%s"), 
        PlayerStarts, Landscapes, Lights, bIsValid ? TEXT("Yes") : TEXT("No")));
    
    return bIsValid;
}

void AStudioDirectorSystem::LogDirectorMessage(const FString& Message)
{
    if (GEngine)
    {
        FString LogMessage = FString::Printf(TEXT("[STUDIO DIRECTOR] %s"), *Message);
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, LogMessage);
        UE_LOG(LogTemp, Warning, TEXT("%s"), *LogMessage);
    }
}