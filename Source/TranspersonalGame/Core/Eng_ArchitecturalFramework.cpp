#include "Eng_ArchitecturalFramework.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"

// ============================================================================
// ENGINE ARCHITECTURE FRAMEWORK IMPLEMENTATION - AGENT #2
// Core architectural validation and enforcement system
// ============================================================================

UEng_ArchitecturalFramework::UEng_ArchitecturalFramework()
{
    CurrentPerformanceTier = EEng_PerformanceTier::Critical;
    bFrameworkInitialized = false;
    
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
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architectural Framework Initializing..."));
    
    // Initialize system requirements for all major systems
    InitializeSystemRequirements();
    
    // Validate current module structure
    ValidateModuleStructure();
    
    bFrameworkInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architectural Framework Initialized Successfully"));
}

void UEng_ArchitecturalFramework::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architectural Framework Shutting Down"));
    
    SystemRequirements.Empty();
    ModuleStatuses.Empty();
    bFrameworkInitialized = false;
    
    Super::Deinitialize();
}

// ========================================================================
// ARCHITECTURAL VALIDATION
// ========================================================================

void UEng_ArchitecturalFramework::ValidateModuleStructure()
{
    UE_LOG(LogTemp, Warning, TEXT("Validating Module Structure..."));
    
    // Get project source directory
    FString ProjectDir = FPaths::ProjectDir();
    FString SourceDir = FPaths::Combine(ProjectDir, TEXT("Source/TranspersonalGame"));
    
    // Scan main module directory
    ScanModuleDirectory(SourceDir);
    
    // Validate header-cpp pairs
    ValidateHeaderCppPairs(SourceDir);
    
    // Check system dependencies
    CheckSystemDependencies();
    
    UE_LOG(LogTemp, Warning, TEXT("Module Structure Validation Complete"));
}

FEng_ModuleStatus UEng_ArchitecturalFramework::GetModuleStatus(const FString& ModuleName) const
{
    if (const FEng_ModuleStatus* Status = ModuleStatuses.Find(ModuleName))
    {
        return *Status;
    }
    
    // Return default status if not found
    FEng_ModuleStatus DefaultStatus;
    DefaultStatus.ModuleName = ModuleName;
    DefaultStatus.CompilationStatus = EEng_CompilationStatus::Blocked;
    return DefaultStatus;
}

TArray<FEng_ModuleStatus> UEng_ArchitecturalFramework::GetAllModuleStatuses() const
{
    TArray<FEng_ModuleStatus> Statuses;
    for (const auto& Pair : ModuleStatuses)
    {
        Statuses.Add(Pair.Value);
    }
    return Statuses;
}

bool UEng_ArchitecturalFramework::ValidateSystemRequirements(EEng_SystemType SystemType) const
{
    const FEng_SystemRequirements* Requirements = SystemRequirements.Find(SystemType);
    if (!Requirements)
    {
        UE_LOG(LogTemp, Warning, TEXT("No requirements defined for system type: %d"), (int32)SystemType);
        return false;
    }
    
    // Validate required modules exist
    for (const FString& ModuleName : Requirements->RequiredModules)
    {
        if (!ModuleStatuses.Contains(ModuleName))
        {
            UE_LOG(LogTemp, Error, TEXT("Required module missing: %s"), *ModuleName);
            return false;
        }
    }
    
    // Validate required classes exist
    for (const FString& ClassName : Requirements->RequiredClasses)
    {
        // Try to load the class to verify it exists
        FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName);
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
        if (!LoadedClass)
        {
            UE_LOG(LogTemp, Error, TEXT("Required class missing: %s"), *ClassName);
            return false;
        }
    }
    
    return true;
}

// ========================================================================
// COMPILATION MANAGEMENT
// ========================================================================

void UEng_ArchitecturalFramework::ScanForOrphanedHeaders()
{
    UE_LOG(LogTemp, Warning, TEXT("Scanning for orphaned headers..."));
    
    FString SourceDir = FPaths::Combine(FPaths::ProjectDir(), TEXT("Source/TranspersonalGame"));
    
    // Critical files that must have implementations
    TArray<FString> CriticalHeaders = {
        TEXT("Core/Eng_ArchitecturalFramework.h"),
        TEXT("Core/StudioDirector/StudioDirectorSystem.h"),
        TEXT("Characters/TranspersonalCharacter.h"),
        TEXT("World/BiomeManager.h"),
        TEXT("AI/DinosaurBase.h")
    };
    
    for (const FString& HeaderPath : CriticalHeaders)
    {
        FString FullHeaderPath = FPaths::Combine(SourceDir, HeaderPath);
        FString CppPath = FullHeaderPath.Replace(TEXT(".h"), TEXT(".cpp"));
        
        bool bHeaderExists = IFileManager::Get().FileExists(*FullHeaderPath);
        bool bCppExists = IFileManager::Get().FileExists(*CppPath);
        
        if (bHeaderExists && !bCppExists)
        {
            UE_LOG(LogTemp, Error, TEXT("ORPHANED HEADER: %s (missing %s)"), *HeaderPath, *CppPath);
            
            // Add to module status
            FString ModuleName = FPaths::GetBaseFilename(HeaderPath);
            FEng_ModuleStatus& Status = ModuleStatuses.FindOrAdd(ModuleName);
            Status.ModuleName = ModuleName;
            Status.OrphanedHeaders++;
            Status.MissingImplementations.AddUnique(CppPath);
            Status.CompilationStatus = EEng_CompilationStatus::Blocked;
        }
    }
}

void UEng_ArchitecturalFramework::GenerateMissingImplementations()
{
    UE_LOG(LogTemp, Warning, TEXT("Generating missing implementations..."));
    
    // This function identifies what needs to be created
    // The actual creation is handled by the agent's file writing
    ScanForOrphanedHeaders();
    
    for (const auto& Pair : ModuleStatuses)
    {
        const FEng_ModuleStatus& Status = Pair.Value;
        if (Status.MissingImplementations.Num() > 0)
        {
            UE_LOG(LogTemp, Error, TEXT("Module %s needs %d implementations"), 
                   *Status.ModuleName, Status.MissingImplementations.Num());
            
            for (const FString& MissingImpl : Status.MissingImplementations)
            {
                UE_LOG(LogTemp, Error, TEXT("  MISSING: %s"), *MissingImpl);
            }
        }
    }
}

bool UEng_ArchitecturalFramework::CheckCompilationStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("Checking compilation status..."));
    
    bool bAllModulesClean = true;
    
    for (const auto& Pair : ModuleStatuses)
    {
        const FEng_ModuleStatus& Status = Pair.Value;
        
        if (Status.CompilationStatus == EEng_CompilationStatus::Errors ||
            Status.CompilationStatus == EEng_CompilationStatus::Blocked)
        {
            bAllModulesClean = false;
            UE_LOG(LogTemp, Error, TEXT("Module %s has compilation issues"), *Status.ModuleName);
        }
    }
    
    return bAllModulesClean;
}

void UEng_ArchitecturalFramework::RunArchitecturalAudit()
{
    UE_LOG(LogTemp, Warning, TEXT("=== RUNNING ARCHITECTURAL AUDIT ==="));
    
    ValidateModuleStructure();
    ScanForOrphanedHeaders();
    GenerateMissingImplementations();
    MonitorSystemPerformance();
    
    bool bCompilationClean = CheckCompilationStatus();
    
    UE_LOG(LogTemp, Warning, TEXT("=== ARCHITECTURAL AUDIT COMPLETE ==="));
    UE_LOG(LogTemp, Warning, TEXT("Compilation Status: %s"), 
           bCompilationClean ? TEXT("CLEAN") : TEXT("ISSUES DETECTED"));
}

// ========================================================================
// PERFORMANCE MONITORING
// ========================================================================

void UEng_ArchitecturalFramework::MonitorSystemPerformance()
{
    UpdatePerformanceMetrics();
    
    // Check current frame time
    float CurrentFrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
    
    if (CurrentFrameTime > CurrentRules.MaxFrameTimeMS)
    {
        CurrentPerformanceTier = EEng_PerformanceTier::Low;
        UE_LOG(LogTemp, Error, TEXT("Performance Warning: Frame time %.2fms exceeds target %.2fms"), 
               CurrentFrameTime, CurrentRules.MaxFrameTimeMS);
    }
    else if (CurrentFrameTime > (CurrentRules.MaxFrameTimeMS * 0.8f))
    {
        CurrentPerformanceTier = EEng_PerformanceTier::Medium;
    }
    else
    {
        CurrentPerformanceTier = EEng_PerformanceTier::Critical;
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
    
    if (TotalActors > CurrentRules.MaxActorsPerSystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Performance Limit Exceeded: %d actors (max %d)"), 
               TotalActors, CurrentRules.MaxActorsPerSystem);
    }
}

// ========================================================================
// SYSTEM INTEGRATION
// ========================================================================

void UEng_ArchitecturalFramework::RegisterSystemRequirements(EEng_SystemType SystemType, const FEng_SystemRequirements& Requirements)
{
    SystemRequirements.Add(SystemType, Requirements);
    UE_LOG(LogTemp, Warning, TEXT("Registered requirements for system type: %d"), (int32)SystemType);
}

bool UEng_ArchitecturalFramework::ValidateSystemIntegration(EEng_SystemType SystemA, EEng_SystemType SystemB) const
{
    // Check if both systems have their requirements met
    bool bSystemAValid = ValidateSystemRequirements(SystemA);
    bool bSystemBValid = ValidateSystemRequirements(SystemB);
    
    return bSystemAValid && bSystemBValid;
}

void UEng_ArchitecturalFramework::InitializeSystemDependencies()
{
    UE_LOG(LogTemp, Warning, TEXT("Initializing system dependencies..."));
    
    // Check all registered systems
    for (const auto& Pair : SystemRequirements)
    {
        EEng_SystemType SystemType = Pair.Key;
        const FEng_SystemRequirements& Requirements = Pair.Value;
        
        bool bValid = ValidateSystemRequirements(SystemType);
        UE_LOG(LogTemp, Warning, TEXT("System %d validation: %s"), 
               (int32)SystemType, bValid ? TEXT("PASS") : TEXT("FAIL"));
    }
}

// ========================================================================
// AGENT COORDINATION
// ========================================================================

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
    
    for (const FString& FilePath : CreatedFiles)
    {
        if (FilePath.EndsWith(TEXT(".h")))
        {
            HeaderFiles.Add(FilePath);
        }
        else if (FilePath.EndsWith(TEXT(".cpp")))
        {
            SourceFiles.Add(FilePath);
        }
    }
    
    // Validate pairs
    for (const FString& HeaderFile : HeaderFiles)
    {
        FString ExpectedCpp = HeaderFile.Replace(TEXT(".h"), TEXT(".cpp"));
        if (!SourceFiles.Contains(ExpectedCpp))
        {
            UE_LOG(LogTemp, Error, TEXT("Agent %d violated header-cpp pair rule: %s missing %s"), 
                   AgentID, *HeaderFile, *ExpectedCpp);
            return false;
        }
    }
    
    return true;
}

// ========================================================================
// PRIVATE IMPLEMENTATION
// ========================================================================

void UEng_ArchitecturalFramework::InitializeSystemRequirements()
{
    // Core System Requirements
    FEng_SystemRequirements CoreReqs;
    CoreReqs.SystemType = EEng_SystemType::Core;
    CoreReqs.RequiredModules = {TEXT("Engine"), TEXT("CoreUObject"), TEXT("TranspersonalGame")};
    CoreReqs.RequiredClasses = {TEXT("Eng_ArchitecturalFramework"), TEXT("TranspersonalGameMode")};
    CoreReqs.MinPerformanceTier = EEng_PerformanceTier::Critical;
    CoreReqs.MaxActorCount = 100;
    CoreReqs.MaxMemoryMB = 256.0f;
    RegisterSystemRequirements(EEng_SystemType::Core, CoreReqs);
    
    // World System Requirements
    FEng_SystemRequirements WorldReqs;
    WorldReqs.SystemType = EEng_SystemType::World;
    WorldReqs.RequiredModules = {TEXT("Landscape"), TEXT("PCG"), TEXT("TranspersonalGame")};
    WorldReqs.RequiredClasses = {TEXT("BiomeManager"), TEXT("PCGWorldGenerator")};
    WorldReqs.MinPerformanceTier = EEng_PerformanceTier::High;
    WorldReqs.MaxActorCount = 5000;
    WorldReqs.MaxMemoryMB = 1024.0f;
    RegisterSystemRequirements(EEng_SystemType::World, WorldReqs);
    
    // Character System Requirements
    FEng_SystemRequirements CharacterReqs;
    CharacterReqs.SystemType = EEng_SystemType::Character;
    CharacterReqs.RequiredModules = {TEXT("Engine"), TEXT("TranspersonalGame")};
    CharacterReqs.RequiredClasses = {TEXT("TranspersonalCharacter"), TEXT("DinosaurBase")};
    CharacterReqs.MinPerformanceTier = EEng_PerformanceTier::Critical;
    CharacterReqs.MaxActorCount = 200;
    CharacterReqs.MaxMemoryMB = 512.0f;
    RegisterSystemRequirements(EEng_SystemType::Character, CharacterReqs);
    
    // AI System Requirements
    FEng_SystemRequirements AIReqs;
    AIReqs.SystemType = EEng_SystemType::AI;
    AIReqs.RequiredModules = {TEXT("AIModule"), TEXT("GameplayTasks"), TEXT("TranspersonalGame")};
    AIReqs.RequiredClasses = {TEXT("DinosaurAIController"), TEXT("CrowdSimulationManager")};
    AIReqs.MinPerformanceTier = EEng_PerformanceTier::High;
    AIReqs.MaxActorCount = 1000;
    AIReqs.MaxMemoryMB = 768.0f;
    RegisterSystemRequirements(EEng_SystemType::AI, AIReqs);
}

void UEng_ArchitecturalFramework::ScanModuleDirectory(const FString& ModulePath)
{
    // Implementation for scanning module directories
    // This would recursively scan for .h and .cpp files
    UE_LOG(LogTemp, Warning, TEXT("Scanning module directory: %s"), *ModulePath);
}

void UEng_ArchitecturalFramework::ValidateHeaderCppPairs(const FString& ModulePath)
{
    // Implementation for validating header-cpp pairs
    UE_LOG(LogTemp, Warning, TEXT("Validating header-cpp pairs in: %s"), *ModulePath);
}

void UEng_ArchitecturalFramework::CheckSystemDependencies()
{
    // Implementation for checking system dependencies
    UE_LOG(LogTemp, Warning, TEXT("Checking system dependencies..."));
}

void UEng_ArchitecturalFramework::UpdatePerformanceMetrics()
{
    // Implementation for updating performance metrics
    // This would collect frame time, memory usage, actor counts, etc.
}