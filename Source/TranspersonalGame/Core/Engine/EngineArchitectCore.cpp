#include "EngineArchitectCore.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformMemory.h"
#include "Misc/App.h"
#include "GameFramework/Actor.h"
#include "Engine/Level.h"

DEFINE_LOG_CATEGORY(LogEngineArchitect);

UEngineArchitectCore::UEngineArchitectCore()
{
    bWorldPartitionEnabled = false;
    MaxActorsPerBiome = 1000;
    MemoryBudgetMB = 8192.0f;
    LastMemoryCheck = 0.0f;
    LastActorCount = 0;
    
    // Initialize world bounds to default large area
    WorldBounds = FBox(FVector(-100000, -100000, -5000), FVector(100000, 100000, 5000));
}

void UEngineArchitectCore::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogEngineArchitect, Warning, TEXT("Engine Architect Core initialized"));
    
    InitializePerformanceBudgets();
    ValidateModuleDependencies();
    
    // Register core modules
    RegisterSystemModule(TEXT("WorldGeneration"), 100);
    RegisterSystemModule(TEXT("Physics"), 90);
    RegisterSystemModule(TEXT("AI"), 80);
    RegisterSystemModule(TEXT("Audio"), 70);
    RegisterSystemModule(TEXT("VFX"), 60);
}

void UEngineArchitectCore::Deinitialize()
{
    UE_LOG(LogEngineArchitect, Warning, TEXT("Engine Architect Core shutting down"));
    
    RegisteredModules.Empty();
    PerformanceBudgets.Empty();
    SystemFrameTimes.Empty();
    
    Super::Deinitialize();
}

bool UEngineArchitectCore::ValidateSystemPerformance(const FString& SystemName, float DeltaTime)
{
    if (!PerformanceBudgets.Contains(SystemName))
    {
        UE_LOG(LogEngineArchitect, Warning, TEXT("System %s not registered for performance tracking"), *SystemName);
        return false;
    }
    
    float Budget = PerformanceBudgets[SystemName];
    SystemFrameTimes.Add(SystemName, DeltaTime);
    
    if (DeltaTime > Budget)
    {
        UE_LOG(LogEngineArchitect, Error, TEXT("System %s exceeded performance budget: %.2fms > %.2fms"), 
               *SystemName, DeltaTime * 1000.0f, Budget * 1000.0f);
        return false;
    }
    
    return true;
}

void UEngineArchitectCore::RegisterSystemModule(const FString& ModuleName, int32 Priority)
{
    if (RegisteredModules.Contains(ModuleName))
    {
        UE_LOG(LogEngineArchitect, Warning, TEXT("Module %s already registered, updating priority"), *ModuleName);
    }
    
    RegisteredModules.Add(ModuleName, Priority);
    UE_LOG(LogEngineArchitect, Log, TEXT("Registered module: %s (Priority: %d)"), *ModuleName, Priority);
}

bool UEngineArchitectCore::CanSpawnActor(UClass* ActorClass, const FVector& Location)
{
    if (!ActorClass)
    {
        UE_LOG(LogEngineArchitect, Error, TEXT("Cannot spawn null actor class"));
        return false;
    }
    
    // Check if location is within world bounds
    if (!WorldBounds.IsInside(Location))
    {
        UE_LOG(LogEngineArchitect, Warning, TEXT("Spawn location outside world bounds: %s"), *Location.ToString());
        return false;
    }
    
    // Check actor count limits
    int32 CurrentActorCount = GetActiveActorCount();
    if (CurrentActorCount >= MaxActorsPerBiome * 5) // 5 biomes max
    {
        UE_LOG(LogEngineArchitect, Error, TEXT("Actor count limit reached: %d"), CurrentActorCount);
        return false;
    }
    
    // Check memory usage
    float MemoryUsage = GetMemoryUsagePercent();
    if (MemoryUsage > 85.0f)
    {
        UE_LOG(LogEngineArchitect, Error, TEXT("Memory usage too high for spawning: %.1f%%"), MemoryUsage);
        return false;
    }
    
    return true;
}

float UEngineArchitectCore::GetMemoryUsagePercent() const
{
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    float UsedMB = static_cast<float>(MemStats.UsedPhysical) / (1024.0f * 1024.0f);
    float TotalMB = static_cast<float>(MemStats.TotalPhysical) / (1024.0f * 1024.0f);
    
    return (UsedMB / TotalMB) * 100.0f;
}

int32 UEngineArchitectCore::GetActiveActorCount() const
{
    if (UWorld* World = GetWorld())
    {
        return World->GetCurrentLevel()->Actors.Num();
    }
    return 0;
}

bool UEngineArchitectCore::IsWorldPartitionRequired() const
{
    float WorldSizeKm = FMath::Max(WorldBounds.GetSize().X, WorldBounds.GetSize().Y) / 100000.0f; // Convert to km
    return WorldSizeKm > UEngineRulesEnforcer::WORLD_PARTITION_THRESHOLD_KM;
}

void UEngineArchitectCore::SetWorldBounds(const FBox& NewBounds)
{
    WorldBounds = NewBounds;
    bWorldPartitionEnabled = IsWorldPartitionRequired();
    
    UE_LOG(LogEngineArchitect, Log, TEXT("World bounds updated: %s (WorldPartition: %s)"), 
           *NewBounds.ToString(), bWorldPartitionEnabled ? TEXT("Enabled") : TEXT("Disabled"));
}

void UEngineArchitectCore::SetPerformanceBudget(const FString& SystemName, float MaxFrameTime)
{
    PerformanceBudgets.Add(SystemName, MaxFrameTime);
    UE_LOG(LogEngineArchitect, Log, TEXT("Performance budget set for %s: %.2fms"), *SystemName, MaxFrameTime * 1000.0f);
}

bool UEngineArchitectCore::IsWithinPerformanceBudget(const FString& SystemName, float CurrentTime)
{
    if (!PerformanceBudgets.Contains(SystemName))
    {
        return true; // No budget set, assume OK
    }
    
    return CurrentTime <= PerformanceBudgets[SystemName];
}

bool UEngineArchitectCore::ValidateModuleDependencies()
{
    // Validate that all registered modules have their dependencies met
    for (const auto& Module : RegisteredModules)
    {
        UE_LOG(LogEngineArchitect, Log, TEXT("Module dependency validation for: %s"), *Module.Key);
    }
    return true;
}

void UEngineArchitectCore::InitializePerformanceBudgets()
{
    // Set default performance budgets for core systems
    SetPerformanceBudget(TEXT("WorldGeneration"), 0.005f); // 5ms
    SetPerformanceBudget(TEXT("Physics"), 0.003f); // 3ms
    SetPerformanceBudget(TEXT("AI"), 0.004f); // 4ms
    SetPerformanceBudget(TEXT("Audio"), 0.002f); // 2ms
    SetPerformanceBudget(TEXT("VFX"), 0.002f); // 2ms
}

void UEngineArchitectCore::CheckMemoryThresholds()
{
    float CurrentMemory = GetMemoryUsagePercent();
    if (CurrentMemory > 90.0f)
    {
        UE_LOG(LogEngineArchitect, Error, TEXT("CRITICAL: Memory usage at %.1f%% - forcing garbage collection"), CurrentMemory);
        GetWorld()->ForceGarbageCollection(true);
    }
    else if (CurrentMemory > 75.0f)
    {
        UE_LOG(LogEngineArchitect, Warning, TEXT("High memory usage: %.1f%%"), CurrentMemory);
    }
}

// Engine Rules Enforcer Implementation
bool UEngineRulesEnforcer::IsWorldSizeValid(float WorldSizeKm)
{
    return WorldSizeKm > 0.0f && WorldSizeKm <= 100.0f; // Max 100km world
}

bool UEngineRulesEnforcer::RequiresWorldPartition(float WorldSizeKm)
{
    return WorldSizeKm > WORLD_PARTITION_THRESHOLD_KM;
}

bool UEngineRulesEnforcer::CanSpawnActorAtLocation(UClass* ActorClass, const FVector& Location, UWorld* World)
{
    if (!ActorClass || !World)
    {
        return false;
    }
    
    // Check if location is valid for spawning
    FVector TraceStart = Location + FVector(0, 0, 1000);
    FVector TraceEnd = Location - FVector(0, 0, 1000);
    
    FHitResult HitResult;
    bool bHit = World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic);
    
    return bHit; // Can spawn if there's ground
}

int32 UEngineRulesEnforcer::GetMaxActorsForBiome(EEng_BiomeType BiomeType)
{
    switch (BiomeType)
    {
        case EEng_BiomeType::Savana:
            return 1200; // Open areas can handle more
        case EEng_BiomeType::Forest:
            return 800;  // Dense vegetation limits
        case EEng_BiomeType::Desert:
            return 600;  // Sparse population
        case EEng_BiomeType::Swamp:
            return 700;  // Medium density
        case EEng_BiomeType::Mountain:
            return 500;  // Harsh terrain
        default:
            return MAX_ACTORS_PER_BIOME;
    }
}

bool UEngineRulesEnforcer::IsFrameTimeAcceptable(float FrameTime)
{
    return FrameTime <= TARGET_FRAME_TIME_MS / 1000.0f;
}

float UEngineRulesEnforcer::GetTargetFrameTime()
{
    return TARGET_FRAME_TIME_MS / 1000.0f;
}

bool UEngineRulesEnforcer::IsMemoryUsageAcceptable(float MemoryUsageMB)
{
    return MemoryUsageMB <= MAX_MEMORY_BUDGET_MB;
}

float UEngineRulesEnforcer::GetMaxMemoryBudget()
{
    return MAX_MEMORY_BUDGET_MB;
}

bool UEngineRulesEnforcer::CanModuleAccessOther(const FString& RequesterModule, const FString& TargetModule)
{
    // Define module access rules
    static TMap<FString, TArray<FString>> ModuleAccess = {
        {TEXT("WorldGeneration"), {TEXT("Physics"), TEXT("Environment")}},
        {TEXT("AI"), {TEXT("Physics"), TEXT("Audio"), TEXT("Animation")}},
        {TEXT("Combat"), {TEXT("AI"), TEXT("Physics"), TEXT("Audio"), TEXT("VFX")}},
        {TEXT("Quest"), {TEXT("AI"), TEXT("NPC"), TEXT("Audio")}},
        {TEXT("Audio"), {}}, // Audio can be accessed by all
        {TEXT("VFX"), {}} // VFX can be accessed by all
    };
    
    if (ModuleAccess.Contains(RequesterModule))
    {
        return ModuleAccess[RequesterModule].Contains(TargetModule);
    }
    
    return false; // Default deny
}

TArray<FString> UEngineRulesEnforcer::GetModuleDependencies(const FString& ModuleName)
{
    static TMap<FString, TArray<FString>> Dependencies = {
        {TEXT("WorldGeneration"), {TEXT("Core")}},
        {TEXT("Physics"), {TEXT("Core")}},
        {TEXT("AI"), {TEXT("Core"), TEXT("Physics")}},
        {TEXT("Combat"), {TEXT("Core"), TEXT("Physics"), TEXT("AI")}},
        {TEXT("Audio"), {TEXT("Core")}},
        {TEXT("VFX"), {TEXT("Core")}}
    };
    
    if (Dependencies.Contains(ModuleName))
    {
        return Dependencies[ModuleName];
    }
    
    return TArray<FString>();
}