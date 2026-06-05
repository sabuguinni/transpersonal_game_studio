#include "Build_CriticalSystemOrchestrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

UBuild_CriticalSystemOrchestrator::UBuild_CriticalSystemOrchestrator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f; // Check every 5 seconds
    
    // Initialize critical system tracking
    CriticalSystems.Empty();
    CriticalSystems.Add(TEXT("TranspersonalGameState"));
    CriticalSystems.Add(TEXT("TranspersonalCharacter"));
    CriticalSystems.Add(TEXT("PCGWorldGenerator"));
    CriticalSystems.Add(TEXT("FoliageManager"));
    CriticalSystems.Add(TEXT("CrowdSimulationManager"));
    
    SystemsLoaded = 0;
    TotalSystems = CriticalSystems.Num();
    bOrchestrationActive = false;
    bValidationComplete = false;
    LastValidationTime = 0.0f;
    ValidationInterval = 10.0f;
    
    OrchestrationStatus = EBuild_OrchestrationStatus::Initializing;
}

void UBuild_CriticalSystemOrchestrator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Critical System Orchestrator starting..."));
    
    // Start orchestration
    StartOrchestration();
}

void UBuild_CriticalSystemOrchestrator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bOrchestrationActive)
    {
        return;
    }
    
    LastValidationTime += DeltaTime;
    
    // Periodic validation
    if (LastValidationTime >= ValidationInterval)
    {
        ValidateCriticalSystems();
        LastValidationTime = 0.0f;
    }
}

void UBuild_CriticalSystemOrchestrator::StartOrchestration()
{
    if (bOrchestrationActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Orchestration already active"));
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Starting critical system orchestration..."));
    
    bOrchestrationActive = true;
    OrchestrationStatus = EBuild_OrchestrationStatus::Running;
    
    // Initial validation
    ValidateCriticalSystems();
    
    // Initialize system dependencies
    InitializeSystemDependencies();
    
    UE_LOG(LogTemp, Log, TEXT("Critical system orchestration started"));
}

void UBuild_CriticalSystemOrchestrator::StopOrchestration()
{
    if (!bOrchestrationActive)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Stopping critical system orchestration..."));
    
    bOrchestrationActive = false;
    OrchestrationStatus = EBuild_OrchestrationStatus::Stopped;
    
    // Generate final report
    GenerateOrchestrationReport();
    
    UE_LOG(LogTemp, Log, TEXT("Critical system orchestration stopped"));
}

void UBuild_CriticalSystemOrchestrator::ValidateCriticalSystems()
{
    UE_LOG(LogTemp, Log, TEXT("Validating critical systems..."));
    
    SystemsLoaded = 0;
    LoadedSystems.Empty();
    FailedSystems.Empty();
    
    for (const FString& SystemName : CriticalSystems)
    {
        if (ValidateSystem(SystemName))
        {
            SystemsLoaded++;
            LoadedSystems.Add(SystemName);
            UE_LOG(LogTemp, Log, TEXT("✓ %s - LOADED"), *SystemName);
        }
        else
        {
            FailedSystems.Add(SystemName);
            UE_LOG(LogTemp, Warning, TEXT("✗ %s - FAILED"), *SystemName);
        }
    }
    
    // Update orchestration status
    if (SystemsLoaded == TotalSystems)
    {
        OrchestrationStatus = EBuild_OrchestrationStatus::AllSystemsLoaded;
        bValidationComplete = true;
    }
    else if (SystemsLoaded >= (TotalSystems * 0.8f)) // 80% threshold
    {
        OrchestrationStatus = EBuild_OrchestrationStatus::MostSystemsLoaded;
    }
    else if (SystemsLoaded > 0)
    {
        OrchestrationStatus = EBuild_OrchestrationStatus::SomeSystemsLoaded;
    }
    else
    {
        OrchestrationStatus = EBuild_OrchestrationStatus::NoSystemsLoaded;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Critical Systems Validation: %d/%d loaded"), SystemsLoaded, TotalSystems);
    
    // Trigger recovery if needed
    if (SystemsLoaded < (TotalSystems * 0.6f)) // Less than 60% loaded
    {
        TriggerSystemRecovery();
    }
}

bool UBuild_CriticalSystemOrchestrator::ValidateSystem(const FString& SystemName)
{
    FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *SystemName);
    
    try
    {
        UClass* SystemClass = LoadClass<UObject>(nullptr, *ClassPath);
        if (SystemClass)
        {
            // Try to get CDO to ensure class is properly constructed
            UObject* CDO = SystemClass->GetDefaultObject();
            return CDO != nullptr;
        }
    }
    catch (...)
    {
        UE_LOG(LogTemp, Error, TEXT("Exception validating system: %s"), *SystemName);
    }
    
    return false;
}

void UBuild_CriticalSystemOrchestrator::InitializeSystemDependencies()
{
    UE_LOG(LogTemp, Log, TEXT("Initializing system dependencies..."));
    
    // Define dependency order
    TArray<FString> DependencyOrder;
    DependencyOrder.Add(TEXT("TranspersonalGameState"));      // Core game state first
    DependencyOrder.Add(TEXT("TranspersonalCharacter"));      // Character system
    DependencyOrder.Add(TEXT("PCGWorldGenerator"));           // World generation
    DependencyOrder.Add(TEXT("FoliageManager"));              // Environment
    DependencyOrder.Add(TEXT("CrowdSimulationManager"));      // AI systems
    
    for (const FString& SystemName : DependencyOrder)
    {
        if (LoadedSystems.Contains(SystemName))
        {
            InitializeSystem(SystemName);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("System dependencies initialized"));
}

void UBuild_CriticalSystemOrchestrator::InitializeSystem(const FString& SystemName)
{
    UE_LOG(LogTemp, Log, TEXT("Initializing system: %s"), *SystemName);
    
    // System-specific initialization logic
    if (SystemName == TEXT("TranspersonalGameState"))
    {
        InitializeGameState();
    }
    else if (SystemName == TEXT("TranspersonalCharacter"))
    {
        InitializeCharacterSystem();
    }
    else if (SystemName == TEXT("PCGWorldGenerator"))
    {
        InitializeWorldGenerator();
    }
    else if (SystemName == TEXT("FoliageManager"))
    {
        InitializeFoliageManager();
    }
    else if (SystemName == TEXT("CrowdSimulationManager"))
    {
        InitializeCrowdSimulation();
    }
}

void UBuild_CriticalSystemOrchestrator::InitializeGameState()
{
    UE_LOG(LogTemp, Log, TEXT("Initializing TranspersonalGameState..."));
    
    if (UWorld* World = GetWorld())
    {
        if (AGameStateBase* GameState = World->GetGameState())
        {
            UE_LOG(LogTemp, Log, TEXT("✓ GameState active: %s"), *GameState->GetClass()->GetName());
        }
    }
}

void UBuild_CriticalSystemOrchestrator::InitializeCharacterSystem()
{
    UE_LOG(LogTemp, Log, TEXT("Initializing TranspersonalCharacter system..."));
    // Character system initialization logic here
}

void UBuild_CriticalSystemOrchestrator::InitializeWorldGenerator()
{
    UE_LOG(LogTemp, Log, TEXT("Initializing PCGWorldGenerator..."));
    // World generator initialization logic here
}

void UBuild_CriticalSystemOrchestrator::InitializeFoliageManager()
{
    UE_LOG(LogTemp, Log, TEXT("Initializing FoliageManager..."));
    // Foliage manager initialization logic here
}

void UBuild_CriticalSystemOrchestrator::InitializeCrowdSimulation()
{
    UE_LOG(LogTemp, Log, TEXT("Initializing CrowdSimulationManager..."));
    // Crowd simulation initialization logic here
}

void UBuild_CriticalSystemOrchestrator::TriggerSystemRecovery()
{
    UE_LOG(LogTemp, Warning, TEXT("Triggering system recovery - too many systems failed"));
    
    OrchestrationStatus = EBuild_OrchestrationStatus::RecoveryMode;
    
    // Attempt to reload failed systems
    for (const FString& FailedSystem : FailedSystems)
    {
        UE_LOG(LogTemp, Warning, TEXT("Attempting recovery for: %s"), *FailedSystem);
        
        // Force garbage collection before retry
        CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
        
        // Retry validation
        if (ValidateSystem(FailedSystem))
        {
            UE_LOG(LogTemp, Log, TEXT("✓ Recovery successful for: %s"), *FailedSystem);
            LoadedSystems.Add(FailedSystem);
            FailedSystems.Remove(FailedSystem);
            SystemsLoaded++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Recovery complete - %d/%d systems loaded"), SystemsLoaded, TotalSystems);
}

void UBuild_CriticalSystemOrchestrator::GenerateOrchestrationReport()
{
    UE_LOG(LogTemp, Log, TEXT("Generating orchestration report..."));
    
    // Create JSON report
    TSharedPtr<FJsonObject> ReportObject = MakeShareable(new FJsonObject);
    
    ReportObject->SetStringField(TEXT("orchestrator"), TEXT("Build_CriticalSystemOrchestrator"));
    ReportObject->SetStringField(TEXT("timestamp"), FDateTime::Now().ToIso8601());
    ReportObject->SetStringField(TEXT("status"), GetOrchestrationStatusString());
    ReportObject->SetNumberField(TEXT("systems_loaded"), SystemsLoaded);
    ReportObject->SetNumberField(TEXT("total_systems"), TotalSystems);
    ReportObject->SetBoolField(TEXT("validation_complete"), bValidationComplete);
    
    // Add loaded systems array
    TArray<TSharedPtr<FJsonValue>> LoadedArray;
    for (const FString& System : LoadedSystems)
    {
        LoadedArray.Add(MakeShareable(new FJsonValueString(System)));
    }
    ReportObject->SetArrayField(TEXT("loaded_systems"), LoadedArray);
    
    // Add failed systems array
    TArray<TSharedPtr<FJsonValue>> FailedArray;
    for (const FString& System : FailedSystems)
    {
        FailedArray.Add(MakeShareable(new FJsonValueString(System)));
    }
    ReportObject->SetArrayField(TEXT("failed_systems"), FailedArray);
    
    // Convert to JSON string
    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(ReportObject.ToSharedRef(), Writer);
    
    // Save to file
    FString ProjectDir = FPaths::ProjectDir();
    FString ReportPath = FPaths::Combine(ProjectDir, TEXT("Intermediate"), TEXT("orchestration_report.json"));
    
    if (FFileHelper::SaveStringToFile(OutputString, *ReportPath))
    {
        UE_LOG(LogTemp, Log, TEXT("Orchestration report saved to: %s"), *ReportPath);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to save orchestration report"));
    }
}

FString UBuild_CriticalSystemOrchestrator::GetOrchestrationStatusString() const
{
    switch (OrchestrationStatus)
    {
        case EBuild_OrchestrationStatus::Initializing: return TEXT("INITIALIZING");
        case EBuild_OrchestrationStatus::Running: return TEXT("RUNNING");
        case EBuild_OrchestrationStatus::AllSystemsLoaded: return TEXT("ALL_SYSTEMS_LOADED");
        case EBuild_OrchestrationStatus::MostSystemsLoaded: return TEXT("MOST_SYSTEMS_LOADED");
        case EBuild_OrchestrationStatus::SomeSystemsLoaded: return TEXT("SOME_SYSTEMS_LOADED");
        case EBuild_OrchestrationStatus::NoSystemsLoaded: return TEXT("NO_SYSTEMS_LOADED");
        case EBuild_OrchestrationStatus::RecoveryMode: return TEXT("RECOVERY_MODE");
        case EBuild_OrchestrationStatus::Stopped: return TEXT("STOPPED");
        case EBuild_OrchestrationStatus::Failed: return TEXT("FAILED");
        default: return TEXT("UNKNOWN");
    }
}

bool UBuild_CriticalSystemOrchestrator::IsOrchestrationSuccessful() const
{
    return OrchestrationStatus == EBuild_OrchestrationStatus::AllSystemsLoaded ||
           OrchestrationStatus == EBuild_OrchestrationStatus::MostSystemsLoaded;
}

void UBuild_CriticalSystemOrchestrator::LogOrchestrationSummary() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== CRITICAL SYSTEM ORCHESTRATION SUMMARY ==="));
    UE_LOG(LogTemp, Warning, TEXT("Status: %s"), *GetOrchestrationStatusString());
    UE_LOG(LogTemp, Warning, TEXT("Systems Loaded: %d/%d"), SystemsLoaded, TotalSystems);
    UE_LOG(LogTemp, Warning, TEXT("Validation Complete: %s"), bValidationComplete ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Orchestration Successful: %s"), IsOrchestrationSuccessful() ? TEXT("YES") : TEXT("NO"));
    
    if (LoadedSystems.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Loaded Systems:"));
        for (const FString& System : LoadedSystems)
        {
            UE_LOG(LogTemp, Warning, TEXT("  ✓ %s"), *System);
        }
    }
    
    if (FailedSystems.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed Systems:"));
        for (const FString& System : FailedSystems)
        {
            UE_LOG(LogTemp, Warning, TEXT("  ✗ %s"), *System);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("============================================="));
}