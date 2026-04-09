// Copyright Epic Games, Inc. All Rights Reserved.

#include "CorePhysicsManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/WheeledVehiclePawn.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Physics/PhysicsInterfaceCore.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Chaos/ChaosEngineInterface.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY(LogCorePhysics);

UCorePhysicsManager::UCorePhysicsManager()
{
    CurrentQualityLevel = EPhysicsQualityLevel::Medium;
    bEnableAsyncPhysics = true;
    bEnableSubstepping = true;
    MaxPhysicsFrameTime = 0.033f;
    MaxSubsteps = 6;
    LastPhysicsFrameTime = 0.0f;
    ActiveRigidBodyCount = 0;
    bIsPerformanceMonitoringActive = false;
    bChaosPhysicsInitialized = false;
    bDebugVisualizationEnabled = false;
    PhysicsInterface = nullptr;
}

void UCorePhysicsManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogCorePhysics, Log, TEXT("Initializing Core Physics Manager"));
    
    InitializePhysicsSystem();
    
    // Start cleanup timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            CleanupTimer,
            this,
            &UCorePhysicsManager::CleanupInvalidReferences,
            5.0f,
            true
        );
    }
    
    UE_LOG(LogCorePhysics, Log, TEXT("Core Physics Manager initialized successfully"));
}

void UCorePhysicsManager::Deinitialize()
{
    UE_LOG(LogCorePhysics, Log, TEXT("Deinitializing Core Physics Manager"));
    
    StopPerformanceMonitoring();
    ShutdownPhysicsSystem();
    
    // Clear timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PerformanceMonitorTimer);
        World->GetTimerManager().ClearTimer(CleanupTimer);
    }
    
    // Clear all references
    ManagedActors.Empty();
    RegisteredVehicles.Empty();
    
    Super::Deinitialize();
    
    UE_LOG(LogCorePhysics, Log, TEXT("Core Physics Manager deinitialized"));
}

bool UCorePhysicsManager::ShouldCreateSubsystem(UObject* Outer) const
{
    // Only create in game worlds, not in editor preview worlds
    if (UWorld* World = Cast<UWorld>(Outer))
    {
        return World->IsGameWorld();
    }
    return false;
}

void UCorePhysicsManager::InitializePhysicsSystem()
{
    UE_LOG(LogCorePhysics, Log, TEXT("Initializing physics system"));
    
    // Get physics interface
    PhysicsInterface = FPhysicsInterface::GetPhysicsInterface();
    if (!PhysicsInterface)
    {
        UE_LOG(LogCorePhysics, Error, TEXT("Failed to get physics interface"));
        return;
    }
    
    // Initialize Chaos Physics
    if (FPhysicsInterface::IsValid())
    {
        bChaosPhysicsInitialized = true;
        UE_LOG(LogCorePhysics, Log, TEXT("Chaos Physics interface is valid"));
    }
    else
    {
        UE_LOG(LogCorePhysics, Warning, TEXT("Chaos Physics interface is not valid"));
    }
    
    // Apply initial physics settings
    UpdatePhysicsSettings();
    
    UE_LOG(LogCorePhysics, Log, TEXT("Physics system initialized"));
}

void UCorePhysicsManager::ShutdownPhysicsSystem()
{
    UE_LOG(LogCorePhysics, Log, TEXT("Shutting down physics system"));
    
    StopPerformanceMonitoring();
    
    // Disable physics for all managed actors
    for (auto& ActorPtr : ManagedActors)
    {
        if (AActor* Actor = ActorPtr.Get())
        {
            EnableCollisionForActor(Actor, false);
        }
    }
    
    bChaosPhysicsInitialized = false;
    PhysicsInterface = nullptr;
    
    UE_LOG(LogCorePhysics, Log, TEXT("Physics system shutdown complete"));
}

void UCorePhysicsManager::SetPhysicsQualityLevel(EPhysicsQualityLevel QualityLevel)
{
    if (CurrentQualityLevel == QualityLevel)
    {
        return;
    }
    
    UE_LOG(LogCorePhysics, Log, TEXT("Setting physics quality level to: %d"), (int32)QualityLevel);
    
    CurrentQualityLevel = QualityLevel;
    UpdatePhysicsSettings();
    
    // Notify about quality change
    FPhysicsEventData EventData;
    EventData.EventType = EPhysicsEventType::Wake; // Reusing for quality change
    EventData.TimeStamp = GetWorld()->GetTimeSeconds();
    OnPhysicsEvent.Broadcast(EventData);
}

void UCorePhysicsManager::UpdatePhysicsSettings()
{
    if (!bChaosPhysicsInitialized)
    {
        return;
    }
    
    UE_LOG(LogCorePhysics, Log, TEXT("Updating physics settings for quality level: %d"), (int32)CurrentQualityLevel);
    
    // Get physics settings
    UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get();
    if (!PhysicsSettings)
    {
        UE_LOG(LogCorePhysics, Error, TEXT("Failed to get physics settings"));
        return;
    }
    
    // Apply quality-specific settings
    switch (CurrentQualityLevel)
    {
        case EPhysicsQualityLevel::Low:
            MaxPhysicsFrameTime = 0.05f; // 20fps
            MaxSubsteps = 2;
            bEnableAsyncPhysics = false;
            bEnableSubstepping = false;
            break;
            
        case EPhysicsQualityLevel::Medium:
            MaxPhysicsFrameTime = 0.033f; // 30fps
            MaxSubsteps = 4;
            bEnableAsyncPhysics = true;
            bEnableSubstepping = true;
            break;
            
        case EPhysicsQualityLevel::High:
            MaxPhysicsFrameTime = 0.016f; // 60fps
            MaxSubsteps = 6;
            bEnableAsyncPhysics = true;
            bEnableSubstepping = true;
            break;
            
        case EPhysicsQualityLevel::Ultra:
            MaxPhysicsFrameTime = 0.008f; // 120fps
            MaxSubsteps = 8;
            bEnableAsyncPhysics = true;
            bEnableSubstepping = true;
            break;
    }
    
    UE_LOG(LogCorePhysics, Log, TEXT("Physics settings updated - MaxFrameTime: %f, MaxSubsteps: %d"), 
           MaxPhysicsFrameTime, MaxSubsteps);
}

void UCorePhysicsManager::StartPerformanceMonitoring()
{
    if (bIsPerformanceMonitoringActive)
    {
        return;
    }
    
    UE_LOG(LogCorePhysics, Log, TEXT("Starting physics performance monitoring"));
    
    bIsPerformanceMonitoringActive = true;
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            PerformanceMonitorTimer,
            this,
            &UCorePhysicsManager::MonitorPhysicsPerformance,
            0.1f, // Monitor every 100ms
            true
        );
    }
}

void UCorePhysicsManager::StopPerformanceMonitoring()
{
    if (!bIsPerformanceMonitoringActive)
    {
        return;
    }
    
    UE_LOG(LogCorePhysics, Log, TEXT("Stopping physics performance monitoring"));
    
    bIsPerformanceMonitoringActive = false;
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PerformanceMonitorTimer);
    }
}

void UCorePhysicsManager::MonitorPhysicsPerformance()
{
    if (!bIsPerformanceMonitoringActive)
    {
        return;
    }
    
    // Get current frame time
    float CurrentFrameTime = FApp::GetDeltaTime();
    LastPhysicsFrameTime = CurrentFrameTime;
    
    // Count active rigid bodies
    ActiveRigidBodyCount = 0;
    for (auto& ActorPtr : ManagedActors)
    {
        if (AActor* Actor = ActorPtr.Get())
        {
            if (UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>())
            {
                if (MeshComp->IsSimulatingPhysics())
                {
                    ActiveRigidBodyCount++;
                }
            }
        }
    }
    
    // Auto-adjust quality if performance is poor
    if (CurrentFrameTime > MaxPhysicsFrameTime * 1.5f)
    {
        AdjustQualityBasedOnPerformance();
    }
    
    // Log performance data periodically
    static float LastLogTime = 0.0f;
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastLogTime > 5.0f)
    {
        UE_LOG(LogCorePhysics, Log, TEXT("Physics Performance - FrameTime: %fms, RigidBodies: %d, Quality: %d"),
               CurrentFrameTime * 1000.0f, ActiveRigidBodyCount, (int32)CurrentQualityLevel);
        LastLogTime = CurrentTime;
    }
}

void UCorePhysicsManager::AdjustQualityBasedOnPerformance()
{
    // Only auto-adjust if we're not at the lowest quality
    if (CurrentQualityLevel == EPhysicsQualityLevel::Low)
    {
        return;
    }
    
    UE_LOG(LogCorePhysics, Warning, TEXT("Poor physics performance detected, reducing quality level"));
    
    // Reduce quality level
    int32 NewQualityLevel = FMath::Max(0, (int32)CurrentQualityLevel - 1);
    SetPhysicsQualityLevel((EPhysicsQualityLevel)NewQualityLevel);
}

void UCorePhysicsManager::EnableCollisionForActor(AActor* Actor, bool bEnable)
{
    if (!Actor)
    {
        UE_LOG(LogCorePhysics, Warning, TEXT("EnableCollisionForActor: Invalid actor"));
        return;
    }
    
    // Find static mesh component
    if (UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>())
    {
        if (bEnable)
        {
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            MeshComp->SetNotifyRigidBodyCollision(true);
        }
        else
        {
            MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            MeshComp->SetNotifyRigidBodyCollision(false);
        }
        
        // Add to managed actors if enabling
        if (bEnable)
        {
            ManagedActors.AddUnique(Actor);
        }
        
        UE_LOG(LogCorePhysics, Log, TEXT("Collision %s for actor: %s"), 
               bEnable ? TEXT("enabled") : TEXT("disabled"), *Actor->GetName());
    }
}

void UCorePhysicsManager::SetCollisionResponseForActor(AActor* Actor, ECollisionChannel Channel, ECollisionResponse Response)
{
    if (!Actor)
    {
        return;
    }
    
    if (UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>())
    {
        MeshComp->SetCollisionResponseToChannel(Channel, Response);
        UE_LOG(LogCorePhysics, Log, TEXT("Set collision response for actor: %s, Channel: %d, Response: %d"),
               *Actor->GetName(), (int32)Channel, (int32)Response);
    }
}

void UCorePhysicsManager::EnableDestructionForActor(AActor* Actor, float DamageThreshold)
{
    if (!Actor)
    {
        return;
    }
    
    // Add destruction component if not present
    if (!Actor->FindComponentByClass<UCorePhysicsComponent>())
    {
        UCorePhysicsComponent* PhysicsComp = NewObject<UCorePhysicsComponent>(Actor);
        Actor->AddInstanceComponent(PhysicsComp);
        PhysicsComp->EnableDestruction(DamageThreshold);
        PhysicsComp->RegisterComponent();
        
        UE_LOG(LogCorePhysics, Log, TEXT("Enabled destruction for actor: %s, Threshold: %f"),
               *Actor->GetName(), DamageThreshold);
    }
}

void UCorePhysicsManager::TriggerDestruction(AActor* Actor, const FVector& ImpactLocation, float ImpactForce)
{
    if (!Actor)
    {
        return;
    }
    
    // Create destruction event
    FPhysicsEventData EventData;
    EventData.EventType = EPhysicsEventType::Destruction;
    EventData.ActorA = Actor;
    EventData.ImpactLocation = ImpactLocation;
    EventData.ImpactVelocity = ImpactForce;
    EventData.TimeStamp = GetWorld()->GetTimeSeconds();
    
    OnPhysicsEvent.Broadcast(EventData);
    
    UE_LOG(LogCorePhysics, Log, TEXT("Triggered destruction for actor: %s, Force: %f"),
           *Actor->GetName(), ImpactForce);
}

void UCorePhysicsManager::EnableRagdollForCharacter(ACharacter* Character)
{
    if (!Character)
    {
        return;
    }
    
    if (USkeletalMeshComponent* MeshComp = Character->GetMesh())
    {
        MeshComp->SetSimulatePhysics(true);
        MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        
        UE_LOG(LogCorePhysics, Log, TEXT("Enabled ragdoll for character: %s"), *Character->GetName());
    }
}

void UCorePhysicsManager::DisableRagdollForCharacter(ACharacter* Character)
{
    if (!Character)
    {
        return;
    }
    
    if (USkeletalMeshComponent* MeshComp = Character->GetMesh())
    {
        MeshComp->SetSimulatePhysics(false);
        MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        
        UE_LOG(LogCorePhysics, Log, TEXT("Disabled ragdoll for character: %s"), *Character->GetName());
    }
}

void UCorePhysicsManager::CreateFluidSimulation(const FVector& Location, const FVector& Size)
{
    UE_LOG(LogCorePhysics, Log, TEXT("Creating fluid simulation at location: %s, Size: %s"),
           *Location.ToString(), *Size.ToString());
    
    // Fluid simulation implementation would go here
    // This is a placeholder for the fluid system integration
    
    FPhysicsEventData EventData;
    EventData.EventType = EPhysicsEventType::Wake;
    EventData.ImpactLocation = Location;
    EventData.TimeStamp = GetWorld()->GetTimeSeconds();
    OnPhysicsEvent.Broadcast(EventData);
}

void UCorePhysicsManager::RegisterVehicle(AWheeledVehiclePawn* Vehicle)
{
    if (!Vehicle)
    {
        return;
    }
    
    RegisteredVehicles.AddUnique(Vehicle);
    UE_LOG(LogCorePhysics, Log, TEXT("Registered vehicle: %s"), *Vehicle->GetName());
}

void UCorePhysicsManager::UnregisterVehicle(AWheeledVehiclePawn* Vehicle)
{
    if (!Vehicle)
    {
        return;
    }
    
    RegisteredVehicles.Remove(Vehicle);
    UE_LOG(LogCorePhysics, Log, TEXT("Unregistered vehicle: %s"), *Vehicle->GetName());
}

void UCorePhysicsManager::EnablePhysicsDebugVisualization(bool bEnable)
{
    bDebugVisualizationEnabled = bEnable;
    
    UE_LOG(LogCorePhysics, Log, TEXT("Physics debug visualization %s"), 
           bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UCorePhysicsManager::ValidatePhysicsConfiguration()
{
    UE_LOG(LogCorePhysics, Log, TEXT("Validating physics configuration"));
    
    bool bConfigurationValid = true;
    
    // Check if Chaos Physics is available
    if (!bChaosPhysicsInitialized)
    {
        UE_LOG(LogCorePhysics, Error, TEXT("Chaos Physics is not initialized"));
        bConfigurationValid = false;
    }
    
    // Check physics settings
    if (MaxPhysicsFrameTime <= 0.0f)
    {
        UE_LOG(LogCorePhysics, Error, TEXT("Invalid MaxPhysicsFrameTime: %f"), MaxPhysicsFrameTime);
        bConfigurationValid = false;
    }
    
    if (MaxSubsteps <= 0)
    {
        UE_LOG(LogCorePhysics, Error, TEXT("Invalid MaxSubsteps: %d"), MaxSubsteps);
        bConfigurationValid = false;
    }
    
    UE_LOG(LogCorePhysics, Log, TEXT("Physics configuration validation %s"), 
           bConfigurationValid ? TEXT("passed") : TEXT("failed"));
}

bool UCorePhysicsManager::RunPhysicsIntegrityTest()
{
    UE_LOG(LogCorePhysics, Log, TEXT("Running physics integrity test"));
    
    // Create test actor
    if (UWorld* World = GetWorld())
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = TEXT("PhysicsTestActor");
        
        AActor* TestActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector(0, 0, 1000), FRotator::ZeroRotator, SpawnParams);
        if (TestActor)
        {
            // Add static mesh component
            UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(TestActor);
            TestActor->SetRootComponent(MeshComp);
            MeshComp->RegisterComponent();
            
            // Enable physics
            EnableCollisionForActor(TestActor, true);
            MeshComp->SetSimulatePhysics(true);
            
            UE_LOG(LogCorePhysics, Log, TEXT("Physics integrity test actor created successfully"));
            
            // Clean up after a short delay
            FTimerHandle CleanupTimerHandle;
            World->GetTimerManager().SetTimer(CleanupTimerHandle, [TestActor]()
            {
                if (TestActor && IsValid(TestActor))
                {
                    TestActor->Destroy();
                }
            }, 2.0f, false);
            
            return true;
        }
    }
    
    UE_LOG(LogCorePhysics, Error, TEXT("Physics integrity test failed"));
    return false;
}

void UCorePhysicsManager::CleanupInvalidReferences()
{
    // Clean up invalid actor references
    for (int32 i = ManagedActors.Num() - 1; i >= 0; i--)
    {
        if (!ManagedActors[i].IsValid())
        {
            ManagedActors.RemoveAtSwap(i);
        }
    }
    
    // Clean up invalid vehicle references
    for (int32 i = RegisteredVehicles.Num() - 1; i >= 0; i--)
    {
        if (!RegisteredVehicles[i].IsValid())
        {
            RegisteredVehicles.RemoveAtSwap(i);
        }
    }
}

// UCorePhysicsComponent Implementation

UCorePhysicsComponent::UCorePhysicsComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    bEnablePhysicsSimulation = true;
    Mass = 1.0f;
    LinearDamping = 0.01f;
    AngularDamping = 0.01f;
    bEnableCollision = true;
    CollisionObjectType = ECollisionChannel::ECC_WorldDynamic;
    bEnableDestruction = false;
    DestructionThreshold = 100.0f;
    PhysicsManager = nullptr;
}

void UCorePhysicsComponent::BeginPlay()
{
    Super::BeginPlay();
    
    InitializePhysicsComponent();
    RegisterWithPhysicsManager();
}

void UCorePhysicsComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UnregisterFromPhysicsManager();
    Super::EndPlay(EndPlayReason);
}

void UCorePhysicsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Component-specific physics updates can go here
}

void UCorePhysicsComponent::InitializePhysicsComponent()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }
    
    // Configure physics properties
    if (UStaticMeshComponent* MeshComp = Owner->FindComponentByClass<UStaticMeshComponent>())
    {
        if (bEnablePhysicsSimulation)
        {
            MeshComp->SetSimulatePhysics(true);
            MeshComp->SetMassOverrideInKg(NAME_None, Mass);
            MeshComp->SetLinearDamping(LinearDamping);
            MeshComp->SetAngularDamping(AngularDamping);
        }
        
        if (bEnableCollision)
        {
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            MeshComp->SetCollisionObjectType(CollisionObjectType);
        }
    }
    
    UE_LOG(LogCorePhysics, Log, TEXT("Physics component initialized for actor: %s"), *Owner->GetName());
}

void UCorePhysicsComponent::RegisterWithPhysicsManager()
{
    if (UWorld* World = GetWorld())
    {
        PhysicsManager = World->GetSubsystem<UCorePhysicsManager>();
        if (PhysicsManager && GetOwner())
        {
            PhysicsManager->EnableCollisionForActor(GetOwner(), bEnableCollision);
            
            if (bEnableDestruction)
            {
                PhysicsManager->EnableDestructionForActor(GetOwner(), DestructionThreshold);
            }
        }
    }
}

void UCorePhysicsComponent::UnregisterFromPhysicsManager()
{
    if (PhysicsManager && GetOwner())
    {
        PhysicsManager->EnableCollisionForActor(GetOwner(), false);
    }
    PhysicsManager = nullptr;
}

void UCorePhysicsComponent::EnablePhysicsSimulation(bool bEnable)
{
    bEnablePhysicsSimulation = bEnable;
    
    if (AActor* Owner = GetOwner())
    {
        if (UStaticMeshComponent* MeshComp = Owner->FindComponentByClass<UStaticMeshComponent>())
        {
            MeshComp->SetSimulatePhysics(bEnable);
        }
    }
}

void UCorePhysicsComponent::SetPhysicsProperties(float NewMass, float NewLinearDamping, float NewAngularDamping)
{
    Mass = NewMass;
    LinearDamping = NewLinearDamping;
    AngularDamping = NewAngularDamping;
    
    if (AActor* Owner = GetOwner())
    {
        if (UStaticMeshComponent* MeshComp = Owner->FindComponentByClass<UStaticMeshComponent>())
        {
            MeshComp->SetMassOverrideInKg(NAME_None, Mass);
            MeshComp->SetLinearDamping(LinearDamping);
            MeshComp->SetAngularDamping(AngularDamping);
        }
    }
}

void UCorePhysicsComponent::ApplyForce(const FVector& Force, const FVector& Location)
{
    if (AActor* Owner = GetOwner())
    {
        if (UStaticMeshComponent* MeshComp = Owner->FindComponentByClass<UStaticMeshComponent>())
        {
            if (Location.IsZero())
            {
                MeshComp->AddForce(Force);
            }
            else
            {
                MeshComp->AddForceAtLocation(Force, Location);
            }
        }
    }
}

void UCorePhysicsComponent::ApplyImpulse(const FVector& Impulse, const FVector& Location)
{
    if (AActor* Owner = GetOwner())
    {
        if (UStaticMeshComponent* MeshComp = Owner->FindComponentByClass<UStaticMeshComponent>())
        {
            if (Location.IsZero())
            {
                MeshComp->AddImpulse(Impulse);
            }
            else
            {
                MeshComp->AddImpulseAtLocation(Impulse, Location);
            }
        }
    }
}

void UCorePhysicsComponent::SetCollisionEnabled(bool bEnable)
{
    bEnableCollision = bEnable;
    
    if (AActor* Owner = GetOwner())
    {
        if (UStaticMeshComponent* MeshComp = Owner->FindComponentByClass<UStaticMeshComponent>())
        {
            MeshComp->SetCollisionEnabled(bEnable ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
        }
    }
}

void UCorePhysicsComponent::SetCollisionObjectType(ECollisionChannel ObjectType)
{
    CollisionObjectType = ObjectType;
    
    if (AActor* Owner = GetOwner())
    {
        if (UStaticMeshComponent* MeshComp = Owner->FindComponentByClass<UStaticMeshComponent>())
        {
            MeshComp->SetCollisionObjectType(ObjectType);
        }
    }
}

void UCorePhysicsComponent::EnableDestruction(float DamageThreshold)
{
    bEnableDestruction = true;
    DestructionThreshold = DamageThreshold;
    
    if (PhysicsManager && GetOwner())
    {
        PhysicsManager->EnableDestructionForActor(GetOwner(), DamageThreshold);
    }
}

void UCorePhysicsComponent::TriggerDestruction(float ImpactForce, const FVector& ImpactLocation)
{
    if (bEnableDestruction && PhysicsManager && GetOwner())
    {
        PhysicsManager->TriggerDestruction(GetOwner(), ImpactLocation, ImpactForce);
        
        // Broadcast component event
        FPhysicsEventData EventData;
        EventData.EventType = EPhysicsEventType::Destruction;
        EventData.ActorA = GetOwner();
        EventData.ImpactLocation = ImpactLocation;
        EventData.ImpactVelocity = ImpactForce;
        EventData.TimeStamp = GetWorld()->GetTimeSeconds();
        OnComponentPhysicsEvent.Broadcast(EventData);
    }
}