// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "PhysicsPerformanceOptimizer.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Engine/StaticMesh.h"
#include "HAL/IConsoleManager.h"

DEFINE_LOG_CATEGORY(LogPhysicsPerformance);

// Console variables for performance tuning
static TAutoConsoleVariable<int32> CVarMaxPhysicsBodies(
    TEXT("tp.Physics.MaxBodies"),
    5000,
    TEXT("Maximum number of physics bodies allowed in simulation"),
    ECVF_Default
);

static TAutoConsoleVariable<float> CVarPhysicsTickRate(
    TEXT("tp.Physics.TickRate"),
    60.0f,
    TEXT("Physics simulation tick rate"),
    ECVF_Default
);

static TAutoConsoleVariable<int32> CVarMaxCollisionQueries(
    TEXT("tp.Physics.MaxCollisionQueries"),
    1000,
    TEXT("Maximum collision queries per frame"),
    ECVF_Default
);

UPhysicsPerformanceOptimizer::UPhysicsPerformanceOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update every 100ms
    
    // Performance targets
    TargetFrameTime = 16.67f; // 60 FPS target
    MaxPhysicsBodies = 5000;
    MaxCollisionQueries = 1000;
    PhysicsTickRate = 60.0f;
    
    // LOD settings
    LODDistanceThresholds.Add(500.0f);   // LOD 0 -> LOD 1
    LODDistanceThresholds.Add(1000.0f);  // LOD 1 -> LOD 2
    LODDistanceThresholds.Add(2000.0f);  // LOD 2 -> LOD 3
    
    // Performance monitoring
    FrameTimeHistory.Reserve(60); // Store 1 second of frame times at 60fps
    PhysicsBodyCount = 0;
    CollisionQueryCount = 0;
    CurrentLODLevel = 0;
}

void UPhysicsPerformanceOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogPhysicsPerformance, Log, TEXT("Physics Performance Optimizer initialized"));
    
    // Cache world reference
    World = GetWorld();
    if (!World)
    {
        UE_LOG(LogPhysicsPerformance, Error, TEXT("Failed to get world reference"));
        return;
    }
    
    // Initialize performance monitoring
    LastFrameTime = FPlatformTime::Seconds();
    
    // Set up performance targets based on platform
    SetupPlatformSpecificSettings();
}

void UPhysicsPerformanceOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!World)
    {
        return;
    }
    
    // Update performance metrics
    UpdatePerformanceMetrics(DeltaTime);
    
    // Check if optimization is needed
    if (ShouldOptimize())
    {
        OptimizePhysicsPerformance();
    }
    
    // Update LOD based on performance
    UpdatePhysicsLOD();
    
    // Log performance stats periodically
    static float LogTimer = 0.0f;
    LogTimer += DeltaTime;
    if (LogTimer >= 5.0f) // Log every 5 seconds
    {
        LogPerformanceStats();
        LogTimer = 0.0f;
    }
}

void UPhysicsPerformanceOptimizer::UpdatePerformanceMetrics(float DeltaTime)
{
    // Update frame time history
    double CurrentTime = FPlatformTime::Seconds();
    float CurrentFrameTime = (CurrentTime - LastFrameTime) * 1000.0f; // Convert to milliseconds
    LastFrameTime = CurrentTime;
    
    FrameTimeHistory.Add(CurrentFrameTime);
    if (FrameTimeHistory.Num() > 60)
    {
        FrameTimeHistory.RemoveAt(0);
    }
    
    // Calculate average frame time
    float TotalFrameTime = 0.0f;
    for (float FrameTime : FrameTimeHistory)
    {
        TotalFrameTime += FrameTime;
    }
    AverageFrameTime = TotalFrameTime / FMath::Max(FrameTimeHistory.Num(), 1);
    
    // Update physics body count
    PhysicsBodyCount = CountActivePhysicsBodies();
    
    // Reset collision query counter (should be updated by collision system)
    CollisionQueryCount = 0;
}

bool UPhysicsPerformanceOptimizer::ShouldOptimize() const
{
    // Check if frame time exceeds target
    if (AverageFrameTime > TargetFrameTime * 1.1f) // 10% tolerance
    {
        return true;
    }
    
    // Check if physics body count exceeds limit
    if (PhysicsBodyCount > MaxPhysicsBodies)
    {
        return true;
    }
    
    // Check if collision queries exceed limit
    if (CollisionQueryCount > MaxCollisionQueries)
    {
        return true;
    }
    
    return false;
}

void UPhysicsPerformanceOptimizer::OptimizePhysicsPerformance()
{
    UE_LOG(LogPhysicsPerformance, Warning, TEXT("Performance optimization triggered - Frame: %.2fms, Bodies: %d, Queries: %d"),
        AverageFrameTime, PhysicsBodyCount, CollisionQueryCount);
    
    // Reduce physics tick rate if performance is poor
    if (AverageFrameTime > TargetFrameTime * 1.2f)
    {
        PhysicsTickRate = FMath::Max(PhysicsTickRate * 0.9f, 30.0f);
        UE_LOG(LogPhysicsPerformance, Log, TEXT("Reduced physics tick rate to %.1f Hz"), PhysicsTickRate);
    }
    
    // Increase LOD level if too many physics bodies
    if (PhysicsBodyCount > MaxPhysicsBodies)
    {
        CurrentLODLevel = FMath::Min(CurrentLODLevel + 1, LODDistanceThresholds.Num());
        UE_LOG(LogPhysicsPerformance, Log, TEXT("Increased physics LOD level to %d"), CurrentLODLevel);
    }
    
    // Cull distant physics objects
    CullDistantPhysicsObjects();
    
    // Optimize collision detection
    OptimizeCollisionDetection();
}

void UPhysicsPerformanceOptimizer::UpdatePhysicsLOD()
{
    if (!World || !World->GetFirstPlayerController())
    {
        return;
    }
    
    // Get player camera location
    FVector CameraLocation = World->GetFirstPlayerController()->PlayerCameraManager->GetCameraLocation();
    
    // Update LOD for all physics objects based on distance
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor || !Actor->GetRootComponent())
        {
            continue;
        }
        
        float Distance = FVector::Dist(Actor->GetActorLocation(), CameraLocation);
        int32 RequiredLOD = CalculateLODLevel(Distance);
        
        // Apply LOD to static mesh components
        TArray<UStaticMeshComponent*> StaticMeshComponents;
        Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);
        
        for (UStaticMeshComponent* MeshComp : StaticMeshComponents)
        {
            if (MeshComp && MeshComp->GetBodyInstance())
            {
                ApplyPhysicsLOD(MeshComp, RequiredLOD);
            }
        }
        
        // Apply LOD to skeletal mesh components
        TArray<USkeletalMeshComponent*> SkeletalMeshComponents;
        Actor->GetComponents<USkeletalMeshComponent>(SkeletalMeshComponents);
        
        for (USkeletalMeshComponent* MeshComp : SkeletalMeshComponents)
        {
            if (MeshComp && MeshComp->GetBodyInstance())
            {
                ApplyPhysicsLOD(MeshComp, RequiredLOD);
            }
        }
    }
}

int32 UPhysicsPerformanceOptimizer::CalculateLODLevel(float Distance) const
{
    for (int32 i = 0; i < LODDistanceThresholds.Num(); ++i)
    {
        if (Distance < LODDistanceThresholds[i])
        {
            return i;
        }
    }
    return LODDistanceThresholds.Num(); // Highest LOD level
}

void UPhysicsPerformanceOptimizer::ApplyPhysicsLOD(UPrimitiveComponent* Component, int32 LODLevel)
{
    if (!Component || !Component->GetBodyInstance())
    {
        return;
    }
    
    FBodyInstance* BodyInstance = Component->GetBodyInstance();
    
    switch (LODLevel)
    {
    case 0: // High detail
        BodyInstance->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        Component->SetNotifyRigidBodyCollision(true);
        break;
        
    case 1: // Medium detail
        BodyInstance->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        Component->SetNotifyRigidBodyCollision(false);
        break;
        
    case 2: // Low detail
        BodyInstance->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        Component->SetNotifyRigidBodyCollision(false);
        break;
        
    case 3: // Very low detail
        BodyInstance->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->SetNotifyRigidBodyCollision(false);
        break;
        
    default:
        // Disable physics completely for highest LOD
        BodyInstance->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->SetNotifyRigidBodyCollision(false);
        break;
    }
}

void UPhysicsPerformanceOptimizer::CullDistantPhysicsObjects()
{
    if (!World || !World->GetFirstPlayerController())
    {
        return;
    }
    
    FVector CameraLocation = World->GetFirstPlayerController()->PlayerCameraManager->GetCameraLocation();
    float CullDistance = LODDistanceThresholds.Num() > 0 ? LODDistanceThresholds.Last() * 1.5f : 3000.0f;
    
    int32 CulledCount = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor || !Actor->GetRootComponent())
        {
            continue;
        }
        
        float Distance = FVector::Dist(Actor->GetActorLocation(), CameraLocation);
        
        if (Distance > CullDistance)
        {
            // Disable physics for very distant objects
            TArray<UPrimitiveComponent*> PrimitiveComponents;
            Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
            
            for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
            {
                if (PrimComp && PrimComp->GetBodyInstance() && 
                    PrimComp->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
                {
                    PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                    CulledCount++;
                }
            }
        }
    }
    
    if (CulledCount > 0)
    {
        UE_LOG(LogPhysicsPerformance, Log, TEXT("Culled %d distant physics objects"), CulledCount);
    }
}

void UPhysicsPerformanceOptimizer::OptimizeCollisionDetection()
{
    // Reduce collision complexity for distant objects
    // This would typically involve switching to simpler collision shapes
    // or reducing the number of collision channels checked
    
    UE_LOG(LogPhysicsPerformance, Log, TEXT("Optimizing collision detection"));
    
    // Example: Reduce collision query complexity
    // In a real implementation, this would modify collision settings
    // based on performance requirements
}

int32 UPhysicsPerformanceOptimizer::CountActivePhysicsBodies() const
{
    if (!World)
    {
        return 0;
    }
    
    int32 Count = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor)
        {
            continue;
        }
        
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
        {
            if (PrimComp && PrimComp->GetBodyInstance() && 
                PrimComp->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
            {
                Count++;
            }
        }
    }
    
    return Count;
}

void UPhysicsPerformanceOptimizer::SetupPlatformSpecificSettings()
{
    // Adjust settings based on platform capabilities
    FString PlatformName = FPlatformProperties::PlatformName();
    
    if (PlatformName == TEXT("Windows") || PlatformName == TEXT("Mac") || PlatformName == TEXT("Linux"))
    {
        // PC settings - higher performance target
        TargetFrameTime = 16.67f; // 60 FPS
        MaxPhysicsBodies = 5000;
        MaxCollisionQueries = 1000;
    }
    else if (PlatformName == TEXT("XboxOne") || PlatformName == TEXT("PS4"))
    {
        // Console settings - moderate performance target
        TargetFrameTime = 33.33f; // 30 FPS
        MaxPhysicsBodies = 3000;
        MaxCollisionQueries = 500;
    }
    else
    {
        // Mobile/other platforms - conservative settings
        TargetFrameTime = 33.33f; // 30 FPS
        MaxPhysicsBodies = 1000;
        MaxCollisionQueries = 200;
    }
    
    UE_LOG(LogPhysicsPerformance, Log, TEXT("Platform: %s, Target Frame Time: %.2fms, Max Bodies: %d"),
        *PlatformName, TargetFrameTime, MaxPhysicsBodies);
}

void UPhysicsPerformanceOptimizer::LogPerformanceStats() const
{
    UE_LOG(LogPhysicsPerformance, Log, 
        TEXT("Performance Stats - Frame: %.2fms (Target: %.2fms), Bodies: %d (Max: %d), Queries: %d (Max: %d), LOD: %d"),
        AverageFrameTime, TargetFrameTime, PhysicsBodyCount, MaxPhysicsBodies, 
        CollisionQueryCount, MaxCollisionQueries, CurrentLODLevel);
}

// Blueprint callable functions
float UPhysicsPerformanceOptimizer::GetCurrentFrameTime() const
{
    return AverageFrameTime;
}

int32 UPhysicsPerformanceOptimizer::GetPhysicsBodyCount() const
{
    return PhysicsBodyCount;
}

int32 UPhysicsPerformanceOptimizer::GetCurrentLODLevel() const
{
    return CurrentLODLevel;
}

void UPhysicsPerformanceOptimizer::SetTargetFrameTime(float NewTargetFrameTime)
{
    TargetFrameTime = FMath::Max(NewTargetFrameTime, 8.33f); // Minimum 120 FPS
    UE_LOG(LogPhysicsPerformance, Log, TEXT("Target frame time set to %.2fms"), TargetFrameTime);
}

void UPhysicsPerformanceOptimizer::SetMaxPhysicsBodies(int32 NewMaxBodies)
{
    MaxPhysicsBodies = FMath::Max(NewMaxBodies, 100);
    UE_LOG(LogPhysicsPerformance, Log, TEXT("Max physics bodies set to %d"), MaxPhysicsBodies);
}

void UPhysicsPerformanceOptimizer::ForceOptimization()
{
    UE_LOG(LogPhysicsPerformance, Log, TEXT("Forcing physics performance optimization"));
    OptimizePhysicsPerformance();
}