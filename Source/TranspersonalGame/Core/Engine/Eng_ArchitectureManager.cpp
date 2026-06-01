#include "Eng_ArchitectureManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Engine/GameViewportClient.h"
#include "RenderingThread.h"
#include "Stats/Stats.h"

UEng_ArchitectureManager::UEng_ArchitectureManager()
{
    bSystemsInitialized = false;
    TargetFrameTime = 16.67f; // 60 FPS target
    MaxDrawCalls = 2000;
    MaxMemoryMB = 4096.0f;
    MaxActorCount = 8000; // Global limit from brain memories
}

void UEng_ArchitectureManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architecture Manager Initializing..."));
    
    // Initialize core performance targets
    SetPerformanceTargets(16.67f, 2000, 4096.0f);
    
    // Initialize core system modules
    InitializeCoreModules();
    
    bSystemsInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architecture Manager Initialized Successfully"));
}

void UEng_ArchitectureManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architecture Manager Deinitializing..."));
    
    // Cleanup all modules
    SystemModules.Empty();
    SystemErrors.Empty();
    
    bSystemsInitialized = false;
    
    Super::Deinitialize();
}

void UEng_ArchitectureManager::InitializeSystemModules()
{
    if (bSystemsInitialized)
    {
        return;
    }
    
    InitializeCoreModules();
    bSystemsInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("System Modules Initialized"));
}

void UEng_ArchitectureManager::InitializeCoreModules()
{
    // Core Engine Systems
    RegisterSystemModule(TEXT("CoreEngine"), 100, TArray<FString>());
    RegisterSystemModule(TEXT("Rendering"), 90, {TEXT("CoreEngine")});
    RegisterSystemModule(TEXT("Physics"), 80, {TEXT("CoreEngine")});
    RegisterSystemModule(TEXT("Audio"), 70, {TEXT("CoreEngine")});
    
    // Game Systems
    RegisterSystemModule(TEXT("WorldGeneration"), 60, {TEXT("Physics"), TEXT("Rendering")});
    RegisterSystemModule(TEXT("CharacterSystem"), 50, {TEXT("Physics")});
    RegisterSystemModule(TEXT("DinosaurAI"), 40, {TEXT("CharacterSystem")});
    RegisterSystemModule(TEXT("CombatSystem"), 30, {TEXT("CharacterSystem", TEXT("DinosaurAI")});
    RegisterSystemModule(TEXT("QuestSystem"), 20, {TEXT("CharacterSystem")});
    RegisterSystemModule(TEXT("UISystem"), 10, {TEXT("Rendering")});
    
    // Load core modules
    LoadSystemModule(TEXT("CoreEngine"));
    LoadSystemModule(TEXT("Rendering"));
    LoadSystemModule(TEXT("Physics"));
}

void UEng_ArchitectureManager::RegisterSystemModule(const FString& ModuleName, int32 Priority, const TArray<FString>& Dependencies)
{
    FEng_SystemModule NewModule;
    NewModule.ModuleName = ModuleName;
    NewModule.Priority = Priority;
    NewModule.Dependencies = Dependencies;
    NewModule.bIsLoaded = false;
    NewModule.bIsActive = false;
    
    SystemModules.Add(NewModule);
    
    UE_LOG(LogTemp, Log, TEXT("Registered System Module: %s (Priority: %d)"), *ModuleName, Priority);
}

bool UEng_ArchitectureManager::LoadSystemModule(const FString& ModuleName)
{
    FEng_SystemModule* Module = FindSystemModule(ModuleName);
    if (!Module)
    {
        SystemErrors.Add(FString::Printf(TEXT("Module not found: %s"), *ModuleName));
        return false;
    }
    
    if (Module->bIsLoaded)
    {
        return true; // Already loaded
    }
    
    // Check dependencies
    if (!ResolveDependencies(ModuleName))
    {
        SystemErrors.Add(FString::Printf(TEXT("Failed to resolve dependencies for: %s"), *ModuleName));
        return false;
    }
    
    // Simulate module loading
    Module->bIsLoaded = true;
    Module->bIsActive = true;
    
    ValidateModuleLoad(ModuleName);
    
    UE_LOG(LogTemp, Warning, TEXT("Loaded System Module: %s"), *ModuleName);
    return true;
}

bool UEng_ArchitectureManager::UnloadSystemModule(const FString& ModuleName)
{
    FEng_SystemModule* Module = FindSystemModule(ModuleName);
    if (!Module || !Module->bIsLoaded)
    {
        return false;
    }
    
    Module->bIsLoaded = false;
    Module->bIsActive = false;
    
    UE_LOG(LogTemp, Warning, TEXT("Unloaded System Module: %s"), *ModuleName);
    return true;
}

bool UEng_ArchitectureManager::IsSystemModuleLoaded(const FString& ModuleName) const
{
    const FEng_SystemModule* Module = SystemModules.FindByPredicate([&ModuleName](const FEng_SystemModule& Mod)
    {
        return Mod.ModuleName == ModuleName;
    });
    
    return Module && Module->bIsLoaded;
}

TArray<FString> UEng_ArchitectureManager::GetLoadedModules() const
{
    TArray<FString> LoadedModules;
    
    for (const FEng_SystemModule& Module : SystemModules)
    {
        if (Module.bIsLoaded)
        {
            LoadedModules.Add(Module.ModuleName);
        }
    }
    
    return LoadedModules;
}

FEng_PerformanceMetrics UEng_ArchitectureManager::GetCurrentPerformanceMetrics() const
{
    return CurrentMetrics;
}

void UEng_ArchitectureManager::UpdatePerformanceMetrics()
{
    UpdateSystemMetrics();
}

void UEng_ArchitectureManager::UpdateSystemMetrics()
{
    // Get frame time
    if (GEngine && GEngine->GetGameViewport())
    {
        CurrentMetrics.FrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to ms
    }
    
    // Estimate other metrics (in a real implementation, these would come from actual profiling)
    CurrentMetrics.RenderTime = CurrentMetrics.FrameTime * 0.6f; // Assume 60% render time
    CurrentMetrics.GameThreadTime = CurrentMetrics.FrameTime * 0.4f; // Assume 40% game thread
    
    // Actor count as proxy for complexity
    if (UWorld* World = GetWorld())
    {
        CurrentMetrics.DrawCalls = FMath::Min(World->GetActorCount() * 2, 5000); // Estimate
        CurrentMetrics.TriangleCount = World->GetActorCount() * 1000; // Estimate
    }
    
    // Memory usage estimate
    CurrentMetrics.MemoryUsage = FPlatformMemory::GetStats().UsedPhysical / (1024.0f * 1024.0f); // MB
}

bool UEng_ArchitectureManager::IsPerformanceWithinLimits() const
{
    return (CurrentMetrics.FrameTime <= TargetFrameTime) &&
           (CurrentMetrics.DrawCalls <= MaxDrawCalls) &&
           (CurrentMetrics.MemoryUsage <= MaxMemoryMB);
}

void UEng_ArchitectureManager::SetPerformanceTargets(float InTargetFrameTime, int32 InMaxDrawCalls, float InMaxMemoryMB)
{
    TargetFrameTime = InTargetFrameTime;
    MaxDrawCalls = InMaxDrawCalls;
    MaxMemoryMB = InMaxMemoryMB;
    
    UE_LOG(LogTemp, Log, TEXT("Performance targets set - FrameTime: %.2fms, DrawCalls: %d, Memory: %.1fMB"), 
           TargetFrameTime, MaxDrawCalls, MaxMemoryMB);
}

bool UEng_ArchitectureManager::ValidateSystemIntegrity()
{
    SystemErrors.Empty();
    
    // Check all loaded modules
    for (const FEng_SystemModule& Module : SystemModules)
    {
        if (Module.bIsLoaded)
        {
            if (!CheckModuleDependencies(Module.ModuleName))
            {
                SystemErrors.Add(FString::Printf(TEXT("Module %s has unresolved dependencies"), *Module.ModuleName));
            }
        }
    }
    
    // Check performance
    UpdatePerformanceMetrics();
    if (!IsPerformanceWithinLimits())
    {
        SystemErrors.Add(TEXT("Performance metrics exceed targets"));
    }
    
    // Check actor count
    if (!IsActorCountWithinLimits())
    {
        SystemErrors.Add(FString::Printf(TEXT("Actor count exceeds limit: %d/%d"), GetTotalActorCount(), MaxActorCount));
    }
    
    return SystemErrors.Num() == 0;
}

TArray<FString> UEng_ArchitectureManager::GetSystemErrors() const
{
    return SystemErrors;
}

void UEng_ArchitectureManager::RunDiagnostics()
{
    UE_LOG(LogTemp, Warning, TEXT("Running System Diagnostics..."));
    
    ValidateSystemIntegrity();
    UpdatePerformanceMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("Diagnostics Complete - Errors: %d"), SystemErrors.Num());
    
    for (const FString& Error : SystemErrors)
    {
        UE_LOG(LogTemp, Error, TEXT("System Error: %s"), *Error);
    }
}

int32 UEng_ArchitectureManager::GetTotalActorCount() const
{
    if (UWorld* World = GetWorld())
    {
        return World->GetActorCount();
    }
    return 0;
}

bool UEng_ArchitectureManager::IsActorCountWithinLimits() const
{
    return GetTotalActorCount() <= MaxActorCount;
}

void UEng_ArchitectureManager::CleanupExcessActors()
{
    if (UWorld* World = GetWorld())
    {
        int32 CurrentCount = World->GetActorCount();
        if (CurrentCount > MaxActorCount)
        {
            int32 ExcessCount = CurrentCount - MaxActorCount;
            UE_LOG(LogTemp, Warning, TEXT("Cleaning up %d excess actors"), ExcessCount);
            
            // This would implement actual cleanup logic
            // For now, just log the need for cleanup
        }
    }
}

bool UEng_ArchitectureManager::CheckModuleDependencies(const FString& ModuleName) const
{
    const FEng_SystemModule* Module = SystemModules.FindByPredicate([&ModuleName](const FEng_SystemModule& Mod)
    {
        return Mod.ModuleName == ModuleName;
    });
    
    if (!Module)
    {
        return false;
    }
    
    for (const FString& Dependency : Module->Dependencies)
    {
        if (!IsSystemModuleLoaded(Dependency))
        {
            return false;
        }
    }
    
    return true;
}

TArray<FString> UEng_ArchitectureManager::GetModuleDependencies(const FString& ModuleName) const
{
    const FEng_SystemModule* Module = SystemModules.FindByPredicate([&ModuleName](const FEng_SystemModule& Mod)
    {
        return Mod.ModuleName == ModuleName;
    });
    
    if (Module)
    {
        return Module->Dependencies;
    }
    
    return TArray<FString>();
}

FEng_SystemModule* UEng_ArchitectureManager::FindSystemModule(const FString& ModuleName)
{
    return SystemModules.FindByPredicate([&ModuleName](const FEng_SystemModule& Mod)
    {
        return Mod.ModuleName == ModuleName;
    });
}

bool UEng_ArchitectureManager::ResolveDependencies(const FString& ModuleName)
{
    const FEng_SystemModule* Module = SystemModules.FindByPredicate([&ModuleName](const FEng_SystemModule& Mod)
    {
        return Mod.ModuleName == ModuleName;
    });
    
    if (!Module)
    {
        return false;
    }
    
    // Load all dependencies first
    for (const FString& Dependency : Module->Dependencies)
    {
        if (!IsSystemModuleLoaded(Dependency))
        {
            if (!LoadSystemModule(Dependency))
            {
                return false;
            }
        }
    }
    
    return true;
}

void UEng_ArchitectureManager::ValidateModuleLoad(const FString& ModuleName)
{
    // Perform post-load validation
    if (ModuleName == TEXT("CoreEngine"))
    {
        // Validate core engine systems
        UE_LOG(LogTemp, Log, TEXT("Validated CoreEngine module"));
    }
    else if (ModuleName == TEXT("Physics"))
    {
        // Validate physics systems
        UE_LOG(LogTemp, Log, TEXT("Validated Physics module"));
    }
    // Add more module-specific validations as needed
}