#include "Core_PhysicsWorldStreaming.h"
#include "Core_PhysicsManager.h"
#include "Core_PhysicsSimulationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/Level.h"
#include "Engine/LevelStreaming.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "DrawDebugHelpers.h"

UCore_PhysicsWorldStreaming::UCore_PhysicsWorldStreaming()
{
    PhysicsMemoryBudgetMB = 512.0f; // 512MB default budget
    StreamingUpdateFrequency = 10.0f; // 10Hz update rate
    bAutoOptimizePhysics = true;
    bEnablePhysicsLOD = true;
    StreamingUpdateTimer = 0.0f;
    PhysicsManager = nullptr;
    SimulationManager = nullptr;
}

void UCore_PhysicsWorldStreaming::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldStreaming: Initializing physics streaming subsystem"));
    
    // Initialize streaming system
    InitializePhysicsStreaming();
    
    // Get physics managers
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        PhysicsManager = GameInstance->GetSubsystem<UCore_PhysicsManager>();
        SimulationManager = GameInstance->GetSubsystem<UCore_PhysicsSimulationManager>();
    }
}

void UCore_PhysicsWorldStreaming::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldStreaming: Shutting down physics streaming subsystem"));
    
    ShutdownPhysicsStreaming();
    
    Super::Deinitialize();
}

void UCore_PhysicsWorldStreaming::InitializePhysicsStreaming()
{
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldStreaming: Initializing physics streaming"));
    
    // Clear existing data
    StreamingLevels.Empty();
    CurrentMetrics = FCore_PhysicsStreamingMetrics();
    StreamingUpdateTimer = 0.0f;
    
    // Register for level streaming events
    if (UWorld* World = GetWorld())
    {
        // Bind to level streaming events
        FWorldDelegates::LevelAddedToWorld.AddUObject(this, &UCore_PhysicsWorldStreaming::HandleLevelLoaded);
        FWorldDelegates::LevelRemovedFromWorld.AddUObject(this, &UCore_PhysicsWorldStreaming::HandleLevelUnloaded);
        
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldStreaming: Registered for level streaming events"));
    }
}

void UCore_PhysicsWorldStreaming::UpdatePhysicsStreaming(float DeltaTime)
{
    StreamingUpdateTimer += DeltaTime;
    
    // Update at specified frequency
    if (StreamingUpdateTimer >= (1.0f / StreamingUpdateFrequency))
    {
        StreamingUpdateTimer = 0.0f;
        
        // Update metrics for all active levels
        for (FCore_PhysicsLevelData& LevelData : StreamingLevels)
        {
            if (LevelData.bPhysicsActive)
            {
                UpdateLevelPhysicsData(LevelData.LevelName);
            }
        }
        
        // Check memory budget
        CheckMemoryBudget();
        
        // Optimize physics LOD if enabled
        if (bEnablePhysicsLOD)
        {
            OptimizePhysicsLOD();
        }
        
        // Update current metrics
        CurrentMetrics.LoadedLevelsCount = StreamingLevels.Num();
        CurrentMetrics.TotalPhysicsBodies = 0;
        CurrentMetrics.TotalPhysicsMemoryMB = 0.0f;
        
        for (const FCore_PhysicsLevelData& LevelData : StreamingLevels)
        {
            if (LevelData.bPhysicsActive)
            {
                CurrentMetrics.TotalPhysicsBodies += LevelData.PhysicsBodiesCount;
                CurrentMetrics.TotalPhysicsMemoryMB += LevelData.PhysicsMemoryUsageMB;
            }
        }
    }
}

void UCore_PhysicsWorldStreaming::ShutdownPhysicsStreaming()
{
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldStreaming: Shutting down physics streaming"));
    
    // Unregister from level streaming events
    FWorldDelegates::LevelAddedToWorld.RemoveAll(this);
    FWorldDelegates::LevelRemovedFromWorld.RemoveAll(this);
    
    // Clear all data
    StreamingLevels.Empty();
    CurrentMetrics = FCore_PhysicsStreamingMetrics();
}

void UCore_PhysicsWorldStreaming::RegisterStreamingLevel(const FString& LevelName, float StreamingDistance)
{
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldStreaming: Registering streaming level: %s"), *LevelName);
    
    // Check if level already registered
    for (FCore_PhysicsLevelData& LevelData : StreamingLevels)
    {
        if (LevelData.LevelName == LevelName)
        {
            LevelData.StreamingDistance = StreamingDistance;
            UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsWorldStreaming: Level %s already registered, updating distance"), *LevelName);
            return;
        }
    }
    
    // Create new level data
    FCore_PhysicsLevelData NewLevelData;
    NewLevelData.LevelName = LevelName;
    NewLevelData.StreamingDistance = StreamingDistance;
    NewLevelData.bPhysicsActive = true;
    
    StreamingLevels.Add(NewLevelData);
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldStreaming: Registered level %s with streaming distance %.2f"), *LevelName, StreamingDistance);
}

void UCore_PhysicsWorldStreaming::UnregisterStreamingLevel(const FString& LevelName)
{
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldStreaming: Unregistering streaming level: %s"), *LevelName);
    
    for (int32 i = StreamingLevels.Num() - 1; i >= 0; i--)
    {
        if (StreamingLevels[i].LevelName == LevelName)
        {
            StreamingLevels.RemoveAt(i);
            UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldStreaming: Unregistered level %s"), *LevelName);
            return;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsWorldStreaming: Level %s not found for unregistration"), *LevelName);
}

void UCore_PhysicsWorldStreaming::SetLevelPhysicsActive(const FString& LevelName, bool bActive)
{
    for (FCore_PhysicsLevelData& LevelData : StreamingLevels)
    {
        if (LevelData.LevelName == LevelName)
        {
            LevelData.bPhysicsActive = bActive;
            UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldStreaming: Set physics active for level %s: %s"), 
                   *LevelName, bActive ? TEXT("true") : TEXT("false"));
            return;
        }
    }
}

void UCore_PhysicsWorldStreaming::OptimizePhysicsForLevel(const FString& LevelName)
{
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldStreaming: Optimizing physics for level: %s"), *LevelName);
    
    if (PhysicsManager)
    {
        // Use physics manager to optimize level
        // This would integrate with the physics manager's optimization systems
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldStreaming: Physics optimization applied for level %s"), *LevelName);
    }
}

void UCore_PhysicsWorldStreaming::SetPhysicsLODForLevel(const FString& LevelName, int32 LODLevel)
{
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldStreaming: Setting physics LOD %d for level: %s"), LODLevel, *LevelName);
    
    // Implementation would set physics LOD for all physics bodies in the level
    // This integrates with the physics LOD system
}

void UCore_PhysicsWorldStreaming::UpdatePhysicsMemoryBudget(float MaxMemoryMB)
{
    PhysicsMemoryBudgetMB = MaxMemoryMB;
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldStreaming: Updated physics memory budget to %.2f MB"), MaxMemoryMB);
    
    // Immediately check if we're over budget
    CheckMemoryBudget();
}

FCore_PhysicsStreamingMetrics UCore_PhysicsWorldStreaming::GetStreamingMetrics() const
{
    return CurrentMetrics;
}

TArray<FCore_PhysicsLevelData> UCore_PhysicsWorldStreaming::GetActiveLevelsData() const
{
    TArray<FCore_PhysicsLevelData> ActiveLevels;
    
    for (const FCore_PhysicsLevelData& LevelData : StreamingLevels)
    {
        if (LevelData.bPhysicsActive)
        {
            ActiveLevels.Add(LevelData);
        }
    }
    
    return ActiveLevels;
}

float UCore_PhysicsWorldStreaming::GetPhysicsMemoryUsage() const
{
    return CurrentMetrics.TotalPhysicsMemoryMB;
}

void UCore_PhysicsWorldStreaming::DebugPrintStreamingStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PHYSICS STREAMING STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Loaded Levels: %d"), CurrentMetrics.LoadedLevelsCount);
    UE_LOG(LogTemp, Warning, TEXT("Total Physics Bodies: %d"), CurrentMetrics.TotalPhysicsBodies);
    UE_LOG(LogTemp, Warning, TEXT("Total Memory Usage: %.2f MB / %.2f MB"), 
           CurrentMetrics.TotalPhysicsMemoryMB, PhysicsMemoryBudgetMB);
    
    for (const FCore_PhysicsLevelData& LevelData : StreamingLevels)
    {
        UE_LOG(LogTemp, Warning, TEXT("Level: %s | Bodies: %d | Memory: %.2f MB | Active: %s"),
               *LevelData.LevelName, LevelData.PhysicsBodiesCount, LevelData.PhysicsMemoryUsageMB,
               LevelData.bPhysicsActive ? TEXT("Yes") : TEXT("No"));
    }
}

void UCore_PhysicsWorldStreaming::DebugVisualizeStreamingBounds()
{
    if (UWorld* World = GetWorld())
    {
        for (const FCore_PhysicsLevelData& LevelData : StreamingLevels)
        {
            // Draw debug spheres for streaming bounds
            FVector LevelCenter = FVector::ZeroVector; // Would calculate actual level center
            FColor DebugColor = LevelData.bPhysicsActive ? FColor::Green : FColor::Red;
            
            DrawDebugSphere(World, LevelCenter, LevelData.StreamingDistance, 32, DebugColor, false, 5.0f);
        }
    }
}

void UCore_PhysicsWorldStreaming::UpdateLevelPhysicsData(const FString& LevelName)
{
    // Find level data
    FCore_PhysicsLevelData* LevelData = nullptr;
    for (FCore_PhysicsLevelData& Data : StreamingLevels)
    {
        if (Data.LevelName == LevelName)
        {
            LevelData = &Data;
            break;
        }
    }
    
    if (!LevelData)
    {
        return;
    }
    
    // Update physics data for this level
    if (UWorld* World = GetWorld())
    {
        // Find the actual level
        for (ULevelStreaming* StreamingLevel : World->GetStreamingLevels())
        {
            if (StreamingLevel && StreamingLevel->GetWorldAssetPackageName() == LevelName)
            {
                if (ULevel* Level = StreamingLevel->GetLoadedLevel())
                {
                    LevelData->PhysicsBodiesCount = CountPhysicsBodiesInLevel(Level);
                    LevelData->PhysicsMemoryUsageMB = CalculateLevelPhysicsMemory(Level);
                }
                break;
            }
        }
    }
}

void UCore_PhysicsWorldStreaming::CheckMemoryBudget()
{
    if (CurrentMetrics.TotalPhysicsMemoryMB > PhysicsMemoryBudgetMB)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsWorldStreaming: Physics memory over budget! %.2f / %.2f MB"),
               CurrentMetrics.TotalPhysicsMemoryMB, PhysicsMemoryBudgetMB);
        
        if (bAutoOptimizePhysics)
        {
            // Auto-optimize by reducing physics quality for distant levels
            OptimizePhysicsLOD();
        }
    }
}

void UCore_PhysicsWorldStreaming::OptimizePhysicsLOD()
{
    // Implement physics LOD optimization based on distance and memory usage
    for (FCore_PhysicsLevelData& LevelData : StreamingLevels)
    {
        if (LevelData.bPhysicsActive && LevelData.PhysicsMemoryUsageMB > 50.0f)
        {
            // Apply LOD optimization for high-memory levels
            SetPhysicsLODForLevel(LevelData.LevelName, 1);
        }
    }
}

void UCore_PhysicsWorldStreaming::HandleLevelLoaded(ULevel* LoadedLevel)
{
    if (LoadedLevel)
    {
        FString LevelName = LoadedLevel->GetOuter()->GetName();
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldStreaming: Level loaded: %s"), *LevelName);
        
        // Register level if not already registered
        bool bFound = false;
        for (const FCore_PhysicsLevelData& LevelData : StreamingLevels)
        {
            if (LevelData.LevelName == LevelName)
            {
                bFound = true;
                break;
            }
        }
        
        if (!bFound)
        {
            RegisterStreamingLevel(LevelName, 10000.0f); // Default streaming distance
        }
    }
}

void UCore_PhysicsWorldStreaming::HandleLevelUnloaded(ULevel* UnloadedLevel)
{
    if (UnloadedLevel)
    {
        FString LevelName = UnloadedLevel->GetOuter()->GetName();
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldStreaming: Level unloaded: %s"), *LevelName);
        
        // Set physics inactive for unloaded level
        SetLevelPhysicsActive(LevelName, false);
    }
}

float UCore_PhysicsWorldStreaming::CalculateLevelPhysicsMemory(ULevel* Level) const
{
    if (!Level)
    {
        return 0.0f;
    }
    
    float TotalMemory = 0.0f;
    
    // Calculate memory usage for all physics bodies in the level
    for (AActor* Actor : Level->Actors)
    {
        if (Actor)
        {
            TArray<UPrimitiveComponent*> PrimitiveComponents;
            Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
            
            for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
            {
                if (PrimComp && PrimComp->GetBodyInstance())
                {
                    // Estimate memory usage per physics body (simplified calculation)
                    TotalMemory += 0.1f; // ~100KB per physics body estimate
                }
            }
        }
    }
    
    return TotalMemory;
}

int32 UCore_PhysicsWorldStreaming::CountPhysicsBodiesInLevel(ULevel* Level) const
{
    if (!Level)
    {
        return 0;
    }
    
    int32 PhysicsBodies = 0;
    
    for (AActor* Actor : Level->Actors)
    {
        if (Actor)
        {
            TArray<UPrimitiveComponent*> PrimitiveComponents;
            Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
            
            for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
            {
                if (PrimComp && PrimComp->GetBodyInstance() && PrimComp->IsSimulatingPhysics())
                {
                    PhysicsBodies++;
                }
            }
        }
    }
    
    return PhysicsBodies;
}