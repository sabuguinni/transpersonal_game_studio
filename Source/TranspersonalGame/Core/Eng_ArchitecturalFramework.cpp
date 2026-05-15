#include "Eng_ArchitecturalFramework.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "Stats/Stats.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"

DEFINE_LOG_CATEGORY(LogEngineArchitect);

UEng_ArchitecturalFramework::UEng_ArchitecturalFramework()
{
    // Initialize critical systems list
    CriticalSystems = {
        TEXT("Core"),
        TEXT("Physics"),
        TEXT("Rendering"), 
        TEXT("Audio"),
        TEXT("Input"),
        TEXT("GameFramework")
    };
}

void UEng_ArchitecturalFramework::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogEngineArchitect, Log, TEXT("Engine Architectural Framework initialized"));
    
    // Initialize module state tracking
    ModuleStates.Empty();
    ModuleStates.Add(TEXT("TranspersonalGame"), EEng_ModuleState::Loading);
    ModuleStates.Add(TEXT("Core"), EEng_ModuleState::Loaded);
    ModuleStates.Add(TEXT("Engine"), EEng_ModuleState::Loaded);
    
    // Register core system dependencies
    FEng_SystemDependency CoreDep;
    CoreDep.SystemName = TEXT("Core");
    CoreDep.Priority = EEng_SystemPriority::Critical;
    CoreDep.bIsOptional = false;
    RegisterSystemDependency(CoreDep);
    
    FEng_SystemDependency PhysicsDep;
    PhysicsDep.SystemName = TEXT("Physics");
    PhysicsDep.Priority = EEng_SystemPriority::Critical;
    PhysicsDep.RequiredSystems.Add(TEXT("Core"));
    PhysicsDep.bIsOptional = false;
    RegisterSystemDependency(PhysicsDep);
    
    FEng_SystemDependency WorldGenDep;
    WorldGenDep.SystemName = TEXT("WorldGeneration");
    WorldGenDep.Priority = EEng_SystemPriority::Core;
    WorldGenDep.RequiredSystems.Add(TEXT("Core"));
    WorldGenDep.RequiredSystems.Add(TEXT("Physics"));
    WorldGenDep.bIsOptional = false;
    RegisterSystemDependency(WorldGenDep);
    
    UE_LOG(LogEngineArchitect, Log, TEXT("Core system dependencies registered"));
}

void UEng_ArchitecturalFramework::Deinitialize()
{
    UE_LOG(LogEngineArchitect, Log, TEXT("Engine Architectural Framework shutting down"));
    
    // Clear all cached data
    RegisteredSystems.Empty();
    ModuleStates.Empty();
    SystemPerformanceMetrics.Empty();
    ValidationCache.Empty();
    
    Super::Deinitialize();
}

FEng_ComplianceResult UEng_ArchitecturalFramework::ValidateSystemCompliance(const FString& SystemName, EEng_ValidationLevel Level)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UEng_ArchitecturalFramework::ValidateSystemCompliance);
    
    FEng_ComplianceResult Result;
    Result.ValidationLevel = Level;
    
    // Check cache first
    if (ValidationCache.Contains(SystemName))
    {
        FEng_ComplianceResult* CachedResult = ValidationCache.Find(SystemName);
        if (CachedResult && CachedResult->ValidationLevel == Level)
        {
            return *CachedResult;
        }
    }
    
    // Validate system exists
    bool bSystemExists = false;
    for (const FEng_SystemDependency& Dep : RegisteredSystems)
    {
        if (Dep.SystemName == SystemName)
        {
            bSystemExists = true;
            break;
        }
    }
    
    if (!bSystemExists)
    {
        Result.bIsCompliant = false;
        Result.ErrorMessage = FString::Printf(TEXT("System '%s' not registered"), *SystemName);
        Result.ComplianceScore = 0.0f;
        ValidationCache.Add(SystemName, Result);
        return Result;
    }
    
    // Perform validation based on level
    float ComplianceScore = 0.0f;
    TArray<FString> Issues;
    
    // Basic validation
    if (Level >= EEng_ValidationLevel::Basic)
    {
        if (ValidateSystemDependencies(SystemName))
        {
            ComplianceScore += 25.0f;
        }
        else
        {
            Issues.Add(TEXT("Dependency validation failed"));
        }
    }
    
    // Standard validation
    if (Level >= EEng_ValidationLevel::Standard)
    {
        if (ValidatePerformanceRequirements(SystemName))
        {
            ComplianceScore += 25.0f;
        }
        else
        {
            Issues.Add(TEXT("Performance requirements not met"));
        }
        
        if (ValidateMemoryUsage(SystemName))
        {
            ComplianceScore += 25.0f;
        }
        else
        {
            Issues.Add(TEXT("Memory usage exceeds limits"));
        }
    }
    
    // Strict validation
    if (Level >= EEng_ValidationLevel::Strict)
    {
        if (ValidateThreadSafety(SystemName))
        {
            ComplianceScore += 25.0f;
        }
        else
        {
            Issues.Add(TEXT("Thread safety validation failed"));
        }
    }
    
    // Set final result
    Result.bIsCompliant = (Issues.Num() == 0);
    Result.ComplianceScore = ComplianceScore;
    
    if (Issues.Num() > 0)
    {
        Result.ErrorMessage = FString::Join(Issues, TEXT("; "));
    }
    else
    {
        Result.ErrorMessage = TEXT("System compliant");
    }
    
    // Cache result
    ValidationCache.Add(SystemName, Result);
    
    UE_LOG(LogEngineArchitect, Log, TEXT("System '%s' validation: %s (Score: %.1f)"), 
           *SystemName, Result.bIsCompliant ? TEXT("PASS") : TEXT("FAIL"), Result.ComplianceScore);
    
    return Result;
}

bool UEng_ArchitecturalFramework::RegisterSystemDependency(const FEng_SystemDependency& Dependency)
{
    // Check for duplicate registration
    for (const FEng_SystemDependency& ExistingDep : RegisteredSystems)
    {
        if (ExistingDep.SystemName == Dependency.SystemName)
        {
            UE_LOG(LogEngineArchitect, Warning, TEXT("System '%s' already registered"), *Dependency.SystemName);
            return false;
        }
    }
    
    // Validate dependency requirements
    for (const FString& RequiredSystem : Dependency.RequiredSystems)
    {
        bool bRequiredExists = false;
        for (const FEng_SystemDependency& ExistingDep : RegisteredSystems)
        {
            if (ExistingDep.SystemName == RequiredSystem)
            {
                bRequiredExists = true;
                break;
            }
        }
        
        if (!bRequiredExists)
        {
            UE_LOG(LogEngineArchitect, Error, TEXT("Cannot register '%s': required system '%s' not found"), 
                   *Dependency.SystemName, *RequiredSystem);
            return false;
        }
    }
    
    RegisteredSystems.Add(Dependency);
    
    UE_LOG(LogEngineArchitect, Log, TEXT("Registered system dependency: %s (Priority: %d)"), 
           *Dependency.SystemName, (int32)Dependency.Priority);
    
    return true;
}

bool UEng_ArchitecturalFramework::ValidateSystemDependencies(const FString& SystemName)
{
    for (const FEng_SystemDependency& Dep : RegisteredSystems)
    {
        if (Dep.SystemName == SystemName)
        {
            // Check all required systems are loaded
            for (const FString& RequiredSystem : Dep.RequiredSystems)
            {
                EEng_ModuleState* State = ModuleStates.Find(RequiredSystem);
                if (!State || *State != EEng_ModuleState::Loaded)
                {
                    UE_LOG(LogEngineArchitect, Warning, TEXT("System '%s' dependency '%s' not loaded"), 
                           *SystemName, *RequiredSystem);
                    return false;
                }
            }
            return true;
        }
    }
    
    return false;
}

TArray<FString> UEng_ArchitecturalFramework::GetSystemLoadOrder()
{
    TArray<FString> LoadOrder;
    
    // Sort by priority
    TArray<FEng_SystemDependency> SortedSystems = RegisteredSystems;
    SortedSystems.Sort([](const FEng_SystemDependency& A, const FEng_SystemDependency& B) {
        return (int32)A.Priority < (int32)B.Priority;
    });
    
    // Build load order respecting dependencies
    for (const FEng_SystemDependency& Dep : SortedSystems)
    {
        // Add required systems first
        for (const FString& RequiredSystem : Dep.RequiredSystems)
        {
            if (!LoadOrder.Contains(RequiredSystem))
            {
                LoadOrder.Add(RequiredSystem);
            }
        }
        
        // Add this system
        if (!LoadOrder.Contains(Dep.SystemName))
        {
            LoadOrder.Add(Dep.SystemName);
        }
    }
    
    return LoadOrder;
}

EEng_ModuleState UEng_ArchitecturalFramework::GetModuleState(const FString& ModuleName)
{
    EEng_ModuleState* State = ModuleStates.Find(ModuleName);
    return State ? *State : EEng_ModuleState::Unloaded;
}

bool UEng_ArchitecturalFramework::ValidatePerformanceRequirements(const FString& SystemName, float MaxFrameTime)
{
    // Check cached performance metrics
    float* CachedMetric = SystemPerformanceMetrics.Find(SystemName);
    if (CachedMetric)
    {
        return *CachedMetric <= MaxFrameTime;
    }
    
    // Default to passing if no metrics available
    return true;
}

bool UEng_ArchitecturalFramework::ValidateMemoryUsage(const FString& SystemName, int32 MaxMemoryMB)
{
    // Basic memory validation - would be expanded with actual memory tracking
    return true;
}

bool UEng_ArchitecturalFramework::ValidateThreadSafety(const FString& SystemName)
{
    // Basic thread safety validation - would be expanded with actual thread analysis
    return true;
}

bool UEng_ArchitecturalFramework::ValidateClassHierarchy(UClass* ClassToValidate)
{
    if (!ClassToValidate)
    {
        return false;
    }
    
    // Validate proper UE5 class hierarchy
    if (ClassToValidate->IsChildOf(AActor::StaticClass()) ||
        ClassToValidate->IsChildOf(UActorComponent::StaticClass()) ||
        ClassToValidate->IsChildOf(UObject::StaticClass()))
    {
        return true;
    }
    
    return false;
}

bool UEng_ArchitecturalFramework::ValidateModuleDependencies(const FString& ModuleName)
{
    // Would validate Build.cs dependencies match actual code dependencies
    return true;
}

bool UEng_ArchitecturalFramework::CheckCircularDependencies(const TArray<FEng_SystemDependency>& Dependencies)
{
    // Simple circular dependency check
    for (const FEng_SystemDependency& Dep : Dependencies)
    {
        for (const FString& RequiredSystem : Dep.RequiredSystems)
        {
            // Find the required system and check if it depends back on this one
            for (const FEng_SystemDependency& RequiredDep : Dependencies)
            {
                if (RequiredDep.SystemName == RequiredSystem)
                {
                    if (RequiredDep.RequiredSystems.Contains(Dep.SystemName))
                    {
                        UE_LOG(LogEngineArchitect, Error, TEXT("Circular dependency detected: %s <-> %s"), 
                               *Dep.SystemName, *RequiredSystem);
                        return true;
                    }
                }
            }
        }
    }
    
    return false;
}

// World Validator Implementation
UEng_WorldArchitectValidator::UEng_WorldArchitectValidator()
{
    bWorldValidated = false;
}

void UEng_WorldArchitectValidator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogEngineArchitect, Log, TEXT("World Architect Validator initialized"));
}

void UEng_WorldArchitectValidator::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    
    UE_LOG(LogEngineArchitect, Log, TEXT("World begin play - starting validation"));
    
    // Perform world validation
    ValidateWorldConfiguration();
    ValidateActorHierarchy();
    ValidateComponentDependencies();
    
    bWorldValidated = true;
}

bool UEng_WorldArchitectValidator::ValidateWorldConfiguration()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        ValidationWarnings.Add(TEXT("No world available for validation"));
        return false;
    }
    
    // Check world settings
    AWorldSettings* WorldSettings = World->GetWorldSettings();
    if (!WorldSettings)
    {
        ValidationWarnings.Add(TEXT("World settings not found"));
        return false;
    }
    
    UE_LOG(LogEngineArchitect, Log, TEXT("World configuration validated"));
    return true;
}

bool UEng_WorldArchitectValidator::ValidateActorHierarchy()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    int32 ActorCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            ActorCount++;
        }
    }
    
    if (ActorCount > EngArchitecturalConstants::MAX_ACTORS_PER_LEVEL)
    {
        ValidationWarnings.Add(FString::Printf(TEXT("Actor count (%d) exceeds limit (%d)"), 
                                               ActorCount, EngArchitecturalConstants::MAX_ACTORS_PER_LEVEL));
        return false;
    }
    
    UE_LOG(LogEngineArchitect, Log, TEXT("Actor hierarchy validated: %d actors"), ActorCount);
    return true;
}

bool UEng_WorldArchitectValidator::ValidateComponentDependencies()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Validate component counts and dependencies
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            TArray<UActorComponent*> Components = Actor->GetRootComponent()->GetAttachChildren();
            if (Components.Num() > EngArchitecturalConstants::MAX_COMPONENTS_PER_ACTOR)
            {
                ValidationWarnings.Add(FString::Printf(TEXT("Actor '%s' has too many components (%d)"), 
                                                       *Actor->GetName(), Components.Num()));
            }
        }
    }
    
    UE_LOG(LogEngineArchitect, Log, TEXT("Component dependencies validated"));
    return true;
}

TArray<FString> UEng_WorldArchitectValidator::GetArchitecturalWarnings()
{
    return ValidationWarnings;
}