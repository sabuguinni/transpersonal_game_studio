#include "Build_FinalSystemOrchestrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UBuild_FinalSystemOrchestrator::UBuild_FinalSystemOrchestrator()
{
    CurrentMetrics = FBuild_SystemIntegrationMetrics();
    CycleData = FBuild_CycleCompletionData();
    IntegrationStartTime = 0.0f;
    bOrchestrationActive = false;
}

void UBuild_FinalSystemOrchestrator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalSystemOrchestrator: Initializing final system orchestration"));
    
    InitializeSystemOrchestration();
}

void UBuild_FinalSystemOrchestrator::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalSystemOrchestrator: Deinitializing orchestration system"));
    
    bOrchestrationActive = false;
    ManagedActors.Empty();
    SystemValidationResults.Empty();
    
    Super::Deinitialize();
}

void UBuild_FinalSystemOrchestrator::InitializeSystemOrchestration()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalSystemOrchestrator: Starting system orchestration initialization"));
    
    IntegrationStartTime = FPlatformTime::Seconds();
    bOrchestrationActive = true;
    
    // Initialize cycle data
    CycleData.CycleNumber = 7; // Current cycle
    CycleData.CycleExecutionTime = 0.0f;
    CycleData.SystemsIntegrated = 0;
    CycleData.ValidationTestsPassed = 0;
    CycleData.bCycleSuccessful = false;
    CycleData.CycleCompletionReport = TEXT("Initializing final cycle orchestration...");
    
    // Initialize metrics
    CurrentMetrics.TotalActorsInWorld = 0;
    CurrentMetrics.VFXSystemsActive = 0;
    CurrentMetrics.QAValidationsPassed = 0;
    CurrentMetrics.SystemIntegrationScore = 0.0f;
    CurrentMetrics.bAllCriticalSystemsOnline = false;
    CurrentMetrics.LastIntegrationTimestamp = FDateTime::Now().ToString();
    CurrentMetrics.ActiveSystemNames.Empty();
    CurrentMetrics.FailedSystemNames.Empty();
    
    UpdateIntegrationMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalSystemOrchestrator: System orchestration initialized successfully"));
}

void UBuild_FinalSystemOrchestrator::ExecuteFinalCycleIntegration()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalSystemOrchestrator: Executing final cycle integration"));
    
    if (!bOrchestrationActive)
    {
        UE_LOG(LogTemp, Error, TEXT("Build_FinalSystemOrchestrator: Cannot execute integration - orchestration not active"));
        return;
    }
    
    // Update cycle execution time
    CycleData.CycleExecutionTime = FPlatformTime::Seconds() - IntegrationStartTime;
    
    // Validate all systems
    ValidateAllSystemsIntegration();
    
    // Orchestrate cross-system integration
    OrchestrateCrossSystemIntegration();
    
    // Update metrics
    UpdateIntegrationMetrics();
    
    // Generate integration report
    GenerateIntegrationReport();
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalSystemOrchestrator: Final cycle integration completed"));
}

void UBuild_FinalSystemOrchestrator::ValidateAllSystemsIntegration()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalSystemOrchestrator: Validating all systems integration"));
    
    SystemValidationResults.Empty();
    CurrentMetrics.ActiveSystemNames.Empty();
    CurrentMetrics.FailedSystemNames.Empty();
    
    // Validate VFX systems
    ValidateVFXSystems();
    
    // Validate QA systems
    ValidateQASystems();
    
    // Validate world systems
    ValidateWorldSystems();
    
    // Calculate overall integration score
    int32 PassedValidations = 0;
    int32 TotalValidations = SystemValidationResults.Num();
    
    for (const auto& ValidationPair : SystemValidationResults)
    {
        if (ValidationPair.Value)
        {
            PassedValidations++;
            CurrentMetrics.ActiveSystemNames.Add(ValidationPair.Key);
        }
        else
        {
            CurrentMetrics.FailedSystemNames.Add(ValidationPair.Key);
        }
    }
    
    CurrentMetrics.QAValidationsPassed = PassedValidations;
    CurrentMetrics.SystemIntegrationScore = TotalValidations > 0 ? (float)PassedValidations / TotalValidations * 100.0f : 0.0f;
    CurrentMetrics.bAllCriticalSystemsOnline = (PassedValidations >= TotalValidations * 0.8f); // 80% threshold
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalSystemOrchestrator: System validation completed - Score: %.2f%%"), CurrentMetrics.SystemIntegrationScore);
}

void UBuild_FinalSystemOrchestrator::GenerateIntegrationReport()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalSystemOrchestrator: Generating comprehensive integration report"));
    
    FString Report = TEXT("=== FINAL CYCLE INTEGRATION REPORT ===\n");
    Report += FString::Printf(TEXT("Cycle Number: %d\n"), CycleData.CycleNumber);
    Report += FString::Printf(TEXT("Execution Time: %.2f seconds\n"), CycleData.CycleExecutionTime);
    Report += FString::Printf(TEXT("Total Actors: %d\n"), CurrentMetrics.TotalActorsInWorld);
    Report += FString::Printf(TEXT("VFX Systems Active: %d\n"), CurrentMetrics.VFXSystemsActive);
    Report += FString::Printf(TEXT("Integration Score: %.2f%%\n"), CurrentMetrics.SystemIntegrationScore);
    Report += FString::Printf(TEXT("Critical Systems Online: %s\n"), CurrentMetrics.bAllCriticalSystemsOnline ? TEXT("YES") : TEXT("NO"));
    
    Report += TEXT("\nActive Systems:\n");
    for (const FString& SystemName : CurrentMetrics.ActiveSystemNames)
    {
        Report += FString::Printf(TEXT("  ✓ %s\n"), *SystemName);
    }
    
    if (CurrentMetrics.FailedSystemNames.Num() > 0)
    {
        Report += TEXT("\nFailed Systems:\n");
        for (const FString& SystemName : CurrentMetrics.FailedSystemNames)
        {
            Report += FString::Printf(TEXT("  ✗ %s\n"), *SystemName);
        }
    }
    
    CycleData.CycleCompletionReport = Report;
    CycleData.bCycleSuccessful = CurrentMetrics.bAllCriticalSystemsOnline;
    CycleData.SystemsIntegrated = CurrentMetrics.ActiveSystemNames.Num();
    CycleData.ValidationTestsPassed = CurrentMetrics.QAValidationsPassed;
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalSystemOrchestrator Integration Report:\n%s"), *Report);
}

void UBuild_FinalSystemOrchestrator::CompleteCycleOrchestration()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalSystemOrchestrator: Completing cycle orchestration"));
    
    // Final validation
    ValidateAllSystemsIntegration();
    
    // Generate final report
    GenerateIntegrationReport();
    
    // Finalize deployment
    FinalizeSystemDeployment();
    
    // Update timestamp
    CurrentMetrics.LastIntegrationTimestamp = FDateTime::Now().ToString();
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalSystemOrchestrator: Cycle orchestration completed successfully"));
}

FBuild_SystemIntegrationMetrics UBuild_FinalSystemOrchestrator::GetCurrentIntegrationMetrics() const
{
    return CurrentMetrics;
}

FBuild_CycleCompletionData UBuild_FinalSystemOrchestrator::GetCycleCompletionData() const
{
    return CycleData;
}

bool UBuild_FinalSystemOrchestrator::ValidateSystemDependencies()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalSystemOrchestrator: Validating system dependencies"));
    
    // Check for critical system dependencies
    bool bDependenciesValid = true;
    
    // Validate world exists
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Build_FinalSystemOrchestrator: World not available"));
        bDependenciesValid = false;
    }
    
    // Validate game instance
    UGameInstance* GameInstance = GetGameInstance();
    if (!GameInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("Build_FinalSystemOrchestrator: GameInstance not available"));
        bDependenciesValid = false;
    }
    
    return bDependenciesValid;
}

void UBuild_FinalSystemOrchestrator::OrchestrateCrossSystemIntegration()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalSystemOrchestrator: Orchestrating cross-system integration"));
    
    // Ensure system dependencies are valid
    if (!ValidateSystemDependencies())
    {
        UE_LOG(LogTemp, Error, TEXT("Build_FinalSystemOrchestrator: System dependencies validation failed"));
        return;
    }
    
    // Update managed actors list
    UWorld* World = GetWorld();
    if (World)
    {
        ManagedActors.Empty();
        for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            AActor* Actor = *ActorIterator;
            if (Actor && IsValid(Actor))
            {
                ManagedActors.Add(Actor);
            }
        }
        
        CurrentMetrics.TotalActorsInWorld = ManagedActors.Num();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalSystemOrchestrator: Cross-system integration orchestrated - Managing %d actors"), ManagedActors.Num());
}

void UBuild_FinalSystemOrchestrator::FinalizeSystemDeployment()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalSystemOrchestrator: Finalizing system deployment"));
    
    // Mark orchestration as complete
    bOrchestrationActive = false;
    
    // Log final status
    LogIntegrationStatus();
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalSystemOrchestrator: System deployment finalized"));
}

void UBuild_FinalSystemOrchestrator::UpdateIntegrationMetrics()
{
    UWorld* World = GetWorld();
    if (World)
    {
        // Count total actors
        int32 ActorCount = 0;
        for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            if (IsValid(*ActorIterator))
            {
                ActorCount++;
            }
        }
        CurrentMetrics.TotalActorsInWorld = ActorCount;
    }
    
    // Update timestamp
    CurrentMetrics.LastIntegrationTimestamp = FDateTime::Now().ToString();
}

void UBuild_FinalSystemOrchestrator::ValidateVFXSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalSystemOrchestrator: Validating VFX systems"));
    
    // Check for VFX-related actors and components
    int32 VFXSystemCount = 0;
    UWorld* World = GetWorld();
    if (World)
    {
        // Count particle systems and VFX actors
        for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            AActor* Actor = *ActorIterator;
            if (Actor && IsValid(Actor))
            {
                FString ActorName = Actor->GetName();
                if (ActorName.Contains(TEXT("VFX")) || ActorName.Contains(TEXT("Particle")) || ActorName.Contains(TEXT("Effect")))
                {
                    VFXSystemCount++;
                }
            }
        }
    }
    
    CurrentMetrics.VFXSystemsActive = VFXSystemCount;
    SystemValidationResults.Add(TEXT("VFXSystems"), VFXSystemCount > 0);
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalSystemOrchestrator: VFX validation completed - %d systems found"), VFXSystemCount);
}

void UBuild_FinalSystemOrchestrator::ValidateQASystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalSystemOrchestrator: Validating QA systems"));
    
    // Check for QA-related systems
    bool bQASystemsValid = true;
    
    // Validate QA subsystems exist
    UWorld* World = GetWorld();
    if (World)
    {
        // Check for QA actors
        bool bQAActorsFound = false;
        for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            AActor* Actor = *ActorIterator;
            if (Actor && IsValid(Actor))
            {
                FString ActorName = Actor->GetName();
                if (ActorName.Contains(TEXT("QA")) || ActorName.Contains(TEXT("Test")) || ActorName.Contains(TEXT("Validation")))
                {
                    bQAActorsFound = true;
                    break;
                }
            }
        }
        bQASystemsValid = bQAActorsFound;
    }
    
    SystemValidationResults.Add(TEXT("QASystems"), bQASystemsValid);
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalSystemOrchestrator: QA validation completed - Valid: %s"), bQASystemsValid ? TEXT("YES") : TEXT("NO"));
}

void UBuild_FinalSystemOrchestrator::ValidateWorldSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalSystemOrchestrator: Validating world systems"));
    
    bool bWorldSystemsValid = true;
    
    UWorld* World = GetWorld();
    if (!World)
    {
        bWorldSystemsValid = false;
    }
    else
    {
        // Check for essential world components
        bool bLightingFound = false;
        bool bTerrainFound = false;
        
        for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            AActor* Actor = *ActorIterator;
            if (Actor && IsValid(Actor))
            {
                FString ActorName = Actor->GetName();
                if (ActorName.Contains(TEXT("Light")) || ActorName.Contains(TEXT("Sun")))
                {
                    bLightingFound = true;
                }
                if (ActorName.Contains(TEXT("Landscape")) || ActorName.Contains(TEXT("Terrain")))
                {
                    bTerrainFound = true;
                }
            }
        }
        
        bWorldSystemsValid = bLightingFound && bTerrainFound;
    }
    
    SystemValidationResults.Add(TEXT("WorldSystems"), bWorldSystemsValid);
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalSystemOrchestrator: World validation completed - Valid: %s"), bWorldSystemsValid ? TEXT("YES") : TEXT("NO"));
}

void UBuild_FinalSystemOrchestrator::GenerateSystemReport()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalSystemOrchestrator: Generating detailed system report"));
    
    FString DetailedReport = TEXT("=== DETAILED SYSTEM REPORT ===\n");
    DetailedReport += FString::Printf(TEXT("Orchestration Active: %s\n"), bOrchestrationActive ? TEXT("YES") : TEXT("NO"));
    DetailedReport += FString::Printf(TEXT("Managed Actors: %d\n"), ManagedActors.Num());
    DetailedReport += FString::Printf(TEXT("Validation Results: %d\n"), SystemValidationResults.Num());
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalSystemOrchestrator System Report:\n%s"), *DetailedReport);
}

void UBuild_FinalSystemOrchestrator::LogIntegrationStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== FINAL INTEGRATION STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Integration Score: %.2f%%"), CurrentMetrics.SystemIntegrationScore);
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), CurrentMetrics.TotalActorsInWorld);
    UE_LOG(LogTemp, Warning, TEXT("VFX Systems: %d"), CurrentMetrics.VFXSystemsActive);
    UE_LOG(LogTemp, Warning, TEXT("QA Validations Passed: %d"), CurrentMetrics.QAValidationsPassed);
    UE_LOG(LogTemp, Warning, TEXT("Critical Systems Online: %s"), CurrentMetrics.bAllCriticalSystemsOnline ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Cycle Successful: %s"), CycleData.bCycleSuccessful ? TEXT("YES") : TEXT("NO"));
}

// Actor Implementation
ABuild_IntegrationOrchestratorActor::ABuild_IntegrationOrchestratorActor()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create root component
    IntegrationRoot = CreateDefaultSubobject<USceneComponent>(TEXT("IntegrationRoot"));
    RootComponent = IntegrationRoot;
    
    // Create orchestrator mesh
    OrchestratorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OrchestratorMesh"));
    OrchestratorMesh->SetupAttachment(RootComponent);
    
    OrchestratorSubsystem = nullptr;
}

void ABuild_IntegrationOrchestratorActor::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Build_IntegrationOrchestratorActor: Beginning play"));
    
    // Get orchestrator subsystem
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        OrchestratorSubsystem = GameInstance->GetSubsystem<UBuild_FinalSystemOrchestrator>();
        if (OrchestratorSubsystem)
        {
            UE_LOG(LogTemp, Warning, TEXT("Build_IntegrationOrchestratorActor: Connected to orchestrator subsystem"));
        }
    }
}

void ABuild_IntegrationOrchestratorActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Periodic integration monitoring
    static float LastUpdateTime = 0.0f;
    LastUpdateTime += DeltaTime;
    
    if (LastUpdateTime >= 5.0f) // Update every 5 seconds
    {
        if (OrchestratorSubsystem)
        {
            // Trigger periodic validation
            OrchestratorSubsystem->ValidateAllSystemsIntegration();
        }
        LastUpdateTime = 0.0f;
    }
}

void ABuild_IntegrationOrchestratorActor::TriggerSystemIntegration()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_IntegrationOrchestratorActor: Triggering system integration"));
    
    if (OrchestratorSubsystem)
    {
        OrchestratorSubsystem->ExecuteFinalCycleIntegration();
    }
}

void ABuild_IntegrationOrchestratorActor::DisplayIntegrationStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_IntegrationOrchestratorActor: Displaying integration status"));
    
    if (OrchestratorSubsystem)
    {
        FBuild_SystemIntegrationMetrics Metrics = OrchestratorSubsystem->GetCurrentIntegrationMetrics();
        UE_LOG(LogTemp, Warning, TEXT("Integration Status - Score: %.2f%%, Actors: %d, VFX: %d"), 
               Metrics.SystemIntegrationScore, Metrics.TotalActorsInWorld, Metrics.VFXSystemsActive);
    }
}

void ABuild_IntegrationOrchestratorActor::ExecuteEmergencyIntegration()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_IntegrationOrchestratorActor: Executing emergency integration"));
    
    if (OrchestratorSubsystem)
    {
        OrchestratorSubsystem->CompleteCycleOrchestration();
    }
}