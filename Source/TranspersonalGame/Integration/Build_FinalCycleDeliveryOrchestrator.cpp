#include "Build_FinalCycleDeliveryOrchestrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"

ABuild_FinalCycleDeliveryOrchestrator::ABuild_FinalCycleDeliveryOrchestrator()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;

    // Initialize delivery state
    bDeliveryOrchestrationComplete = false;
    DeliveryCompletionTime = FDateTime::MinValue();

    // Initialize system definitions
    InitializeAgentSystemDefinitions();
    InitializeBuildComponentDefinitions();
}

void ABuild_FinalCycleDeliveryOrchestrator::BeginPlay()
{
    Super::BeginPlay();
    
    LogDeliveryProgress(TEXT("Final Cycle Delivery Orchestrator initialized"));
    
    // Auto-execute final delivery orchestration
    FTimerHandle TimerHandle;
    GetWorldTimerManager().SetTimer(TimerHandle, this, &ABuild_FinalCycleDeliveryOrchestrator::ExecuteFinalDeliveryOrchestration, 1.0f, false);
}

void ABuild_FinalCycleDeliveryOrchestrator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ABuild_FinalCycleDeliveryOrchestrator::ExecuteFinalDeliveryOrchestration()
{
    LogDeliveryProgress(TEXT("Starting final delivery orchestration for Cycle 006"));

    // Validate all agent integrations
    CurrentDeliveryMetrics = ValidateAllAgentIntegrations();
    
    // Validate build health
    bool bBuildHealthy = ValidateBuildHealth();
    
    // Update world actor count
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        CurrentDeliveryMetrics.WorldActorCount = AllActors.Num();
    }

    // Determine final build status
    CurrentDeliveryMetrics.BuildStatus = DetermineFinalBuildStatus(CurrentDeliveryMetrics);
    CurrentDeliveryMetrics.bProductionReady = IsProductionReady(CurrentDeliveryMetrics);
    CurrentDeliveryMetrics.IntegrationScore = CalculateIntegrationScore(CurrentDeliveryMetrics);

    // Generate final delivery report
    GenerateFinalDeliveryReport();

    // Mark delivery orchestration as complete
    bDeliveryOrchestrationComplete = true;
    DeliveryCompletionTime = FDateTime::Now();

    LogDeliveryProgress(FString::Printf(TEXT("Final delivery orchestration complete - Status: %s"), *CurrentDeliveryMetrics.BuildStatus));
}

FBuild_FinalDeliveryMetrics ABuild_FinalCycleDeliveryOrchestrator::ValidateAllAgentIntegrations()
{
    FBuild_FinalDeliveryMetrics Metrics;
    AgentIntegrationStatuses.Empty();

    LogDeliveryProgress(TEXT("Validating all agent integrations"));

    int32 IntegratedCount = 0;
    int32 TotalCount = AgentSystemClasses.Num();

    for (const auto& AgentPair : AgentSystemClasses)
    {
        FString AgentName = AgentPair.Key;
        TArray<FString> RequiredClasses = AgentPair.Value;

        FBuild_AgentIntegrationStatus Status = ValidateAgentIntegration(AgentName, RequiredClasses);
        AgentIntegrationStatuses.Add(Status);

        if (Status.bFullyIntegrated || Status.LoadedClasses > 0)
        {
            IntegratedCount++;
        }

        LogDeliveryProgress(FString::Printf(TEXT("Agent %s: %d/%d classes integrated"), 
            *AgentName, Status.LoadedClasses, Status.TotalClasses));
    }

    Metrics.IntegratedAgentSystems = IntegratedCount;
    Metrics.TotalAgentSystems = TotalCount;

    return Metrics;
}

bool ABuild_FinalCycleDeliveryOrchestrator::ValidateBuildHealth()
{
    LogDeliveryProgress(TEXT("Validating build health components"));

    int32 HealthyComponents = 0;
    
    for (const FString& Component : CriticalBuildComponents)
    {
        if (ValidateClassExists(Component))
        {
            HealthyComponents++;
            LogDeliveryProgress(FString::Printf(TEXT("Build component OK: %s"), *Component));
        }
        else
        {
            LogDeliveryProgress(FString::Printf(TEXT("Build component MISSING: %s"), *Component));
        }
    }

    CurrentDeliveryMetrics.BuildHealthComponents = HealthyComponents;
    CurrentDeliveryMetrics.TotalBuildComponents = CriticalBuildComponents.Num();

    return HealthyComponents >= 3; // Require at least 3/4 components
}

void ABuild_FinalCycleDeliveryOrchestrator::GenerateFinalDeliveryReport()
{
    LogDeliveryProgress(TEXT("=== FINAL CYCLE 006 DELIVERY REPORT ==="));
    LogDeliveryProgress(FString::Printf(TEXT("World State: %d actors loaded"), CurrentDeliveryMetrics.WorldActorCount));
    LogDeliveryProgress(FString::Printf(TEXT("Agent Integration: %d/%d systems"), 
        CurrentDeliveryMetrics.IntegratedAgentSystems, CurrentDeliveryMetrics.TotalAgentSystems));
    LogDeliveryProgress(FString::Printf(TEXT("Build Health: %d/%d components"), 
        CurrentDeliveryMetrics.BuildHealthComponents, CurrentDeliveryMetrics.TotalBuildComponents));
    LogDeliveryProgress(FString::Printf(TEXT("Integration Score: %.2f"), CurrentDeliveryMetrics.IntegrationScore));
    LogDeliveryProgress(FString::Printf(TEXT("Production Ready: %s"), 
        CurrentDeliveryMetrics.bProductionReady ? TEXT("YES") : TEXT("NO")));
    LogDeliveryProgress(FString::Printf(TEXT("Final Status: %s"), *CurrentDeliveryMetrics.BuildStatus));

    // Log individual agent statuses
    for (const FBuild_AgentIntegrationStatus& Status : AgentIntegrationStatuses)
    {
        LogDeliveryProgress(FString::Printf(TEXT("Agent %s: %d/%d classes (%s)"), 
            *Status.AgentName, Status.LoadedClasses, Status.TotalClasses,
            Status.bFullyIntegrated ? TEXT("FULL") : TEXT("PARTIAL")));
    }

    LogDeliveryProgress(TEXT("=== END DELIVERY REPORT ==="));
}

FBuild_AgentIntegrationStatus ABuild_FinalCycleDeliveryOrchestrator::ValidateAgentIntegration(const FString& AgentName, const TArray<FString>& RequiredClasses)
{
    FBuild_AgentIntegrationStatus Status;
    Status.AgentName = AgentName;
    Status.TotalClasses = RequiredClasses.Num();
    Status.LoadedClasses = 0;
    Status.MissingClasses.Empty();

    for (const FString& ClassName : RequiredClasses)
    {
        if (ValidateClassExists(ClassName))
        {
            Status.LoadedClasses++;
        }
        else
        {
            Status.MissingClasses.Add(ClassName);
        }
    }

    Status.bFullyIntegrated = (Status.LoadedClasses == Status.TotalClasses);

    return Status;
}

TArray<FBuild_AgentIntegrationStatus> ABuild_FinalCycleDeliveryOrchestrator::GetAllAgentIntegrationStatus()
{
    return AgentIntegrationStatuses;
}

FString ABuild_FinalCycleDeliveryOrchestrator::DetermineFinalBuildStatus(const FBuild_FinalDeliveryMetrics& Metrics)
{
    if (Metrics.BuildHealthComponents >= 3 && Metrics.IntegratedAgentSystems >= 6)
    {
        return TEXT("PRODUCTION_READY");
    }
    else if (Metrics.BuildHealthComponents >= 2 && Metrics.IntegratedAgentSystems >= 4)
    {
        return TEXT("STABLE");
    }
    else if (Metrics.BuildHealthComponents >= 1 && Metrics.IntegratedAgentSystems >= 2)
    {
        return TEXT("REQUIRES_ATTENTION");
    }
    else
    {
        return TEXT("CRITICAL");
    }
}

bool ABuild_FinalCycleDeliveryOrchestrator::IsProductionReady(const FBuild_FinalDeliveryMetrics& Metrics)
{
    return Metrics.BuildStatus == TEXT("PRODUCTION_READY") && 
           Metrics.IntegrationScore >= 0.75f &&
           Metrics.WorldActorCount >= 10;
}

float ABuild_FinalCycleDeliveryOrchestrator::CalculateIntegrationScore(const FBuild_FinalDeliveryMetrics& Metrics)
{
    float AgentScore = (float)Metrics.IntegratedAgentSystems / (float)FMath::Max(1, Metrics.TotalAgentSystems);
    float BuildScore = (float)Metrics.BuildHealthComponents / (float)FMath::Max(1, Metrics.TotalBuildComponents);
    float WorldScore = FMath::Clamp((float)Metrics.WorldActorCount / 50.0f, 0.0f, 1.0f);

    return (AgentScore * 0.5f) + (BuildScore * 0.3f) + (WorldScore * 0.2f);
}

void ABuild_FinalCycleDeliveryOrchestrator::InitializeAgentSystemDefinitions()
{
    AgentSystemClasses.Empty();

    // Define required classes for each agent system
    AgentSystemClasses.Add(TEXT("Animation"), {TEXT("AnimationManager"), TEXT("AnimationController")});
    AgentSystemClasses.Add(TEXT("NPC"), {TEXT("NPCBehaviorManager"), TEXT("NPCController")});
    AgentSystemClasses.Add(TEXT("Combat"), {TEXT("CombatManager"), TEXT("CombatSystem")});
    AgentSystemClasses.Add(TEXT("Crowd"), {TEXT("CrowdSimulationManager"), TEXT("CrowdController")});
    AgentSystemClasses.Add(TEXT("Quest"), {TEXT("QuestManager"), TEXT("QuestSystem")});
    AgentSystemClasses.Add(TEXT("Narrative"), {TEXT("NarrativeManager"), TEXT("DialogueSystem")});
    AgentSystemClasses.Add(TEXT("Audio"), {TEXT("AudioManager"), TEXT("AudioSystem")});
    AgentSystemClasses.Add(TEXT("VFX"), {TEXT("VFXManager"), TEXT("VFXSystem")});
    AgentSystemClasses.Add(TEXT("QA"), {TEXT("QATestFramework"), TEXT("QAValidationSystem")});
}

void ABuild_FinalCycleDeliveryOrchestrator::InitializeBuildComponentDefinitions()
{
    CriticalBuildComponents.Empty();

    // Define critical build components
    CriticalBuildComponents.Add(TEXT("BuildIntegrationManager"));
    CriticalBuildComponents.Add(TEXT("BuildValidationSystem"));
    CriticalBuildComponents.Add(TEXT("Build_FinalCycleValidator"));
    CriticalBuildComponents.Add(TEXT("Build_CriticalCompilationValidator"));
}

bool ABuild_FinalCycleDeliveryOrchestrator::ValidateClassExists(const FString& ClassName)
{
    FString FullClassName = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName);
    UClass* FoundClass = FindObject<UClass>(ANY_PACKAGE, *FullClassName);
    return FoundClass != nullptr;
}

void ABuild_FinalCycleDeliveryOrchestrator::LogDeliveryProgress(const FString& Message)
{
    UE_LOG(LogTemp, Warning, TEXT("[FinalDeliveryOrchestrator] %s"), *Message);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, 
            FString::Printf(TEXT("[FinalDelivery] %s"), *Message));
    }
}