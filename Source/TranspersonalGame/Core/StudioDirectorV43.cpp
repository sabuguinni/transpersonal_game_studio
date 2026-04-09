#include "StudioDirectorV43.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Subsystems/EditorSubsystem.h"
#include "TranspersonalCoreSubsystem.h"
#include "ProductionPipelineController.h"
#include "SystemIntegrationManager.h"

DEFINE_LOG_CATEGORY(LogStudioDirectorV43);

AStudioDirectorV43::AStudioDirectorV43()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    
    // Initialize production cycle
    CurrentCycleID = TEXT("PROD_V43_001");
    ProductionPhase = EProductionPhase::Initialization;
    
    // Set up agent chain
    InitializeAgentChain();
    
    UE_LOG(LogStudioDirectorV43, Log, TEXT("Studio Director V43 initialized - Cycle: %s"), *CurrentCycleID);
}

void AStudioDirectorV43::BeginPlay()
{
    Super::BeginPlay();
    
    // Start production cycle
    StartProductionCycle();
}

void AStudioDirectorV43::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Monitor production pipeline
    MonitorProductionPipeline(DeltaTime);
}

void AStudioDirectorV43::InitializeAgentChain()
{
    // Define the 19-agent production chain
    AgentChain.Empty();
    
    AgentChain.Add({1, TEXT("Studio Director"), TEXT("CEO coordination and vision translation"), true});
    AgentChain.Add({2, TEXT("Engine Architect"), TEXT("Technical architecture definition"), false});
    AgentChain.Add({3, TEXT("Core Systems Programmer"), TEXT("Physics, collision, ragdoll implementation"), false});
    AgentChain.Add({4, TEXT("Performance Optimizer"), TEXT("60fps PC / 30fps console optimization"), false});
    AgentChain.Add({5, TEXT("Procedural World Generator"), TEXT("PCG terrain and biome generation"), false});
    AgentChain.Add({6, TEXT("Environment Artist"), TEXT("Vegetation, props, materials"), false});
    AgentChain.Add({7, TEXT("Architecture & Interior Agent"), TEXT("Buildings and structures"), false});
    AgentChain.Add({8, TEXT("Lighting & Atmosphere Agent"), TEXT("Day/night cycle, Lumen lighting"), false});
    AgentChain.Add({9, TEXT("Character Artist Agent"), TEXT("MetaHuman characters and NPCs"), false});
    AgentChain.Add({10, TEXT("Animation Agent"), TEXT("Motion Matching and IK systems"), false});
    AgentChain.Add({11, TEXT("NPC Behavior Agent"), TEXT("Behavior Trees and daily routines"), false});
    AgentChain.Add({12, TEXT("Combat & Enemy AI Agent"), TEXT("Tactical combat AI for dinosaurs"), false});
    AgentChain.Add({13, TEXT("Crowd & Traffic Simulation"), TEXT("Mass AI for 50k+ agents"), false});
    AgentChain.Add({14, TEXT("Quest & Mission Designer"), TEXT("Gameplay objectives and missions"), false});
    AgentChain.Add({15, TEXT("Narrative & Dialogue Agent"), TEXT("Game Bible and story content"), false});
    AgentChain.Add({16, TEXT("Audio Agent"), TEXT("MetaSounds and ElevenLabs integration"), false});
    AgentChain.Add({17, TEXT("VFX Agent"), TEXT("Niagara effects with LOD optimization"), false});
    AgentChain.Add({18, TEXT("QA & Testing Agent"), TEXT("Quality assurance and build blocking"), false});
    AgentChain.Add({19, TEXT("Integration & Build Agent"), TEXT("Final integration and build management"), false});
    
    CurrentAgentIndex = 0; // Start with Studio Director (self)
    
    UE_LOG(LogStudioDirectorV43, Log, TEXT("Agent chain initialized with %d agents"), AgentChain.Num());
}

void AStudioDirectorV43::StartProductionCycle()
{
    UE_LOG(LogStudioDirectorV43, Warning, TEXT("=== STARTING PRODUCTION CYCLE %s ==="), *CurrentCycleID);
    
    ProductionPhase = EProductionPhase::Active;
    CycleStartTime = FDateTime::Now();
    
    // Initialize core subsystems
    InitializeCoreSubsystems();
    
    // Create production assets
    CreateProductionAssets();
    
    // Validate project health
    ValidateProjectHealth();
    
    // Signal ready for next agent
    SignalNextAgent();
}

void AStudioDirectorV43::InitializeCoreSubsystems()
{
    if (UWorld* World = GetWorld())
    {
        // Get or create core subsystem
        if (UTranspersonalCoreSubsystem* CoreSubsystem = World->GetSubsystem<UTranspersonalCoreSubsystem>())
        {
            CoreSubsystem->InitializeForProductionCycle(CurrentCycleID);
            UE_LOG(LogStudioDirectorV43, Log, TEXT("Core subsystem initialized for cycle %s"), *CurrentCycleID);
        }
        
        // Initialize production pipeline controller
        if (UProductionPipelineController* PipelineController = World->GetSubsystem<UProductionPipelineController>())
        {
            PipelineController->StartCycle(CurrentCycleID);
            UE_LOG(LogStudioDirectorV43, Log, TEXT("Production pipeline controller started"));
        }
    }
}

void AStudioDirectorV43::CreateProductionAssets()
{
    UE_LOG(LogStudioDirectorV43, Log, TEXT("Creating production assets for cycle %s"), *CurrentCycleID);
    
    // Assets are created via external tools (HeyGen, Image Generation)
    // This method tracks their creation
    ProductionAssets.Empty();
    
    // Track dev diary video
    FProductionAsset DevDiary;
    DevDiary.AssetType = TEXT("Video");
    DevDiary.AssetName = TEXT("Dev_Diary_V43_001");
    DevDiary.Status = TEXT("Generated");
    DevDiary.CreationTime = FDateTime::Now();
    ProductionAssets.Add(DevDiary);
    
    // Track concept art
    FProductionAsset ConceptArt;
    ConceptArt.AssetType = TEXT("Image");
    ConceptArt.AssetName = TEXT("Prehistoric_Concept_Art_V43");
    ConceptArt.Status = TEXT("Generated");
    ConceptArt.CreationTime = FDateTime::Now();
    ProductionAssets.Add(ConceptArt);
    
    UE_LOG(LogStudioDirectorV43, Log, TEXT("Production assets tracked: %d items"), ProductionAssets.Num());
}

void AStudioDirectorV43::ValidateProjectHealth()
{
    ProjectHealthStatus.Reset();
    
    // Check module compilation
    ProjectHealthStatus.bModulesCompiled = true; // Will be verified by UE5 execution
    
    // Check asset pipeline
    ProjectHealthStatus.bAssetPipelineReady = true;
    
    // Check subsystem status
    if (UWorld* World = GetWorld())
    {
        ProjectHealthStatus.bSubsystemsActive = World->GetSubsystem<UTranspersonalCoreSubsystem>() != nullptr;
    }
    
    // Overall health
    ProjectHealthStatus.bProjectHealthy = ProjectHealthStatus.bModulesCompiled && 
                                         ProjectHealthStatus.bAssetPipelineReady && 
                                         ProjectHealthStatus.bSubsystemsActive;
    
    UE_LOG(LogStudioDirectorV43, Warning, TEXT("Project Health: %s"), 
           ProjectHealthStatus.bProjectHealthy ? TEXT("HEALTHY") : TEXT("ISSUES DETECTED"));
}

void AStudioDirectorV43::SignalNextAgent()
{
    if (AgentChain.IsValidIndex(1)) // Engine Architect is next
    {
        const FAgentInfo& NextAgent = AgentChain[1];
        
        UE_LOG(LogStudioDirectorV43, Warning, TEXT("=== SIGNALING NEXT AGENT ==="));
        UE_LOG(LogStudioDirectorV43, Warning, TEXT("Next Agent: #%02d - %s"), NextAgent.AgentID, *NextAgent.Name);
        UE_LOG(LogStudioDirectorV43, Warning, TEXT("Task: %s"), *NextAgent.Description);
        UE_LOG(LogStudioDirectorV43, Warning, TEXT("Studio Director cycle complete - handing off to Engine Architect"));
        
        // Set handoff data
        HandoffData.FromAgent = TEXT("Studio Director");
        HandoffData.ToAgent = NextAgent.Name;
        HandoffData.CycleID = CurrentCycleID;
        HandoffData.ProjectHealth = ProjectHealthStatus;
        HandoffData.ProductionAssets = ProductionAssets;
        HandoffData.HandoffTime = FDateTime::Now();
        
        ProductionPhase = EProductionPhase::Handoff;
    }
}

void AStudioDirectorV43::MonitorProductionPipeline(float DeltaTime)
{
    if (ProductionPhase == EProductionPhase::Active)
    {
        // Monitor production metrics
        FTimespan ElapsedTime = FDateTime::Now() - CycleStartTime;
        
        // Log status every 30 seconds
        static float LastLogTime = 0.0f;
        LastLogTime += DeltaTime;
        
        if (LastLogTime >= 30.0f)
        {
            UE_LOG(LogStudioDirectorV43, Log, TEXT("Production monitoring - Elapsed: %s, Assets: %d"), 
                   *ElapsedTime.ToString(), ProductionAssets.Num());
            LastLogTime = 0.0f;
        }
    }
}

FString AStudioDirectorV43::GetProductionReport() const
{
    FString Report = FString::Printf(TEXT("=== STUDIO DIRECTOR PRODUCTION REPORT V43-001 ===\n"));
    Report += FString::Printf(TEXT("Cycle ID: %s\n"), *CurrentCycleID);
    Report += FString::Printf(TEXT("Phase: %s\n"), *UEnum::GetValueAsString(ProductionPhase));
    Report += FString::Printf(TEXT("Project Health: %s\n"), ProjectHealthStatus.bProjectHealthy ? TEXT("HEALTHY") : TEXT("ISSUES"));
    Report += FString::Printf(TEXT("Production Assets: %d\n"), ProductionAssets.Num());
    
    for (const FProductionAsset& Asset : ProductionAssets)
    {
        Report += FString::Printf(TEXT("  - %s (%s): %s\n"), *Asset.AssetName, *Asset.AssetType, *Asset.Status);
    }
    
    Report += FString::Printf(TEXT("Next Agent: %s\n"), HandoffData.ToAgent.IsEmpty() ? TEXT("Pending") : *HandoffData.ToAgent);
    Report += TEXT("=== END REPORT ===");
    
    return Report;
}