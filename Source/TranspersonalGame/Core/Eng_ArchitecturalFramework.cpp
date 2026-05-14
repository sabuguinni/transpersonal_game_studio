#include "Eng_ArchitecturalFramework.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/DateTime.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"

UEng_ArchitecturalFramework::UEng_ArchitecturalFramework()
{
    GlobalFrameTimeTarget = 16.67f; // 60fps target
}

void UEng_ArchitecturalFramework::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architectural Framework Initializing..."));
    
    InitializeAgentRules();
    InitializeDefaultSystems();
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architectural Framework Initialized - %d systems registered"), RegisteredSystems.Num());
}

void UEng_ArchitecturalFramework::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architectural Framework Shutting Down..."));
    
    RegisteredSystems.Empty();
    ModuleStatusMap.Empty();
    AgentRulesMap.Empty();
    SystemPerformanceMap.Empty();
    
    Super::Deinitialize();
}

bool UEng_ArchitecturalFramework::RegisterSystem(const FString& SystemName, const FEng_SystemRequirements& Requirements)
{
    if (SystemName.IsEmpty())
    {
        LogArchitecturalViolation(TEXT("Attempted to register system with empty name"));
        return false;
    }
    
    if (RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("System %s already registered - updating requirements"), *SystemName);
    }
    
    RegisteredSystems.Add(SystemName, Requirements);
    
    // Initialize performance tracking
    SystemPerformanceMap.Add(SystemName, 0.0f);
    
    UE_LOG(LogTemp, Log, TEXT("System registered: %s (Type: %d, Tier: %d)"), 
        *SystemName, 
        (int32)Requirements.SystemType, 
        (int32)Requirements.PerformanceTier);
    
    return true;
}

bool UEng_ArchitecturalFramework::ValidateSystemDependencies(const FString& SystemName)
{
    if (!RegisteredSystems.Contains(SystemName))
    {
        LogArchitecturalViolation(FString::Printf(TEXT("System %s not registered"), *SystemName));
        return false;
    }
    
    const FEng_SystemRequirements& Requirements = RegisteredSystems[SystemName];
    
    // Check all required modules
    for (const FString& RequiredModule : Requirements.RequiredModules)
    {
        if (!ModuleStatusMap.Contains(RequiredModule))
        {
            LogArchitecturalViolation(FString::Printf(TEXT("System %s requires module %s which is not available"), *SystemName, *RequiredModule));
            return false;
        }
        
        const FEng_ModuleStatus& ModuleStatus = ModuleStatusMap[RequiredModule];
        if (ModuleStatus.CompilationStatus != EEng_CompilationStatus::Success)
        {
            LogArchitecturalViolation(FString::Printf(TEXT("System %s requires module %s which failed compilation"), *SystemName, *RequiredModule));
            return false;
        }
    }
    
    return true;
}

EEng_CompilationStatus UEng_ArchitecturalFramework::CheckModuleCompilation(const FString& ModuleName)
{
    if (ModuleStatusMap.Contains(ModuleName))
    {
        return ModuleStatusMap[ModuleName].CompilationStatus;
    }
    
    // Auto-detect module status if not tracked
    FEng_ModuleStatus NewStatus;
    NewStatus.ModuleName = ModuleName;
    NewStatus.CompilationStatus = EEng_CompilationStatus::NotCompiled;
    NewStatus.LastCompiled = FDateTime::Now();
    
    // Try to load a class from this module to test compilation
    FString TestClassName = FString::Printf(TEXT("/Script/%s.%sGameMode"), *ModuleName, *ModuleName);
    UClass* TestClass = LoadClass<UObject>(nullptr, *TestClassName);
    
    if (TestClass)
    {
        NewStatus.CompilationStatus = EEng_CompilationStatus::Success;
        NewStatus.ClassCount = 1; // At least one class found
    }
    
    ModuleStatusMap.Add(ModuleName, NewStatus);
    return NewStatus.CompilationStatus;
}

bool UEng_ArchitecturalFramework::ValidateAgentOutput(int32 AgentID, const TArray<FString>& CreatedFiles)
{
    if (!AgentRulesMap.Contains(AgentID))
    {
        LogArchitecturalViolation(FString::Printf(TEXT("Agent %d not registered in system"), AgentID));
        return false;
    }
    
    const FEng_AgentIntegrationRules& Rules = AgentRulesMap[AgentID];
    
    // Check file count limit
    if (CreatedFiles.Num() > Rules.MaxFilesPerCycle)
    {
        LogArchitecturalViolation(FString::Printf(TEXT("Agent %d exceeded file limit: %d > %d"), AgentID, CreatedFiles.Num(), Rules.MaxFilesPerCycle));
        return false;
    }
    
    // Check for required .cpp implementations
    if (Rules.bRequiresCppImplementation)
    {
        TArray<FString> HeaderFiles;
        TArray<FString> CppFiles;
        
        for (const FString& File : CreatedFiles)
        {
            if (File.EndsWith(TEXT(".h")))
            {
                HeaderFiles.Add(File);
            }
            else if (File.EndsWith(TEXT(".cpp")))
            {
                CppFiles.Add(File);
            }
        }
        
        // Every .h should have a corresponding .cpp
        for (const FString& HeaderFile : HeaderFiles)
        {
            FString ExpectedCppFile = HeaderFile.Replace(TEXT(".h"), TEXT(".cpp"));
            if (!CppFiles.Contains(ExpectedCppFile))
            {
                LogArchitecturalViolation(FString::Printf(TEXT("Agent %d created header %s without corresponding .cpp file"), AgentID, *HeaderFile));
                return false;
            }
        }
    }
    
    return true;
}

FEng_AgentIntegrationRules UEng_ArchitecturalFramework::GetAgentRules(int32 AgentID)
{
    if (AgentRulesMap.Contains(AgentID))
    {
        return AgentRulesMap[AgentID];
    }
    
    // Return default rules for unknown agents
    FEng_AgentIntegrationRules DefaultRules;
    DefaultRules.AgentID = AgentID;
    DefaultRules.AgentName = FString::Printf(TEXT("Unknown Agent %d"), AgentID);
    DefaultRules.PrimarySystemType = EEng_SystemType::Core;
    DefaultRules.MaxFilesPerCycle = 8;
    DefaultRules.bRequiresCppImplementation = true;
    
    return DefaultRules;
}

bool UEng_ArchitecturalFramework::CheckPerformanceCompliance(const FString& SystemName, float CurrentFrameTime)
{
    if (!RegisteredSystems.Contains(SystemName))
    {
        return false;
    }
    
    const FEng_SystemRequirements& Requirements = RegisteredSystems[SystemName];
    
    bool bCompliant = CurrentFrameTime <= Requirements.TargetFrameTime;
    
    if (!bCompliant)
    {
        LogArchitecturalViolation(FString::Printf(TEXT("System %s performance violation: %.2fms > %.2fms"), 
            *SystemName, CurrentFrameTime, Requirements.TargetFrameTime));
    }
    
    return bCompliant;
}

void UEng_ArchitecturalFramework::LogPerformanceMetrics(const FString& SystemName, float FrameTime, int32 ActorCount)
{
    SystemPerformanceMap.Add(SystemName, FrameTime);
    
    UE_LOG(LogTemp, Log, TEXT("Performance: %s - %.2fms, %d actors"), *SystemName, FrameTime, ActorCount);
    
    // Check against registered requirements
    if (RegisteredSystems.Contains(SystemName))
    {
        const FEng_SystemRequirements& Requirements = RegisteredSystems[SystemName];
        
        if (ActorCount > Requirements.MaxActorsPerSystem)
        {
            LogArchitecturalViolation(FString::Printf(TEXT("System %s actor count violation: %d > %d"), 
                *SystemName, ActorCount, Requirements.MaxActorsPerSystem));
        }
    }
}

TArray<FEng_ModuleStatus> UEng_ArchitecturalFramework::GetAllModuleStatus()
{
    TArray<FEng_ModuleStatus> StatusArray;
    
    for (const auto& Pair : ModuleStatusMap)
    {
        StatusArray.Add(Pair.Value);
    }
    
    return StatusArray;
}

bool UEng_ArchitecturalFramework::ForceModuleRecompilation(const FString& ModuleName)
{
    if (ModuleStatusMap.Contains(ModuleName))
    {
        FEng_ModuleStatus& Status = ModuleStatusMap[ModuleName];
        Status.CompilationStatus = EEng_CompilationStatus::Compiling;
        Status.LastCompiled = FDateTime::Now();
        
        UE_LOG(LogTemp, Warning, TEXT("Forcing recompilation of module: %s"), *ModuleName);
        return true;
    }
    
    return false;
}

void UEng_ArchitecturalFramework::ValidateEntireArchitecture()
{
    UE_LOG(LogTemp, Warning, TEXT("=== ARCHITECTURAL VALIDATION STARTING ==="));
    
    int32 TotalSystems = RegisteredSystems.Num();
    int32 ValidSystems = 0;
    int32 TotalModules = ModuleStatusMap.Num();
    int32 CompiledModules = 0;
    
    // Validate all registered systems
    for (const auto& SystemPair : RegisteredSystems)
    {
        if (ValidateSystemDependencies(SystemPair.Key))
        {
            ValidSystems++;
        }
    }
    
    // Check module compilation status
    for (const auto& ModulePair : ModuleStatusMap)
    {
        if (ModulePair.Value.CompilationStatus == EEng_CompilationStatus::Success)
        {
            CompiledModules++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Systems: %d/%d valid"), ValidSystems, TotalSystems);
    UE_LOG(LogTemp, Warning, TEXT("Modules: %d/%d compiled"), CompiledModules, TotalModules);
    UE_LOG(LogTemp, Warning, TEXT("=== ARCHITECTURAL VALIDATION COMPLETE ==="));
}

void UEng_ArchitecturalFramework::GenerateArchitectureReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== ARCHITECTURE REPORT ==="));
    
    // System breakdown by type
    TMap<EEng_SystemType, int32> SystemTypeCount;
    for (const auto& SystemPair : RegisteredSystems)
    {
        EEng_SystemType Type = SystemPair.Value.SystemType;
        SystemTypeCount.Add(Type, SystemTypeCount.FindRef(Type) + 1);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("System Distribution:"));
    for (const auto& TypePair : SystemTypeCount)
    {
        UE_LOG(LogTemp, Warning, TEXT("  %d: %d systems"), (int32)TypePair.Key, TypePair.Value);
    }
    
    // Performance summary
    float AverageFrameTime = 0.0f;
    int32 PerformanceViolations = 0;
    
    for (const auto& PerfPair : SystemPerformanceMap)
    {
        AverageFrameTime += PerfPair.Value;
        if (PerfPair.Value > GlobalFrameTimeTarget)
        {
            PerformanceViolations++;
        }
    }
    
    if (SystemPerformanceMap.Num() > 0)
    {
        AverageFrameTime /= SystemPerformanceMap.Num();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Performance: %.2fms average, %d violations"), AverageFrameTime, PerformanceViolations);
    UE_LOG(LogTemp, Warning, TEXT("=== END REPORT ==="));
}

void UEng_ArchitecturalFramework::InitializeAgentRules()
{
    // Agent #2 - Engine Architect (self)
    FEng_AgentIntegrationRules EngineArchitectRules;
    EngineArchitectRules.AgentID = 2;
    EngineArchitectRules.AgentName = TEXT("Engine Architect");
    EngineArchitectRules.PrimarySystemType = EEng_SystemType::Core;
    EngineArchitectRules.AllowedSystemTypes = { EEng_SystemType::Core, EEng_SystemType::Performance, EEng_SystemType::Integration };
    EngineArchitectRules.MaxFilesPerCycle = 8;
    EngineArchitectRules.bRequiresCppImplementation = true;
    EngineArchitectRules.MandatoryIncludes = { TEXT("CoreMinimal.h"), TEXT("Engine/Engine.h") };
    AgentRulesMap.Add(2, EngineArchitectRules);
    
    // Agent #3 - Core Systems Programmer
    FEng_AgentIntegrationRules CoreSystemsRules;
    CoreSystemsRules.AgentID = 3;
    CoreSystemsRules.AgentName = TEXT("Core Systems Programmer");
    CoreSystemsRules.PrimarySystemType = EEng_SystemType::Core;
    CoreSystemsRules.AllowedSystemTypes = { EEng_SystemType::Core };
    CoreSystemsRules.MaxFilesPerCycle = 8;
    CoreSystemsRules.bRequiresCppImplementation = true;
    CoreSystemsRules.MandatoryIncludes = { TEXT("CoreMinimal.h"), TEXT("Components/ActorComponent.h") };
    AgentRulesMap.Add(3, CoreSystemsRules);
    
    // Agent #5 - Procedural World Generator
    FEng_AgentIntegrationRules WorldGenRules;
    WorldGenRules.AgentID = 5;
    WorldGenRules.AgentName = TEXT("Procedural World Generator");
    WorldGenRules.PrimarySystemType = EEng_SystemType::World;
    WorldGenRules.AllowedSystemTypes = { EEng_SystemType::World };
    WorldGenRules.MaxFilesPerCycle = 8;
    WorldGenRules.bRequiresCppImplementation = true;
    WorldGenRules.MandatoryIncludes = { TEXT("CoreMinimal.h"), TEXT("Engine/World.h"), TEXT("PCGComponent.h") };
    AgentRulesMap.Add(5, WorldGenRules);
    
    // Add more agent rules as needed...
}

void UEng_ArchitecturalFramework::InitializeDefaultSystems()
{
    // Core Engine System
    FEng_SystemRequirements CoreRequirements;
    CoreRequirements.SystemType = EEng_SystemType::Core;
    CoreRequirements.PerformanceTier = EEng_PerformanceTier::Critical;
    CoreRequirements.DependencyLevel = EEng_DependencyLevel::Core;
    CoreRequirements.MaxActorsPerSystem = 100;
    CoreRequirements.TargetFrameTime = 16.67f;
    CoreRequirements.RequiredModules = { TEXT("Core"), TEXT("Engine"), TEXT("TranspersonalGame") };
    RegisterSystem(TEXT("CoreEngine"), CoreRequirements);
    
    // World Generation System
    FEng_SystemRequirements WorldGenRequirements;
    WorldGenRequirements.SystemType = EEng_SystemType::World;
    WorldGenRequirements.PerformanceTier = EEng_PerformanceTier::High;
    WorldGenRequirements.DependencyLevel = EEng_DependencyLevel::System;
    WorldGenRequirements.MaxActorsPerSystem = 5000;
    WorldGenRequirements.TargetFrameTime = 33.33f; // 30fps for world gen
    WorldGenRequirements.RequiredModules = { TEXT("Core"), TEXT("Engine"), TEXT("TranspersonalGame"), TEXT("PCG") };
    RegisterSystem(TEXT("WorldGeneration"), WorldGenRequirements);
    
    // Character System
    FEng_SystemRequirements CharacterRequirements;
    CharacterRequirements.SystemType = EEng_SystemType::Character;
    CharacterRequirements.PerformanceTier = EEng_PerformanceTier::Critical;
    CharacterRequirements.DependencyLevel = EEng_DependencyLevel::Gameplay;
    CharacterRequirements.MaxActorsPerSystem = 200;
    CharacterRequirements.TargetFrameTime = 16.67f;
    CharacterRequirements.RequiredModules = { TEXT("Core"), TEXT("Engine"), TEXT("TranspersonalGame") };
    RegisterSystem(TEXT("CharacterSystem"), CharacterRequirements);
    
    // Initialize module status tracking
    FEng_ModuleStatus TranspersonalStatus;
    TranspersonalStatus.ModuleName = TEXT("TranspersonalGame");
    TranspersonalStatus.CompilationStatus = EEng_CompilationStatus::Success;
    TranspersonalStatus.ClassCount = 10; // Estimated
    TranspersonalStatus.LastCompiled = FDateTime::Now();
    ModuleStatusMap.Add(TEXT("TranspersonalGame"), TranspersonalStatus);
}

bool UEng_ArchitecturalFramework::ValidateFileStructure(const FString& FilePath)
{
    // Basic file validation
    if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*FilePath))
    {
        return false;
    }
    
    // Check file extension
    if (!FilePath.EndsWith(TEXT(".h")) && !FilePath.EndsWith(TEXT(".cpp")))
    {
        LogArchitecturalViolation(FString::Printf(TEXT("Invalid file type: %s"), *FilePath));
        return false;
    }
    
    return true;
}

void UEng_ArchitecturalFramework::LogArchitecturalViolation(const FString& Violation)
{
    UE_LOG(LogTemp, Error, TEXT("[ARCHITECTURAL VIOLATION] %s"), *Violation);
}