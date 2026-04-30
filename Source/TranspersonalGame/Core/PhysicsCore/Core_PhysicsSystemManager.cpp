#include "Core_PhysicsSystemManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UCore_PhysicsSystemManager::UCore_PhysicsSystemManager()
{
    CurrentPhysicsQuality = ECore_PhysicsQuality::High;
    ActivePhysicsActors = 0;
    LastPhysicsFrameTime = 0.0f;
    MaxPhysicsDistance = 5000.0f;
    MaxSimulatedActors = 100;
    CollisionImpactThreshold = 500.0f;
}

void UCore_PhysicsSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Core Physics System Manager: Initializing physics subsystem"));
    
    // Set up initial physics settings
    UpdatePhysicsSettings();
    
    // Start performance monitoring
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            PerformanceMonitorTimer,
            FTimerDelegate::CreateUObject(this, &UCore_PhysicsSystemManager::OptimizePhysicsPerformance),
            1.0f,
            true
        );
        
        World->GetTimerManager().SetTimer(
            PhysicsOptimizationTimer,
            FTimerDelegate::CreateUObject(this, &UCore_PhysicsSystemManager::CleanupInactivePhysics),
            5.0f,
            true
        );
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core Physics System Manager: Initialization complete"));
}

void UCore_PhysicsSystemManager::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("Core Physics System Manager: Shutting down physics subsystem"));
    
    // Clear timer handles
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PerformanceMonitorTimer);
        World->GetTimerManager().ClearTimer(PhysicsOptimizationTimer);
    }
    
    // Cleanup ragdoll actors
    RagdollActors.Empty();
    
    Super::Deinitialize();
}

bool UCore_PhysicsSystemManager::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

void UCore_PhysicsSystemManager::SetPhysicsQuality(ECore_PhysicsQuality Quality)
{
    if (CurrentPhysicsQuality != Quality)
    {
        CurrentPhysicsQuality = Quality;
        UpdatePhysicsSettings();
        
        UE_LOG(LogTemp, Log, TEXT("Physics quality changed to: %d"), (int32)Quality);
    }
}

void UCore_PhysicsSystemManager::HandleImpact(const FHitResult& HitResult, float ImpactForce)
{
    if (!HitResult.GetActor() || ImpactForce < CollisionImpactThreshold)
    {
        return;
    }
    
    // Create physics interaction data
    FCore_PhysicsInteraction Interaction;
    Interaction.SurfaceType = GetSurfaceType(HitResult);
    Interaction.ImpactForce = ImpactForce;
    Interaction.ImpactLocation = HitResult.ImpactPoint;
    Interaction.ImpactNormal = HitResult.ImpactNormal;
    Interaction.bShouldCreateDebris = ImpactForce > 1000.0f;
    
    // Apply physics response
    if (UPrimitiveComponent* HitComponent = HitResult.GetComponent())
    {
        if (HitComponent->IsSimulatingPhysics())
        {
            FVector ImpulseDirection = HitResult.ImpactNormal * -1.0f;
            FVector Impulse = ImpulseDirection * ImpactForce * 0.1f; // Scale down for realistic response
            
            HitComponent->AddImpulseAtLocation(Impulse, HitResult.ImpactPoint);
        }
    }
    
    // Create debris if impact is strong enough
    if (Interaction.bShouldCreateDebris)
    {
        CreateDebris(HitResult.ImpactPoint, Interaction.SurfaceType, FMath::RandRange(3, 8));
    }
    
    UE_LOG(LogTemp, Log, TEXT("Physics impact processed: Force=%.2f, Surface=%d"), 
           ImpactForce, (int32)Interaction.SurfaceType);
}

FCore_PhysicsInteraction UCore_PhysicsSystemManager::ProcessCollision(AActor* ActorA, AActor* ActorB, const FVector& ImpactPoint, float Force)
{
    FCore_PhysicsInteraction Interaction;
    
    if (!ActorA || !ActorB)
    {
        return Interaction;
    }
    
    // Calculate collision response
    Interaction.ImpactLocation = ImpactPoint;
    Interaction.ImpactForce = Force;
    
    // Determine surface type from the actors involved
    if (ActorA->GetName().Contains(TEXT("Rock")) || ActorB->GetName().Contains(TEXT("Rock")))
    {
        Interaction.SurfaceType = ECore_SurfaceType::Rock;
    }
    else if (ActorA->GetName().Contains(TEXT("Tree")) || ActorB->GetName().Contains(TEXT("Tree")))
    {
        Interaction.SurfaceType = ECore_SurfaceType::Wood;
    }
    else
    {
        Interaction.SurfaceType = ECore_SurfaceType::Dirt;
    }
    
    // Apply mutual forces
    if (UPrimitiveComponent* CompA = ActorA->FindComponentByClass<UPrimitiveComponent>())
    {
        if (CompA->IsSimulatingPhysics())
        {
            FVector ImpulseA = (ActorA->GetActorLocation() - ImpactPoint).GetSafeNormal() * Force * 0.05f;
            CompA->AddImpulse(ImpulseA);
        }
    }
    
    if (UPrimitiveComponent* CompB = ActorB->FindComponentByClass<UPrimitiveComponent>())
    {
        if (CompB->IsSimulatingPhysics())
        {
            FVector ImpulseB = (ActorB->GetActorLocation() - ImpactPoint).GetSafeNormal() * Force * 0.05f;
            CompB->AddImpulse(ImpulseB);
        }
    }
    
    return Interaction;
}

void UCore_PhysicsSystemManager::EnableRagdoll(AActor* Actor, const FCore_RagdollConfig& Config)
{
    if (!Actor)
    {
        return;
    }
    
    USkeletalMeshComponent* SkeletalMesh = Actor->FindComponentByClass<USkeletalMeshComponent>();
    if (!SkeletalMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot enable ragdoll: Actor %s has no skeletal mesh component"), *Actor->GetName());
        return;
    }
    
    // Enable physics simulation
    SkeletalMesh->SetSimulatePhysics(Config.bSimulatePhysics);
    SkeletalMesh->SetEnableGravity(Config.bEnableGravity);
    SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    
    // Blend to physics
    if (UAnimInstance* AnimInstance = SkeletalMesh->GetAnimInstance())
    {
        // Note: In a full implementation, you would use a custom anim BP with physics blending
        // For now, we just disable animation updates
        SkeletalMesh->bPauseAnims = true;
    }
    
    // Add to ragdoll tracking
    RagdollActors.AddUnique(Actor);
    
    UE_LOG(LogTemp, Log, TEXT("Ragdoll enabled for actor: %s"), *Actor->GetName());
}

void UCore_PhysicsSystemManager::DisableRagdoll(AActor* Actor, float BlendOutTime)
{
    if (!Actor)
    {
        return;
    }
    
    USkeletalMeshComponent* SkeletalMesh = Actor->FindComponentByClass<USkeletalMeshComponent>();
    if (!SkeletalMesh)
    {
        return;
    }
    
    // Disable physics simulation
    SkeletalMesh->SetSimulatePhysics(false);
    SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    
    // Re-enable animation
    SkeletalMesh->bPauseAnims = false;
    
    // Remove from ragdoll tracking
    RagdollActors.Remove(Actor);
    
    UE_LOG(LogTemp, Log, TEXT("Ragdoll disabled for actor: %s"), *Actor->GetName());
}

void UCore_PhysicsSystemManager::TriggerDestruction(AActor* Actor, const FVector& ImpactPoint, float DestructionForce)
{
    if (!Actor)
    {
        return;
    }
    
    // Check if actor has physics component with destruction enabled
    if (UCore_PhysicsComponent* PhysicsComp = Actor->FindComponentByClass<UCore_PhysicsComponent>())
    {
        if (!PhysicsComp->bCanBeDestroyed || DestructionForce < PhysicsComp->DestructionThreshold)
        {
            return;
        }
    }
    
    // Create debris at destruction point
    CreateDebris(ImpactPoint, ECore_SurfaceType::Rock, FMath::RandRange(5, 15));
    
    // Apply destruction effects
    if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
    {
        // Disable collision and physics
        PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        PrimComp->SetSimulatePhysics(false);
        
        // Hide the original actor
        Actor->SetActorHiddenInGame(true);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Destruction triggered for actor: %s with force: %.2f"), 
           *Actor->GetName(), DestructionForce);
}

void UCore_PhysicsSystemManager::CreateDebris(const FVector& Location, ECore_SurfaceType SurfaceType, int32 DebrisCount)
{
    if (!GetWorld())
    {
        return;
    }
    
    // Limit debris count based on performance settings
    int32 MaxDebris = 10;
    switch (CurrentPhysicsQuality)
    {
        case ECore_PhysicsQuality::Low:
            MaxDebris = 3;
            break;
        case ECore_PhysicsQuality::Medium:
            MaxDebris = 6;
            break;
        case ECore_PhysicsQuality::High:
            MaxDebris = 10;
            break;
        case ECore_PhysicsQuality::Ultra:
            MaxDebris = 15;
            break;
    }
    
    DebrisCount = FMath::Min(DebrisCount, MaxDebris);
    
    // Spawn debris pieces
    for (int32 i = 0; i < DebrisCount; i++)
    {
        FVector DebrisLocation = Location + FVector(
            FMath::RandRange(-100.0f, 100.0f),
            FMath::RandRange(-100.0f, 100.0f),
            FMath::RandRange(0.0f, 50.0f)
        );
        
        // In a full implementation, you would spawn actual debris actors
        // For now, we just log the creation
        UE_LOG(LogTemp, Log, TEXT("Debris created at: %s (Type: %d)"), 
               *DebrisLocation.ToString(), (int32)SurfaceType);
    }
}

void UCore_PhysicsSystemManager::SetPhysicsSimulation(AActor* Actor, bool bSimulate)
{
    if (!Actor)
    {
        return;
    }
    
    if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
    {
        PrimComp->SetSimulatePhysics(bSimulate);
        
        if (bSimulate)
        {
            ActivePhysicsActors++;
        }
        else
        {
            ActivePhysicsActors = FMath::Max(0, ActivePhysicsActors - 1);
        }
    }
}

void UCore_PhysicsSystemManager::ApplyImpulse(AActor* Actor, const FVector& Impulse, const FVector& Location)
{
    if (!Actor)
    {
        return;
    }
    
    if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
    {
        if (PrimComp->IsSimulatingPhysics())
        {
            if (Location.IsZero())
            {
                PrimComp->AddImpulse(Impulse);
            }
            else
            {
                PrimComp->AddImpulseAtLocation(Impulse, Location);
            }
        }
    }
}

ECore_SurfaceType UCore_PhysicsSystemManager::GetSurfaceType(const FHitResult& HitResult)
{
    if (!HitResult.PhysMaterial.IsValid())
    {
        return ECore_SurfaceType::Rock; // Default
    }
    
    // In a full implementation, you would check the physical material properties
    // For now, we use simple name-based detection
    FString MaterialName = HitResult.PhysMaterial->GetName();
    
    if (MaterialName.Contains(TEXT("Grass")))
    {
        return ECore_SurfaceType::Grass;
    }
    else if (MaterialName.Contains(TEXT("Dirt")))
    {
        return ECore_SurfaceType::Dirt;
    }
    else if (MaterialName.Contains(TEXT("Water")))
    {
        return ECore_SurfaceType::Water;
    }
    else if (MaterialName.Contains(TEXT("Wood")))
    {
        return ECore_SurfaceType::Wood;
    }
    else if (MaterialName.Contains(TEXT("Metal")))
    {
        return ECore_SurfaceType::Metal;
    }
    else if (MaterialName.Contains(TEXT("Bone")))
    {
        return ECore_SurfaceType::Bone;
    }
    
    return ECore_SurfaceType::Rock;
}

void UCore_PhysicsSystemManager::UpdatePhysicsSettings()
{
    // Adjust physics settings based on quality level
    switch (CurrentPhysicsQuality)
    {
        case ECore_PhysicsQuality::Low:
            MaxSimulatedActors = 25;
            MaxPhysicsDistance = 2000.0f;
            break;
        case ECore_PhysicsQuality::Medium:
            MaxSimulatedActors = 50;
            MaxPhysicsDistance = 3500.0f;
            break;
        case ECore_PhysicsQuality::High:
            MaxSimulatedActors = 100;
            MaxPhysicsDistance = 5000.0f;
            break;
        case ECore_PhysicsQuality::Ultra:
            MaxSimulatedActors = 200;
            MaxPhysicsDistance = 7500.0f;
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Physics settings updated: MaxActors=%d, MaxDistance=%.0f"), 
           MaxSimulatedActors, MaxPhysicsDistance);
}

void UCore_PhysicsSystemManager::OptimizePhysicsPerformance()
{
    float StartTime = FPlatformTime::Seconds();
    
    // Count active physics actors
    int32 ActualActiveActors = 0;
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->IsValidLowLevel())
            {
                if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
                {
                    if (PrimComp->IsSimulatingPhysics())
                    {
                        ActualActiveActors++;
                    }
                }
            }
        }
    }
    
    ActivePhysicsActors = ActualActiveActors;
    LastPhysicsFrameTime = (FPlatformTime::Seconds() - StartTime) * 1000.0f; // Convert to ms
    
    // Log performance if it's concerning
    if (LastPhysicsFrameTime > 5.0f) // More than 5ms for physics update
    {
        UE_LOG(LogTemp, Warning, TEXT("Physics performance warning: %.2fms for %d actors"), 
               LastPhysicsFrameTime, ActivePhysicsActors);
    }
}

void UCore_PhysicsSystemManager::CleanupInactivePhysics()
{
    // Remove destroyed actors from ragdoll list
    RagdollActors.RemoveAll([](AActor* Actor) {
        return !IsValid(Actor);
    });
    
    UE_LOG(LogTemp, Log, TEXT("Physics cleanup: %d ragdoll actors tracked"), RagdollActors.Num());
}

// UCore_PhysicsComponent Implementation

UCore_PhysicsComponent::UCore_PhysicsComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    SurfaceType = ECore_SurfaceType::Rock;
    bCanBeDestroyed = false;
    DestructionThreshold = 1000.0f;
    bEnableRagdoll = false;
    PhysicsManager = nullptr;
}

void UCore_PhysicsComponent::BeginPlay()
{
    Super::BeginPlay();
    
    InitializePhysicsComponent();
}

void UCore_PhysicsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Component-specific physics updates can go here
}

void UCore_PhysicsComponent::InitializePhysicsComponent()
{
    if (UWorld* World = GetWorld())
    {
        PhysicsManager = World->GetSubsystem<UCore_PhysicsSystemManager>();
        
        if (PhysicsManager)
        {
            UE_LOG(LogTemp, Log, TEXT("Physics component initialized for actor: %s"), 
                   *GetOwner()->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to get Physics System Manager for actor: %s"), 
                   *GetOwner()->GetName());
        }
    }
}