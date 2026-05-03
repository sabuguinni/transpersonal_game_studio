#include "Core_PhysicsSystemManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "PhysicsEngine/PhysicsSettings.h"

UCore_PhysicsSystemManager::UCore_PhysicsSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize physics settings
    bEnablePhysicsSimulation = true;
    bEnableRagdollPhysics = true;
    bEnableDestructionPhysics = true;
    PhysicsGravity = FVector(0.0f, 0.0f, -980.0f); // Standard Earth gravity
    PhysicsSubsteps = 4;
    MaxPhysicsDeltaTime = 0.033f; // 30 FPS physics cap
    
    // Performance limits
    MaxSimulatingBodies = 500;
    MaxRagdollBodies = 50;
    MaxDestructibleActors = 100;
    
    // Initialize counters
    CurrentSimulatingBodies = 0;
    CurrentRagdollBodies = 0;
    CurrentDestructibleActors = 0;
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Initialized with gravity %s"), *PhysicsGravity.ToString());
}

void UCore_PhysicsSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Apply global physics settings
    ApplyGlobalPhysicsSettings();
    
    // Register with world for physics events
    if (UWorld* World = GetWorld())
    {
        World->OnWorldBeginPlay.AddUObject(this, &UCore_PhysicsSystemManager::OnWorldBeginPlay);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: BeginPlay - Physics system active"));
}

void UCore_PhysicsSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update physics statistics
    UpdatePhysicsStatistics();
    
    // Check performance limits
    EnforcePerformanceLimits();
    
    // Update physics simulation if needed
    if (bEnablePhysicsSimulation)
    {
        UpdatePhysicsSimulation(DeltaTime);
    }
}

void UCore_PhysicsSystemManager::ApplyGlobalPhysicsSettings()
{
    if (UWorld* World = GetWorld())
    {
        // Set world gravity
        World->GetPhysicsScene()->GetPxScene()->setGravity(FPhysicsInterface::UToP(PhysicsGravity));
        
        // Apply physics substeps
        if (UPhysicsSettings* PhysSettings = UPhysicsSettings::Get())
        {
            PhysSettings->MaxSubsteps = PhysicsSubsteps;
            PhysSettings->MaxSubstepDeltaTime = MaxPhysicsDeltaTime / PhysicsSubsteps;
        }
        
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Applied global physics settings"));
    }
}

void UCore_PhysicsSystemManager::UpdatePhysicsStatistics()
{
    CurrentSimulatingBodies = 0;
    CurrentRagdollBodies = 0;
    CurrentDestructibleActors = 0;
    
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (!Actor) continue;
            
            // Count simulating bodies
            TArray<UPrimitiveComponent*> PrimitiveComponents;
            Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
            
            for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
            {
                if (PrimComp && PrimComp->IsSimulatingPhysics())
                {
                    CurrentSimulatingBodies++;
                    
                    // Check if it's a ragdoll
                    if (USkeletalMeshComponent* SkelComp = Cast<USkeletalMeshComponent>(PrimComp))
                    {
                        if (SkelComp->GetPhysicsAsset())
                        {
                            CurrentRagdollBodies++;
                        }
                    }
                }
            }
        }
    }
}

void UCore_PhysicsSystemManager::EnforcePerformanceLimits()
{
    // Disable physics on excess bodies if over limit
    if (CurrentSimulatingBodies > MaxSimulatingBodies)
    {
        int32 BodiesOverLimit = CurrentSimulatingBodies - MaxSimulatingBodies;
        DisableExcessPhysicsBodies(BodiesOverLimit);
        
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Over physics limit! Disabled %d bodies"), BodiesOverLimit);
    }
    
    // Similar checks for ragdolls and destructibles
    if (CurrentRagdollBodies > MaxRagdollBodies)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Over ragdoll limit! %d/%d"), CurrentRagdollBodies, MaxRagdollBodies);
    }
}

void UCore_PhysicsSystemManager::DisableExcessPhysicsBodies(int32 NumToDisable)
{
    int32 DisabledCount = 0;
    
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr && DisabledCount < NumToDisable; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (!Actor) continue;
            
            TArray<UPrimitiveComponent*> PrimitiveComponents;
            Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
            
            for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
            {
                if (PrimComp && PrimComp->IsSimulatingPhysics() && DisabledCount < NumToDisable)
                {
                    // Disable physics on least important bodies first
                    FVector ActorLocation = Actor->GetActorLocation();
                    FVector PlayerLocation = FVector::ZeroVector; // TODO: Get actual player location
                    
                    float DistanceToPlayer = FVector::Dist(ActorLocation, PlayerLocation);
                    
                    // Disable physics on distant objects first
                    if (DistanceToPlayer > 2000.0f) // 20 meters
                    {
                        PrimComp->SetSimulatePhysics(false);
                        DisabledCount++;
                    }
                }
            }
        }
    }
}

void UCore_PhysicsSystemManager::UpdatePhysicsSimulation(float DeltaTime)
{
    // Custom physics update logic if needed
    // This runs in TG_PrePhysics tick group
    
    // Update gravity if it changed
    if (UWorld* World = GetWorld())
    {
        FVector CurrentGravity = World->GetGravityZ() * FVector::UpVector;
        if (!CurrentGravity.Equals(PhysicsGravity, 1.0f))
        {
            World->GetPhysicsScene()->GetPxScene()->setGravity(FPhysicsInterface::UToP(PhysicsGravity));
        }
    }
}

void UCore_PhysicsSystemManager::OnWorldBeginPlay()
{
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: World begin play - Physics system ready"));
    
    // Perform initial physics setup
    ApplyGlobalPhysicsSettings();
    UpdatePhysicsStatistics();
}

bool UCore_PhysicsSystemManager::EnablePhysicsOnActor(AActor* Actor)
{
    if (!Actor || !bEnablePhysicsSimulation)
    {
        return false;
    }
    
    // Check if we're under performance limits
    if (CurrentSimulatingBodies >= MaxSimulatingBodies)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Cannot enable physics - at body limit"));
        return false;
    }
    
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    bool bSuccessfullyEnabled = false;
    for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
    {
        if (PrimComp && !PrimComp->IsSimulatingPhysics())
        {
            PrimComp->SetSimulatePhysics(true);
            bSuccessfullyEnabled = true;
        }
    }
    
    if (bSuccessfullyEnabled)
    {
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Enabled physics on actor %s"), *Actor->GetName());
    }
    
    return bSuccessfullyEnabled;
}

bool UCore_PhysicsSystemManager::DisablePhysicsOnActor(AActor* Actor)
{
    if (!Actor)
    {
        return false;
    }
    
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    bool bSuccessfullyDisabled = false;
    for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
    {
        if (PrimComp && PrimComp->IsSimulatingPhysics())
        {
            PrimComp->SetSimulatePhysics(false);
            bSuccessfullyDisabled = true;
        }
    }
    
    if (bSuccessfullyDisabled)
    {
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Disabled physics on actor %s"), *Actor->GetName());
    }
    
    return bSuccessfullyDisabled;
}

void UCore_PhysicsSystemManager::SetGravity(const FVector& NewGravity)
{
    PhysicsGravity = NewGravity;
    ApplyGlobalPhysicsSettings();
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Gravity set to %s"), *PhysicsGravity.ToString());
}

FCore_PhysicsStats UCore_PhysicsSystemManager::GetPhysicsStatistics() const
{
    FCore_PhysicsStats Stats;
    Stats.SimulatingBodies = CurrentSimulatingBodies;
    Stats.RagdollBodies = CurrentRagdollBodies;
    Stats.DestructibleActors = CurrentDestructibleActors;
    Stats.MaxSimulatingBodies = MaxSimulatingBodies;
    Stats.MaxRagdollBodies = MaxRagdollBodies;
    Stats.MaxDestructibleActors = MaxDestructibleActors;
    Stats.CurrentGravity = PhysicsGravity;
    Stats.PhysicsSubsteps = PhysicsSubsteps;
    
    return Stats;
}