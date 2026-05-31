#include "Core_PhysicsSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"

UCore_PhysicsSystemManager::UCore_PhysicsSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    
    // Initialize default settings
    PhysicsSettings = FCore_PhysicsSettings();
    bAutoApplySettings = true;
    PhysicsUpdateRate = 60.0f;
    
    bEnableCollisionOptimization = true;
    CollisionCheckDistance = 1000.0f;
    MaxCollisionChecksPerFrame = 100;
    
    bEnableRagdollSystem = true;
    RagdollActivationThreshold = 50.0f;
    RagdollDeactivationDelay = 5.0f;
    
    bEnableDestructionSystem = true;
    DestructionForceThreshold = 100.0f;
    MaxDestructibleObjects = 50;
    
    CurrentPhysicsFrameTime = 0.0f;
    ActivePhysicsObjects = 0;
    ActiveCollisionChecks = 0;
    
    LastPerformanceCheck = 0.0f;
    PerformanceCheckInterval = 1.0f;
}

void UCore_PhysicsSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoApplySettings)
    {
        ApplyPhysicsSettings();
    }
    
    // Initialize tracking arrays
    TrackedPhysicsActors.Empty();
    ActiveRagdollActors.Empty();
    DestructibleActors.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: System initialized with mode %d"), 
           static_cast<int32>(PhysicsSettings.PhysicsMode));
}

void UCore_PhysicsSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    CurrentPhysicsFrameTime = DeltaTime;
    
    // Update physics tracking
    UpdatePhysicsTracking();
    
    // Process collision optimization
    if (bEnableCollisionOptimization)
    {
        ProcessCollisionOptimization();
    }
    
    // Update ragdoll states
    if (bEnableRagdollSystem)
    {
        UpdateRagdollStates();
    }
    
    // Process destruction queue
    if (bEnableDestructionSystem)
    {
        ProcessDestructionQueue();
    }
    
    // Performance monitoring
    LastPerformanceCheck += DeltaTime;
    if (LastPerformanceCheck >= PerformanceCheckInterval)
    {
        CheckPerformance();
        LastPerformanceCheck = 0.0f;
    }
    
    // Cleanup invalid actors
    CleanupInvalidActors();
}

void UCore_PhysicsSystemManager::ApplyPhysicsSettings()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Apply gravity settings
    if (AWorldSettings* WorldSettings = World->GetWorldSettings())
    {
        WorldSettings->GlobalGravityZ = -980.0f * PhysicsSettings.GravityScale;
        WorldSettings->bGlobalGravitySet = PhysicsSettings.bEnableGravity;
    }
    
    // Apply physics mode specific settings
    switch (PhysicsSettings.PhysicsMode)
    {
        case ECore_PhysicsMode::HighPrecision:
            PhysicsUpdateRate = 120.0f;
            MaxCollisionChecksPerFrame = 200;
            break;
            
        case ECore_PhysicsMode::Performance:
            PhysicsUpdateRate = 30.0f;
            MaxCollisionChecksPerFrame = 50;
            break;
            
        case ECore_PhysicsMode::Cinematic:
            PhysicsUpdateRate = 24.0f;
            MaxCollisionChecksPerFrame = 25;
            break;
            
        default: // Standard
            PhysicsUpdateRate = 60.0f;
            MaxCollisionChecksPerFrame = 100;
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Applied physics settings - Mode: %d, UpdateRate: %.1f"), 
           static_cast<int32>(PhysicsSettings.PhysicsMode), PhysicsUpdateRate);
}

void UCore_PhysicsSystemManager::SetPhysicsMode(ECore_PhysicsMode NewMode)
{
    PhysicsSettings.PhysicsMode = NewMode;
    ApplyPhysicsSettings();
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Physics mode changed to %d"), 
           static_cast<int32>(NewMode));
}

void UCore_PhysicsSystemManager::EnableRagdollForActor(AActor* TargetActor)
{
    if (!TargetActor || !bEnableRagdollSystem)
    {
        return;
    }
    
    if (USkeletalMeshComponent* SkelMeshComp = TargetActor->FindComponentByClass<USkeletalMeshComponent>())
    {
        SkelMeshComp->SetSimulatePhysics(true);
        SkelMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        
        // Add to tracking
        ActiveRagdollActors.AddUnique(TargetActor);
        
        // Broadcast event
        OnRagdollActivated.Broadcast(TargetActor, RagdollActivationThreshold);
        
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Ragdoll enabled for actor %s"), 
               *TargetActor->GetName());
    }
}

void UCore_PhysicsSystemManager::DisableRagdollForActor(AActor* TargetActor)
{
    if (!TargetActor)
    {
        return;
    }
    
    if (USkeletalMeshComponent* SkelMeshComp = TargetActor->FindComponentByClass<USkeletalMeshComponent>())
    {
        SkelMeshComp->SetSimulatePhysics(false);
        SkelMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        
        // Remove from tracking
        ActiveRagdollActors.RemoveSingle(TargetActor);
        
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Ragdoll disabled for actor %s"), 
               *TargetActor->GetName());
    }
}

void UCore_PhysicsSystemManager::TriggerDestruction(AActor* TargetActor, FVector ImpactPoint, float Force)
{
    if (!TargetActor || !bEnableDestructionSystem || Force < DestructionForceThreshold)
    {
        return;
    }
    
    // Apply destruction force
    if (UPrimitiveComponent* PrimComp = TargetActor->FindComponentByClass<UPrimitiveComponent>())
    {
        FVector ForceDirection = (TargetActor->GetActorLocation() - ImpactPoint).GetSafeNormal();
        PrimComp->AddImpulseAtLocation(ForceDirection * Force, ImpactPoint);
    }
    
    // Broadcast destruction event
    OnDestructionTriggered.Broadcast(TargetActor);
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Destruction triggered for actor %s with force %.1f"), 
           *TargetActor->GetName(), Force);
}

void UCore_PhysicsSystemManager::OptimizePhysicsPerformance()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    int32 OptimizedCount = 0;
    
    // Optimize distant physics objects
    for (auto ActorIter = World->GetActorIterator(); ActorIter; ++ActorIter)
    {
        AActor* Actor = *ActorIter;
        if (!Actor)
        {
            continue;
        }
        
        float Distance = FVector::Dist(Actor->GetActorLocation(), GetOwner()->GetActorLocation());
        
        if (Distance > CollisionCheckDistance)
        {
            if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
            {
                if (PrimComp->IsSimulatingPhysics())
                {
                    PrimComp->SetSimulatePhysics(false);
                    OptimizedCount++;
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Optimized %d distant physics objects"), OptimizedCount);
}

bool UCore_PhysicsSystemManager::IsPhysicsObjectActive(AActor* TargetActor) const
{
    if (!TargetActor)
    {
        return false;
    }
    
    if (UPrimitiveComponent* PrimComp = TargetActor->FindComponentByClass<UPrimitiveComponent>())
    {
        return PrimComp->IsSimulatingPhysics();
    }
    
    return false;
}

float UCore_PhysicsSystemManager::GetPhysicsPerformanceScore() const
{
    // Calculate performance score based on frame time and active objects
    float BaseScore = 100.0f;
    float FrameTimePenalty = FMath::Clamp(CurrentPhysicsFrameTime * 1000.0f - 16.67f, 0.0f, 50.0f);
    float ObjectCountPenalty = FMath::Clamp(static_cast<float>(ActivePhysicsObjects) / 100.0f * 25.0f, 0.0f, 25.0f);
    
    return FMath::Clamp(BaseScore - FrameTimePenalty - ObjectCountPenalty, 0.0f, 100.0f);
}

void UCore_PhysicsSystemManager::UpdatePhysicsTracking()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    ActivePhysicsObjects = 0;
    TrackedPhysicsActors.Empty();
    
    for (auto ActorIter = World->GetActorIterator(); ActorIter; ++ActorIter)
    {
        AActor* Actor = *ActorIter;
        if (!Actor)
        {
            continue;
        }
        
        if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
        {
            if (PrimComp->IsSimulatingPhysics())
            {
                TrackedPhysicsActors.Add(Actor);
                ActivePhysicsObjects++;
            }
        }
    }
}

void UCore_PhysicsSystemManager::CheckPerformance()
{
    float PerformanceScore = GetPhysicsPerformanceScore();
    
    if (PerformanceScore < 60.0f)
    {
        OnPhysicsPerformanceWarning.Broadcast(PerformanceScore);
        
        // Auto-optimize if performance is critical
        if (PerformanceScore < 30.0f)
        {
            OptimizePhysicsPerformance();
        }
    }
}

void UCore_PhysicsSystemManager::CleanupInvalidActors()
{
    // Clean up tracked actors
    TrackedPhysicsActors.RemoveAll([](const TWeakObjectPtr<AActor>& ActorPtr)
    {
        return !ActorPtr.IsValid();
    });
    
    ActiveRagdollActors.RemoveAll([](const TWeakObjectPtr<AActor>& ActorPtr)
    {
        return !ActorPtr.IsValid();
    });
    
    DestructibleActors.RemoveAll([](const TWeakObjectPtr<AActor>& ActorPtr)
    {
        return !ActorPtr.IsValid();
    });
}

void UCore_PhysicsSystemManager::ProcessCollisionOptimization()
{
    // Limit collision checks per frame for performance
    ActiveCollisionChecks = FMath::Min(ActiveCollisionChecks, MaxCollisionChecksPerFrame);
}

void UCore_PhysicsSystemManager::UpdateRagdollStates()
{
    // Update ragdoll states and handle deactivation
    for (int32 i = ActiveRagdollActors.Num() - 1; i >= 0; i--)
    {
        if (TWeakObjectPtr<AActor> ActorPtr = ActiveRagdollActors[i])
        {
            if (AActor* Actor = ActorPtr.Get())
            {
                if (USkeletalMeshComponent* SkelMeshComp = Actor->FindComponentByClass<USkeletalMeshComponent>())
                {
                    // Check if ragdoll should be deactivated (velocity threshold)
                    FVector Velocity = SkelMeshComp->GetPhysicsLinearVelocity();
                    if (Velocity.Size() < 10.0f)
                    {
                        // Start deactivation timer logic here if needed
                    }
                }
            }
        }
    }
}

void UCore_PhysicsSystemManager::ProcessDestructionQueue()
{
    // Process destruction queue and cleanup destroyed objects
    DestructibleActors.RemoveAll([](const TWeakObjectPtr<AActor>& ActorPtr)
    {
        return !ActorPtr.IsValid();
    });
    
    // Limit active destructible objects
    if (DestructibleActors.Num() > MaxDestructibleObjects)
    {
        int32 ExcessCount = DestructibleActors.Num() - MaxDestructibleObjects;
        DestructibleActors.RemoveAt(0, ExcessCount);
    }
}