// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "PhysicsPerformanceOptimizer.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Physics/PhysicsFiltering.h"
#include "Physics/Experimental/PhysScene_Chaos.h"
#include "Chaos/ChaosEngineInterface.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Stats/StatsData.h"

DEFINE_LOG_CATEGORY_STATIC(LogPhysicsPerformanceOptimizer, Log, All);

UPhysicsPerformanceOptimizer::UPhysicsPerformanceOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update every 100ms
    
    // Initialize physics LOD distances (meters)
    PhysicsLODDistances.Add(EPhysicsLODLevel::LOD0_Full, 50.0f);
    PhysicsLODDistances.Add(EPhysicsLODLevel::LOD1_Reduced, 150.0f);
    PhysicsLODDistances.Add(EPhysicsLODLevel::LOD2_Basic, 300.0f);
    PhysicsLODDistances.Add(EPhysicsLODLevel::LOD3_Kinematic, 500.0f);
    
    // Platform-specific settings
#if PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
    MaxConcurrentRagdolls = 16;
    MaxDestructionDebris = 1000;
    PhysicsPerformanceBudgetMS = 8.0f; // 8ms of 16.67ms frame budget for PC
#else
    MaxConcurrentRagdolls = 8;
    MaxDestructionDebris = 500;
    PhysicsPerformanceBudgetMS = 5.0f; // 5ms of 33.33ms frame budget for console
#endif

    // Initialize physics object type toggles
    PhysicsObjectTypeEnabled.Add(EPhysicsObjectType::CreatureRagdolls, true);
    PhysicsObjectTypeEnabled.Add(EPhysicsObjectType::DestructionDebris, true);
    PhysicsObjectTypeEnabled.Add(EPhysicsObjectType::EnvironmentalProps, true);
    PhysicsObjectTypeEnabled.Add(EPhysicsObjectType::ProjectilePhysics, true);
    PhysicsObjectTypeEnabled.Add(EPhysicsObjectType::VehiclePhysics, true);
    
    // Initialize adaptive physics settings
    MinPhysicsSubsteps = 2;
    MaxPhysicsSubsteps = 8;
    CurrentPhysicsSubsteps = 6;
    
    UE_LOG(LogPhysicsPerformanceOptimizer, Log, TEXT("PhysicsPerformanceOptimizer initialized"));
}

void UPhysicsPerformanceOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    InitializePhysicsOptimization();
    
    // Start performance monitoring timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            PerformanceUpdateTimer,
            this,
            &UPhysicsPerformanceOptimizer::UpdatePhysicsPerformanceMetrics,
            1.0f, // Update every second
            true
        );
    }
    
    UE_LOG(LogPhysicsPerformanceOptimizer, Log, TEXT("PhysicsPerformanceOptimizer started"));
}

void UPhysicsPerformanceOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update physics LOD system
    UpdatePhysicsLOD();
    
    // Adaptive substep scaling
    AdaptiveSubstepScaling();
    
    // Manage ragdoll performance
    ManageRagdollPerformance(MaxConcurrentRagdolls);
    
    // Optimize destruction system
    OptimizeDestructionSystem(MaxDestructionDebris, DestructionDebrisLifetime);
}

void UPhysicsPerformanceOptimizer::InitializePhysicsOptimization()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogPhysicsPerformanceOptimizer, Warning, TEXT("Cannot initialize physics optimization without valid world"));
        return;
    }
    
    // Initialize physics scene optimization
    if (FPhysScene* PhysScene = World->GetPhysicsScene())
    {
        // Set up physics scene optimization parameters
        PhysScene->SetKinematicUpdateFunction(FPhysScene::FKinematicUpdateFunction::CreateUObject(
            this, &UPhysicsPerformanceOptimizer::OptimizedKinematicUpdate));
    }
    
    // Cache all physics objects in the world for optimization
    CachePhysicsObjects();
    
    UE_LOG(LogPhysicsPerformanceOptimizer, Log, TEXT("Physics optimization initialized"));
}

void UPhysicsPerformanceOptimizer::OptimizeCollisionDetection()
{
    if (!PhysicsObjectTypeEnabled[EPhysicsObjectType::EnvironmentalProps])
    {
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    APawn* PlayerPawn = World->GetFirstPlayerController() ? World->GetFirstPlayerController()->GetPawn() : nullptr;
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Iterate through all static mesh actors and optimize collision based on distance
    for (TActorIterator<AStaticMeshActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AStaticMeshActor* StaticMeshActor = *ActorItr;
        if (!StaticMeshActor || !StaticMeshActor->GetStaticMeshComponent())
        {
            continue;
        }
        
        UStaticMeshComponent* MeshComp = StaticMeshActor->GetStaticMeshComponent();
        float Distance = FVector::Dist(PlayerLocation, StaticMeshActor->GetActorLocation());
        
        // Apply distance-based collision optimization
        if (Distance <= 50.0f)
        {
            // Full collision for close objects
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            MeshComp->SetCollisionResponseToAllChannels(ECR_Block);
        }
        else if (Distance <= 200.0f)
        {
            // Simplified collision for medium distance
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            MeshComp->SetCollisionResponseToAllChannels(ECR_Block);
        }
        else if (Distance <= 500.0f)
        {
            // Basic bounds collision for far objects
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            MeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);
            MeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
        }
        else
        {
            // No collision for very distant objects
            MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
    }
}

void UPhysicsPerformanceOptimizer::ManageRagdollPerformance(int32 MaxConcurrentRagdolls)
{
    if (!PhysicsObjectTypeEnabled[EPhysicsObjectType::CreatureRagdolls])
    {
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    TArray<ACharacter*> RagdollCharacters;
    APawn* PlayerPawn = World->GetFirstPlayerController() ? World->GetFirstPlayerController()->GetPawn() : nullptr;
    FVector PlayerLocation = PlayerPawn ? PlayerPawn->GetActorLocation() : FVector::ZeroVector;
    
    // Find all characters with ragdoll physics enabled
    for (TActorIterator<ACharacter> ActorItr(World); ActorItr; ++ActorItr)
    {
        ACharacter* Character = *ActorItr;
        if (!Character || !Character->GetMesh())
        {
            continue;
        }
        
        USkeletalMeshComponent* MeshComp = Character->GetMesh();
        if (MeshComp->IsSimulatingPhysics())
        {
            RagdollCharacters.Add(Character);
        }
    }
    
    // Sort by distance from player
    RagdollCharacters.Sort([PlayerLocation](const ACharacter& A, const ACharacter& B) {
        float DistA = FVector::Dist(PlayerLocation, A.GetActorLocation());
        float DistB = FVector::Dist(PlayerLocation, B.GetActorLocation());
        return DistA < DistB;
    });
    
    // Disable ragdoll physics for excess characters (keep closest ones)
    for (int32 i = MaxConcurrentRagdolls; i < RagdollCharacters.Num(); ++i)
    {
        if (ACharacter* Character = RagdollCharacters[i])
        {
            if (USkeletalMeshComponent* MeshComp = Character->GetMesh())
            {
                MeshComp->SetSimulatePhysics(false);
                MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                
                // Schedule for cleanup if too far away
                float Distance = FVector::Dist(PlayerLocation, Character->GetActorLocation());
                if (Distance > 1000.0f)
                {
                    Character->Destroy();
                }
            }
        }
    }
    
    // Optimize remaining ragdolls based on distance
    for (int32 i = 0; i < FMath::Min(MaxConcurrentRagdolls, RagdollCharacters.Num()); ++i)
    {
        if (ACharacter* Character = RagdollCharacters[i])
        {
            float Distance = FVector::Dist(PlayerLocation, Character->GetActorLocation());
            if (USkeletalMeshComponent* MeshComp = Character->GetMesh())
            {
                if (Distance > 200.0f)
                {
                    // Simplified ragdoll physics for distant characters
                    MeshComp->SetAllBodiesBelowSimulatePhysics(FName("Spine"), false);
                }
            }
        }
    }
}

void UPhysicsPerformanceOptimizer::OptimizeDestructionSystem(int32 MaxDebrisCount, float DebrisLifetime)
{
    if (!PhysicsObjectTypeEnabled[EPhysicsObjectType::DestructionDebris])
    {
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    TArray<UGeometryCollectionComponent*> DestructionComponents;
    APawn* PlayerPawn = World->GetFirstPlayerController() ? World->GetFirstPlayerController()->GetPawn() : nullptr;
    FVector PlayerLocation = PlayerPawn ? PlayerPawn->GetActorLocation() : FVector::ZeroVector;
    
    // Find all geometry collection components (destruction debris)
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor)
        {
            continue;
        }
        
        if (UGeometryCollectionComponent* GeoComp = Actor->FindComponentByClass<UGeometryCollectionComponent>())
        {
            if (GeoComp->GetPhysicsObjectsNum() > 0)
            {
                DestructionComponents.Add(GeoComp);
            }
        }
    }
    
    // Sort by distance from player
    DestructionComponents.Sort([PlayerLocation](const UGeometryCollectionComponent& A, const UGeometryCollectionComponent& B) {
        float DistA = FVector::Dist(PlayerLocation, A.GetOwner()->GetActorLocation());
        float DistB = FVector::Dist(PlayerLocation, B.GetOwner()->GetActorLocation());
        return DistA < DistB;
    });
    
    // Clean up excess debris
    int32 CurrentDebrisCount = 0;
    for (UGeometryCollectionComponent* GeoComp : DestructionComponents)
    {
        CurrentDebrisCount += GeoComp->GetPhysicsObjectsNum();
        
        if (CurrentDebrisCount > MaxDebrisCount)
        {
            // Disable physics for excess debris
            GeoComp->SetSimulatePhysics(false);
            
            // Schedule for destruction
            if (AActor* Owner = GeoComp->GetOwner())
            {
                float Distance = FVector::Dist(PlayerLocation, Owner->GetActorLocation());
                if (Distance > 500.0f)
                {
                    Owner->Destroy();
                }
            }
        }
        else
        {
            // Optimize remaining debris based on distance
            float Distance = FVector::Dist(PlayerLocation, GeoComp->GetOwner()->GetActorLocation());
            if (Distance > 300.0f)
            {
                // Simplified physics for distant debris
                GeoComp->SetNotifyBreaks(false);
            }
        }
    }
}

void UPhysicsPerformanceOptimizer::UpdatePhysicsLOD()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    APawn* PlayerPawn = World->GetFirstPlayerController() ? World->GetFirstPlayerController()->GetPawn() : nullptr;
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Update physics LOD for all physics objects
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor)
        {
            continue;
        }
        
        float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
        EPhysicsLODLevel LODLevel = DeterminePhysicsLODLevel(Distance);
        
        ApplyPhysicsLOD(Actor, LODLevel);
    }
}

UPhysicsPerformanceOptimizer::EPhysicsLODLevel UPhysicsPerformanceOptimizer::DeterminePhysicsLODLevel(float Distance)
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

void UPhysicsPerformanceOptimizer::ApplyPhysicsLOD(AActor* Actor, EPhysicsLODLevel LODLevel)
{
    if (!Actor)
    {
        return;
    }
    
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
    {
        if (!PrimComp)
        {
            continue;
        }
        
        switch (LODLevel)
        {
            case EPhysicsLODLevel::LOD0_Full:
                // Full physics simulation
                PrimComp->SetSimulatePhysics(true);
                PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                break;
                
            case EPhysicsLODLevel::LOD1_Reduced:
                // Reduced physics simulation
                PrimComp->SetSimulatePhysics(true);
                PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                // Reduce physics substeps for this object
                break;
                
            case EPhysicsLODLevel::LOD2_Basic:
                // Basic physics, no destruction
                PrimComp->SetSimulatePhysics(true);
                PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
                break;
                
            case EPhysicsLODLevel::LOD3_Kinematic:
                // Kinematic only, no physics simulation
                PrimComp->SetSimulatePhysics(false);
                PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
                break;
        }
    }
}

void UPhysicsPerformanceOptimizer::AdaptiveSubstepScaling(float TargetFrameTimeMS)
{
    // Get current frame time
    float CurrentFrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
    
    // Adjust physics substeps based on performance
    if (CurrentFrameTime > TargetFrameTimeMS * 1.2f)
    {
        // Performance is poor, reduce substeps
        CurrentPhysicsSubsteps = FMath::Max(MinPhysicsSubsteps, CurrentPhysicsSubsteps - 1);
    }
    else if (CurrentFrameTime < TargetFrameTimeMS * 0.8f)
    {
        // Performance is good, can increase substeps for better quality
        CurrentPhysicsSubsteps = FMath::Min(MaxPhysicsSubsteps, CurrentPhysicsSubsteps + 1);
    }
    
    // Apply substep count to physics world
    if (UWorld* World = GetWorld())
    {
        if (FPhysScene* PhysScene = World->GetPhysicsScene())
        {
            // Set physics substep count (this is a simplified approach)
            // In practice, this would involve more complex physics world settings
        }
    }
}

UPhysicsPerformanceOptimizer::FPhysicsPerformanceMetrics UPhysicsPerformanceOptimizer::GetPhysicsPerformanceMetrics() const
{
    return CurrentPerformanceMetrics;
}

void UPhysicsPerformanceOptimizer::CleanupDistantPhysicsObjects(float DistanceThreshold)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    APawn* PlayerPawn = World->GetFirstPlayerController() ? World->GetFirstPlayerController()->GetPawn() : nullptr;
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Clean up distant physics objects
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor)
        {
            continue;
        }
        
        float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
        if (Distance > DistanceThreshold)
        {
            // Check if actor has physics components
            TArray<UPrimitiveComponent*> PrimitiveComponents;
            Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
            
            bool bHasPhysics = false;
            for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
            {
                if (PrimComp && PrimComp->IsSimulatingPhysics())
                {
                    bHasPhysics = true;
                    break;
                }
            }
            
            if (bHasPhysics)
            {
                // Disable physics for distant objects
                for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
                {
                    if (PrimComp)
                    {
                        PrimComp->SetSimulatePhysics(false);
                        PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                    }
                }
                
                // Optionally destroy the actor if it's debris or temporary
                if (Actor->GetClass()->GetName().Contains(TEXT("Debris")) ||
                    Actor->GetClass()->GetName().Contains(TEXT("Destruction")))
                {
                    Actor->Destroy();
                }
            }
        }
    }
}

void UPhysicsPerformanceOptimizer::TogglePhysicsObjectType(EPhysicsObjectType ObjectType, bool bEnabled)
{
    PhysicsObjectTypeEnabled[ObjectType] = bEnabled;
    
    UE_LOG(LogPhysicsPerformanceOptimizer, Log, TEXT("Physics object type %d set to %s"), 
           static_cast<int32>(ObjectType), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UPhysicsPerformanceOptimizer::UpdatePhysicsPerformanceMetrics()
{
    // Update physics performance metrics
    CurrentPerformanceMetrics.PhysicsSimulationTimeMS = GetPhysicsSimulationTime();
    CurrentPerformanceMetrics.ActivePhysicsBodies = CountActivePhysicsBodies();
    CurrentPerformanceMetrics.ActiveRagdolls = CountActiveRagdolls();
    CurrentPerformanceMetrics.DestructionDebrisCount = CountDestructionDebris();
    CurrentPerformanceMetrics.CurrentSubsteps = CurrentPhysicsSubsteps;
    CurrentPerformanceMetrics.CollisionChecksPerFrame = GetCollisionChecksPerFrame();
    CurrentPerformanceMetrics.PhysicsMemoryUsageMB = GetPhysicsMemoryUsage();
    CurrentPerformanceMetrics.PhysicsCPUUsagePercent = GetPhysicsCPUUsage();
    CurrentPerformanceMetrics.bWithinPerformanceBudget = 
        CurrentPerformanceMetrics.PhysicsSimulationTimeMS <= PhysicsPerformanceBudgetMS;
}

float UPhysicsPerformanceOptimizer::GetPhysicsSimulationTime() const
{
    // Simplified physics simulation time calculation
    // In practice, this would query the actual physics world timing
    return FApp::GetDeltaTime() * 1000.0f * 0.3f; // Assume physics takes 30% of frame time
}

int32 UPhysicsPerformanceOptimizer::CountActivePhysicsBodies() const
{
    int32 Count = 0;
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            if (AActor* Actor = *ActorItr)
            {
                TArray<UPrimitiveComponent*> PrimitiveComponents;
                Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
                
                for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
                {
                    if (PrimComp && PrimComp->IsSimulatingPhysics())
                    {
                        Count++;
                    }
                }
            }
        }
    }
    return Count;
}

int32 UPhysicsPerformanceOptimizer::CountActiveRagdolls() const
{
    int32 Count = 0;
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<ACharacter> ActorItr(World); ActorItr; ++ActorItr)
        {
            if (ACharacter* Character = *ActorItr)
            {
                if (USkeletalMeshComponent* MeshComp = Character->GetMesh())
                {
                    if (MeshComp->IsSimulatingPhysics())
                    {
                        Count++;
                    }
                }
            }
        }
    }
    return Count;
}

int32 UPhysicsPerformanceOptimizer::CountDestructionDebris() const
{
    int32 Count = 0;
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            if (AActor* Actor = *ActorItr)
            {
                if (UGeometryCollectionComponent* GeoComp = Actor->FindComponentByClass<UGeometryCollectionComponent>())
                {
                    Count += GeoComp->GetPhysicsObjectsNum();
                }
            }
        }
    }
    return Count;
}

int32 UPhysicsPerformanceOptimizer::GetCollisionChecksPerFrame() const
{
    // Simplified collision check count
    // In practice, this would query the physics world collision statistics
    return CountActivePhysicsBodies() * 10; // Estimate 10 collision checks per body per frame
}

float UPhysicsPerformanceOptimizer::GetPhysicsMemoryUsage() const
{
    // Simplified physics memory usage calculation
    // In practice, this would query the actual physics world memory usage
    return CountActivePhysicsBodies() * 0.1f; // Estimate 0.1MB per physics body
}

float UPhysicsPerformanceOptimizer::GetPhysicsCPUUsage() const
{
    // Simplified physics CPU usage calculation
    // In practice, this would use actual profiling data
    return (CurrentPerformanceMetrics.PhysicsSimulationTimeMS / 16.67f) * 100.0f; // Percentage of 60fps frame time
}

void UPhysicsPerformanceOptimizer::CachePhysicsObjects()
{
    // Cache physics objects for faster access during optimization
    CachedPhysicsObjects.Empty();
    
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            if (AActor* Actor = *ActorItr)
            {
                TArray<UPrimitiveComponent*> PrimitiveComponents;
                Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
                
                for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
                {
                    if (PrimComp && (PrimComp->IsSimulatingPhysics() || PrimComp->IsCollisionEnabled()))
                    {
                        CachedPhysicsObjects.Add(PrimComp);
                    }
                }
            }
        }
    }
    
    UE_LOG(LogPhysicsPerformanceOptimizer, Log, TEXT("Cached %d physics objects"), CachedPhysicsObjects.Num());
}

void UPhysicsPerformanceOptimizer::OptimizedKinematicUpdate(float DeltaTime)
{
    // Optimized kinematic update function for physics scene
    // This would contain custom physics update logic for better performance
}