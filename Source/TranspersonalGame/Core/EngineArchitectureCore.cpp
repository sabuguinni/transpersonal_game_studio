#include "EngineArchitectureCore.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"

DEFINE_LOG_CATEGORY(LogEngineArchitecture);

UEngineArchitectureCore::UEngineArchitectureCore()
{
    // Initialize default metrics
    CurrentMetrics.FrameTime = 0.0f;
    CurrentMetrics.MemoryUsage = 0;
    CurrentMetrics.ActiveActors = 0;
    CurrentMetrics.PhysicsObjects = 0;
    CurrentMetrics.RenderTime = 0.0f;
}

void UEngineArchitectureCore::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogEngineArchitecture, Log, TEXT("Engine Architecture Core initializing..."));
    
    // Initialize core architecture
    InitializeCoreArchitecture();
    
    // Start periodic validation if enabled
    if (bEnableArchitectureValidation)
    {
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().SetTimer(
                ValidationTimerHandle,
                this,
                &UEngineArchitectureCore::PerformPeriodicValidation,
                5.0f,  // Every 5 seconds
                true   // Loop
            );
        }
    }
    
    UE_LOG(LogEngineArchitecture, Log, TEXT("Engine Architecture Core initialized successfully"));
}

void UEngineArchitectureCore::Deinitialize()
{
    UE_LOG(LogEngineArchitecture, Log, TEXT("Engine Architecture Core deinitializing..."));
    
    // Clear validation timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ValidationTimerHandle);
    }
    
    // Cleanup registered systems
    CleanupOrphanedSystems();
    RegisteredSystems.Empty();
    
    Super::Deinitialize();
    
    UE_LOG(LogEngineArchitecture, Log, TEXT("Engine Architecture Core deinitialized"));
}

UEngineArchitectureCore* UEngineArchitectureCore::Get(const UObject* WorldContext)
{
    if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull))
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            return GameInstance->GetSubsystem<UEngineArchitectureCore>();
        }
    }
    return nullptr;
}

void UEngineArchitectureCore::InitializeCoreArchitecture()
{
    UE_LOG(LogEngineArchitecture, Log, TEXT("Initializing core architecture systems..."));
    
    // Clear existing systems
    RegisteredSystems.Empty();
    
    // Register self as the core system
    RegisterSystemComponent(TEXT("EngineArchitectureCore"), this);
    
    // Initialize performance metrics
    UpdatePerformanceMetrics();
    
    UE_LOG(LogEngineArchitecture, Log, TEXT("Core architecture initialized with %d systems"), RegisteredSystems.Num());
}

bool UEngineArchitectureCore::ValidateArchitecturalCompliance()
{
    UE_LOG(LogEngineArchitecture, Log, TEXT("Validating architectural compliance..."));
    
    bool bCompliant = true;
    int32 ValidSystems = 0;
    int32 InvalidSystems = 0;
    
    // Validate each registered system
    for (const auto& SystemPair : RegisteredSystems)
    {
        const FString& SystemName = SystemPair.Key;
        UObject* SystemInstance = SystemPair.Value;
        
        if (IsValid(SystemInstance))
        {
            ValidSystems++;
            UE_LOG(LogEngineArchitecture, VeryVerbose, TEXT("System '%s' is valid"), *SystemName);
        }
        else
        {
            InvalidSystems++;
            bCompliant = false;
            UE_LOG(LogEngineArchitecture, Warning, TEXT("System '%s' is invalid or null"), *SystemName);
        }
    }
    
    // Check system count limits
    if (RegisteredSystems.Num() > MaxRegisteredSystems)
    {
        bCompliant = false;
        UE_LOG(LogEngineArchitecture, Error, TEXT("Too many registered systems: %d (max: %d)"), 
               RegisteredSystems.Num(), MaxRegisteredSystems);
    }
    
    UE_LOG(LogEngineArchitecture, Log, TEXT("Architecture validation complete: %s (Valid: %d, Invalid: %d)"), 
           bCompliant ? TEXT("COMPLIANT") : TEXT("NON-COMPLIANT"), ValidSystems, InvalidSystems);
    
    return bCompliant;
}

void UEngineArchitectureCore::RegisterSystemComponent(const FString& SystemName, UObject* SystemInstance)
{
    if (!IsValid(SystemInstance))
    {
        UE_LOG(LogEngineArchitecture, Warning, TEXT("Attempted to register invalid system: %s"), *SystemName);
        return;
    }
    
    if (RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogEngineArchitecture, Warning, TEXT("System '%s' already registered, replacing..."), *SystemName);
    }
    
    RegisteredSystems.Add(SystemName, SystemInstance);
    UE_LOG(LogEngineArchitecture, Log, TEXT("Registered system: %s (%s)"), 
           *SystemName, *SystemInstance->GetClass()->GetName());
}

FEnginePerformanceMetrics UEngineArchitectureCore::GetPerformanceMetrics() const
{
    return CurrentMetrics;
}

void UEngineArchitectureCore::ForceMemoryCleanup()
{
    UE_LOG(LogEngineArchitecture, Log, TEXT("Forcing memory cleanup..."));
    
    // Cleanup orphaned systems first
    CleanupOrphanedSystems();
    
    // Force garbage collection
    GEngine->ForceGarbageCollection(true);
    
    // Update metrics after cleanup
    UpdatePerformanceMetrics();
    
    UE_LOG(LogEngineArchitecture, Log, TEXT("Memory cleanup complete. Memory usage: %d MB"), 
           CurrentMetrics.MemoryUsage / 1024 / 1024);
}

void UEngineArchitectureCore::PerformPeriodicValidation()
{
    // Update performance metrics
    UpdatePerformanceMetrics();
    
    // Cleanup orphaned systems
    CleanupOrphanedSystems();
    
    // Validate architecture if enabled
    if (bEnableArchitectureValidation)
    {
        ValidateArchitecturalCompliance();
    }
}

void UEngineArchitectureCore::UpdatePerformanceMetrics()
{
    // Get current frame time
    CurrentMetrics.FrameTime = FApp::GetDeltaTime();
    
    // Get memory usage
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsage = MemStats.UsedPhysical;
    
    // Get actor count
    if (UWorld* World = GetWorld())
    {
        CurrentMetrics.ActiveActors = World->GetActorCount();
        
        // Count physics objects (approximate)
        CurrentMetrics.PhysicsObjects = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (IsValid(Actor) && Actor->GetRootComponent() && 
                Actor->GetRootComponent()->IsSimulatingPhysics())
            {
                CurrentMetrics.PhysicsObjects++;
            }
        }
    }
    
    // Render time (approximate from frame time)
    CurrentMetrics.RenderTime = CurrentMetrics.FrameTime * 0.6f; // Rough estimate
}

void UEngineArchitectureCore::CleanupOrphanedSystems()
{
    TArray<FString> SystemsToRemove;
    
    // Find invalid systems
    for (const auto& SystemPair : RegisteredSystems)
    {
        if (!IsValid(SystemPair.Value))
        {
            SystemsToRemove.Add(SystemPair.Key);
        }
    }
    
    // Remove invalid systems
    for (const FString& SystemName : SystemsToRemove)
    {
        RegisteredSystems.Remove(SystemName);
        UE_LOG(LogEngineArchitecture, Log, TEXT("Removed orphaned system: %s"), *SystemName);
    }
    
    if (SystemsToRemove.Num() > 0)
    {
        UE_LOG(LogEngineArchitecture, Log, TEXT("Cleaned up %d orphaned systems"), SystemsToRemove.Num());
    }
}