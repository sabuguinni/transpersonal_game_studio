#include "PhysicsPerformanceMonitor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"

DEFINE_LOG_CATEGORY(LogPhysicsPerformance);

UPhysicsPerformanceMonitor::UPhysicsPerformanceMonitor()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update every 100ms
    
    // Initialize performance tracking
    MaxPhysicsObjects = 500;
    TargetFrameTime = 16.67f; // 60fps
    CurrentFrameTime = 0.0f;
    PhysicsObjectCount = 0;
    
    // LOD settings
    LODDistances.Add(500.0f);  // Near
    LODDistances.Add(1000.0f); // Medium
    LODDistances.Add(2000.0f); // Far
    
    bPerformanceOptimizationEnabled = true;
    bLODSystemEnabled = true;
    bAutoOptimization = true;
    
    PerformanceLevel = ECore_PhysicsPerformanceLevel::High;
    
    // Initialize counters
    NearObjectCount = 0;
    MediumObjectCount = 0;
    FarObjectCount = 0;
    DisabledObjectCount = 0;
    
    LastOptimizationTime = 0.0f;
    OptimizationInterval = 1.0f; // Optimize every second
}

void UPhysicsPerformanceMonitor::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogPhysicsPerformance, Log, TEXT("Physics Performance Monitor initialized"));
    
    // Start performance monitoring
    StartPerformanceMonitoring();
    
    // Initial scan of physics objects
    ScanPhysicsObjects();
}

void UPhysicsPerformanceMonitor::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update performance metrics
    UpdatePerformanceMetrics(DeltaTime);
    
    // Check if optimization is needed
    if (bAutoOptimization && ShouldOptimize())
    {
        OptimizePhysicsPerformance();
    }
    
    // Update LOD system
    if (bLODSystemEnabled)
    {
        UpdateLODSystem();
    }
}

void UPhysicsPerformanceMonitor::StartPerformanceMonitoring()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogPhysicsPerformance, Error, TEXT("Cannot start monitoring - no world"));
        return;
    }
    
    // Reset performance data
    PerformanceData.Reset();
    
    UE_LOG(LogPhysicsPerformance, Log, TEXT("Performance monitoring started"));
}

void UPhysicsPerformanceMonitor::UpdatePerformanceMetrics(float DeltaTime)
{
    // Update frame time tracking
    CurrentFrameTime = DeltaTime * 1000.0f; // Convert to milliseconds
    
    // Track performance over time
    FCore_PhysicsPerformanceData NewData;
    NewData.FrameTime = CurrentFrameTime;
    NewData.PhysicsObjectCount = PhysicsObjectCount;
    NewData.Timestamp = FDateTime::Now();
    
    PerformanceData.Add(NewData);
    
    // Keep only recent data (last 60 seconds)
    if (PerformanceData.Num() > 600) // 60 seconds at 10Hz
    {
        PerformanceData.RemoveAt(0);
    }
    
    // Update performance level based on frame time
    UpdatePerformanceLevel();
}

void UPhysicsPerformanceMonitor::UpdatePerformanceLevel()
{
    if (CurrentFrameTime > TargetFrameTime * 2.0f) // Below 30fps
    {
        PerformanceLevel = ECore_PhysicsPerformanceLevel::Low;
    }
    else if (CurrentFrameTime > TargetFrameTime * 1.5f) // Below 40fps
    {
        PerformanceLevel = ECore_PhysicsPerformanceLevel::Medium;
    }
    else
    {
        PerformanceLevel = ECore_PhysicsPerformanceLevel::High;
    }
}

bool UPhysicsPerformanceMonitor::ShouldOptimize()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Don't optimize too frequently
    if (CurrentTime - LastOptimizationTime < OptimizationInterval)
    {
        return false;
    }
    
    // Optimize if performance is poor
    if (PerformanceLevel == ECore_PhysicsPerformanceLevel::Low)
    {
        return true;
    }
    
    // Optimize if too many physics objects
    if (PhysicsObjectCount > MaxPhysicsObjects)
    {
        return true;
    }
    
    return false;
}

void UPhysicsPerformanceMonitor::OptimizePhysicsPerformance()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    LastOptimizationTime = World->GetTimeSeconds();
    
    UE_LOG(LogPhysicsPerformance, Log, TEXT("Optimizing physics performance - Level: %d, Objects: %d"), 
           (int32)PerformanceLevel, PhysicsObjectCount);
    
    // Get player location for distance-based optimization
    FVector PlayerLocation = GetPlayerLocation();
    
    // Apply optimizations based on performance level
    switch (PerformanceLevel)
    {
        case ECore_PhysicsPerformanceLevel::Low:
            ApplyLowPerformanceOptimizations(PlayerLocation);
            break;
            
        case ECore_PhysicsPerformanceLevel::Medium:
            ApplyMediumPerformanceOptimizations(PlayerLocation);
            break;
            
        case ECore_PhysicsPerformanceLevel::High:
            ApplyHighPerformanceOptimizations(PlayerLocation);
            break;
    }
    
    // Rescan physics objects after optimization
    ScanPhysicsObjects();
}

void UPhysicsPerformanceMonitor::ApplyLowPerformanceOptimizations(const FVector& PlayerLocation)
{
    // Aggressive optimization for low performance
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor || !IsValidPhysicsActor(Actor))
        {
            continue;
        }
        
        float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
        
        // Disable physics for far objects
        if (Distance > LODDistances[0]) // Beyond near distance
        {
            DisablePhysicsForActor(Actor);
        }
    }
}

void UPhysicsPerformanceMonitor::ApplyMediumPerformanceOptimizations(const FVector& PlayerLocation)
{
    // Moderate optimization
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor || !IsValidPhysicsActor(Actor))
        {
            continue;
        }
        
        float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
        
        // Disable physics for very far objects
        if (Distance > LODDistances[2]) // Beyond far distance
        {
            DisablePhysicsForActor(Actor);
        }
        // Simplify physics for medium distance objects
        else if (Distance > LODDistances[1])
        {
            SimplifyPhysicsForActor(Actor);
        }
    }
}

void UPhysicsPerformanceMonitor::ApplyHighPerformanceOptimizations(const FVector& PlayerLocation)
{
    // Light optimization - only disable very distant objects
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor || !IsValidPhysicsActor(Actor))
        {
            continue;
        }
        
        float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
        
        // Only disable physics for extremely far objects
        if (Distance > LODDistances[2] * 2.0f)
        {
            DisablePhysicsForActor(Actor);
        }
    }
}

void UPhysicsPerformanceMonitor::UpdateLODSystem()
{
    if (!bLODSystemEnabled)
    {
        return;
    }
    
    FVector PlayerLocation = GetPlayerLocation();
    
    // Reset counters
    NearObjectCount = 0;
    MediumObjectCount = 0;
    FarObjectCount = 0;
    DisabledObjectCount = 0;
    
    // Update LOD for all physics objects
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor || !IsValidPhysicsActor(Actor))
        {
            continue;
        }
        
        float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
        ECore_PhysicsLODLevel LODLevel = CalculateLODLevel(Distance);
        
        ApplyLODToActor(Actor, LODLevel);
        
        // Update counters
        switch (LODLevel)
        {
            case ECore_PhysicsLODLevel::High:
                NearObjectCount++;
                break;
            case ECore_PhysicsLODLevel::Medium:
                MediumObjectCount++;
                break;
            case ECore_PhysicsLODLevel::Low:
                FarObjectCount++;
                break;
            case ECore_PhysicsLODLevel::Disabled:
                DisabledObjectCount++;
                break;
        }
    }
}

ECore_PhysicsLODLevel UPhysicsPerformanceMonitor::CalculateLODLevel(float Distance)
{
    if (Distance <= LODDistances[0])
    {
        return ECore_PhysicsLODLevel::High;
    }
    else if (Distance <= LODDistances[1])
    {
        return ECore_PhysicsLODLevel::Medium;
    }
    else if (Distance <= LODDistances[2])
    {
        return ECore_PhysicsLODLevel::Low;
    }
    else
    {
        return ECore_PhysicsLODLevel::Disabled;
    }
}

void UPhysicsPerformanceMonitor::ApplyLODToActor(AActor* Actor, ECore_PhysicsLODLevel LODLevel)
{
    if (!Actor)
    {
        return;
    }
    
    UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
    if (!MeshComp)
    {
        return;
    }
    
    switch (LODLevel)
    {
        case ECore_PhysicsLODLevel::High:
            // Full physics simulation
            MeshComp->SetSimulatePhysics(true);
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            break;
            
        case ECore_PhysicsLODLevel::Medium:
            // Simplified physics
            MeshComp->SetSimulatePhysics(true);
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            break;
            
        case ECore_PhysicsLODLevel::Low:
            // Query only collision
            MeshComp->SetSimulatePhysics(false);
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            break;
            
        case ECore_PhysicsLODLevel::Disabled:
            // No collision or physics
            MeshComp->SetSimulatePhysics(false);
            MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            break;
    }
}

void UPhysicsPerformanceMonitor::ScanPhysicsObjects()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    PhysicsObjectCount = 0;
    
    // Count all physics-enabled objects
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && IsValidPhysicsActor(Actor))
        {
            PhysicsObjectCount++;
        }
    }
    
    UE_LOG(LogPhysicsPerformance, Log, TEXT("Physics object scan complete - Found %d objects"), PhysicsObjectCount);
}

bool UPhysicsPerformanceMonitor::IsValidPhysicsActor(AActor* Actor)
{
    if (!Actor)
    {
        return false;
    }
    
    UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
    if (!MeshComp)
    {
        return false;
    }
    
    // Check if it has physics simulation or collision
    return MeshComp->IsSimulatingPhysics() || 
           MeshComp->GetCollisionEnabled() != ECollisionEnabled::NoCollision;
}

FVector UPhysicsPerformanceMonitor::GetPlayerLocation()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return FVector::ZeroVector;
    }
    
    // Try to get player pawn location
    APawn* PlayerPawn = World->GetFirstPlayerController()->GetPawn();
    if (PlayerPawn)
    {
        return PlayerPawn->GetActorLocation();
    }
    
    // Fallback to player start
    for (TActorIterator<APlayerStart> ActorItr(World); ActorItr; ++ActorItr)
    {
        return ActorItr->GetActorLocation();
    }
    
    return FVector::ZeroVector;
}

void UPhysicsPerformanceMonitor::DisablePhysicsForActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
    if (MeshComp)
    {
        MeshComp->SetSimulatePhysics(false);
        MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        
        // Add tag to track disabled state
        Actor->Tags.AddUnique(FName("PhysicsDisabled"));
    }
}

void UPhysicsPerformanceMonitor::SimplifyPhysicsForActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
    if (MeshComp)
    {
        // Keep collision but disable physics simulation
        MeshComp->SetSimulatePhysics(false);
        MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        
        // Add tag to track simplified state
        Actor->Tags.AddUnique(FName("PhysicsSimplified"));
    }
}

FString UPhysicsPerformanceMonitor::GetPerformanceReport()
{
    FString Report;
    
    Report += FString::Printf(TEXT("=== Physics Performance Report ===\n"));
    Report += FString::Printf(TEXT("Frame Time: %.2f ms (Target: %.2f ms)\n"), CurrentFrameTime, TargetFrameTime);
    Report += FString::Printf(TEXT("Performance Level: %s\n"), 
                             PerformanceLevel == ECore_PhysicsPerformanceLevel::High ? TEXT("High") :
                             PerformanceLevel == ECore_PhysicsPerformanceLevel::Medium ? TEXT("Medium") : TEXT("Low"));
    Report += FString::Printf(TEXT("Total Physics Objects: %d (Max: %d)\n"), PhysicsObjectCount, MaxPhysicsObjects);
    Report += FString::Printf(TEXT("LOD Distribution:\n"));
    Report += FString::Printf(TEXT("  Near (High): %d\n"), NearObjectCount);
    Report += FString::Printf(TEXT("  Medium: %d\n"), MediumObjectCount);
    Report += FString::Printf(TEXT("  Far (Low): %d\n"), FarObjectCount);
    Report += FString::Printf(TEXT("  Disabled: %d\n"), DisabledObjectCount);
    Report += FString::Printf(TEXT("Optimization: %s\n"), bPerformanceOptimizationEnabled ? TEXT("Enabled") : TEXT("Disabled"));
    Report += FString::Printf(TEXT("LOD System: %s\n"), bLODSystemEnabled ? TEXT("Enabled") : TEXT("Disabled"));
    
    return Report;
}

void UPhysicsPerformanceMonitor::SavePerformanceData(const FString& FilePath)
{
    FString DataString;
    DataString += TEXT("Timestamp,FrameTime,PhysicsObjects\n");
    
    for (const FCore_PhysicsPerformanceData& Data : PerformanceData)
    {
        DataString += FString::Printf(TEXT("%s,%.2f,%d\n"), 
                                     *Data.Timestamp.ToString(), 
                                     Data.FrameTime, 
                                     Data.PhysicsObjectCount);
    }
    
    FFileHelper::SaveStringToFile(DataString, *FilePath);
    
    UE_LOG(LogPhysicsPerformance, Log, TEXT("Performance data saved to: %s"), *FilePath);
}