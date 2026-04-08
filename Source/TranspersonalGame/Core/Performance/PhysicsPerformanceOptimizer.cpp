// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "PhysicsPerformanceOptimizer.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"
#include "HAL/IConsoleManager.h"
#include "Stats/Stats.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"

DECLARE_STATS_GROUP(TEXT("Physics Performance"), STATGROUP_PhysicsPerformance, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Physics Optimization"), STAT_PhysicsOptimization, STATGROUP_PhysicsPerformance);
DECLARE_CYCLE_STAT(TEXT("Ragdoll Management"), STAT_RagdollManagement, STATGROUP_PhysicsPerformance);
DECLARE_CYCLE_STAT(TEXT("Destruction Optimization"), STAT_DestructionOptimization, STATGROUP_PhysicsPerformance);
DECLARE_CYCLE_STAT(TEXT("Physics LOD Update"), STAT_PhysicsLODUpdate, STATGROUP_PhysicsPerformance);

DECLARE_DWORD_COUNTER_STAT(TEXT("Active Physics Bodies"), STAT_ActivePhysicsBodies, STATGROUP_PhysicsPerformance);
DECLARE_DWORD_COUNTER_STAT(TEXT("Active Ragdolls"), STAT_ActiveRagdolls, STATGROUP_PhysicsPerformance);
DECLARE_DWORD_COUNTER_STAT(TEXT("Destruction Debris"), STAT_DestructionDebris, STATGROUP_PhysicsPerformance);
DECLARE_FLOAT_COUNTER_STAT(TEXT("Physics Time MS"), STAT_PhysicsTimeMS, STATGROUP_PhysicsPerformance);

UPhysicsPerformanceOptimizer::UPhysicsPerformanceOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    PrimaryComponentTick.TickInterval = 0.1f; // Optimize every 100ms
    
    // Initialize default values based on platform
    SetDefaultPerformanceSettings();
}

void UPhysicsPerformanceOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize physics optimization systems
    InitializePhysicsOptimization();
    
    UE_LOG(LogTemp, Log, TEXT("Physics Performance Optimizer initialized for %s"), 
           *GetOwner()->GetName());
}

void UPhysicsPerformanceOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, 
                                               FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    SCOPE_CYCLE_COUNTER(STAT_PhysicsOptimization);
    
    // Update physics LOD based on camera position
    UpdatePhysicsLOD();
    
    // Manage ragdoll performance
    ManageRagdollPerformance(MaxConcurrentRagdolls);
    
    // Optimize destruction system
    OptimizeDestructionSystem(MaxDestructionDebris, DestructionDebrisLifetime);
    
    // Adaptive substep scaling based on performance
    AdaptiveSubstepScaling(16.67f); // Target 60 FPS
    
    // Cleanup distant physics objects
    CleanupDistantPhysicsObjects(1000.0f);
    
    // Update performance metrics
    UpdatePerformanceMetrics();
}

void UPhysicsPerformanceOptimizer::InitializePhysicsOptimization()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("PhysicsPerformanceOptimizer: No valid world found"));
        return;
    }
    
    // Set platform-specific performance settings
    SetPlatformSpecificSettings();
    
    // Initialize physics object tracking
    InitializePhysicsObjectTracking();
    
    // Set up performance monitoring
    SetupPerformanceMonitoring();
    
    UE_LOG(LogTemp, Log, TEXT("Physics Performance Optimization initialized successfully"));
}

void UPhysicsPerformanceOptimizer::OptimizeCollisionDetection()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    APawn* PlayerPawn = World->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn) return;
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Iterate through all physics objects and optimize collision based on distance
    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (!Actor || !Actor->GetRootComponent()) continue;
        
        UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
        if (!PrimComp) continue;
        
        float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
        
        // Apply distance-based collision optimization
        if (Distance <= 50.0f)
        {
            // Full collision for close objects
            PrimComp->SetCollisionResponseToAllChannels(ECR_Block);
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        }
        else if (Distance <= 200.0f)
        {
            // Simplified collision for medium distance
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        }
        else if (Distance <= 500.0f)
        {
            // Basic bounds collision for far objects
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            PrimComp->SetCollisionResponseToAllChannels(ECR_Ignore);
            PrimComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
        }
        else
        {
            // No collision for very distant objects
            PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
    }
}

void UPhysicsPerformanceOptimizer::ManageRagdollPerformance(int32 MaxConcurrentRagdolls)
{
    SCOPE_CYCLE_COUNTER(STAT_RagdollManagement);
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    TArray<USkeletalMeshComponent*> ActiveRagdolls;
    
    // Find all active ragdolls
    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (!Actor) continue;
        
        USkeletalMeshComponent* SkeletalMesh = Actor->FindComponentByClass<USkeletalMeshComponent>();
        if (SkeletalMesh && SkeletalMesh->IsSimulatingPhysics())
        {
            ActiveRagdolls.Add(SkeletalMesh);
        }
    }
    
    SET_DWORD_STAT(STAT_ActiveRagdolls, ActiveRagdolls.Num());
    
    // If we have too many ragdolls, disable the furthest ones
    if (ActiveRagdolls.Num() > MaxConcurrentRagdolls)
    {
        APawn* PlayerPawn = World->GetFirstPlayerController()->GetPawn();
        if (PlayerPawn)
        {
            FVector PlayerLocation = PlayerPawn->GetActorLocation();
            
            // Sort ragdolls by distance from player
            ActiveRagdolls.Sort([PlayerLocation](const USkeletalMeshComponent& A, const USkeletalMeshComponent& B)
            {
                float DistA = FVector::DistSquared(PlayerLocation, A.GetComponentLocation());
                float DistB = FVector::DistSquared(PlayerLocation, B.GetComponentLocation());
                return DistA < DistB;
            });
            
            // Disable ragdolls beyond the limit
            for (int32 i = MaxConcurrentRagdolls; i < ActiveRagdolls.Num(); ++i)
            {
                ActiveRagdolls[i]->SetSimulatePhysics(false);
                ActiveRagdolls[i]->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                
                UE_LOG(LogTemp, VeryVerbose, TEXT("Disabled ragdoll for performance: %s"), 
                       *ActiveRagdolls[i]->GetOwner()->GetName());
            }
        }
    }
}

void UPhysicsPerformanceOptimizer::OptimizeDestructionSystem(int32 MaxDebrisCount, float DebrisLifetime)
{
    SCOPE_CYCLE_COUNTER(STAT_DestructionOptimization);
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    TArray<UGeometryCollectionComponent*> DestructionComponents;
    
    // Find all geometry collection components (destruction debris)
    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (!Actor) continue;
        
        UGeometryCollectionComponent* GeomComp = Actor->FindComponentByClass<UGeometryCollectionComponent>();
        if (GeomComp && GeomComp->IsSimulatingPhysics())
        {
            DestructionComponents.Add(GeomComp);
        }
    }
    
    SET_DWORD_STAT(STAT_DestructionDebris, DestructionComponents.Num());
    
    // If we have too much debris, clean up the oldest/furthest
    if (DestructionComponents.Num() > MaxDebrisCount)
    {
        APawn* PlayerPawn = World->GetFirstPlayerController()->GetPawn();
        if (PlayerPawn)
        {
            FVector PlayerLocation = PlayerPawn->GetActorLocation();
            
            // Sort by distance and age
            DestructionComponents.Sort([PlayerLocation](const UGeometryCollectionComponent& A, const UGeometryCollectionComponent& B)
            {
                float DistA = FVector::DistSquared(PlayerLocation, A.GetComponentLocation());
                float DistB = FVector::DistSquared(PlayerLocation, B.GetComponentLocation());
                return DistA > DistB; // Furthest first
            });
            
            // Remove excess debris
            for (int32 i = MaxDebrisCount; i < DestructionComponents.Num(); ++i)
            {
                if (DestructionComponents[i]->GetOwner())
                {
                    DestructionComponents[i]->GetOwner()->Destroy();
                }
            }
        }
    }
    
    // Clean up debris based on lifetime
    CleanupOldDebris(DebrisLifetime);
}

void UPhysicsPerformanceOptimizer::UpdatePhysicsLOD()
{
    SCOPE_CYCLE_COUNTER(STAT_PhysicsLODUpdate);
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    APawn* PlayerPawn = World->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn) return;
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Update physics LOD for all physics objects
    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (!Actor) continue;
        
        UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
        if (!PrimComp || !PrimComp->IsSimulatingPhysics()) continue;
        
        float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
        EPhysicsLODLevel LODLevel = DeterminePhysicsLODLevel(Distance);
        
        ApplyPhysicsLOD(PrimComp, LODLevel);
    }
}

void UPhysicsPerformanceOptimizer::AdaptiveSubstepScaling(float TargetFrameTimeMS)
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Get current frame time
    float CurrentFrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
    
    // Adjust substeps based on performance
    if (CurrentFrameTime > TargetFrameTimeMS * 1.2f) // 20% over target
    {
        // Reduce substeps to improve performance
        CurrentPhysicsSubsteps = FMath::Max(MinPhysicsSubsteps, CurrentPhysicsSubsteps - 1);
    }
    else if (CurrentFrameTime < TargetFrameTimeMS * 0.8f) // 20% under target
    {
        // Increase substeps for better quality
        CurrentPhysicsSubsteps = FMath::Min(MaxPhysicsSubsteps, CurrentPhysicsSubsteps + 1);
    }
    
    // Apply new substep count
    if (UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get())
    {
        PhysicsSettings->MaxSubsteps = CurrentPhysicsSubsteps;
    }
}

UPhysicsPerformanceOptimizer::FPhysicsPerformanceMetrics UPhysicsPerformanceOptimizer::GetPhysicsPerformanceMetrics() const
{
    FPhysicsPerformanceMetrics Metrics;
    
    // Get current performance data
    Metrics.PhysicsSimulationTimeMS = GET_FLOAT_STAT(STAT_PhysicsTimeMS);
    Metrics.ActivePhysicsBodies = GET_DWORD_STAT(STAT_ActivePhysicsBodies);
    Metrics.ActiveRagdolls = GET_DWORD_STAT(STAT_ActiveRagdolls);
    Metrics.DestructionDebrisCount = GET_DWORD_STAT(STAT_DestructionDebris);
    Metrics.CurrentSubsteps = CurrentPhysicsSubsteps;
    Metrics.bWithinPerformanceBudget = Metrics.PhysicsSimulationTimeMS <= PhysicsPerformanceBudgetMS;
    
    // Calculate physics CPU usage percentage
    float FrameTime = FApp::GetDeltaTime() * 1000.0f;
    Metrics.PhysicsCPUUsagePercent = (Metrics.PhysicsSimulationTimeMS / FrameTime) * 100.0f;
    
    return Metrics;
}

void UPhysicsPerformanceOptimizer::CleanupDistantPhysicsObjects(float DistanceThreshold)
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    APawn* PlayerPawn = World->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn) return;
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (!Actor) continue;
        
        float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
        
        if (Distance > DistanceThreshold)
        {
            UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                // Disable physics for very distant objects
                PrimComp->SetSimulatePhysics(false);
                PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            }
        }
    }
}

void UPhysicsPerformanceOptimizer::TogglePhysicsObjectType(EPhysicsObjectType ObjectType, bool bEnabled)
{
    PhysicsObjectTypeEnabled[ObjectType] = bEnabled;
    
    UE_LOG(LogTemp, Log, TEXT("Physics object type %s %s"), 
           *UEnum::GetValueAsString(ObjectType),
           bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UPhysicsPerformanceOptimizer::SetDefaultPerformanceSettings()
{
    // Set platform-specific defaults
    #if PLATFORM_DESKTOP
        MaxConcurrentRagdolls = 16;
        MaxDestructionDebris = 1000;
        PhysicsPerformanceBudgetMS = 8.0f;
    #else
        MaxConcurrentRagdolls = 8;
        MaxDestructionDebris = 500;
        PhysicsPerformanceBudgetMS = 5.0f;
    #endif
}

void UPhysicsPerformanceOptimizer::SetPlatformSpecificSettings()
{
    // Console-specific optimizations
    #if PLATFORM_CONSOLE
        // More aggressive optimization for consoles
        PhysicsLODDistances[EPhysicsLODLevel::LOD0_Full] = 30.0f;
        PhysicsLODDistances[EPhysicsLODLevel::LOD1_Reduced] = 100.0f;
        PhysicsLODDistances[EPhysicsLODLevel::LOD2_Basic] = 200.0f;
        PhysicsLODDistances[EPhysicsLODLevel::LOD3_Kinematic] = 300.0f;
    #endif
}

void UPhysicsPerformanceOptimizer::InitializePhysicsObjectTracking()
{
    // Initialize tracking arrays and maps for physics objects
    // This will be used to efficiently manage physics objects by type
}

void UPhysicsPerformanceOptimizer::SetupPerformanceMonitoring()
{
    // Set up performance monitoring hooks
    // This will track physics performance metrics in real-time
}

UPhysicsPerformanceOptimizer::EPhysicsLODLevel UPhysicsPerformanceOptimizer::DeterminePhysicsLODLevel(float Distance) const
{
    if (Distance <= PhysicsLODDistances[EPhysicsLODLevel::LOD0_Full])
    {
        return EPhysicsLODLevel::LOD0_Full;
    }
    else if (Distance <= PhysicsLODDistances[EPhysicsLODLevel::LOD1_Reduced])
    {
        return EPhysicsLODLevel::LOD1_Reduced;
    }
    else if (Distance <= PhysicsLODDistances[EPhysicsLODLevel::LOD2_Basic])
    {
        return EPhysicsLODLevel::LOD2_Basic;
    }
    else
    {
        return EPhysicsLODLevel::LOD3_Kinematic;
    }
}

void UPhysicsPerformanceOptimizer::ApplyPhysicsLOD(UPrimitiveComponent* Component, EPhysicsLODLevel LODLevel)
{
    if (!Component) return;
    
    switch (LODLevel)
    {
        case EPhysicsLODLevel::LOD0_Full:
            // Full physics simulation
            Component->SetSimulatePhysics(true);
            Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            break;
            
        case EPhysicsLODLevel::LOD1_Reduced:
            // Reduced physics simulation
            Component->SetSimulatePhysics(true);
            Component->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            break;
            
        case EPhysicsLODLevel::LOD2_Basic:
            // Basic physics simulation
            Component->SetSimulatePhysics(false);
            Component->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            break;
            
        case EPhysicsLODLevel::LOD3_Kinematic:
            // Kinematic only
            Component->SetSimulatePhysics(false);
            Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            break;
    }
}

void UPhysicsPerformanceOptimizer::CleanupOldDebris(float MaxLifetime)
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    float CurrentTime = World->GetTimeSeconds();
    
    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (!Actor) continue;
        
        UGeometryCollectionComponent* GeomComp = Actor->FindComponentByClass<UGeometryCollectionComponent>();
        if (GeomComp && GeomComp->IsSimulatingPhysics())
        {
            float ActorAge = CurrentTime - Actor->GetGameTimeSinceCreation();
            if (ActorAge > MaxLifetime)
            {
                Actor->Destroy();
            }
        }
    }
}

void UPhysicsPerformanceOptimizer::UpdatePerformanceMetrics()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Count active physics bodies
    int32 ActiveBodies = 0;
    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (!Actor) continue;
        
        UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
        if (PrimComp && PrimComp->IsSimulatingPhysics())
        {
            ActiveBodies++;
        }
    }
    
    SET_DWORD_STAT(STAT_ActivePhysicsBodies, ActiveBodies);
    
    // Update physics time
    float PhysicsTime = FApp::GetDeltaTime() * 1000.0f * 0.3f; // Estimate 30% of frame time for physics
    SET_FLOAT_STAT(STAT_PhysicsTimeMS, PhysicsTime);
}