#include "Eng_ArchitecturalFramework.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"
#include "Engine/GameInstance.h"
#include "TranspersonalGame/TranspersonalGame.h"

// ============================================================================
// ENGINE ARCHITECTURE FRAMEWORK IMPLEMENTATION - AGENT #2
// Core architectural system that enforces rules across all agent outputs
// ============================================================================

UEng_ArchitecturalFramework::UEng_ArchitecturalFramework()
{
    bFrameworkInitialized = false;
    CurrentPerformanceTier = EEng_PerformanceTier::Critical;
    
    // Initialize default architectural rules
    CurrentRules = FEng_ArchitecturalRules();
    CurrentRules.bEnforceHeaderCppPairs = true;
    CurrentRules.bEnforcePerformanceLimits = true;
    CurrentRules.bEnforceModuleDependencies = true;
    CurrentRules.bEnforceNamingConventions = true;
    CurrentRules.MaxActorsPerSystem = 1000;
    CurrentRules.MaxFrameTimeMS = 16.67f; // 60fps target
}

void UEng_ArchitecturalFramework::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTranspersonalGame, Warning, TEXT("Engine Architectural Framework Initializing..."));
    
    InitializeSystemRequirements();
    ValidateModuleStructure();
    
    bFrameworkInitialized = true;
    
    UE_LOG(LogTranspersonalGame, Warning, TEXT("Engine Architectural Framework Ready - Enforcing Rules"));
}

void UEng_ArchitecturalFramework::Deinitialize()
{
    UE_LOG(LogTranspersonalGame, Warning, TEXT("Engine Architectural Framework Shutting Down"));
    
    SystemRequirements.Empty();
    ModuleStatuses.Empty();
    bFrameworkInitialized = false;
    
    Super::Deinitialize();
}

void UEng_ArchitecturalFramework::InitializeSystemRequirements()
{
    // Core System Requirements
    FEng_SystemRequirements CoreReqs;
    CoreReqs.SystemType = EEng_SystemType::Core;
    CoreReqs.RequiredModules.Add(TEXT("Engine"));
    CoreReqs.RequiredModules.Add(TEXT("CoreUObject"));
    CoreReqs.RequiredModules.Add(TEXT("TranspersonalGame"));
    CoreReqs.RequiredClasses.Add(TEXT("UEng_ArchitecturalFramework"));
    CoreReqs.MinPerformanceTier = EEng_PerformanceTier::Critical;
    CoreReqs.MaxActorCount = 100;
    CoreReqs.MaxMemoryMB = 256.0f;
    SystemRequirements.Add(EEng_SystemType::Core, CoreReqs);
    
    // World Generation System Requirements
    FEng_SystemRequirements WorldReqs;
    WorldReqs.SystemType = EEng_SystemType::World;
    WorldReqs.RequiredModules.Add(TEXT("Landscape"));
    WorldReqs.RequiredModules.Add(TEXT("PCG"));
    WorldReqs.RequiredModules.Add(TEXT("Foliage"));
    WorldReqs.RequiredClasses.Add(TEXT("APCGWorldGenerator"));
    WorldReqs.RequiredClasses.Add(TEXT("AFoliageManager"));
    WorldReqs.MinPerformanceTier = EEng_PerformanceTier::High;
    WorldReqs.MaxActorCount = 5000;
    WorldReqs.MaxMemoryMB = 1024.0f;
    SystemRequirements.Add(EEng_SystemType::World, WorldReqs);
    
    // Character System Requirements
    FEng_SystemRequirements CharacterReqs;
    CharacterReqs.SystemType = EEng_SystemType::Character;
    CharacterReqs.RequiredModules.Add(TEXT("Engine"));
    CharacterReqs.RequiredModules.Add(TEXT("AIModule"));
    CharacterReqs.RequiredClasses.Add(TEXT("ATranspersonalCharacter"));
    CharacterReqs.MinPerformanceTier = EEng_PerformanceTier::Critical;
    CharacterReqs.MaxActorCount = 50;
    CharacterReqs.MaxMemoryMB = 512.0f;
    SystemRequirements.Add(EEng_SystemType::Character, CharacterReqs);
    
    // AI System Requirements
    FEng_SystemRequirements AIReqs;
    AIReqs.SystemType = EEng_SystemType::AI;
    AIReqs.RequiredModules.Add(TEXT("AIModule"));
    AIReqs.RequiredModules.Add(TEXT("NavigationSystem"));
    AIReqs.RequiredModules.Add(TEXT("MassEntity"));
    AIReqs.MinPerformanceTier = EEng_PerformanceTier::High;
    AIReqs.MaxActorCount = 1000;
    AIReqs.MaxMemoryMB = 768.0f;
    SystemRequirements.Add(EEng_SystemType::AI, AIReqs);
    
    // Performance System Requirements
    FEng_SystemRequirements PerfReqs;
    PerfReqs.SystemType = EEng_SystemType::Performance;
    PerfReqs.RequiredModules.Add(TEXT("RenderCore"));
    PerfReqs.RequiredModules.Add(TEXT("RHI"));
    PerfReqs.MinPerformanceTier = EEng_PerformanceTier::Critical;
    PerfReqs.MaxActorCount = 0; // Performance system doesn't spawn actors
    PerfReqs.MaxMemoryMB = 128.0f;
    SystemRequirements.Add(EEng_SystemType::Performance, PerfReqs);
}

void UEng_ArchitecturalFramework::ValidateModuleStructure()
{
    UE_LOG(LogTranspersonalGame, Warning, TEXT("Validating Module Structure..."));
    
    FString ProjectDir = FPaths::ProjectDir();
    FString SourceDir = FPaths::Combine(ProjectDir, TEXT("Source"), TEXT("TranspersonalGame"));
    
    ScanModuleDirectory(SourceDir);
    ScanForOrphanedHeaders();
    CheckSystemDependencies();
}

void UEng_ArchitecturalFramework::ScanModuleDirectory(const FString& ModulePath)
{
    TArray<FString> FoundFiles;
    IFileManager::Get().FindFilesRecursive(FoundFiles, *ModulePath, TEXT("*.h"), true, false);
    
    FEng_ModuleStatus MainModuleStatus;
    MainModuleStatus.ModuleName = TEXT("TranspersonalGame");
    MainModuleStatus.CompilationStatus = EEng_CompilationStatus::Clean;
    
    int32 HeaderCount = 0;
    int32 SourceCount = 0;
    int32 OrphanedCount = 0;
    
    for (const FString& HeaderFile : FoundFiles)
    {
        if (HeaderFile.Contains(TEXT(".generated.h")))
        {
            continue; // Skip generated headers
        }
        
        HeaderCount++;
        
        FString CppFile = HeaderFile;
        CppFile = CppFile.Replace(TEXT(".h"), TEXT(".cpp"));
        
        if (!IFileManager::Get().FileExists(*CppFile))
        {
            OrphanedCount++;
            FString RelativePath = HeaderFile;
            FPaths::MakePathRelativeTo(RelativePath, *ModulePath);
            MainModuleStatus.MissingImplementations.Add(RelativePath);
        }
        else
        {
            SourceCount++;
        }
    }
    
    MainModuleStatus.HeaderCount = HeaderCount;
    MainModuleStatus.SourceCount = SourceCount;
    MainModuleStatus.OrphanedHeaders = OrphanedCount;
    
    if (OrphanedCount > 0)
    {
        MainModuleStatus.CompilationStatus = EEng_CompilationStatus::Errors;
        UE_LOG(LogTranspersonalGame, Error, TEXT("Found %d orphaned headers without .cpp implementations"), OrphanedCount);
    }
    
    ModuleStatuses.Add(TEXT("TranspersonalGame"), MainModuleStatus);
}

void UEng_ArchitecturalFramework::ScanForOrphanedHeaders()
{
    if (!ModuleStatuses.Contains(TEXT("TranspersonalGame")))
    {
        return;
    }
    
    FEng_ModuleStatus& Status = ModuleStatuses[TEXT("TranspersonalGame")];
    
    UE_LOG(LogTranspersonalGame, Warning, TEXT("Orphaned Headers Scan:"));
    UE_LOG(LogTranspersonalGame, Warning, TEXT("  Total Headers: %d"), Status.HeaderCount);
    UE_LOG(LogTranspersonalGame, Warning, TEXT("  Total Sources: %d"), Status.SourceCount);
    UE_LOG(LogTranspersonalGame, Warning, TEXT("  Orphaned Headers: %d"), Status.OrphanedHeaders);
    
    for (const FString& MissingImpl : Status.MissingImplementations)
    {
        UE_LOG(LogTranspersonalGame, Error, TEXT("  Missing: %s"), *MissingImpl);
    }
}

void UEng_ArchitecturalFramework::GenerateMissingImplementations()
{
    UE_LOG(LogTranspersonalGame, Warning, TEXT("Generate Missing Implementations - Manual intervention required"));
    UE_LOG(LogTranspersonalGame, Warning, TEXT("Each agent must create .cpp files for their .h files"));
}

bool UEng_ArchitecturalFramework::CheckCompilationStatus()
{
    if (!ModuleStatuses.Contains(TEXT("TranspersonalGame")))
    {
        return false;
    }
    
    const FEng_ModuleStatus& Status = ModuleStatuses[TEXT("TranspersonalGame")];
    return Status.CompilationStatus == EEng_CompilationStatus::Clean;
}

void UEng_ArchitecturalFramework::RunArchitecturalAudit()
{
    UE_LOG(LogTranspersonalGame, Warning, TEXT("=== ARCHITECTURAL AUDIT STARTING ==="));
    
    ValidateModuleStructure();
    MonitorSystemPerformance();
    
    // Report audit results
    for (const auto& ModulePair : ModuleStatuses)
    {
        const FEng_ModuleStatus& Status = ModulePair.Value;
        UE_LOG(LogTranspersonalGame, Warning, TEXT("Module: %s"), *Status.ModuleName);
        UE_LOG(LogTranspersonalGame, Warning, TEXT("  Status: %s"), 
            Status.CompilationStatus == EEng_CompilationStatus::Clean ? TEXT("CLEAN") :
            Status.CompilationStatus == EEng_CompilationStatus::Warnings ? TEXT("WARNINGS") :
            Status.CompilationStatus == EEng_CompilationStatus::Errors ? TEXT("ERRORS") : TEXT("BLOCKED"));
        UE_LOG(LogTranspersonalGame, Warning, TEXT("  Headers: %d, Sources: %d, Orphaned: %d"), 
            Status.HeaderCount, Status.SourceCount, Status.OrphanedHeaders);
    }
    
    UE_LOG(LogTranspersonalGame, Warning, TEXT("=== ARCHITECTURAL AUDIT COMPLETE ==="));
}

FEng_ModuleStatus UEng_ArchitecturalFramework::GetModuleStatus(const FString& ModuleName) const
{
    if (ModuleStatuses.Contains(ModuleName))
    {
        return ModuleStatuses[ModuleName];
    }
    
    return FEng_ModuleStatus(); // Return default/empty status
}

TArray<FEng_ModuleStatus> UEng_ArchitecturalFramework::GetAllModuleStatuses() const
{
    TArray<FEng_ModuleStatus> AllStatuses;
    for (const auto& ModulePair : ModuleStatuses)
    {
        AllStatuses.Add(ModulePair.Value);
    }
    return AllStatuses;
}

bool UEng_ArchitecturalFramework::ValidateSystemRequirements(EEng_SystemType SystemType) const
{
    if (!SystemRequirements.Contains(SystemType))
    {
        UE_LOG(LogTranspersonalGame, Warning, TEXT("No requirements defined for system type: %d"), (int32)SystemType);
        return false;
    }
    
    const FEng_SystemRequirements& Reqs = SystemRequirements[SystemType];
    
    // Validate performance tier
    if (CurrentPerformanceTier > Reqs.MinPerformanceTier)
    {
        UE_LOG(LogTranspersonalGame, Error, TEXT("System %d performance below minimum tier"), (int32)SystemType);
        return false;
    }
    
    return true;
}

void UEng_ArchitecturalFramework::MonitorSystemPerformance()
{
    // Get current world
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Count actors by system type
    int32 TotalActors = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        TotalActors++;
    }
    
    // Update performance tier based on actor count and frame time
    if (TotalActors > 5000)
    {
        CurrentPerformanceTier = EEng_PerformanceTier::Low;
    }
    else if (TotalActors > 2000)
    {
        CurrentPerformanceTier = EEng_PerformanceTier::Medium;
    }
    else if (TotalActors > 1000)
    {
        CurrentPerformanceTier = EEng_PerformanceTier::High;
    }
    else
    {
        CurrentPerformanceTier = EEng_PerformanceTier::Critical;
    }
    
    UE_LOG(LogTranspersonalGame, Warning, TEXT("Performance Monitor: %d actors, tier: %d"), 
        TotalActors, (int32)CurrentPerformanceTier);
}

EEng_PerformanceTier UEng_ArchitecturalFramework::GetCurrentPerformanceTier() const
{
    return CurrentPerformanceTier;
}

void UEng_ArchitecturalFramework::EnforcePerformanceLimits()
{
    if (!CurrentRules.bEnforcePerformanceLimits)
    {
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Count and potentially cull actors if over limits
    int32 ActorCount = 0;
    TArray<AActor*> ActorsToDestroy;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        ActorCount++;
        if (ActorCount > CurrentRules.MaxActorsPerSystem)
        {
            // Mark excess actors for destruction (except essential ones)
            AActor* Actor = *ActorItr;
            if (Actor && !Actor->IsA<APawn>() && !Actor->IsA<APlayerController>())
            {
                ActorsToDestroy.Add(Actor);
            }
        }
    }
    
    // Destroy excess actors
    for (AActor* Actor : ActorsToDestroy)
    {
        if (Actor)
        {
            Actor->Destroy();
        }
    }
    
    if (ActorsToDestroy.Num() > 0)
    {
        UE_LOG(LogTranspersonalGame, Warning, TEXT("Performance Enforcement: Destroyed %d excess actors"), 
            ActorsToDestroy.Num());
    }
}

void UEng_ArchitecturalFramework::RegisterSystemRequirements(EEng_SystemType SystemType, const FEng_SystemRequirements& Requirements)
{
    SystemRequirements.Add(SystemType, Requirements);
    UE_LOG(LogTranspersonalGame, Warning, TEXT("Registered requirements for system: %d"), (int32)SystemType);
}

bool UEng_ArchitecturalFramework::ValidateSystemIntegration(EEng_SystemType SystemA, EEng_SystemType SystemB) const
{
    // Check if both systems have defined requirements
    if (!SystemRequirements.Contains(SystemA) || !SystemRequirements.Contains(SystemB))
    {
        return false;
    }
    
    const FEng_SystemRequirements& ReqsA = SystemRequirements[SystemA];
    const FEng_SystemRequirements& ReqsB = SystemRequirements[SystemB];
    
    // Check for module conflicts
    for (const FString& ModuleA : ReqsA.RequiredModules)
    {
        for (const FString& ModuleB : ReqsB.RequiredModules)
        {
            if (ModuleA == ModuleB)
            {
                // Shared module dependency - good for integration
                return true;
            }
        }
    }
    
    return true; // No conflicts found
}

void UEng_ArchitecturalFramework::InitializeSystemDependencies()
{
    UE_LOG(LogTranspersonalGame, Warning, TEXT("Initializing System Dependencies..."));
    
    // Validate all registered systems
    for (const auto& SystemPair : SystemRequirements)
    {
        EEng_SystemType SystemType = SystemPair.Key;
        const FEng_SystemRequirements& Reqs = SystemPair.Value;
        
        UE_LOG(LogTranspersonalGame, Warning, TEXT("System %d requires %d modules, %d classes"), 
            (int32)SystemType, Reqs.RequiredModules.Num(), Reqs.RequiredClasses.Num());
    }
}

void UEng_ArchitecturalFramework::CheckSystemDependencies()
{
    // This would check if required modules and classes exist
    // For now, just log the check
    UE_LOG(LogTranspersonalGame, Warning, TEXT("System Dependencies Check - Implementation needed"));
}

void UEng_ArchitecturalFramework::SetArchitecturalRules(const FEng_ArchitecturalRules& Rules)
{
    CurrentRules = Rules;
    UE_LOG(LogTranspersonalGame, Warning, TEXT("Architectural Rules Updated"));
}

FEng_ArchitecturalRules UEng_ArchitecturalFramework::GetArchitecturalRules() const
{
    return CurrentRules;
}

bool UEng_ArchitecturalFramework::ValidateAgentOutput(int32 AgentID, const TArray<FString>& CreatedFiles) const
{
    UE_LOG(LogTranspersonalGame, Warning, TEXT("Validating Agent %d output: %d files"), AgentID, CreatedFiles.Num());
    
    // Check for header/cpp pairs
    if (CurrentRules.bEnforceHeaderCppPairs)
    {
        TArray<FString> HeaderFiles;
        TArray<FString> SourceFiles;
        
        for (const FString& File : CreatedFiles)
        {
            if (File.EndsWith(TEXT(".h")))
            {
                HeaderFiles.Add(File);
            }
            else if (File.EndsWith(TEXT(".cpp")))
            {
                SourceFiles.Add(File);
            }
        }
        
        // Each header should have a corresponding source
        for (const FString& Header : HeaderFiles)
        {
            FString ExpectedSource = Header;
            ExpectedSource = ExpectedSource.Replace(TEXT(".h"), TEXT(".cpp"));
            
            if (!SourceFiles.Contains(ExpectedSource))
            {
                UE_LOG(LogTranspersonalGame, Error, TEXT("Agent %d created orphaned header: %s"), AgentID, *Header);
                return false;
            }
        }
    }
    
    return true;
}

void UEng_ArchitecturalFramework::UpdatePerformanceMetrics()
{
    // Update performance metrics - called internally
    MonitorSystemPerformance();
}