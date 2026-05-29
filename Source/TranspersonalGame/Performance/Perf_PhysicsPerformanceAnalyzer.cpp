#include "Perf_PhysicsPerformanceAnalyzer.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameViewportClient.h"
#include "HAL/PlatformFilemanager.h"

UPerf_PhysicsPerformanceAnalyzer::UPerf_PhysicsPerformanceAnalyzer()
    : bAnalysisActive(false)
    , LastFramePhysicsTime(0.0f)
    , AveragePhysicsTime(0.0f)
    , FramesSampled(0)
    , PhysicsAnalysisTimer(0.0f)
    , OptimizationUpdateTimer(0.0f)
    , CurrentOptimizationLevel(EPerf_PhysicsOptimizationLevel::Medium)
    , bPhysicsLODEnabled(true)
    , bCollisionCullingEnabled(true)
    , PhysicsCullingDistance(5000.0f)
{
    PhysicsTimeSamples.Reserve(120); // 2 seconds at 60fps
}

void UPerf_PhysicsPerformanceAnalyzer::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Physics Performance Analyzer initialized"));
    
    // Initialize optimization settings
    OptimizationSettings = FPerf_PhysicsOptimizationSettings();
    CurrentMetrics = FPerf_PhysicsMetrics();
    
    // Start automatic analysis
    StartPhysicsAnalysis();
}

void UPerf_PhysicsPerformanceAnalyzer::Deinitialize()
{
    StopPhysicsAnalysis();
    
    // Clear cached references
    TrackedPhysicsActors.Empty();
    ActiveRagdolls.Empty();
    DestructionDebris.Empty();
    PhysicsTimeSamples.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Physics Performance Analyzer deinitialized"));
    
    Super::Deinitialize();
}

void UPerf_PhysicsPerformanceAnalyzer::StartPhysicsAnalysis()
{
    if (bAnalysisActive)
    {
        return;
    }
    
    bAnalysisActive = true;
    PhysicsAnalysisTimer = 0.0f;
    OptimizationUpdateTimer = 0.0f;
    FramesSampled = 0;
    
    // Clear previous samples
    PhysicsTimeSamples.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Physics performance analysis started"));
}

void UPerf_PhysicsPerformanceAnalyzer::StopPhysicsAnalysis()
{
    if (!bAnalysisActive)
    {
        return;
    }
    
    bAnalysisActive = false;
    
    UE_LOG(LogTemp, Log, TEXT("Physics performance analysis stopped"));
}

FPerf_PhysicsMetrics UPerf_PhysicsPerformanceAnalyzer::GetCurrentPhysicsMetrics() const
{
    return CurrentMetrics;
}

void UPerf_PhysicsPerformanceAnalyzer::ApplyOptimizationLevel(EPerf_PhysicsOptimizationLevel Level)
{
    CurrentOptimizationLevel = Level;
    
    switch (Level)
    {
        case EPerf_PhysicsOptimizationLevel::None:
            OptimizationSettings.CullingDistance = 10000.0f;
            OptimizationSettings.MaxActiveRigidBodies = 1000;
            OptimizationSettings.PhysicsTickRate = 60.0f;
            OptimizationSettings.bEnablePhysicsLOD = false;
            OptimizationSettings.bEnableCollisionCulling = false;
            break;
            
        case EPerf_PhysicsOptimizationLevel::Low:
            OptimizationSettings.CullingDistance = 7500.0f;
            OptimizationSettings.MaxActiveRigidBodies = 750;
            OptimizationSettings.PhysicsTickRate = 60.0f;
            OptimizationSettings.bEnablePhysicsLOD = true;
            OptimizationSettings.bEnableCollisionCulling = true;
            break;
            
        case EPerf_PhysicsOptimizationLevel::Medium:
            OptimizationSettings.CullingDistance = 5000.0f;
            OptimizationSettings.MaxActiveRigidBodies = 500;
            OptimizationSettings.PhysicsTickRate = 60.0f;
            OptimizationSettings.bEnablePhysicsLOD = true;
            OptimizationSettings.bEnableCollisionCulling = true;
            break;
            
        case EPerf_PhysicsOptimizationLevel::High:
            OptimizationSettings.CullingDistance = 3000.0f;
            OptimizationSettings.MaxActiveRigidBodies = 300;
            OptimizationSettings.PhysicsTickRate = 45.0f;
            OptimizationSettings.bEnablePhysicsLOD = true;
            OptimizationSettings.bEnableCollisionCulling = true;
            break;
            
        case EPerf_PhysicsOptimizationLevel::Extreme:
            OptimizationSettings.CullingDistance = 2000.0f;
            OptimizationSettings.MaxActiveRigidBodies = 150;
            OptimizationSettings.PhysicsTickRate = 30.0f;
            OptimizationSettings.bEnablePhysicsLOD = true;
            OptimizationSettings.bEnableCollisionCulling = true;
            break;
    }
    
    ApplyPhysicsOptimizations();
    
    UE_LOG(LogTemp, Log, TEXT("Applied physics optimization level: %d"), (int32)Level);
}

void UPerf_PhysicsPerformanceAnalyzer::OptimizePhysicsForFrameRate(float TargetFrameRate)
{
    float CurrentFPS = 1.0f / FApp::GetDeltaTime();
    
    if (CurrentFPS < TargetFrameRate * 0.8f) // 80% of target
    {
        // Increase optimization level
        if (CurrentOptimizationLevel == EPerf_PhysicsOptimizationLevel::None)
        {
            ApplyOptimizationLevel(EPerf_PhysicsOptimizationLevel::Low);
        }
        else if (CurrentOptimizationLevel == EPerf_PhysicsOptimizationLevel::Low)
        {
            ApplyOptimizationLevel(EPerf_PhysicsOptimizationLevel::Medium);
        }
        else if (CurrentOptimizationLevel == EPerf_PhysicsOptimizationLevel::Medium)
        {
            ApplyOptimizationLevel(EPerf_PhysicsOptimizationLevel::High);
        }
        else if (CurrentOptimizationLevel == EPerf_PhysicsOptimizationLevel::High)
        {
            ApplyOptimizationLevel(EPerf_PhysicsOptimizationLevel::Extreme);
        }
    }
    else if (CurrentFPS > TargetFrameRate * 1.2f) // 120% of target
    {
        // Decrease optimization level
        if (CurrentOptimizationLevel == EPerf_PhysicsOptimizationLevel::Extreme)
        {
            ApplyOptimizationLevel(EPerf_PhysicsOptimizationLevel::High);
        }
        else if (CurrentOptimizationLevel == EPerf_PhysicsOptimizationLevel::High)
        {
            ApplyOptimizationLevel(EPerf_PhysicsOptimizationLevel::Medium);
        }
        else if (CurrentOptimizationLevel == EPerf_PhysicsOptimizationLevel::Medium)
        {
            ApplyOptimizationLevel(EPerf_PhysicsOptimizationLevel::Low);
        }
        else if (CurrentOptimizationLevel == EPerf_PhysicsOptimizationLevel::Low)
        {
            ApplyOptimizationLevel(EPerf_PhysicsOptimizationLevel::None);
        }
    }
}

void UPerf_PhysicsPerformanceAnalyzer::CullDistantPhysicsObjects(const FVector& PlayerLocation, float CullDistance)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    int32 CulledObjects = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor)
        {
            continue;
        }
        
        UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>();
        if (!PrimComp || !PrimComp->IsSimulatingPhysics())
        {
            continue;
        }
        
        float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
        
        if (Distance > CullDistance)
        {
            // Disable physics simulation for distant objects
            PrimComp->SetSimulatePhysics(false);
            CulledObjects++;
        }
        else if (Distance < CullDistance * 0.8f && !PrimComp->IsSimulatingPhysics())
        {
            // Re-enable physics for objects that came back into range
            PrimComp->SetSimulatePhysics(true);
        }
    }
    
    if (CulledObjects > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Culled %d distant physics objects"), CulledObjects);
    }
}

void UPerf_PhysicsPerformanceAnalyzer::ForcePhysicsSleep(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>();
    if (PrimComp && PrimComp->IsSimulatingPhysics())
    {
        PrimComp->PutRigidBodyToSleep();
        UE_LOG(LogTemp, Verbose, TEXT("Forced physics sleep for actor: %s"), *Actor->GetName());
    }
}

void UPerf_PhysicsPerformanceAnalyzer::WakePhysicsObject(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>();
    if (PrimComp && PrimComp->IsSimulatingPhysics())
    {
        PrimComp->WakeRigidBody();
        UE_LOG(LogTemp, Verbose, TEXT("Woke physics object: %s"), *Actor->GetName());
    }
}

float UPerf_PhysicsPerformanceAnalyzer::GetPhysicsPerformanceScore() const
{
    if (PhysicsTimeSamples.Num() == 0)
    {
        return 100.0f; // Perfect score if no data
    }
    
    // Calculate performance score based on physics time
    float TargetPhysicsTime = 1.0f / 60.0f * 0.1f; // 10% of 60fps frame time
    float Score = FMath::Clamp((TargetPhysicsTime / AveragePhysicsTime) * 100.0f, 0.0f, 100.0f);
    
    return Score;
}

void UPerf_PhysicsPerformanceAnalyzer::OptimizeRagdollPerformance()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    int32 OptimizedRagdolls = 0;
    
    // Find and optimize active ragdolls
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor)
        {
            continue;
        }
        
        USkeletalMeshComponent* SkelMeshComp = Actor->FindComponentByClass<USkeletalMeshComponent>();
        if (SkelMeshComp && SkelMeshComp->IsSimulatingPhysics())
        {
            // Apply ragdoll optimizations
            SkelMeshComp->SetAllBodiesBelowSimulatePhysics(FName(), true, false);
            OptimizedRagdolls++;
            
            // Add to tracked ragdolls
            ActiveRagdolls.AddUnique(Actor);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Optimized %d ragdolls"), OptimizedRagdolls);
}

void UPerf_PhysicsPerformanceAnalyzer::SetRagdollLODLevel(AActor* Actor, int32 LODLevel)
{
    if (!Actor)
    {
        return;
    }
    
    USkeletalMeshComponent* SkelMeshComp = Actor->FindComponentByClass<USkeletalMeshComponent>();
    if (SkelMeshComp)
    {
        SkelMeshComp->SetForcedLOD(LODLevel);
        UE_LOG(LogTemp, Verbose, TEXT("Set ragdoll LOD level %d for actor: %s"), LODLevel, *Actor->GetName());
    }
}

void UPerf_PhysicsPerformanceAnalyzer::OptimizeDestructionSystem()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Clean up old debris
    CleanupDestructionDebris();
    
    // Limit active destruction objects
    LimitActiveDebris(OptimizationSettings.MaxActiveRigidBodies / 4); // 25% of max rigid bodies for debris
}

void UPerf_PhysicsPerformanceAnalyzer::LimitActiveDebris(int32 MaxDebrisCount)
{
    if (DestructionDebris.Num() <= MaxDebrisCount)
    {
        return;
    }
    
    // Remove oldest debris first
    int32 DebrisToRemove = DestructionDebris.Num() - MaxDebrisCount;
    
    for (int32 i = 0; i < DebrisToRemove; i++)
    {
        if (DestructionDebris.IsValidIndex(i) && DestructionDebris[i].IsValid())
        {
            AActor* DebrisActor = DestructionDebris[i].Get();
            if (DebrisActor)
            {
                DebrisActor->Destroy();
            }
        }
    }
    
    // Remove invalid references
    DestructionDebris.RemoveAll([](const TWeakObjectPtr<AActor>& WeakPtr) {
        return !WeakPtr.IsValid();
    });
    
    UE_LOG(LogTemp, Log, TEXT("Limited debris count to %d (removed %d)"), MaxDebrisCount, DebrisToRemove);
}

void UPerf_PhysicsPerformanceAnalyzer::UpdatePhysicsMetrics()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Reset metrics
    CurrentMetrics = FPerf_PhysicsMetrics();
    
    // Count physics objects
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor)
        {
            continue;
        }
        
        UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>();
        if (PrimComp && PrimComp->IsSimulatingPhysics())
        {
            if (PrimComp->RigidBodyIsAwake())
            {
                CurrentMetrics.ActiveRigidBodies++;
            }
            else
            {
                CurrentMetrics.SleepingRigidBodies++;
            }
        }
        
        // Count ragdolls
        USkeletalMeshComponent* SkelMeshComp = Actor->FindComponentByClass<USkeletalMeshComponent>();
        if (SkelMeshComp && SkelMeshComp->IsSimulatingPhysics())
        {
            CurrentMetrics.ActiveRagdolls++;
        }
    }
    
    // Update physics timing
    CurrentMetrics.PhysicsStepTime = LastFramePhysicsTime;
    CurrentMetrics.CollisionDetectionTime = LastFramePhysicsTime * 0.6f; // Estimate
    CurrentMetrics.RagdollUpdateTime = CurrentMetrics.ActiveRagdolls * 0.001f; // Estimate
}

void UPerf_PhysicsPerformanceAnalyzer::AnalyzePhysicsPerformance()
{
    if (!bAnalysisActive)
    {
        return;
    }
    
    // Update metrics
    UpdatePhysicsMetrics();
    
    // Track physics timing
    float CurrentPhysicsTime = FApp::GetDeltaTime() * 0.1f; // Estimate physics portion
    PhysicsTimeSamples.Add(CurrentPhysicsTime);
    
    if (PhysicsTimeSamples.Num() > 120) // Keep last 2 seconds
    {
        PhysicsTimeSamples.RemoveAt(0);
    }
    
    // Calculate average
    float TotalTime = 0.0f;
    for (float Sample : PhysicsTimeSamples)
    {
        TotalTime += Sample;
    }
    
    if (PhysicsTimeSamples.Num() > 0)
    {
        AveragePhysicsTime = TotalTime / PhysicsTimeSamples.Num();
    }
    
    LastFramePhysicsTime = CurrentPhysicsTime;
    FramesSampled++;
}

void UPerf_PhysicsPerformanceAnalyzer::ApplyPhysicsOptimizations()
{
    if (!OptimizationSettings.bEnablePhysicsLOD && !OptimizationSettings.bEnableCollisionCulling)
    {
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Apply optimizations based on settings
    if (OptimizationSettings.bEnableCollisionCulling)
    {
        CullPhysicsObjects();
    }
    
    if (OptimizationSettings.bEnablePhysicsLOD)
    {
        ManagePhysicsLOD();
    }
    
    OptimizeCollisionDetection();
    UpdateRagdollOptimization();
}

void UPerf_PhysicsPerformanceAnalyzer::CullPhysicsObjects()
{
    // Implementation handled by CullDistantPhysicsObjects
    // This could be enhanced with more sophisticated culling
}

void UPerf_PhysicsPerformanceAnalyzer::OptimizeCollisionDetection()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Optimize collision detection for performance
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor)
        {
            continue;
        }
        
        UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>();
        if (PrimComp)
        {
            // Simplify collision for distant objects
            float Distance = FVector::Dist(Actor->GetActorLocation(), FVector::ZeroVector); // Simplified
            
            if (Distance > OptimizationSettings.CullingDistance * 0.5f)
            {
                PrimComp->SetCollisionResponseToAllChannels(ECR_Ignore);
            }
            else
            {
                PrimComp->SetCollisionResponseToAllChannels(ECR_Block);
            }
        }
    }
}

void UPerf_PhysicsPerformanceAnalyzer::ManagePhysicsLOD()
{
    // Manage physics LOD based on distance and performance
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor)
        {
            continue;
        }
        
        USkeletalMeshComponent* SkelMeshComp = Actor->FindComponentByClass<USkeletalMeshComponent>();
        if (SkelMeshComp)
        {
            float Distance = FVector::Dist(Actor->GetActorLocation(), FVector::ZeroVector); // Simplified
            
            if (Distance > OptimizationSettings.CullingDistance * 0.75f)
            {
                SetRagdollLODLevel(Actor, 3); // Lowest LOD
            }
            else if (Distance > OptimizationSettings.CullingDistance * 0.5f)
            {
                SetRagdollLODLevel(Actor, 2);
            }
            else if (Distance > OptimizationSettings.CullingDistance * 0.25f)
            {
                SetRagdollLODLevel(Actor, 1);
            }
            else
            {
                SetRagdollLODLevel(Actor, 0); // Highest LOD
            }
        }
    }
}

void UPerf_PhysicsPerformanceAnalyzer::UpdateRagdollOptimization()
{
    // Clean up invalid ragdoll references
    ActiveRagdolls.RemoveAll([](const TWeakObjectPtr<AActor>& WeakPtr) {
        return !WeakPtr.IsValid();
    });
    
    // Apply ragdoll-specific optimizations
    for (const TWeakObjectPtr<AActor>& WeakRagdoll : ActiveRagdolls)
    {
        if (AActor* RagdollActor = WeakRagdoll.Get())
        {
            USkeletalMeshComponent* SkelMeshComp = RagdollActor->FindComponentByClass<USkeletalMeshComponent>();
            if (SkelMeshComp)
            {
                // Optimize ragdoll physics settings
                SkelMeshComp->SetAllBodiesBelowLinearDamping(FName(), 0.1f, false);
                SkelMeshComp->SetAllBodiesBelowAngularDamping(FName(), 0.1f, false);
            }
        }
    }
}

void UPerf_PhysicsPerformanceAnalyzer::CleanupDestructionDebris()
{
    // Remove invalid debris references
    DestructionDebris.RemoveAll([](const TWeakObjectPtr<AActor>& WeakPtr) {
        return !WeakPtr.IsValid();
    });
    
    // Clean up old debris based on time or distance
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    for (int32 i = DestructionDebris.Num() - 1; i >= 0; i--)
    {
        if (AActor* DebrisActor = DestructionDebris[i].Get())
        {
            // Remove debris that's been around too long (simplified cleanup)
            float DebrisAge = CurrentTime - DebrisActor->GetGameTimeSinceCreation();
            if (DebrisAge > 30.0f) // 30 seconds
            {
                DebrisActor->Destroy();
                DestructionDebris.RemoveAt(i);
            }
        }
    }
}