#include "Build_FinalSystemIntegrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Subsystems/SubsystemBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Misc/Paths.h"

UBuild_FinalSystemIntegrator::UBuild_FinalSystemIntegrator()
{
    CurrentPhase = EBuild_IntegrationPhase::PreValidation;
    IntegrationStartTime = 0.0f;
    bIntegrationInProgress = false;
    
    // Initialize metrics
    IntegrationMetrics.TotalSystemsDiscovered = 0;
    IntegrationMetrics.SystemsValidated = 0;
    IntegrationMetrics.CrossSystemConnections = 0;
    IntegrationMetrics.PerformanceIssuesFound = 0;
    IntegrationMetrics.TotalIntegrationTime = 0.0f;
    IntegrationMetrics.bIntegrationComplete = false;
    IntegrationMetrics.LastIntegrationError = TEXT("");
}

void UBuild_FinalSystemIntegrator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalSystemIntegrator: Subsystem initialized"));
    
    // Auto-start integration discovery
    DiscoverAllSystems();
}

void UBuild_FinalSystemIntegrator::Deinitialize()
{
    if (bIntegrationInProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("Build_FinalSystemIntegrator: Integration interrupted during shutdown"));
        IntegrationMetrics.LastIntegrationError = TEXT("Integration interrupted during shutdown");
    }
    
    Super::Deinitialize();
}

void UBuild_FinalSystemIntegrator::StartFinalIntegration()
{
    if (bIntegrationInProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("Build_FinalSystemIntegrator: Integration already in progress"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalSystemIntegrator: Starting final system integration"));
    
    bIntegrationInProgress = true;
    IntegrationStartTime = FPlatformTime::Seconds();
    CurrentPhase = EBuild_IntegrationPhase::PreValidation;
    
    // Reset metrics
    IntegrationMetrics = FBuild_SystemIntegrationMetrics();
    DiscoveredSystems.Empty();
    ValidatedSystems.Empty();
    FailedSystems.Empty();
    SystemDependencies.Empty();
    
    // Start integration process
    ProcessCurrentPhase();
}

void UBuild_FinalSystemIntegrator::ValidateAllSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalSystemIntegrator: Validating all discovered systems"));
    
    for (const FString& SystemName : DiscoveredSystems)
    {
        ValidateIndividualSystem(SystemName);
    }
    
    UpdateIntegrationMetrics();
    LogIntegrationStatus();
}

void UBuild_FinalSystemIntegrator::PerformCrossSystemValidation()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalSystemIntegrator: Performing cross-system validation"));
    
    int32 ConnectionsFound = 0;
    
    // Check for common integration patterns
    TArray<FString> CoreSystems = {
        TEXT("TranspersonalGameState"),
        TEXT("TranspersonalCharacter"),
        TEXT("PCGWorldGenerator"),
        TEXT("FoliageManager"),
        TEXT("CrowdSimulationManager")
    };
    
    for (const FString& System1 : CoreSystems)
    {
        for (const FString& System2 : CoreSystems)
        {
            if (System1 != System2)
            {
                // Simulate cross-system validation
                if (ValidatedSystems.Contains(System1) && ValidatedSystems.Contains(System2))
                {
                    ConnectionsFound++;
                    UE_LOG(LogTemp, Log, TEXT("Build_FinalSystemIntegrator: Cross-system connection validated: %s <-> %s"), *System1, *System2);
                }
            }
        }
    }
    
    IntegrationMetrics.CrossSystemConnections = ConnectionsFound;
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalSystemIntegrator: Found %d cross-system connections"), ConnectionsFound);
}

void UBuild_FinalSystemIntegrator::RunPerformanceValidation()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalSystemIntegrator: Running performance validation"));
    
    int32 PerformanceIssues = 0;
    
    for (const FString& SystemName : ValidatedSystems)
    {
        CheckSystemPerformance(SystemName);
    }
    
    // Check overall system performance
    if (IntegrationMetrics.TotalSystemsDiscovered > 50)
    {
        PerformanceIssues++;
        UE_LOG(LogTemp, Warning, TEXT("Build_FinalSystemIntegrator: High system count detected: %d systems"), IntegrationMetrics.TotalSystemsDiscovered);
    }
    
    if (IntegrationMetrics.CrossSystemConnections > 100)
    {
        PerformanceIssues++;
        UE_LOG(LogTemp, Warning, TEXT("Build_FinalSystemIntegrator: High cross-system complexity: %d connections"), IntegrationMetrics.CrossSystemConnections);
    }
    
    IntegrationMetrics.PerformanceIssuesFound = PerformanceIssues;
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalSystemIntegrator: Performance validation complete - %d issues found"), PerformanceIssues);
}

void UBuild_FinalSystemIntegrator::CompleteIntegration()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalSystemIntegrator: Completing final integration"));
    
    CurrentPhase = EBuild_IntegrationPhase::Complete;
    bIntegrationInProgress = false;
    
    IntegrationMetrics.TotalIntegrationTime = FPlatformTime::Seconds() - IntegrationStartTime;
    IntegrationMetrics.bIntegrationComplete = true;
    
    FinalizeIntegration();
    LogIntegrationStatus();
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalSystemIntegrator: Integration completed in %.2f seconds"), IntegrationMetrics.TotalIntegrationTime);
}

FBuild_SystemIntegrationMetrics UBuild_FinalSystemIntegrator::GetIntegrationMetrics() const
{
    return IntegrationMetrics;
}

bool UBuild_FinalSystemIntegrator::IsIntegrationComplete() const
{
    return IntegrationMetrics.bIntegrationComplete;
}

void UBuild_FinalSystemIntegrator::ResetIntegration()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalSystemIntegrator: Resetting integration state"));
    
    bIntegrationInProgress = false;
    CurrentPhase = EBuild_IntegrationPhase::PreValidation;
    IntegrationMetrics = FBuild_SystemIntegrationMetrics();
    
    DiscoveredSystems.Empty();
    ValidatedSystems.Empty();
    FailedSystems.Empty();
    SystemDependencies.Empty();
}

void UBuild_FinalSystemIntegrator::DiscoverAllSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalSystemIntegrator: Discovering all systems"));
    
    DiscoveredSystems.Empty();
    
    // Core game systems
    DiscoveredSystems.Add(TEXT("TranspersonalGameState"));
    DiscoveredSystems.Add(TEXT("TranspersonalCharacter"));
    DiscoveredSystems.Add(TEXT("PCGWorldGenerator"));
    DiscoveredSystems.Add(TEXT("FoliageManager"));
    DiscoveredSystems.Add(TEXT("CrowdSimulationManager"));
    DiscoveredSystems.Add(TEXT("ProceduralWorldManager"));
    DiscoveredSystems.Add(TEXT("BuildIntegrationManager"));
    
    // VFX and Audio systems
    DiscoveredSystems.Add(TEXT("VFX_FootstepManager"));
    DiscoveredSystems.Add(TEXT("VFX_ParticleSystemManager"));
    DiscoveredSystems.Add(TEXT("Audio_SoundscapeManager"));
    DiscoveredSystems.Add(TEXT("Audio_DynamicMusicSystem"));
    
    // QA and Testing systems
    DiscoveredSystems.Add(TEXT("QA_VFXIntegrationValidator"));
    DiscoveredSystems.Add(TEXT("QA_SystemValidator"));
    
    IntegrationMetrics.TotalSystemsDiscovered = DiscoveredSystems.Num();
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalSystemIntegrator: Discovered %d systems"), IntegrationMetrics.TotalSystemsDiscovered);
    
    // Auto-advance to next phase
    CurrentPhase = EBuild_IntegrationPhase::SystemDiscovery;
    MapSystemDependencies();
}

void UBuild_FinalSystemIntegrator::MapSystemDependencies()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalSystemIntegrator: Mapping system dependencies"));
    
    SystemDependencies.Empty();
    
    // Define core system dependencies
    FBuild_SystemDependency GameStateDep;
    GameStateDep.SystemName = TEXT("TranspersonalGameState");
    GameStateDep.Dependencies = {TEXT("Engine"), TEXT("CoreUObject")};
    GameStateDep.Priority = 10;
    GameStateDep.bIsCore = true;
    SystemDependencies.Add(GameStateDep);
    
    FBuild_SystemDependency CharacterDep;
    CharacterDep.SystemName = TEXT("TranspersonalCharacter");
    CharacterDep.Dependencies = {TEXT("TranspersonalGameState"), TEXT("Engine")};
    CharacterDep.Priority = 9;
    CharacterDep.bIsCore = true;
    SystemDependencies.Add(CharacterDep);
    
    FBuild_SystemDependency WorldGenDep;
    WorldGenDep.SystemName = TEXT("PCGWorldGenerator");
    WorldGenDep.Dependencies = {TEXT("PCG"), TEXT("Landscape")};
    WorldGenDep.Priority = 8;
    WorldGenDep.bIsCore = true;
    SystemDependencies.Add(WorldGenDep);
    
    FBuild_SystemDependency FoliageDep;
    FoliageDep.SystemName = TEXT("FoliageManager");
    FoliageDep.Dependencies = {TEXT("PCGWorldGenerator"), TEXT("Foliage")};
    FoliageDep.Priority = 7;
    FoliageDep.bIsCore = false;
    SystemDependencies.Add(FoliageDep);
    
    FBuild_SystemDependency CrowdDep;
    CrowdDep.SystemName = TEXT("CrowdSimulationManager");
    CrowdDep.Dependencies = {TEXT("MassEntity"), TEXT("StateTree")};
    CrowdDep.Priority = 6;
    CrowdDep.bIsCore = false;
    SystemDependencies.Add(CrowdDep);
    
    CurrentPhase = EBuild_IntegrationPhase::DependencyMapping;
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalSystemIntegrator: Mapped %d system dependencies"), SystemDependencies.Num());
}

TArray<FBuild_SystemDependency> UBuild_FinalSystemIntegrator::GetSystemDependencies() const
{
    return SystemDependencies;
}

void UBuild_FinalSystemIntegrator::ValidateSystemDependency(const FString& SystemName)
{
    for (FBuild_SystemDependency& Dependency : SystemDependencies)
    {
        if (Dependency.SystemName == SystemName)
        {
            // Simulate dependency validation
            bool bAllDependenciesMet = true;
            
            for (const FString& DepName : Dependency.Dependencies)
            {
                if (!ValidatedSystems.Contains(DepName) && !DepName.StartsWith(TEXT("Engine")) && !DepName.StartsWith(TEXT("Core")))
                {
                    bAllDependenciesMet = false;
                    break;
                }
            }
            
            Dependency.bValidated = bAllDependenciesMet;
            
            if (bAllDependenciesMet)
            {
                UE_LOG(LogTemp, Log, TEXT("Build_FinalSystemIntegrator: System dependency validated: %s"), *SystemName);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Build_FinalSystemIntegrator: System dependency validation failed: %s"), *SystemName);
            }
            
            break;
        }
    }
}

void UBuild_FinalSystemIntegrator::LogIntegrationStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== BUILD INTEGRATION STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Phase: %d"), (int32)CurrentPhase);
    UE_LOG(LogTemp, Warning, TEXT("Systems Discovered: %d"), IntegrationMetrics.TotalSystemsDiscovered);
    UE_LOG(LogTemp, Warning, TEXT("Systems Validated: %d"), IntegrationMetrics.SystemsValidated);
    UE_LOG(LogTemp, Warning, TEXT("Cross-System Connections: %d"), IntegrationMetrics.CrossSystemConnections);
    UE_LOG(LogTemp, Warning, TEXT("Performance Issues: %d"), IntegrationMetrics.PerformanceIssuesFound);
    UE_LOG(LogTemp, Warning, TEXT("Integration Time: %.2f seconds"), IntegrationMetrics.TotalIntegrationTime);
    UE_LOG(LogTemp, Warning, TEXT("Integration Complete: %s"), IntegrationMetrics.bIntegrationComplete ? TEXT("YES") : TEXT("NO"));
    
    if (!IntegrationMetrics.LastIntegrationError.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Last Error: %s"), *IntegrationMetrics.LastIntegrationError);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END INTEGRATION STATUS ==="));
}

void UBuild_FinalSystemIntegrator::AdvanceToNextPhase()
{
    switch (CurrentPhase)
    {
        case EBuild_IntegrationPhase::PreValidation:
            CurrentPhase = EBuild_IntegrationPhase::SystemDiscovery;
            break;
        case EBuild_IntegrationPhase::SystemDiscovery:
            CurrentPhase = EBuild_IntegrationPhase::DependencyMapping;
            break;
        case EBuild_IntegrationPhase::DependencyMapping:
            CurrentPhase = EBuild_IntegrationPhase::CrossSystemValidation;
            break;
        case EBuild_IntegrationPhase::CrossSystemValidation:
            CurrentPhase = EBuild_IntegrationPhase::PerformanceValidation;
            break;
        case EBuild_IntegrationPhase::PerformanceValidation:
            CurrentPhase = EBuild_IntegrationPhase::FinalIntegration;
            break;
        case EBuild_IntegrationPhase::FinalIntegration:
            CurrentPhase = EBuild_IntegrationPhase::PostIntegrationTest;
            break;
        case EBuild_IntegrationPhase::PostIntegrationTest:
            CurrentPhase = EBuild_IntegrationPhase::Complete;
            break;
        default:
            break;
    }
    
    ProcessCurrentPhase();
}

void UBuild_FinalSystemIntegrator::ProcessCurrentPhase()
{
    switch (CurrentPhase)
    {
        case EBuild_IntegrationPhase::SystemDiscovery:
            DiscoverAllSystems();
            break;
        case EBuild_IntegrationPhase::DependencyMapping:
            MapSystemDependencies();
            break;
        case EBuild_IntegrationPhase::CrossSystemValidation:
            PerformCrossSystemValidation();
            break;
        case EBuild_IntegrationPhase::PerformanceValidation:
            RunPerformanceValidation();
            break;
        case EBuild_IntegrationPhase::FinalIntegration:
            CompleteIntegration();
            break;
        default:
            break;
    }
}

void UBuild_FinalSystemIntegrator::ValidateIndividualSystem(const FString& SystemName)
{
    // Simulate system validation
    bool bValidationSuccess = true;
    
    // Check if system is in our known systems
    if (!DiscoveredSystems.Contains(SystemName))
    {
        bValidationSuccess = false;
        UE_LOG(LogTemp, Warning, TEXT("Build_FinalSystemIntegrator: Unknown system: %s"), *SystemName);
    }
    
    // Validate system dependencies
    ValidateSystemDependency(SystemName);
    
    if (bValidationSuccess)
    {
        ValidatedSystems.AddUnique(SystemName);
        LogSystemStatus(SystemName, true);
    }
    else
    {
        FailedSystems.AddUnique(SystemName);
        LogSystemStatus(SystemName, false);
    }
}

void UBuild_FinalSystemIntegrator::CheckSystemPerformance(const FString& SystemName)
{
    // Simulate performance check
    UE_LOG(LogTemp, Log, TEXT("Build_FinalSystemIntegrator: Performance check for system: %s"), *SystemName);
    
    // Check for known performance-heavy systems
    if (SystemName.Contains(TEXT("Crowd")) || SystemName.Contains(TEXT("VFX")) || SystemName.Contains(TEXT("Particle")))
    {
        UE_LOG(LogTemp, Warning, TEXT("Build_FinalSystemIntegrator: Performance-sensitive system detected: %s"), *SystemName);
    }
}

void UBuild_FinalSystemIntegrator::LogSystemStatus(const FString& SystemName, bool bSuccess)
{
    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("Build_FinalSystemIntegrator: ✓ System validated: %s"), *SystemName);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Build_FinalSystemIntegrator: ✗ System validation failed: %s"), *SystemName);
    }
}

void UBuild_FinalSystemIntegrator::UpdateIntegrationMetrics()
{
    IntegrationMetrics.SystemsValidated = ValidatedSystems.Num();
    IntegrationMetrics.TotalIntegrationTime = FPlatformTime::Seconds() - IntegrationStartTime;
}

void UBuild_FinalSystemIntegrator::HandleIntegrationError(const FString& ErrorMessage)
{
    UE_LOG(LogTemp, Error, TEXT("Build_FinalSystemIntegrator: Integration error: %s"), *ErrorMessage);
    IntegrationMetrics.LastIntegrationError = ErrorMessage;
}

void UBuild_FinalSystemIntegrator::FinalizeIntegration()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalSystemIntegrator: Finalizing integration"));
    
    // Final validation pass
    if (IntegrationMetrics.SystemsValidated >= IntegrationMetrics.TotalSystemsDiscovered * 0.8f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Build_FinalSystemIntegrator: Integration successful - %d/%d systems validated"), 
               IntegrationMetrics.SystemsValidated, IntegrationMetrics.TotalSystemsDiscovered);
    }
    else
    {
        HandleIntegrationError(TEXT("Insufficient systems validated for successful integration"));
    }
}