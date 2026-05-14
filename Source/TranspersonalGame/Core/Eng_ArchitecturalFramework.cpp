#include "Eng_ArchitecturalFramework.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Engine/GameInstance.h"
#include "UObject/UObjectGlobals.h"

UEng_ArchitecturalFramework::UEng_ArchitecturalFramework()
{
    bFrameworkInitialized = false;
    CurrentPerformanceTier = EEng_PerformanceTier::Critical;
    
    // Initialize default architectural rules
    CurrentRules = FEng_ArchitecturalRules();
}

void UEng_ArchitecturalFramework::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architectural Framework Initializing..."));
    
    InitializeSystemRequirements();
    ValidateModuleStructure();
    
    bFrameworkInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architectural Framework Initialized Successfully"));
}

void UEng_ArchitecturalFramework::Deinitialize()
{
    bFrameworkInitialized = false;
    SystemRequirements.Empty();
    ModuleStatuses.Empty();
    
    Super::Deinitialize();
}

void UEng_ArchitecturalFramework::InitializeSystemRequirements()
{
    // Core System Requirements
    FEng_SystemRequirements CoreReqs;
    CoreReqs.SystemType = EEng_SystemType::Core;
    CoreReqs.RequiredModules.Add(TEXT("Engine"));
    CoreReqs.RequiredModules.Add(TEXT("CoreUObject"));
    CoreReqs.RequiredClasses.Add(TEXT("GameMode"));
    CoreReqs.RequiredClasses.Add(TEXT("PlayerController"));
    CoreReqs.MinPerformanceTier = EEng_PerformanceTier::Critical;
    CoreReqs.MaxActorCount = 500;
    CoreReqs.MaxMemoryMB = 256.0f;
    SystemRequirements.Add(EEng_SystemType::Core, CoreReqs);
    
    // World System Requirements
    FEng_SystemRequirements WorldReqs;
    WorldReqs.SystemType = EEng_SystemType::World;
    WorldReqs.RequiredModules.Add(TEXT("Landscape"));
    WorldReqs.RequiredModules.Add(TEXT("PCG"));
    WorldReqs.RequiredClasses.Add(TEXT("WorldGenerator"));
    WorldReqs.RequiredClasses.Add(TEXT("BiomeManager"));
    WorldReqs.MinPerformanceTier = EEng_PerformanceTier::High;
    WorldReqs.MaxActorCount = 2000;
    WorldReqs.MaxMemoryMB = 512.0f;
    SystemRequirements.Add(EEng_SystemType::World, WorldReqs);
    
    // Character System Requirements
    FEng_SystemRequirements CharacterReqs;
    CharacterReqs.SystemType = EEng_SystemType::Character;
    CharacterReqs.RequiredModules.Add(TEXT("AIModule"));
    CharacterReqs.RequiredClasses.Add(TEXT("Character"));
    CharacterReqs.RequiredClasses.Add(TEXT("PlayerController"));
    CharacterReqs.MinPerformanceTier = EEng_PerformanceTier::Critical;
    CharacterReqs.MaxActorCount = 100;
    CharacterReqs.MaxMemoryMB = 128.0f;
    SystemRequirements.Add(EEng_SystemType::Character, CharacterReqs);
    
    // AI System Requirements
    FEng_SystemRequirements AIReqs;
    AIReqs.SystemType = EEng_SystemType::AI;
    AIReqs.RequiredModules.Add(TEXT("AIModule"));
    AIReqs.RequiredModules.Add(TEXT("NavigationSystem"));
    AIReqs.RequiredClasses.Add(TEXT("DinosaurAI"));
    AIReqs.RequiredClasses.Add(TEXT("BehaviorTree"));
    AIReqs.MinPerformanceTier = EEng_PerformanceTier::High;
    AIReqs.MaxActorCount = 500;
    AIReqs.MaxMemoryMB = 256.0f;
    SystemRequirements.Add(EEng_SystemType::AI, AIReqs);
    
    UE_LOG(LogTemp, Warning, TEXT("Architectural Framework: System requirements initialized"));
}

void UEng_ArchitecturalFramework::ValidateModuleStructure()
{
    FString ModulePath = FPaths::ProjectDir() + TEXT("Source/TranspersonalGame/");
    
    if (!IFileManager::Get().DirectoryExists(*ModulePath))
    {
        UE_LOG(LogTemp, Error, TEXT("Module directory not found: %s"), *ModulePath);
        return;
    }
    
    ScanModuleDirectory(ModulePath);
    ValidateHeaderCppPairs(ModulePath);
    
    UE_LOG(LogTemp, Warning, TEXT("Module structure validation completed"));
}

void UEng_ArchitecturalFramework::ScanModuleDirectory(const FString& ModulePath)
{
    TArray<FString> FoundFiles;
    IFileManager::Get().FindFilesRecursive(FoundFiles, *ModulePath, TEXT("*.h"), true, false);
    
    FEng_ModuleStatus MainModuleStatus;
    MainModuleStatus.ModuleName = TEXT("TranspersonalGame");
    MainModuleStatus.HeaderCount = 0;
    MainModuleStatus.SourceCount = 0;
    MainModuleStatus.OrphanedHeaders = 0;
    
    // Count headers
    for (const FString& HeaderFile : FoundFiles)
    {
        if (HeaderFile.EndsWith(TEXT(".h")))
        {
            MainModuleStatus.HeaderCount++;
            
            // Check if corresponding .cpp exists
            FString CppFile = HeaderFile;
            CppFile = CppFile.Replace(TEXT(".h"), TEXT(".cpp"));
            
            if (!IFileManager::Get().FileExists(*CppFile))
            {
                MainModuleStatus.OrphanedHeaders++;
                FString RelativePath = HeaderFile;
                FPaths::MakePathRelativeTo(RelativePath, *ModulePath);
                MainModuleStatus.MissingImplementations.Add(RelativePath);
            }
        }
    }
    
    // Count source files
    FoundFiles.Empty();
    IFileManager::Get().FindFilesRecursive(FoundFiles, *ModulePath, TEXT("*.cpp"), true, false);
    MainModuleStatus.SourceCount = FoundFiles.Num();
    
    // Determine compilation status
    if (MainModuleStatus.OrphanedHeaders == 0)
    {
        MainModuleStatus.CompilationStatus = EEng_CompilationStatus::Clean;
    }
    else if (MainModuleStatus.OrphanedHeaders < 5)
    {
        MainModuleStatus.CompilationStatus = EEng_CompilationStatus::Warnings;
    }
    else
    {
        MainModuleStatus.CompilationStatus = EEng_CompilationStatus::Errors;
    }
    
    ModuleStatuses.Add(TEXT("TranspersonalGame"), MainModuleStatus);
    
    UE_LOG(LogTemp, Warning, TEXT("Module scan complete: %d headers, %d sources, %d orphaned"), 
           MainModuleStatus.HeaderCount, MainModuleStatus.SourceCount, MainModuleStatus.OrphanedHeaders);
}

void UEng_ArchitecturalFramework::ValidateHeaderCppPairs(const FString& ModulePath)
{
    if (!ModuleStatuses.Contains(TEXT("TranspersonalGame")))
    {
        return;
    }
    
    FEng_ModuleStatus& Status = ModuleStatuses[TEXT("TranspersonalGame")];
    
    // Additional validation logic for header/cpp pairs
    for (const FString& MissingImpl : Status.MissingImplementations)
    {
        UE_LOG(LogTemp, Warning, TEXT("Missing implementation for: %s"), *MissingImpl);
    }
}

FEng_ModuleStatus UEng_ArchitecturalFramework::GetModuleStatus(const FString& ModuleName) const
{
    if (ModuleStatuses.Contains(ModuleName))
    {
        return ModuleStatuses[ModuleName];
    }
    
    return FEng_ModuleStatus();
}

TArray<FEng_ModuleStatus> UEng_ArchitecturalFramework::GetAllModuleStatuses() const
{
    TArray<FEng_ModuleStatus> AllStatuses;
    
    for (const auto& StatusPair : ModuleStatuses)
    {
        AllStatuses.Add(StatusPair.Value);
    }
    
    return AllStatuses;
}

bool UEng_ArchitecturalFramework::ValidateSystemRequirements(EEng_SystemType SystemType) const
{
    if (!SystemRequirements.Contains(SystemType))
    {
        return false;
    }
    
    const FEng_SystemRequirements& Reqs = SystemRequirements[SystemType];
    
    // Validate required modules are loaded
    for (const FString& RequiredModule : Reqs.RequiredModules)
    {
        // In a real implementation, we would check if the module is actually loaded
        UE_LOG(LogTemp, Log, TEXT("Validating module: %s"), *RequiredModule);
    }
    
    return true;
}

void UEng_ArchitecturalFramework::ScanForOrphanedHeaders()
{
    ValidateModuleStructure();
}

void UEng_ArchitecturalFramework::GenerateMissingImplementations()
{
    if (!ModuleStatuses.Contains(TEXT("TranspersonalGame")))
    {
        return;
    }
    
    const FEng_ModuleStatus& Status = ModuleStatuses[TEXT("TranspersonalGame")];
    
    for (const FString& MissingImpl : Status.MissingImplementations)
    {
        UE_LOG(LogTemp, Warning, TEXT("Would generate implementation for: %s"), *MissingImpl);
        // In a real implementation, we would generate stub .cpp files here
    }
}

bool UEng_ArchitecturalFramework::CheckCompilationStatus()
{
    ValidateModuleStructure();
    
    if (!ModuleStatuses.Contains(TEXT("TranspersonalGame")))
    {
        return false;
    }
    
    const FEng_ModuleStatus& Status = ModuleStatuses[TEXT("TranspersonalGame")];
    return Status.CompilationStatus == EEng_CompilationStatus::Clean;
}

void UEng_ArchitecturalFramework::RunArchitecturalAudit()
{
    UE_LOG(LogTemp, Warning, TEXT("=== ARCHITECTURAL AUDIT STARTING ==="));
    
    ValidateModuleStructure();
    MonitorSystemPerformance();
    CheckSystemDependencies();
    
    // Report findings
    for (const auto& StatusPair : ModuleStatuses)
    {
        const FEng_ModuleStatus& Status = StatusPair.Value;
        UE_LOG(LogTemp, Warning, TEXT("Module %s: %d headers, %d sources, %d orphaned"), 
               *Status.ModuleName, Status.HeaderCount, Status.SourceCount, Status.OrphanedHeaders);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== ARCHITECTURAL AUDIT COMPLETE ==="));
}

void UEng_ArchitecturalFramework::MonitorSystemPerformance()
{
    // Get current world and count actors
    UWorld* World = GetWorld();
    if (!World)
    {
        CurrentPerformanceTier = EEng_PerformanceTier::Low;
        return;
    }
    
    int32 ActorCount = World->GetActorCount();
    
    // Simple performance tier calculation based on actor count
    if (ActorCount < 1000)
    {
        CurrentPerformanceTier = EEng_PerformanceTier::Critical;
    }
    else if (ActorCount < 2000)
    {
        CurrentPerformanceTier = EEng_PerformanceTier::High;
    }
    else if (ActorCount < 5000)
    {
        CurrentPerformanceTier = EEng_PerformanceTier::Medium;
    }
    else
    {
        CurrentPerformanceTier = EEng_PerformanceTier::Low;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Performance monitoring: %d actors, tier %d"), 
           ActorCount, (int32)CurrentPerformanceTier);
}

EEng_PerformanceTier UEng_ArchitecturalFramework::GetCurrentPerformanceTier() const
{
    return CurrentPerformanceTier;
}

void UEng_ArchitecturalFramework::EnforcePerformanceLimits()
{
    MonitorSystemPerformance();
    
    if (CurrentPerformanceTier == EEng_PerformanceTier::Low)
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance tier LOW - enforcement needed"));
        // In a real implementation, we would trigger cleanup or optimization
    }
}

void UEng_ArchitecturalFramework::RegisterSystemRequirements(EEng_SystemType SystemType, const FEng_SystemRequirements& Requirements)
{
    SystemRequirements.Add(SystemType, Requirements);
    UE_LOG(LogTemp, Log, TEXT("Registered requirements for system type %d"), (int32)SystemType);
}

bool UEng_ArchitecturalFramework::ValidateSystemIntegration(EEng_SystemType SystemA, EEng_SystemType SystemB) const
{
    // Basic integration validation
    bool bAExists = SystemRequirements.Contains(SystemA);
    bool bBExists = SystemRequirements.Contains(SystemB);
    
    return bAExists && bBExists;
}

void UEng_ArchitecturalFramework::InitializeSystemDependencies()
{
    CheckSystemDependencies();
}

void UEng_ArchitecturalFramework::CheckSystemDependencies()
{
    for (const auto& SystemPair : SystemRequirements)
    {
        const FEng_SystemRequirements& Reqs = SystemPair.Value;
        ValidateSystemRequirements(Reqs.SystemType);
    }
}

void UEng_ArchitecturalFramework::SetArchitecturalRules(const FEng_ArchitecturalRules& Rules)
{
    CurrentRules = Rules;
    UE_LOG(LogTemp, Log, TEXT("Architectural rules updated"));
}

FEng_ArchitecturalRules UEng_ArchitecturalFramework::GetArchitecturalRules() const
{
    return CurrentRules;
}

bool UEng_ArchitecturalFramework::ValidateAgentOutput(int32 AgentID, const TArray<FString>& CreatedFiles) const
{
    // Validate that agent created proper .h/.cpp pairs
    TArray<FString> Headers;
    TArray<FString> Sources;
    
    for (const FString& File : CreatedFiles)
    {
        if (File.EndsWith(TEXT(".h")))
        {
            Headers.Add(File);
        }
        else if (File.EndsWith(TEXT(".cpp")))
        {
            Sources.Add(File);
        }
    }
    
    // Check for orphaned headers
    for (const FString& Header : Headers)
    {
        FString ExpectedSource = Header;
        ExpectedSource = ExpectedSource.Replace(TEXT(".h"), TEXT(".cpp"));
        
        if (!Sources.Contains(ExpectedSource))
        {
            UE_LOG(LogTemp, Warning, TEXT("Agent %d created orphaned header: %s"), AgentID, *Header);
            return false;
        }
    }
    
    return true;
}

void UEng_ArchitecturalFramework::UpdatePerformanceMetrics()
{
    MonitorSystemPerformance();
}