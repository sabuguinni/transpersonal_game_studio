#include "EngArchitect_TechnicalSpecs.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Stats/Stats.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "Engine/GameInstance.h"

void UEngArchitect_TechnicalSpecs::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect Technical Specs Subsystem Initialized"));
    
    InitializeDefaultConstraints();
    ValidateSystemRequirements();
    
    // Register core modules that must exist
    FEng_ModuleArchitecture CoreModule;
    CoreModule.ModuleName = TEXT("TranspersonalGame");
    CoreModule.bIsCore = true;
    CoreModule.TargetPerformance = EEng_PerformanceTarget::PC_60FPS;
    RegisterModule(CoreModule);
    
    FEng_ModuleArchitecture PhysicsModule;
    PhysicsModule.ModuleName = TEXT("PhysicsCore");
    PhysicsModule.Dependencies.Add(TEXT("TranspersonalGame"));
    PhysicsModule.bRequiresWorldPartition = true;
    RegisterModule(PhysicsModule);
    
    FEng_ModuleArchitecture WorldGenModule;
    WorldGenModule.ModuleName = TEXT("WorldGeneration");
    WorldGenModule.Dependencies.Add(TEXT("TranspersonalGame"));
    WorldGenModule.bRequiresWorldPartition = true;
    RegisterModule(WorldGenModule);
}

void UEngArchitect_TechnicalSpecs::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect Technical Specs Subsystem Deinitialized"));
    RegisteredModules.Empty();
    Super::Deinitialize();
}

bool UEngArchitect_TechnicalSpecs::ValidateModuleArchitecture(const FEng_ModuleArchitecture& Module)
{
    if (Module.ModuleName.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Module validation failed: Empty module name"));
        return false;
    }
    
    // Check for circular dependencies
    for (const FString& Dependency : Module.Dependencies)
    {
        if (Dependency == Module.ModuleName)
        {
            UE_LOG(LogTemp, Error, TEXT("Module validation failed: Circular dependency in %s"), *Module.ModuleName);
            return false;
        }
    }
    
    // Validate performance targets
    if (Module.TargetPerformance == EEng_PerformanceTarget::PC_60FPS && CurrentConstraints.MaxFrameTimeMS > 16.67f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Module %s targets 60 FPS but constraints allow slower frame times"), *Module.ModuleName);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Module %s validated successfully"), *Module.ModuleName);
    return true;
}

bool UEngArchitect_TechnicalSpecs::EnforcePerformanceConstraints(const FEng_PerformanceConstraints& Constraints)
{
    CurrentConstraints = Constraints;
    
    // Validate constraints are reasonable
    if (Constraints.MaxActorsPerFrame <= 0 || Constraints.MaxTrianglesPerFrame <= 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid performance constraints: Values must be positive"));
        return false;
    }
    
    if (Constraints.MaxFrameTimeMS < 8.33f) // Faster than 120 FPS is unrealistic
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance constraint too aggressive: Frame time %f ms"), Constraints.MaxFrameTimeMS);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Performance constraints updated: %d actors, %d triangles, %.2f ms frame time"), 
           Constraints.MaxActorsPerFrame, Constraints.MaxTrianglesPerFrame, Constraints.MaxFrameTimeMS);
    
    return true;
}

void UEngArchitect_TechnicalSpecs::SetWorldPartitionMode(EEng_WorldPartitionMode Mode)
{
    CurrentWorldPartitionMode = Mode;
    
    switch (Mode)
    {
        case EEng_WorldPartitionMode::Disabled:
            UE_LOG(LogTemp, Warning, TEXT("World Partition disabled - performance may suffer on large worlds"));
            break;
        case EEng_WorldPartitionMode::Standard:
            UE_LOG(LogTemp, Log, TEXT("World Partition set to Standard (4km²)"));
            break;
        case EEng_WorldPartitionMode::Large:
            UE_LOG(LogTemp, Log, TEXT("World Partition set to Large (16km²)"));
            break;
        case EEng_WorldPartitionMode::Massive:
            UE_LOG(LogTemp, Log, TEXT("World Partition set to Massive (64km²)"));
            break;
    }
}

float UEngArchitect_TechnicalSpecs::GetCurrentFrameTime() const
{
    if (GEngine && GEngine->GetWorld())
    {
        return GEngine->GetWorld()->GetDeltaSeconds() * 1000.0f; // Convert to milliseconds
    }
    return 0.0f;
}

int32 UEngArchitect_TechnicalSpecs::GetCurrentActorCount() const
{
    if (GEngine && GEngine->GetWorld())
    {
        return GEngine->GetWorld()->GetActorCount();
    }
    return 0;
}

bool UEngArchitect_TechnicalSpecs::IsPerformanceWithinLimits() const
{
    float CurrentFrameTime = GetCurrentFrameTime();
    int32 CurrentActors = GetCurrentActorCount();
    
    bool bFrameTimeOK = CurrentFrameTime <= CurrentConstraints.MaxFrameTimeMS;
    bool bActorCountOK = CurrentActors <= CurrentConstraints.MaxActorsPerFrame;
    
    if (!bFrameTimeOK)
    {
        UE_LOG(LogTemp, Warning, TEXT("Frame time exceeded: %.2f ms > %.2f ms limit"), 
               CurrentFrameTime, CurrentConstraints.MaxFrameTimeMS);
    }
    
    if (!bActorCountOK)
    {
        UE_LOG(LogTemp, Warning, TEXT("Actor count exceeded: %d > %d limit"), 
               CurrentActors, CurrentConstraints.MaxActorsPerFrame);
    }
    
    return bFrameTimeOK && bActorCountOK;
}

EEng_LODLevel UEngArchitect_TechnicalSpecs::CalculateLODLevel(float Distance) const
{
    if (Distance <= LOD1Distance)
    {
        return EEng_LODLevel::LOD0_High;
    }
    else if (Distance <= LOD2Distance)
    {
        return EEng_LODLevel::LOD1_Medium;
    }
    else if (Distance <= CullDistance)
    {
        return EEng_LODLevel::LOD2_Low;
    }
    else
    {
        return EEng_LODLevel::LOD3_Culled;
    }
}

void UEngArchitect_TechnicalSpecs::SetLODDistances(float LOD1Dist, float LOD2Dist, float CullDist)
{
    LOD1Distance = LOD1Dist;
    LOD2Distance = LOD2Dist;
    CullDistance = CullDist;
    
    UE_LOG(LogTemp, Log, TEXT("LOD distances updated: LOD1=%.0f, LOD2=%.0f, Cull=%.0f"), 
           LOD1Distance, LOD2Distance, CullDistance);
}

void UEngArchitect_TechnicalSpecs::RegisterModule(const FEng_ModuleArchitecture& Module)
{
    if (ValidateModuleArchitecture(Module))
    {
        // Check if module already exists
        for (int32 i = 0; i < RegisteredModules.Num(); i++)
        {
            if (RegisteredModules[i].ModuleName == Module.ModuleName)
            {
                RegisteredModules[i] = Module; // Update existing
                UE_LOG(LogTemp, Log, TEXT("Updated existing module: %s"), *Module.ModuleName);
                return;
            }
        }
        
        RegisteredModules.Add(Module);
        UE_LOG(LogTemp, Log, TEXT("Registered new module: %s"), *Module.ModuleName);
    }
}

void UEngArchitect_TechnicalSpecs::EnforceArchitectureRules()
{
    UE_LOG(LogTemp, Warning, TEXT("=== ENFORCING ARCHITECTURE RULES ==="));
    
    // Rule 1: Core modules must be present
    bool bHasCoreModule = false;
    for (const FEng_ModuleArchitecture& Module : RegisteredModules)
    {
        if (Module.bIsCore)
        {
            bHasCoreModule = true;
            break;
        }
    }
    
    if (!bHasCoreModule)
    {
        UE_LOG(LogTemp, Error, TEXT("ARCHITECTURE VIOLATION: No core module registered"));
    }
    
    // Rule 2: World Partition must be enabled for large worlds
    if (CurrentWorldPartitionMode == EEng_WorldPartitionMode::Disabled)
    {
        for (const FEng_ModuleArchitecture& Module : RegisteredModules)
        {
            if (Module.bRequiresWorldPartition)
            {
                UE_LOG(LogTemp, Error, TEXT("ARCHITECTURE VIOLATION: Module %s requires World Partition but it's disabled"), *Module.ModuleName);
            }
        }
    }
    
    // Rule 3: Performance constraints must be realistic
    if (CurrentConstraints.MaxFrameTimeMS > 33.33f) // Slower than 30 FPS
    {
        UE_LOG(LogTemp, Error, TEXT("ARCHITECTURE VIOLATION: Frame time constraint too slow: %.2f ms"), CurrentConstraints.MaxFrameTimeMS);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== ARCHITECTURE RULES ENFORCEMENT COMPLETE ==="));
}

void UEngArchitect_TechnicalSpecs::ValidateAllModules()
{
    UE_LOG(LogTemp, Warning, TEXT("=== VALIDATING ALL MODULES ==="));
    
    int32 ValidModules = 0;
    int32 InvalidModules = 0;
    
    for (const FEng_ModuleArchitecture& Module : RegisteredModules)
    {
        if (ValidateModuleArchitecture(Module) && CheckModuleDependencies(Module))
        {
            ValidModules++;
        }
        else
        {
            InvalidModules++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Module validation complete: %d valid, %d invalid"), ValidModules, InvalidModules);
}

void UEngArchitect_TechnicalSpecs::InitializeDefaultConstraints()
{
    CurrentConstraints.MaxActorsPerFrame = 10000;
    CurrentConstraints.MaxTrianglesPerFrame = 2000000;
    CurrentConstraints.MaxFrameTimeMS = 16.67f; // 60 FPS
    CurrentConstraints.MaxCrowdAgents = 50000;
    CurrentConstraints.CullingDistance = 10000.0f;
    
    LOD1Distance = 2000.0f;
    LOD2Distance = 5000.0f;
    CullDistance = 10000.0f;
}

void UEngArchitect_TechnicalSpecs::ValidateSystemRequirements()
{
    // Check if we're running in Editor vs Game
    bool bIsEditor = GIsEditor;
    UE_LOG(LogTemp, Log, TEXT("Running in %s mode"), bIsEditor ? TEXT("Editor") : TEXT("Game"));
    
    // Validate UE5 version compatibility
    FString EngineVersion = FEngineVersion::Current().ToString();
    UE_LOG(LogTemp, Log, TEXT("Engine Version: %s"), *EngineVersion);
    
    // Check available memory
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    UE_LOG(LogTemp, Log, TEXT("Available Memory: %.2f GB"), MemStats.AvailablePhysical / (1024.0f * 1024.0f * 1024.0f));
}

bool UEngArchitect_TechnicalSpecs::CheckModuleDependencies(const FEng_ModuleArchitecture& Module)
{
    for (const FString& Dependency : Module.Dependencies)
    {
        bool bDependencyFound = false;
        for (const FEng_ModuleArchitecture& RegisteredModule : RegisteredModules)
        {
            if (RegisteredModule.ModuleName == Dependency)
            {
                bDependencyFound = true;
                break;
            }
        }
        
        if (!bDependencyFound)
        {
            UE_LOG(LogTemp, Error, TEXT("Module %s missing dependency: %s"), *Module.ModuleName, *Dependency);
            return false;
        }
    }
    
    return true;
}