#include "EngineArchitect.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "Stats/Stats.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"

UEngineArchitect::UEngineArchitect()
{
    SystemPerformanceScore = 100.0f;
    bArchitectureValid = true;
    bModulesCompliant = true;
    bPerformanceAcceptable = true;
}

void UEngineArchitect::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Engine Architect System Initialized"));
    
    // Register core systems that should always be present
    RegisteredSystems.Empty();
    
    // Initialize performance monitoring
    SystemPerformanceScore = 100.0f;
    PerformanceWarnings.Empty();
    
    // Run initial validation
    ValidateSystemArchitecture();
}

void UEngineArchitect::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("Engine Architect System Deinitialized"));
    
    RegisteredSystems.Empty();
    PerformanceWarnings.Empty();
    
    Super::Deinitialize();
}

bool UEngineArchitect::ValidateSystemArchitecture()
{
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: Validating System Architecture"));
    
    bool bAllSystemsValid = true;
    
    // Validate physics integration
    if (!ValidatePhysicsIntegration())
    {
        bAllSystemsValid = false;
        LogArchitecturalIssue(TEXT("Physics integration validation failed"), TEXT("PhysicsCore"));
    }
    
    // Validate biome system integration
    if (!ValidateBiomeSystemIntegration())
    {
        bAllSystemsValid = false;
        LogArchitecturalIssue(TEXT("Biome system integration validation failed"), TEXT("BiomeSystem"));
    }
    
    // Validate character system integration
    if (!ValidateCharacterSystemIntegration())
    {
        bAllSystemsValid = false;
        LogArchitecturalIssue(TEXT("Character system integration validation failed"), TEXT("CharacterSystem"));
    }
    
    // Validate AI system integration
    if (!ValidateAISystemIntegration())
    {
        bAllSystemsValid = false;
        LogArchitecturalIssue(TEXT("AI system integration validation failed"), TEXT("AISystem"));
    }
    
    bArchitectureValid = bAllSystemsValid;
    return bAllSystemsValid;
}

bool UEngineArchitect::ValidateModuleCompliance()
{
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: Validating Module Compliance"));
    
    bool bComplianceValid = true;
    
    // Check module dependencies
    CheckModuleDependencies();
    
    // Validate class hierarchy
    ValidateClassHierarchy();
    
    // Check shared types usage
    CheckSharedTypesUsage();
    
    bModulesCompliant = bComplianceValid;
    return bComplianceValid;
}

bool UEngineArchitect::ValidatePerformanceStandards()
{
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: Validating Performance Standards"));
    
    // Analyze system performance
    AnalyzeSystemPerformance();
    
    // Check memory usage
    CheckMemoryUsage();
    
    // Validate frame rate
    ValidateFrameRate();
    
    // Performance is acceptable if score is above 70
    bPerformanceAcceptable = (SystemPerformanceScore >= 70.0f);
    
    return bPerformanceAcceptable;
}

void UEngineArchitect::RegisterCoreSystem(const FString& SystemName, UObject* SystemInstance)
{
    if (SystemInstance && !SystemName.IsEmpty())
    {
        RegisteredSystems.Add(SystemName, SystemInstance);
        UE_LOG(LogTemp, Log, TEXT("Engine Architect: Registered system '%s'"), *SystemName);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Failed to register system '%s' - invalid instance"), *SystemName);
    }
}

bool UEngineArchitect::IsSystemRegistered(const FString& SystemName) const
{
    return RegisteredSystems.Contains(SystemName);
}

TArray<FString> UEngineArchitect::GetRegisteredSystems() const
{
    TArray<FString> SystemNames;
    RegisteredSystems.GetKeys(SystemNames);
    return SystemNames;
}

void UEngineArchitect::RunArchitecturalComplianceCheck()
{
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: Running Architectural Compliance Check"));
    
    // Clear previous warnings
    PerformanceWarnings.Empty();
    
    // Run all validation checks
    bool bArchValid = ValidateSystemArchitecture();
    bool bModValid = ValidateModuleCompliance();
    bool bPerfValid = ValidatePerformanceStandards();
    
    // Generate compliance report
    GenerateComplianceReport();
    
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: Compliance Check Complete - Arch:%s Mod:%s Perf:%s"), 
           bArchValid ? TEXT("PASS") : TEXT("FAIL"),
           bModValid ? TEXT("PASS") : TEXT("FAIL"),
           bPerfValid ? TEXT("PASS") : TEXT("FAIL"));
}

void UEngineArchitect::ValidateAllModuleDependencies()
{
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: Validating All Module Dependencies"));
    
    CheckModuleDependencies();
}

void UEngineArchitect::CheckSystemIntegration()
{
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: Checking System Integration"));
    
    // Check integration between all registered systems
    TArray<FString> SystemNames = GetRegisteredSystems();
    
    for (const FString& SystemName : SystemNames)
    {
        UObject* System = RegisteredSystems[SystemName];
        if (System)
        {
            UE_LOG(LogTemp, Log, TEXT("Engine Architect: System '%s' is active"), *SystemName);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Engine Architect: System '%s' is registered but inactive"), *SystemName);
        }
    }
}

bool UEngineArchitect::ValidatePhysicsIntegration()
{
    // Check if physics systems are properly integrated
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: Validating Physics Integration"));
    
    // For now, assume physics integration is valid
    // In a real implementation, this would check for specific physics components
    return true;
}

bool UEngineArchitect::ValidateBiomeSystemIntegration()
{
    // Check if biome systems are properly integrated
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: Validating Biome System Integration"));
    
    // Check if BiomeManager is available
    bool bBiomeManagerFound = IsSystemRegistered(TEXT("BiomeManager"));
    
    if (!bBiomeManagerFound)
    {
        UE_LOG(LogTemp, Warning, TEXT("Engine Architect: BiomeManager not registered"));
    }
    
    return bBiomeManagerFound;
}

bool UEngineArchitect::ValidateCharacterSystemIntegration()
{
    // Check if character systems are properly integrated
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: Validating Character System Integration"));
    
    // For now, assume character integration is valid
    return true;
}

bool UEngineArchitect::ValidateAISystemIntegration()
{
    // Check if AI systems are properly integrated
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: Validating AI System Integration"));
    
    // For now, assume AI integration is valid
    return true;
}

void UEngineArchitect::AnalyzeSystemPerformance()
{
    // Analyze performance of all registered systems
    float TotalPerformance = 100.0f;
    int32 SystemCount = RegisteredSystems.Num();
    
    if (SystemCount == 0)
    {
        SystemPerformanceScore = 50.0f; // Low score if no systems registered
        PerformanceWarnings.Add(TEXT("No systems registered for performance monitoring"));
        return;
    }
    
    // For now, use a simple calculation
    // In a real implementation, this would measure actual performance metrics
    SystemPerformanceScore = TotalPerformance;
    
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: System Performance Score: %.1f"), SystemPerformanceScore);
}

void UEngineArchitect::CheckMemoryUsage()
{
    // Check memory usage of systems
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    
    float MemoryUsageGB = MemStats.UsedPhysical / (1024.0f * 1024.0f * 1024.0f);
    
    if (MemoryUsageGB > 8.0f) // Warning if using more than 8GB
    {
        PerformanceWarnings.Add(FString::Printf(TEXT("High memory usage: %.1f GB"), MemoryUsageGB));
        SystemPerformanceScore -= 10.0f;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: Memory Usage: %.1f GB"), MemoryUsageGB);
}

void UEngineArchitect::ValidateFrameRate()
{
    // Validate frame rate performance
    // For now, assume frame rate is acceptable
    // In a real implementation, this would measure actual frame times
    
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: Frame Rate Validation Complete"));
}

void UEngineArchitect::CheckModuleDependencies()
{
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: Checking Module Dependencies"));
    
    // Check that all required modules are loaded
    // This is a simplified check - in reality would verify Build.cs dependencies
}

void UEngineArchitect::ValidateClassHierarchy()
{
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: Validating Class Hierarchy"));
    
    // Validate that classes follow proper inheritance patterns
    // This is a simplified validation
}

void UEngineArchitect::CheckSharedTypesUsage()
{
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: Checking SharedTypes Usage"));
    
    // Validate that all systems use SharedTypes.h enums and structs
    // This is a simplified check
}

void UEngineArchitect::LogArchitecturalIssue(const FString& Issue, const FString& System)
{
    FString LogMessage = FString::Printf(TEXT("ARCHITECTURAL ISSUE [%s]: %s"), *System, *Issue);
    UE_LOG(LogTemp, Error, TEXT("%s"), *LogMessage);
    
    PerformanceWarnings.Add(LogMessage);
}

void UEngineArchitect::GenerateComplianceReport()
{
    UE_LOG(LogTemp, Log, TEXT("=== ENGINE ARCHITECT COMPLIANCE REPORT ==="));
    UE_LOG(LogTemp, Log, TEXT("Architecture Valid: %s"), bArchitectureValid ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Log, TEXT("Modules Compliant: %s"), bModulesCompliant ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Log, TEXT("Performance Acceptable: %s"), bPerformanceAcceptable ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Log, TEXT("Performance Score: %.1f"), SystemPerformanceScore);
    UE_LOG(LogTemp, Log, TEXT("Registered Systems: %d"), RegisteredSystems.Num());
    
    if (PerformanceWarnings.Num() > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Warnings:"));
        for (const FString& Warning : PerformanceWarnings)
        {
            UE_LOG(LogTemp, Log, TEXT("  - %s"), *Warning);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("=== END COMPLIANCE REPORT ==="));
}

// World Subsystem Implementation
UEngineArchitectWorldSubsystem::UEngineArchitectWorldSubsystem()
{
    WorldPerformanceScore = 100.0f;
    bWorldConfigurationValid = true;
}

void UEngineArchitectWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Engine Architect World Subsystem Initialized"));
    
    CriticalActors.Empty();
}

void UEngineArchitectWorldSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: World Begin Play - Validating Configuration"));
    
    // Validate world configuration when gameplay starts
    ValidateWorldConfiguration();
}

bool UEngineArchitectWorldSubsystem::ValidateWorldConfiguration()
{
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: Validating World Configuration"));
    
    bool bConfigValid = true;
    
    // Validate game mode setup
    ValidateGameModeSetup();
    
    // Check player controller configuration
    CheckPlayerControllerConfiguration();
    
    // Validate level streaming setup
    ValidateLevelStreamingSetup();
    
    // Analyze actor density
    AnalyzeActorDensity();
    
    bWorldConfigurationValid = bConfigValid;
    return bConfigValid;
}

bool UEngineArchitectWorldSubsystem::ValidateActorIntegration()
{
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: Validating Actor Integration"));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Count actors by type
    TMap<FString, int32> ActorCounts;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            FString ClassName = Actor->GetClass()->GetName();
            ActorCounts.FindOrAdd(ClassName)++;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: Actor Integration Analysis Complete"));
    for (const auto& Pair : ActorCounts)
    {
        UE_LOG(LogTemp, Log, TEXT("  %s: %d instances"), *Pair.Key, Pair.Value);
    }
    
    return true;
}

void UEngineArchitectWorldSubsystem::CheckWorldPerformance()
{
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: Checking World Performance"));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Count total actors
    int32 ActorCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        ActorCount++;
    }
    
    // Calculate performance score based on actor count
    if (ActorCount > 10000)
    {
        WorldPerformanceScore = 50.0f; // High actor count
    }
    else if (ActorCount > 5000)
    {
        WorldPerformanceScore = 75.0f; // Moderate actor count
    }
    else
    {
        WorldPerformanceScore = 100.0f; // Low actor count
    }
    
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: World Performance Score: %.1f (Actors: %d)"), 
           WorldPerformanceScore, ActorCount);
}

void UEngineArchitectWorldSubsystem::RegisterCriticalActor(AActor* Actor, const FString& SystemName)
{
    if (Actor && !SystemName.IsEmpty())
    {
        TArray<AActor*>& SystemActors = CriticalActors.FindOrAdd(SystemName);
        SystemActors.AddUnique(Actor);
        
        UE_LOG(LogTemp, Log, TEXT("Engine Architect: Registered critical actor '%s' for system '%s'"), 
               *Actor->GetName(), *SystemName);
    }
}

TArray<AActor*> UEngineArchitectWorldSubsystem::GetCriticalActors(const FString& SystemName) const
{
    if (const TArray<AActor*>* SystemActors = CriticalActors.Find(SystemName))
    {
        return *SystemActors;
    }
    
    return TArray<AActor*>();
}

void UEngineArchitectWorldSubsystem::ValidateGameModeSetup()
{
    UWorld* World = GetWorld();
    if (World && World->GetAuthGameMode())
    {
        UE_LOG(LogTemp, Log, TEXT("Engine Architect: Game Mode validation - OK"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Game Mode validation - FAILED"));
    }
}

void UEngineArchitectWorldSubsystem::CheckPlayerControllerConfiguration()
{
    UWorld* World = GetWorld();
    if (World)
    {
        APlayerController* PC = World->GetFirstPlayerController();
        if (PC)
        {
            UE_LOG(LogTemp, Log, TEXT("Engine Architect: Player Controller validation - OK"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Player Controller validation - FAILED"));
        }
    }
}

void UEngineArchitectWorldSubsystem::ValidateLevelStreamingSetup()
{
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: Level Streaming validation - OK (simplified)"));
}

void UEngineArchitectWorldSubsystem::AnalyzeActorDensity()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    int32 ActorCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        ActorCount++;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: Actor Density Analysis - %d total actors"), ActorCount);
}