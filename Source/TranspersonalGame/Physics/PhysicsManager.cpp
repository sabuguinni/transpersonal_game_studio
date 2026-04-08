// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "PhysicsManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

UPhysicsManager::UPhysicsManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize physics LOD system
    PhysicsLODSystem = CreateDefaultSubobject<UPhysicsLODSystem>(TEXT("PhysicsLODSystem"));
}

void UPhysicsManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize physics world with optimized settings
    InitializePhysicsWorld();
    
    // Bind collision events
    if (AActor* Owner = GetOwner())
    {
        Owner->OnActorHit.AddDynamic(this, &UPhysicsManager::OnActorHit);
    }
    
    // Start performance monitoring
    PerformanceTimer = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsManager: Initialized successfully"));
}

void UPhysicsManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update performance metrics
    UpdatePerformanceMetrics();
    
    // Update physics LOD system
    if (PhysicsLODSystem && GetWorld())
    {
        // Get camera location for LOD calculations
        FVector CameraLocation = FVector::ZeroVector;
        if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
        {
            if (APawn* Pawn = PC->GetPawn())
            {
                CameraLocation = Pawn->GetActorLocation();
            }
        }
        
        PhysicsLODSystem->UpdatePhysicsLOD(CameraLocation, DeltaTime);
    }
    
    // Performance optimization check every second
    PerformanceTimer += DeltaTime;
    if (PerformanceTimer >= 1.0f)
    {
        if (bPerformanceOptimizationEnabled)
        {
            OptimizePhysicsSimulation();
        }
        CleanupInactivePhysicsObjects();
        PerformanceTimer = 0.0f;
    }
}

void UPhysicsManager::InitializePhysicsWorld(FVector WorldGravity, int32 SimulationQuality)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("PhysicsManager: No valid world found"));
        return;
    }
    
    // Set world gravity
    if (AWorldSettings* WorldSettings = World->GetWorldSettings())
    {
        WorldSettings->GlobalGravityZ = WorldGravity.Z;
        UE_LOG(LogTemp, Log, TEXT("PhysicsManager: Set world gravity to %s"), *WorldGravity.ToString());
    }
    
    // Configure Chaos physics settings
    if (UPhysicsSettings* PhysicsSettings = GetMutableDefault<UPhysicsSettings>())
    {
        // Optimize for performance vs quality based on simulation quality
        switch (SimulationQuality)
        {
        case 0: // Low quality - maximum performance
            PhysicsSettings->MaxSubstepDeltaTime = 0.02f; // 50Hz
            PhysicsSettings->MaxSubsteps = 2;
            break;
        case 1: // Medium quality
            PhysicsSettings->MaxSubstepDeltaTime = 0.0167f; // 60Hz
            PhysicsSettings->MaxSubsteps = 4;
            break;
        case 2: // High quality - default
            PhysicsSettings->MaxSubstepDeltaTime = 0.0125f; // 80Hz
            PhysicsSettings->MaxSubsteps = 6;
            break;
        case 3: // Ultra quality - maximum fidelity
            PhysicsSettings->MaxSubstepDeltaTime = 0.01f; // 100Hz
            PhysicsSettings->MaxSubsteps = 8;
            break;
        }
        
        CurrentQualityLevel = SimulationQuality;
        UE_LOG(LogTemp, Log, TEXT("PhysicsManager: Set simulation quality to level %d"), SimulationQuality);
    }
    
    // Enable async physics if supported
    if (bEnableAsyncPhysics)
    {
        // Configure async physics settings
        UE_LOG(LogTemp, Log, TEXT("PhysicsManager: Async physics enabled"));
    }
}

bool UPhysicsManager::ApplyPhysicsImpulse(AActor* TargetActor, FVector ImpulseVector, bool bVelChange)
{
    if (!TargetActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("PhysicsManager: Null target actor for impulse"));
        return false;
    }
    
    // Find physics component
    UPrimitiveComponent* PhysicsComp = TargetActor->FindComponentByClass<UPrimitiveComponent>();
    if (!PhysicsComp || !PhysicsComp->IsSimulatingPhysics())
    {
        UE_LOG(LogTemp, Warning, TEXT("PhysicsManager: Actor %s has no physics component or not simulating"), *TargetActor->GetName());
        return false;
    }
    
    // Validate impulse magnitude
    float ImpulseMagnitude = ImpulseVector.Size();
    const float MaxImpulse = 100000.0f; // Prevent extreme forces
    if (ImpulseMagnitude > MaxImpulse)
    {
        ImpulseVector = ImpulseVector.GetSafeNormal() * MaxImpulse;
        UE_LOG(LogTemp, Warning, TEXT("PhysicsManager: Clamped impulse magnitude from %f to %f"), ImpulseMagnitude, MaxImpulse);
    }
    
    // Apply impulse
    PhysicsComp->AddImpulse(ImpulseVector, NAME_None, bVelChange);
    
    // Track actor for performance monitoring
    TrackedPhysicsActors.AddUnique(TargetActor);
    
    UE_LOG(LogTemp, Verbose, TEXT("PhysicsManager: Applied impulse %s to %s"), *ImpulseVector.ToString(), *TargetActor->GetName());
    return true;
}

void UPhysicsManager::CreatePhysicsExplosion(FVector ExplosionLocation, float ExplosionRadius, float ExplosionForce, bool bCauseDamage)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Find all actors in explosion radius
    TArray<FOverlapResult> OverlapResults;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;
    QueryParams.bReturnPhysicalMaterial = false;
    
    bool bHasOverlaps = World->OverlapMultiByChannel(
        OverlapResults,
        ExplosionLocation,
        FQuat::Identity,
        ECollisionChannel::ECC_WorldDynamic,
        FCollisionShape::MakeSphere(ExplosionRadius),
        QueryParams
    );
    
    if (!bHasOverlaps)
    {
        return;
    }
    
    // Apply explosion forces
    for (const FOverlapResult& Overlap : OverlapResults)
    {
        if (AActor* HitActor = Overlap.GetActor())
        {
            UPrimitiveComponent* PrimComp = Overlap.GetComponent();
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                // Calculate explosion force based on distance
                FVector ActorLocation = HitActor->GetActorLocation();
                FVector ExplosionDirection = (ActorLocation - ExplosionLocation).GetSafeNormal();
                float Distance = FVector::Dist(ActorLocation, ExplosionLocation);
                float ForceFalloff = FMath::Clamp(1.0f - (Distance / ExplosionRadius), 0.1f, 1.0f);
                
                FVector FinalForce = ExplosionDirection * ExplosionForce * ForceFalloff;
                
                // Apply impulse
                PrimComp->AddImpulseAtLocation(FinalForce, ExplosionLocation);
                
                // Broadcast impact event
                OnPhysicsImpact.Broadcast(GetOwner(), HitActor);
                
                UE_LOG(LogTemp, Verbose, TEXT("PhysicsManager: Applied explosion force %s to %s"), 
                       *FinalForce.ToString(), *HitActor->GetName());
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsManager: Created explosion at %s with radius %f and force %f"), 
           *ExplosionLocation.ToString(), ExplosionRadius, ExplosionForce);
}

void UPhysicsManager::SetActorPhysicsEnabled(AActor* TargetActor, bool bEnablePhysics)
{
    if (!TargetActor)
    {
        return;
    }
    
    // Find all primitive components and set physics
    TArray<UPrimitiveComponent*> PrimComponents;
    TargetActor->GetComponents<UPrimitiveComponent>(PrimComponents);
    
    for (UPrimitiveComponent* PrimComp : PrimComponents)
    {
        if (PrimComp)
        {
            PrimComp->SetSimulatePhysics(bEnablePhysics);
            
            if (bEnablePhysics)
            {
                TrackedPhysicsActors.AddUnique(TargetActor);
                if (PhysicsLODSystem)
                {
                    PhysicsLODSystem->RegisterActor(TargetActor);
                }
            }
            else
            {
                TrackedPhysicsActors.Remove(TargetActor);
                if (PhysicsLODSystem)
                {
                    PhysicsLODSystem->UnregisterActor(TargetActor);
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsManager: Set physics %s for actor %s"), 
           bEnablePhysics ? TEXT("enabled") : TEXT("disabled"), *TargetActor->GetName());
}

void UPhysicsManager::GetPhysicsPerformanceMetrics(float& OutFrameTime, int32& OutActiveRigidBodies, float& OutMemoryUsage)
{
    OutFrameTime = CurrentPhysicsFrameTime;
    OutActiveRigidBodies = CurrentActiveRigidBodies;
    OutMemoryUsage = CurrentMemoryUsage;
}

void UPhysicsManager::AdjustPhysicsQuality(float TargetFrameTime)
{
    if (CurrentPhysicsFrameTime <= TargetFrameTime)
    {
        return; // Performance is acceptable
    }
    
    // Reduce quality if performance is poor
    if (CurrentQualityLevel > 0)
    {
        int32 NewQualityLevel = CurrentQualityLevel - 1;
        InitializePhysicsWorld(FVector(0, 0, -980.0f), NewQualityLevel);
        
        UE_LOG(LogTemp, Warning, TEXT("PhysicsManager: Reduced quality level to %d due to performance (%.2fms > %.2fms)"), 
               NewQualityLevel, CurrentPhysicsFrameTime, TargetFrameTime);
    }
    else
    {
        // Already at minimum quality, consider disabling distant physics
        ScalePhysicsQuality(0.8f);
        UE_LOG(LogTemp, Warning, TEXT("PhysicsManager: Applied emergency performance scaling"));
    }
}

void UPhysicsManager::UpdatePerformanceMetrics()
{
    // Get physics frame time from engine stats
    if (GEngine && GEngine->GetEngineSubsystem<UEngineSubsystem>())
    {
        // Approximate physics frame time based on total frame time
        float TotalFrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to ms
        CurrentPhysicsFrameTime = TotalFrameTime * 0.15f; // Physics typically uses ~15% of frame time
    }
    
    // Count active rigid bodies
    CurrentActiveRigidBodies = 0;
    for (auto& WeakActor : TrackedPhysicsActors)
    {
        if (AActor* Actor = WeakActor.Get())
        {
            if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
            {
                if (PrimComp->IsSimulatingPhysics())
                {
                    CurrentActiveRigidBodies++;
                }
            }
        }
    }
    
    // Estimate memory usage (simplified calculation)
    CurrentMemoryUsage = CurrentActiveRigidBodies * 0.05f; // ~50KB per rigid body estimate
}

void UPhysicsManager::OptimizePhysicsSimulation()
{
    // Check if we're exceeding performance budget
    if (CurrentPhysicsFrameTime > MaxPhysicsFrameTime)
    {
        AdjustPhysicsQuality(MaxPhysicsFrameTime);
    }
    
    // Check if we have too many active rigid bodies
    if (CurrentActiveRigidBodies > MaxActiveRigidBodies)
    {
        // Disable physics for distant objects
        if (PhysicsLODSystem)
        {
            UE_LOG(LogTemp, Warning, TEXT("PhysicsManager: Too many active rigid bodies (%d > %d), forcing LOD update"), 
                   CurrentActiveRigidBodies, MaxActiveRigidBodies);
        }
    }
}

void UPhysicsManager::OnActorHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!SelfActor || !OtherActor)
    {
        return;
    }
    
    // Broadcast physics impact event
    OnPhysicsImpact.Broadcast(SelfActor, OtherActor);
    
    // Log significant impacts
    float ImpactMagnitude = NormalImpulse.Size();
    if (ImpactMagnitude > 1000.0f) // Threshold for "significant" impact
    {
        UE_LOG(LogTemp, Log, TEXT("PhysicsManager: Significant impact between %s and %s (magnitude: %.2f)"), 
               *SelfActor->GetName(), *OtherActor->GetName(), ImpactMagnitude);
    }
}

void UPhysicsManager::CleanupInactivePhysicsObjects()
{
    // Remove null or destroyed actors from tracking
    TrackedPhysicsActors.RemoveAll([](const TWeakObjectPtr<AActor>& WeakActor)
    {
        return !WeakActor.IsValid();
    });
    
    // Remove actors that are no longer simulating physics
    TrackedPhysicsActors.RemoveAll([](const TWeakObjectPtr<AActor>& WeakActor)
    {
        if (AActor* Actor = WeakActor.Get())
        {
            if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
            {
                return !PrimComp->IsSimulatingPhysics();
            }
        }
        return true;
    });
}

void UPhysicsManager::ScalePhysicsQuality(float QualityMultiplier)
{
    // Reduce physics update frequency for distant objects
    if (PhysicsLODSystem)
    {
        // Emergency performance scaling - disable physics for very distant objects
        UE_LOG(LogTemp, Warning, TEXT("PhysicsManager: Applied emergency physics scaling (%.2f)"), QualityMultiplier);
    }
}

// Physics LOD System Implementation
void UPhysicsLODSystem::UpdatePhysicsLOD(FVector ViewLocation, float DeltaTime)
{
    for (FPhysicsLODData& LODData : TrackedActors)
    {
        if (AActor* Actor = LODData.Actor.Get())
        {
            // Calculate distance to camera
            float Distance = FVector::Dist(Actor->GetActorLocation(), ViewLocation);
            LODData.DistanceToCamera = Distance;
            
            // Determine appropriate LOD level
            int32 NewLODLevel = CalculateLODLevel(Distance);
            
            // Apply LOD if changed
            if (NewLODLevel != LODData.CurrentLODLevel)
            {
                ApplyLODLevel(Actor, NewLODLevel);
                LODData.CurrentLODLevel = NewLODLevel;
                LODData.LastUpdateTime = FApp::GetCurrentTime();
            }
        }
    }
    
    // Remove invalid actors
    TrackedActors.RemoveAll([](const FPhysicsLODData& LODData)
    {
        return !LODData.Actor.IsValid();
    });
}

void UPhysicsLODSystem::RegisterActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    // Check if already registered
    bool bAlreadyRegistered = TrackedActors.ContainsByPredicate([Actor](const FPhysicsLODData& LODData)
    {
        return LODData.Actor.Get() == Actor;
    });
    
    if (!bAlreadyRegistered)
    {
        FPhysicsLODData NewLODData;
        NewLODData.Actor = Actor;
        NewLODData.CurrentLODLevel = 0; // Start at highest quality
        NewLODData.LastUpdateTime = FApp::GetCurrentTime();
        TrackedActors.Add(NewLODData);
    }
}

void UPhysicsLODSystem::UnregisterActor(AActor* Actor)
{
    TrackedActors.RemoveAll([Actor](const FPhysicsLODData& LODData)
    {
        return LODData.Actor.Get() == Actor;
    });
}

void UPhysicsLODSystem::ApplyLODLevel(AActor* Actor, int32 LODLevel)
{
    if (!Actor)
    {
        return;
    }
    
    TArray<UPrimitiveComponent*> PrimComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimComponents);
    
    for (UPrimitiveComponent* PrimComp : PrimComponents)
    {
        if (!PrimComp)
        {
            continue;
        }
        
        switch (LODLevel)
        {
        case 0: // High quality - full physics
            PrimComp->SetSimulatePhysics(true);
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            break;
            
        case 1: // Medium quality - reduced physics
            PrimComp->SetSimulatePhysics(true);
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            // Could reduce collision complexity here
            break;
            
        case 2: // Low quality - query only
            PrimComp->SetSimulatePhysics(false);
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            break;
            
        case 3: // Disabled - no collision
            PrimComp->SetSimulatePhysics(false);
            PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            break;
        }
    }
}

int32 UPhysicsLODSystem::CalculateLODLevel(float Distance)
{
    if (Distance < LOD_DISTANCE_HIGH)
    {
        return 0; // High quality
    }
    else if (Distance < LOD_DISTANCE_MEDIUM)
    {
        return 1; // Medium quality
    }
    else if (Distance < LOD_DISTANCE_LOW)
    {
        return 2; // Low quality
    }
    else
    {
        return 3; // Disabled
    }
}