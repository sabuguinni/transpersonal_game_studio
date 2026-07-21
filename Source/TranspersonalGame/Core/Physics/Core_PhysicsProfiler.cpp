#include "Core_PhysicsProfiler.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Stats/Stats.h"

UCore_PhysicsProfiler::UCore_PhysicsProfiler()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    
    bIsProfiling = false;
    MaxHistoryFrames = 120; // 2 seconds at 60fps
    TargetFrameTime = 16.67f; // 60fps target
    MaxPhysicsStepTime = 8.33f; // Half frame budget for physics
    LastFrameTime = 0.0f;
    PhysicsWorld = nullptr;
}

void UCore_PhysicsProfiler::BeginPlay()
{
    Super::BeginPlay();
    
    PhysicsWorld = GetWorld();
    if (PhysicsWorld)
    {
        UE_LOG(LogTemp, Log, TEXT("PhysicsProfiler: Initialized for world %s"), *PhysicsWorld->GetName());
    }
    
    // Reset profiling data
    ResetProfilingData();
    
    // Start profiling by default in development builds
#if UE_BUILD_DEVELOPMENT
    StartProfiling();
#endif
}

void UCore_PhysicsProfiler::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsProfiling && PhysicsWorld)
    {
        UpdateCurrentFrameData();
        AddFrameDataToHistory();
        
        // Log performance warnings
        if (CurrentFrameData.PhysicsStepTime > MaxPhysicsStepTime)
        {
            UE_LOG(LogTemp, Warning, TEXT("PhysicsProfiler: Physics step time %.2fms exceeds target %.2fms"), 
                CurrentFrameData.PhysicsStepTime, MaxPhysicsStepTime);
        }
    }
}

void UCore_PhysicsProfiler::StartProfiling()
{
    if (!bIsProfiling)
    {
        bIsProfiling = true;
        ResetProfilingData();
        UE_LOG(LogTemp, Log, TEXT("PhysicsProfiler: Started profiling"));
    }
}

void UCore_PhysicsProfiler::StopProfiling()
{
    if (bIsProfiling)
    {
        bIsProfiling = false;
        UE_LOG(LogTemp, Log, TEXT("PhysicsProfiler: Stopped profiling"));
    }
}

FCore_PhysicsProfileData UCore_PhysicsProfiler::GetCurrentProfileData() const
{
    return CurrentFrameData;
}

FCore_PhysicsProfileData UCore_PhysicsProfiler::GetAverageProfileData() const
{
    if (ProfileDataHistory.Num() == 0)
    {
        return FCore_PhysicsProfileData();
    }
    
    FCore_PhysicsProfileData AverageData;
    float TotalFrameTime = 0.0f;
    int32 TotalRigidBodies = 0;
    int32 TotalConstraints = 0;
    float TotalPhysicsStepTime = 0.0f;
    int32 TotalCollisionQueries = 0;
    
    for (const FCore_PhysicsProfileData& Data : ProfileDataHistory)
    {
        TotalFrameTime += Data.FrameTime;
        TotalRigidBodies += Data.ActiveRigidBodies;
        TotalConstraints += Data.PhysicsConstraints;
        TotalPhysicsStepTime += Data.PhysicsStepTime;
        TotalCollisionQueries += Data.CollisionQueries;
    }
    
    int32 NumFrames = ProfileDataHistory.Num();
    AverageData.FrameTime = TotalFrameTime / NumFrames;
    AverageData.ActiveRigidBodies = TotalRigidBodies / NumFrames;
    AverageData.PhysicsConstraints = TotalConstraints / NumFrames;
    AverageData.PhysicsStepTime = TotalPhysicsStepTime / NumFrames;
    AverageData.CollisionQueries = TotalCollisionQueries / NumFrames;
    
    return AverageData;
}

void UCore_PhysicsProfiler::ResetProfilingData()
{
    ProfileDataHistory.Empty();
    CurrentFrameData = FCore_PhysicsProfileData();
    LastFrameTime = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsProfiler: Profiling data reset"));
}

bool UCore_PhysicsProfiler::IsPhysicsPerformanceAcceptable() const
{
    if (ProfileDataHistory.Num() < 10) // Need at least 10 frames of data
    {
        return true; // Assume acceptable until we have enough data
    }
    
    FCore_PhysicsProfileData AverageData = GetAverageProfileData();
    
    // Check if average frame time is within target
    if (AverageData.FrameTime > TargetFrameTime * 1.2f) // 20% tolerance
    {
        return false;
    }
    
    // Check if physics step time is within budget
    if (AverageData.PhysicsStepTime > MaxPhysicsStepTime)
    {
        return false;
    }
    
    return true;
}

void UCore_PhysicsProfiler::UpdateCurrentFrameData()
{
    if (!PhysicsWorld)
    {
        return;
    }
    
    // Get current frame time
    float CurrentTime = PhysicsWorld->GetTimeSeconds();
    if (LastFrameTime > 0.0f)
    {
        CurrentFrameData.FrameTime = (CurrentTime - LastFrameTime) * 1000.0f; // Convert to milliseconds
    }
    LastFrameTime = CurrentTime;
    
    CalculatePhysicsMetrics();
}

void UCore_PhysicsProfiler::AddFrameDataToHistory()
{
    ProfileDataHistory.Add(CurrentFrameData);
    
    // Remove old data if we exceed max history
    if (ProfileDataHistory.Num() > MaxHistoryFrames)
    {
        ProfileDataHistory.RemoveAt(0);
    }
}

void UCore_PhysicsProfiler::CalculatePhysicsMetrics()
{
    if (!PhysicsWorld)
    {
        return;
    }
    
    // Count active rigid bodies
    CurrentFrameData.ActiveRigidBodies = 0;
    CurrentFrameData.PhysicsConstraints = 0;
    CurrentFrameData.CollisionQueries = 0;
    
    // Iterate through all actors to count physics objects
    for (TActorIterator<AActor> ActorItr(PhysicsWorld); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && !Actor->IsPendingKill())
        {
            // Check for physics components
            TArray<UPrimitiveComponent*> PrimitiveComponents;
            Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
            
            for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
            {
                if (PrimComp && PrimComp->IsSimulatingPhysics())
                {
                    CurrentFrameData.ActiveRigidBodies++;
                }
            }
        }
    }
    
    // Estimate physics step time based on frame time and physics settings
    const UPhysicsSettings* PhysicsSettings = GetDefault<UPhysicsSettings>();
    if (PhysicsSettings)
    {
        // Rough estimation: physics typically takes 20-40% of frame time
        CurrentFrameData.PhysicsStepTime = CurrentFrameData.FrameTime * 0.3f;
        
        // Adjust based on number of active bodies
        if (CurrentFrameData.ActiveRigidBodies > 100)
        {
            CurrentFrameData.PhysicsStepTime *= 1.2f; // 20% penalty for high body count
        }
    }
    
    // Estimate collision queries (rough approximation)
    CurrentFrameData.CollisionQueries = CurrentFrameData.ActiveRigidBodies * 2; // Each body might do 2 queries per frame
}