#include "Eng_ArchitecturalFramework.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "UObject/UObjectGlobals.h"
#include "Engine/GameInstance.h"

UEng_ArchitecturalFramework::UEng_ArchitecturalFramework()
{
    bFrameworkInitialized = false;
    CurrentPerformanceTier = EEng_PerformanceTier::Critical;
    
    // Initialize default architectural rules
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
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architectural Framework - Initializing..."));
    
    InitializeSystemRequirements();
    ValidateModuleStructure();
    
    bFrameworkInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architectural Framework - Initialized successfully"));
}

void UEng_ArchitecturalFramework::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architectural Framework - Deinitializing..."));
    
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
    
    // World System Requirements
    FEng_SystemRequirements WorldReqs;
    WorldReqs.SystemType = EEng_SystemType::World;
    WorldReqs.RequiredModules.Add(TEXT("Landscape"));
    WorldReqs.RequiredModules.Add(TEXT("Foliage"));
    WorldReqs.RequiredModules.Add(TEXT("PCG"));
    WorldReqs.RequiredClasses.Add(TEXT("ALandscape"));
    WorldReqs.RequiredClasses.Add(TEXT("UPCGComponent"));
    WorldReqs.MinPerformanceTier = EEng_PerformanceTier::High;
    WorldReqs.MaxActorCount = 5000;
    WorldReqs.MaxMemoryMB = 1024.0f;
    SystemRequirements.Add(EEng_SystemType::World, WorldReqs);
    
    // Character System Requirements
    FEng_SystemRequirements CharacterReqs;
    CharacterReqs.SystemType = EEng_SystemType::Character;
    CharacterReqs.RequiredModules.Add(TEXT("Engine"));
    CharacterReqs.RequiredModules.Add(TEXT("AIModule"));
    CharacterReqs.RequiredClasses.Add(TEXT("ACharacter"));
    CharacterReqs.RequiredClasses.Add(TEXT("UCharacterMovementComponent"));
    CharacterReqs.MinPerformanceTier = EEng_PerformanceTier::Critical;
    CharacterReqs.MaxActorCount = 200;
    CharacterReqs.MaxMemoryMB = 512.0f;
    SystemRequirements.Add(EEng_SystemType::Character, CharacterReqs);
    
    // AI System Requirements
    FEng_SystemRequirements AIReqs;
    AIReqs.SystemType = EEng_SystemType::AI;
    AIReqs.RequiredModules.Add(TEXT("AIModule"));
    AIReqs.RequiredModules.Add(TEXT("GameplayTasks"));
    AIReqs.RequiredModules.Add(TEXT("NavigationSystem"));
    AIReqs.RequiredClasses.Add(TEXT("UBehaviorTreeComponent"));
    AIReqs.RequiredClasses.Add(TEXT("UBlackboardComponent"));
    AIReqs.MinPerformanceTier = EEng_PerformanceTier::High;
    AIReqs.MaxActorCount = 1000;
    AIReqs.MaxMemoryMB = 768.0f;
    SystemRequirements.Add(EEng_SystemType::AI, AIReqs);
    
    // Performance System Requirements
    FEng_SystemRequirements PerfReqs;
    PerfReqs.SystemType = EEng_SystemType::Performance;
    PerfReqs.RequiredModules.Add(TEXT("RenderCore"));
    PerfReqs.RequiredModules.Add(TEXT("Engine"));
    PerfReqs.MinPerformanceTier = EEng_PerformanceTier::Critical;
    PerfReqs.MaxActorCount = 50;
    PerfReqs.MaxMemoryMB = 128.0f;
    SystemRequirements.Add(EEng_SystemType::Performance, PerfReqs);
}

void UEng_ArchitecturalFramework::ValidateModuleStructure()
{
    UE_LOG(LogTemp, Warning, TEXT("Validating module structure..."));
    
    // Get the TranspersonalGame source directory
    FString ProjectDir = FPaths::ProjectDir();
    FString SourceDir = FPaths::Combine(ProjectDir, TEXT("Source"), TEXT("TranspersonalGame"));
    
    ScanModuleDirectory(SourceDir);
    
    UE_LOG(LogTemp, Warning, TEXT("Module structure validation complete"));
}

void UEng_ArchitecturalFramework::ScanModuleDirectory(const FString& ModulePath)
{
    TArray<FString> FoundFiles;
    IFileManager& FileManager = IFileManager::Get();
    
    // Find all .h files
    FileManager.FindFilesRecursive(FoundFiles, *ModulePath, TEXT("*.h"), true, false);
    
    FEng_ModuleStatus ModuleStatus;
    ModuleStatus.ModuleName = TEXT("TranspersonalGame");
    ModuleStatus.HeaderCount = FoundFiles.Num();
    
    // Check for corresponding .cpp files
    int32 OrphanedCount = 0;
    for (const FString& HeaderFile : FoundFiles)
    {
        FString CppFile = HeaderFile.Replace(TEXT(".h"), TEXT(".cpp"));
        if (!FileManager.FileExists(*CppFile))
        {
            OrphanedCount++;
            FString RelativePath = HeaderFile.Replace(*ModulePath, TEXT(""));
            ModuleStatus.MissingImplementations.Add(RelativePath);
        }
    }
    
    ModuleStatus.OrphanedHeaders = OrphanedCount;
    
    // Count .cpp files
    FoundFiles.Empty();
    FileManager.FindFilesRecursive(FoundFiles, *ModulePath, TEXT("*.cpp"), true, false);
    ModuleStatus.SourceCount = FoundFiles.Num();
    
    // Determine compilation status
    if (OrphanedCount == 0)
    {
        ModuleStatus.CompilationStatus = EEng_CompilationStatus::Clean;
    }
    else if (OrphanedCount < 5)
    {
        ModuleStatus.CompilationStatus = EEng_CompilationStatus::Warnings;
    }
    else
    {
        ModuleStatus.CompilationStatus = EEng_CompilationStatus::Errors;
    }
    
    ModuleStatuses.Add(ModuleStatus.ModuleName, ModuleStatus);
    
    UE_LOG(LogTemp, Warning, TEXT("Module: %s - Headers: %d, Sources: %d, Orphaned: %d"), 
           *ModuleStatus.ModuleName, ModuleStatus.HeaderCount, ModuleStatus.SourceCount, ModuleStatus.OrphanedHeaders);
}

FEng_ModuleStatus UEng_ArchitecturalFramework::GetModuleStatus(const FString& ModuleName) const
{
    if (const FEng_ModuleStatus* Status = ModuleStatuses.Find(ModuleName))
    {
        return *Status;
    }
    
    return FEng_ModuleStatus(); // Return default empty status
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
    if (const FEng_SystemRequirements* Requirements = SystemRequirements.Find(SystemType))
    {
        // Check if all required modules are available
        for (const FString& RequiredModule : Requirements->RequiredModules)
        {
            // In a real implementation, we would check if the module is loaded
            // For now, we'll assume they are available
        }
        
        // Check performance requirements
        if (CurrentPerformanceTier < Requirements->MinPerformanceTier)
        {
            UE_LOG(LogTemp, Warning, TEXT("System %d performance below minimum tier"), (int32)SystemType);
            return false;
        }
        
        return true;
    }
    
    return false;
}

void UEng_ArchitecturalFramework::ScanForOrphanedHeaders()
{
    ValidateModuleStructure();
    
    for (const auto& StatusPair : ModuleStatuses)
    {
        const FEng_ModuleStatus& Status = StatusPair.Value;
        if (Status.OrphanedHeaders > 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("Module %s has %d orphaned headers"), 
                   *Status.ModuleName, Status.OrphanedHeaders);
            
            for (const FString& MissingImpl : Status.MissingImplementations)
            {
                UE_LOG(LogTemp, Warning, TEXT("  Missing implementation: %s"), *MissingImpl);
            }
        }
    }
}

void UEng_ArchitecturalFramework::GenerateMissingImplementations()
{
    UE_LOG(LogTemp, Warning, TEXT("GenerateMissingImplementations called - would generate .cpp stubs for orphaned headers"));
    
    // In a real implementation, this would generate basic .cpp files
    // For now, we'll just log what needs to be done
    ScanForOrphanedHeaders();
}

bool UEng_ArchitecturalFramework::CheckCompilationStatus()
{
    ValidateModuleStructure();
    
    for (const auto& StatusPair : ModuleStatuses)
    {
        const FEng_ModuleStatus& Status = StatusPair.Value;
        if (Status.CompilationStatus == EEng_CompilationStatus::Errors || 
            Status.CompilationStatus == EEng_CompilationStatus::Blocked)
        {
            return false;
        }
    }
    
    return true;
}

void UEng_ArchitecturalFramework::RunArchitecturalAudit()
{
    UE_LOG(LogTemp, Warning, TEXT("=== ARCHITECTURAL AUDIT STARTING ==="));
    
    ValidateModuleStructure();
    ScanForOrphanedHeaders();
    MonitorSystemPerformance();
    CheckSystemDependencies();
    
    UE_LOG(LogTemp, Warning, TEXT("=== ARCHITECTURAL AUDIT COMPLETE ==="));
}

void UEng_ArchitecturalFramework::MonitorSystemPerformance()
{
    UpdatePerformanceMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("Current Performance Tier: %d"), (int32)CurrentPerformanceTier);
    
    if (CurrentPerformanceTier < EEng_PerformanceTier::High)
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance below optimal - consider optimization"));
    }
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
    
    // Check frame time
    float CurrentFrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
    
    if (CurrentFrameTime > CurrentRules.MaxFrameTimeMS)
    {
        UE_LOG(LogTemp, Warning, TEXT("Frame time exceeded limit: %.2fms > %.2fms"), 
               CurrentFrameTime, CurrentRules.MaxFrameTimeMS);
        
        // Update performance tier based on frame time
        if (CurrentFrameTime > 33.33f) // 30fps
        {
            CurrentPerformanceTier = EEng_PerformanceTier::Low;
        }
        else if (CurrentFrameTime > 22.22f) // 45fps
        {
            CurrentPerformanceTier = EEng_PerformanceTier::Medium;
        }
        else
        {
            CurrentPerformanceTier = EEng_PerformanceTier::High;
        }
    }
    else
    {
        CurrentPerformanceTier = EEng_PerformanceTier::Critical;
    }
}

void UEng_ArchitecturalFramework::RegisterSystemRequirements(EEng_SystemType SystemType, const FEng_SystemRequirements& Requirements)
{
    SystemRequirements.Add(SystemType, Requirements);
    UE_LOG(LogTemp, Warning, TEXT("Registered system requirements for type: %d"), (int32)SystemType);
}

bool UEng_ArchitecturalFramework::ValidateSystemIntegration(EEng_SystemType SystemA, EEng_SystemType SystemB) const
{
    // Check if both systems have valid requirements
    const FEng_SystemRequirements* ReqsA = SystemRequirements.Find(SystemA);
    const FEng_SystemRequirements* ReqsB = SystemRequirements.Find(SystemB);
    
    if (!ReqsA || !ReqsB)
    {
        return false;
    }
    
    // Check for conflicting performance requirements
    if (ReqsA->MinPerformanceTier != ReqsB->MinPerformanceTier)
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance tier mismatch between systems %d and %d"), 
               (int32)SystemA, (int32)SystemB);
    }
    
    // Check for shared module dependencies
    for (const FString& ModuleA : ReqsA->RequiredModules)
    {
        for (const FString& ModuleB : ReqsB->RequiredModules)
        {
            if (ModuleA == ModuleB)
            {
                UE_LOG(LogTemp, Log, TEXT("Systems %d and %d share module dependency: %s"), 
                       (int32)SystemA, (int32)SystemB, *ModuleA);
            }
        }
    }
    
    return true;
}

void UEng_ArchitecturalFramework::InitializeSystemDependencies()
{
    UE_LOG(LogTemp, Warning, TEXT("Initializing system dependencies..."));
    
    // Validate all system integrations
    TArray<EEng_SystemType> SystemTypes = {
        EEng_SystemType::Core,
        EEng_SystemType::World,
        EEng_SystemType::Character,
        EEng_SystemType::AI,
        EEng_SystemType::Performance
    };
    
    for (int32 i = 0; i < SystemTypes.Num(); i++)
    {
        for (int32 j = i + 1; j < SystemTypes.Num(); j++)
        {
            ValidateSystemIntegration(SystemTypes[i], SystemTypes[j]);
        }
    }
}

void UEng_ArchitecturalFramework::SetArchitecturalRules(const FEng_ArchitecturalRules& Rules)
{
    CurrentRules = Rules;
    UE_LOG(LogTemp, Warning, TEXT("Architectural rules updated"));
}

FEng_ArchitecturalRules UEng_ArchitecturalFramework::GetArchitecturalRules() const
{
    return CurrentRules;
}

bool UEng_ArchitecturalFramework::ValidateAgentOutput(int32 AgentID, const TArray<FString>& CreatedFiles) const
{
    if (!CurrentRules.bEnforceHeaderCppPairs)
    {
        return true;
    }
    
    // Check that every .h file has a corresponding .cpp file
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
    
    for (const FString& HeaderFile : HeaderFiles)
    {
        FString ExpectedCppFile = HeaderFile.Replace(TEXT(".h"), TEXT(".cpp"));
        if (!SourceFiles.Contains(ExpectedCppFile))
        {
            UE_LOG(LogTemp, Warning, TEXT("Agent %d created orphaned header: %s"), AgentID, *HeaderFile);
            return false;
        }
    }
    
    return true;
}

void UEng_ArchitecturalFramework::ValidateHeaderCppPairs(const FString& ModulePath)
{
    // This is called by ScanModuleDirectory and updates the ModuleStatus
    // Implementation is already handled in ScanModuleDirectory
}

void UEng_ArchitecturalFramework::CheckSystemDependencies()
{
    UE_LOG(LogTemp, Warning, TEXT("Checking system dependencies..."));
    
    for (const auto& SystemPair : SystemRequirements)
    {
        const FEng_SystemRequirements& Requirements = SystemPair.Value;
        
        UE_LOG(LogTemp, Log, TEXT("System %d requires %d modules"), 
               (int32)Requirements.SystemType, Requirements.RequiredModules.Num());
        
        for (const FString& RequiredModule : Requirements.RequiredModules)
        {
            UE_LOG(LogTemp, Log, TEXT("  - %s"), *RequiredModule);
        }
    }
}

void UEng_ArchitecturalFramework::UpdatePerformanceMetrics()
{
    // Update current performance tier based on various metrics
    float CurrentFPS = 1.0f / FApp::GetDeltaTime();
    
    if (CurrentFPS >= 60.0f)
    {
        CurrentPerformanceTier = EEng_PerformanceTier::Critical;
    }
    else if (CurrentFPS >= 45.0f)
    {
        CurrentPerformanceTier = EEng_PerformanceTier::High;
    }
    else if (CurrentFPS >= 30.0f)
    {
        CurrentPerformanceTier = EEng_PerformanceTier::Medium;
    }
    else
    {
        CurrentPerformanceTier = EEng_PerformanceTier::Low;
    }
}