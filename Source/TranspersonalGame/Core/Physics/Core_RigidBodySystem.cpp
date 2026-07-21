#include "Core_RigidBodySystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"

UCore_RigidBodySystem::UCore_RigidBodySystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.016f; // 60 FPS
    
    // Initialize default settings
    GlobalGravityScale = 1.0f;
    GlobalDamping = 0.01f;
    GlobalAngularDamping = 0.01f;
    MaxSimulationSubsteps = 6;
    MaxSubstepDeltaTime = 0.016f;
    
    bEnableAsyncPhysics = true;
    bEnableCCD = false;
    bEnableStabilization = true;
    
    // Initialize performance settings
    MaxActiveRigidBodies = 1000;
    SleepThreshold = 0.1f;
    WakeThreshold = 0.2f;
}

void UCore_RigidBodySystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize rigid body system
    InitializeRigidBodySystem();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_RigidBodySystem: Rigid body system initialized"));
}

void UCore_RigidBodySystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update rigid body system
    UpdateRigidBodySystem(DeltaTime);
}

void UCore_RigidBodySystem::InitializeRigidBodySystem()
{
    // Setup physics world settings
    ConfigurePhysicsWorld();
    
    // Initialize rigid body tracking
    ActiveRigidBodies.Empty();
    SleepingRigidBodies.Empty();
    
    // Find all rigid bodies in the world
    DiscoverRigidBodies();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_RigidBodySystem: Found %d rigid bodies"), ActiveRigidBodies.Num() + SleepingRigidBodies.Num());
}

void UCore_RigidBodySystem::ConfigurePhysicsWorld()
{
    if (UWorld* World = GetWorld())
    {
        // Configure physics settings
        if (UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get())
        {
            // Apply global physics settings
            PhysicsSettings->DefaultGravityZ = -980.0f * GlobalGravityScale;
            PhysicsSettings->MaxSubsteps = MaxSimulationSubsteps;
            PhysicsSettings->MaxSubstepDeltaTime = MaxSubstepDeltaTime;
            
            // Enable/disable features
            PhysicsSettings->bEnableAsyncScene = bEnableAsyncPhysics;
            PhysicsSettings->bEnableStabilization = bEnableStabilization;
        }
        
        UE_LOG(LogTemp, Log, TEXT("Core_RigidBodySystem: Physics world configured"));
    }
}

void UCore_RigidBodySystem::DiscoverRigidBodies()
{
    if (UWorld* World = GetWorld())
    {
        // Iterate through all actors in the world
        for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            AActor* Actor = *ActorIterator;
            if (!Actor || Actor->IsPendingKill())
            {
                continue;
            }
            
            // Find all primitive components with physics enabled
            TArray<UPrimitiveComponent*> PrimitiveComponents;
            Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
            
            for (UPrimitiveComponent* Component : PrimitiveComponents)
            {
                if (Component && IsRigidBodyComponent(Component))
                {
                    RegisterRigidBody(Component);
                }
            }
        }
    }
}

bool UCore_RigidBodySystem::IsRigidBodyComponent(UPrimitiveComponent* Component)
{
    if (!Component)
    {
        return false;
    }
    
    // Check if component has physics simulation enabled
    if (!Component->IsSimulatingPhysics())
    {
        return false;
    }
    
    // Check if component has a valid body instance
    if (!Component->GetBodyInstance())
    {
        return false;
    }
    
    // Check collision settings
    if (Component->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
    {
        return false;
    }
    
    return true;
}

void UCore_RigidBodySystem::RegisterRigidBody(UPrimitiveComponent* Component)
{
    if (!Component)
    {
        return;
    }
    
    // Create rigid body data
    FCore_RigidBodyData RigidBodyData;
    RigidBodyData.Component = Component;
    RigidBodyData.Mass = Component->GetMass();
    RigidBodyData.LinearVelocity = Component->GetPhysicsLinearVelocity();
    RigidBodyData.AngularVelocity = Component->GetPhysicsAngularVelocityInRadians();
    RigidBodyData.bIsAwake = !Component->IsSimulatingPhysics() || Component->GetPhysicsLinearVelocity().SizeSquared() > SleepThreshold * SleepThreshold;
    RigidBodyData.LastUpdateTime = GetWorld()->GetTimeSeconds();
    
    // Apply default rigid body settings
    ApplyRigidBodySettings(Component);
    
    // Add to appropriate list
    if (RigidBodyData.bIsAwake)
    {
        ActiveRigidBodies.Add(RigidBodyData);
    }
    else
    {
        SleepingRigidBodies.Add(RigidBodyData);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_RigidBodySystem: Registered rigid body %s (Mass: %.2f)"), *Component->GetName(), RigidBodyData.Mass);
}

void UCore_RigidBodySystem::UnregisterRigidBody(UPrimitiveComponent* Component)
{
    if (!Component)
    {
        return;
    }
    
    // Remove from active list
    ActiveRigidBodies.RemoveAll([Component](const FCore_RigidBodyData& Data)
    {
        return Data.Component == Component;
    });
    
    // Remove from sleeping list
    SleepingRigidBodies.RemoveAll([Component](const FCore_RigidBodyData& Data)
    {
        return Data.Component == Component;
    });
    
    UE_LOG(LogTemp, Log, TEXT("Core_RigidBodySystem: Unregistered rigid body %s"), *Component->GetName());
}

void UCore_RigidBodySystem::ApplyRigidBodySettings(UPrimitiveComponent* Component)
{
    if (!Component)
    {
        return;
    }
    
    FBodyInstance* BodyInstance = Component->GetBodyInstance();
    if (!BodyInstance)
    {
        return;
    }
    
    // Apply damping settings
    BodyInstance->LinearDamping = GlobalDamping;
    BodyInstance->AngularDamping = GlobalAngularDamping;
    
    // Apply CCD settings
    BodyInstance->bUseCCD = bEnableCCD;
    
    // Apply sleep settings
    BodyInstance->SleepFamily = ESleepFamily::Normal;
    
    // Update the body instance
    BodyInstance->UpdatePhysicsFilterData();
}

void UCore_RigidBodySystem::UpdateRigidBodySystem(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update active rigid bodies
    UpdateActiveRigidBodies(DeltaTime, CurrentTime);
    
    // Check for sleeping rigid bodies that should wake up
    CheckSleepingRigidBodies(CurrentTime);
    
    // Enforce performance limits
    EnforcePerformanceLimits();
}

void UCore_RigidBodySystem::UpdateActiveRigidBodies(float DeltaTime, float CurrentTime)
{
    for (int32 i = ActiveRigidBodies.Num() - 1; i >= 0; --i)
    {
        FCore_RigidBodyData& RigidBodyData = ActiveRigidBodies[i];
        
        // Check if component is still valid
        if (!IsValid(RigidBodyData.Component))
        {
            ActiveRigidBodies.RemoveAt(i);
            continue;
        }
        
        // Update rigid body data
        RigidBodyData.LinearVelocity = RigidBodyData.Component->GetPhysicsLinearVelocity();
        RigidBodyData.AngularVelocity = RigidBodyData.Component->GetPhysicsAngularVelocityInRadians();
        RigidBodyData.LastUpdateTime = CurrentTime;
        
        // Check if rigid body should go to sleep
        float VelocitySquared = RigidBodyData.LinearVelocity.SizeSquared();
        float AngularVelocitySquared = RigidBodyData.AngularVelocity.SizeSquared();
        
        if (VelocitySquared < SleepThreshold * SleepThreshold && AngularVelocitySquared < SleepThreshold * SleepThreshold)
        {
            // Move to sleeping list
            RigidBodyData.bIsAwake = false;
            SleepingRigidBodies.Add(RigidBodyData);
            ActiveRigidBodies.RemoveAt(i);
            
            UE_LOG(LogTemp, VeryVerbose, TEXT("Core_RigidBodySystem: Rigid body %s went to sleep"), *RigidBodyData.Component->GetName());
        }
    }
}

void UCore_RigidBodySystem::CheckSleepingRigidBodies(float CurrentTime)
{
    for (int32 i = SleepingRigidBodies.Num() - 1; i >= 0; --i)
    {
        FCore_RigidBodyData& RigidBodyData = SleepingRigidBodies[i];
        
        // Check if component is still valid
        if (!IsValid(RigidBodyData.Component))
        {
            SleepingRigidBodies.RemoveAt(i);
            continue;
        }
        
        // Check if rigid body should wake up
        FVector CurrentVelocity = RigidBodyData.Component->GetPhysicsLinearVelocity();
        float VelocitySquared = CurrentVelocity.SizeSquared();
        
        if (VelocitySquared > WakeThreshold * WakeThreshold)
        {
            // Move to active list
            RigidBodyData.bIsAwake = true;
            RigidBodyData.LinearVelocity = CurrentVelocity;
            RigidBodyData.AngularVelocity = RigidBodyData.Component->GetPhysicsAngularVelocityInRadians();
            RigidBodyData.LastUpdateTime = CurrentTime;
            
            ActiveRigidBodies.Add(RigidBodyData);
            SleepingRigidBodies.RemoveAt(i);
            
            UE_LOG(LogTemp, VeryVerbose, TEXT("Core_RigidBodySystem: Rigid body %s woke up"), *RigidBodyData.Component->GetName());
        }
    }
}

void UCore_RigidBodySystem::EnforcePerformanceLimits()
{
    // Check if we have too many active rigid bodies
    if (ActiveRigidBodies.Num() > MaxActiveRigidBodies)
    {
        // Sort by distance from player or importance
        // For now, just remove the oldest ones
        int32 ExcessCount = ActiveRigidBodies.Num() - MaxActiveRigidBodies;
        
        for (int32 i = 0; i < ExcessCount; ++i)
        {
            if (ActiveRigidBodies.Num() > 0)
            {
                // Force sleep the oldest rigid body
                FCore_RigidBodyData& RigidBodyData = ActiveRigidBodies[0];
                if (IsValid(RigidBodyData.Component))
                {
                    RigidBodyData.bIsAwake = false;
                    SleepingRigidBodies.Add(RigidBodyData);
                }
                ActiveRigidBodies.RemoveAt(0);
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Core_RigidBodySystem: Enforced performance limit - forced %d rigid bodies to sleep"), ExcessCount);
    }
}

void UCore_RigidBodySystem::SetGlobalGravityScale(float NewGravityScale)
{
    GlobalGravityScale = NewGravityScale;
    
    // Apply to physics world
    if (UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get())
    {
        PhysicsSettings->DefaultGravityZ = -980.0f * GlobalGravityScale;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_RigidBodySystem: Global gravity scale set to %.2f"), NewGravityScale);
}

void UCore_RigidBodySystem::SetGlobalDamping(float LinearDamping, float AngularDamping)
{
    GlobalDamping = LinearDamping;
    GlobalAngularDamping = AngularDamping;
    
    // Apply to all registered rigid bodies
    for (FCore_RigidBodyData& RigidBodyData : ActiveRigidBodies)
    {
        if (IsValid(RigidBodyData.Component))
        {
            ApplyRigidBodySettings(RigidBodyData.Component);
        }
    }
    
    for (FCore_RigidBodyData& RigidBodyData : SleepingRigidBodies)
    {
        if (IsValid(RigidBodyData.Component))
        {
            ApplyRigidBodySettings(RigidBodyData.Component);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_RigidBodySystem: Global damping set to Linear: %.3f, Angular: %.3f"), LinearDamping, AngularDamping);
}

void UCore_RigidBodySystem::SetSleepThresholds(float NewSleepThreshold, float NewWakeThreshold)
{
    SleepThreshold = NewSleepThreshold;
    WakeThreshold = NewWakeThreshold;
    
    UE_LOG(LogTemp, Log, TEXT("Core_RigidBodySystem: Sleep thresholds set to Sleep: %.2f, Wake: %.2f"), NewSleepThreshold, NewWakeThreshold);
}

void UCore_RigidBodySystem::ForceWakeAllRigidBodies()
{
    // Move all sleeping rigid bodies to active list
    for (FCore_RigidBodyData& RigidBodyData : SleepingRigidBodies)
    {
        if (IsValid(RigidBodyData.Component))
        {
            RigidBodyData.bIsAwake = true;
            RigidBodyData.LastUpdateTime = GetWorld()->GetTimeSeconds();
            ActiveRigidBodies.Add(RigidBodyData);
            
            // Wake up the component
            RigidBodyData.Component->WakeAllRigidBodies();
        }
    }
    
    SleepingRigidBodies.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_RigidBodySystem: Forced all rigid bodies to wake up"));
}

void UCore_RigidBodySystem::ForceSleepAllRigidBodies()
{
    // Move all active rigid bodies to sleeping list
    for (FCore_RigidBodyData& RigidBodyData : ActiveRigidBodies)
    {
        if (IsValid(RigidBodyData.Component))
        {
            RigidBodyData.bIsAwake = false;
            SleepingRigidBodies.Add(RigidBodyData);
            
            // Put the component to sleep
            RigidBodyData.Component->PutAllRigidBodiesToSleep();
        }
    }
    
    ActiveRigidBodies.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_RigidBodySystem: Forced all rigid bodies to sleep"));
}

int32 UCore_RigidBodySystem::GetActiveRigidBodyCount() const
{
    return ActiveRigidBodies.Num();
}

int32 UCore_RigidBodySystem::GetSleepingRigidBodyCount() const
{
    return SleepingRigidBodies.Num();
}

int32 UCore_RigidBodySystem::GetTotalRigidBodyCount() const
{
    return ActiveRigidBodies.Num() + SleepingRigidBodies.Num();
}