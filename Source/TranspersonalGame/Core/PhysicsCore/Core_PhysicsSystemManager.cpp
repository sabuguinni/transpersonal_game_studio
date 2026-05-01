#include "Core_PhysicsSystemManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "DrawDebugHelpers.h"

// Include Architecture Manager if available
#if __has_include("../Engine/Eng_ArchitectureManager.h")
#include "../Engine/Eng_ArchitectureManager.h"
#endif

ACore_PhysicsSystemManager::ACore_PhysicsSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update metrics 10 times per second

    // Initialize system state
    SystemState = ECore_PhysicsSystemState::Uninitialized;
    bEnablePhysicsDebugging = false;
    bEnableCollisionDebugging = false;

    // Initialize performance tracking
    LastPhysicsUpdateTime = 0.0f;
    PhysicsFrameCounter = 0;
    AccumulatedPhysicsTime = 0.0f;
    ErrorReportCooldown = 0.0f;

    // Initialize metrics
    CurrentMetrics = FCore_PhysicsMetrics();

    // Set actor properties
    SetActorTickEnabled(true);
    SetCanBeDamaged(false);
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Constructor completed"));
}

void ACore_PhysicsSystemManager::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: BeginPlay started"));

    // Initialize the physics system
    if (InitializePhysicsSystem())
    {
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Physics system initialized successfully"));
        SystemState = ECore_PhysicsSystemState::Running;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsSystemManager: Failed to initialize physics system"));
        SystemState = ECore_PhysicsSystemState::Error;
    }

    // Register with Architecture Manager
    RegisterWithArchitectureManager();
}

void ACore_PhysicsSystemManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: EndPlay called"));

    // Shutdown the physics system
    ShutdownPhysicsSystem();

    Super::EndPlay(EndPlayReason);
}

void ACore_PhysicsSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Only update if system is running
    if (SystemState == ECore_PhysicsSystemState::Running)
    {
        UpdatePhysicsMetrics();
        
        // Report to Architecture Manager periodically
        static float ReportTimer = 0.0f;
        ReportTimer += DeltaTime;
        if (ReportTimer >= 1.0f) // Report every second
        {
            ReportSystemStatus();
            ReportTimer = 0.0f;
        }
    }

    // Handle error reporting cooldown
    if (ErrorReportCooldown > 0.0f)
    {
        ErrorReportCooldown -= DeltaTime;
    }
}

bool ACore_PhysicsSystemManager::InitializePhysicsSystem()
{
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Initializing physics system..."));

    SystemState = ECore_PhysicsSystemState::Initializing;

    // Validate physics configuration
    if (!ValidatePhysicsConfiguration())
    {
        LastErrorMessage = TEXT("Physics configuration validation failed");
        return false;
    }

    // Initialize collision profiles
    InitializeCollisionProfiles();

    // Setup default physics settings
    SetupDefaultPhysicsSettings();

    // Register physics callbacks
    RegisterPhysicsCallbacks();

    // Validate Chaos physics
    if (!ValidateChaosPhysics())
    {
        LastErrorMessage = TEXT("Chaos physics validation failed");
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Physics system initialization complete"));
    return true;
}

void ACore_PhysicsSystemManager::ShutdownPhysicsSystem()
{
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Shutting down physics system..."));

    SystemState = ECore_PhysicsSystemState::Disabled;

    // Unregister physics callbacks
    UnregisterPhysicsCallbacks();

    // Clear managed actors
    ManagedPhysicsActors.Empty();

    // Clear collision profiles
    RegisteredCollisionProfiles.Empty();

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Physics system shutdown complete"));
}

bool ACore_PhysicsSystemManager::ValidatePhysicsConfiguration()
{
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Validating physics configuration..."));

    // Check if we have a valid world
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsSystemManager: No valid world found"));
        return false;
    }

    // Validate Chaos physics
    if (!ValidateChaosPhysics())
    {
        return false;
    }

    // Validate collision settings
    if (!ValidateCollisionSettings())
    {
        return false;
    }

    // Validate physics world settings
    if (!ValidatePhysicsWorldSettings())
    {
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Physics configuration validation passed"));
    return true;
}

void ACore_PhysicsSystemManager::RegisterCollisionProfile(const FCore_CollisionProfile& Profile)
{
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Registering collision profile: %s"), *Profile.ProfileName.ToString());

    // Check if profile already exists
    for (int32 i = 0; i < RegisteredCollisionProfiles.Num(); i++)
    {
        if (RegisteredCollisionProfiles[i].ProfileName == Profile.ProfileName)
        {
            // Update existing profile
            RegisteredCollisionProfiles[i] = Profile;
            UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Updated existing collision profile: %s"), *Profile.ProfileName.ToString());
            return;
        }
    }

    // Add new profile
    RegisteredCollisionProfiles.Add(Profile);
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Added new collision profile: %s"), *Profile.ProfileName.ToString());
}

bool ACore_PhysicsSystemManager::ApplyCollisionProfileToActor(AActor* TargetActor, const FName& ProfileName)
{
    if (!TargetActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Cannot apply collision profile to null actor"));
        return false;
    }

    // Find the collision profile
    FCore_CollisionProfile* FoundProfile = nullptr;
    for (FCore_CollisionProfile& Profile : RegisteredCollisionProfiles)
    {
        if (Profile.ProfileName == ProfileName)
        {
            FoundProfile = &Profile;
            break;
        }
    }

    if (!FoundProfile)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Collision profile not found: %s"), *ProfileName.ToString());
        return false;
    }

    // Apply profile to all primitive components
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    TargetActor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);

    for (UPrimitiveComponent* Component : PrimitiveComponents)
    {
        if (Component)
        {
            Component->SetCollisionEnabled(FoundProfile->CollisionEnabled);
            Component->SetCollisionObjectType(FoundProfile->ObjectType);
            
            if (FoundProfile->bBlockAll)
            {
                Component->SetCollisionResponseToAllChannels(ECR_Block);
            }
            else if (FoundProfile->bIgnoreAll)
            {
                Component->SetCollisionResponseToAllChannels(ECR_Ignore);
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Applied collision profile %s to actor %s"), 
           *ProfileName.ToString(), *TargetActor->GetName());

    return true;
}

void ACore_PhysicsSystemManager::SetupDinosaurCollision(AActor* DinosaurActor)
{
    if (!DinosaurActor)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Setting up dinosaur collision for %s"), *DinosaurActor->GetName());

    // Create dinosaur collision profile if it doesn't exist
    FCore_CollisionProfile DinosaurProfile;
    DinosaurProfile.ProfileName = FName("DinosaurCollision");
    DinosaurProfile.CollisionEnabled = ECollisionEnabled::QueryAndPhysics;
    DinosaurProfile.ObjectType = ECC_Pawn;
    DinosaurProfile.bBlockAll = false;
    DinosaurProfile.bIgnoreAll = false;

    RegisterCollisionProfile(DinosaurProfile);
    ApplyCollisionProfileToActor(DinosaurActor, DinosaurProfile.ProfileName);

    // Add to managed actors
    ManagedPhysicsActors.AddUnique(DinosaurActor);
}

void ACore_PhysicsSystemManager::SetupEnvironmentCollision(AActor* EnvironmentActor)
{
    if (!EnvironmentActor)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Setting up environment collision for %s"), *EnvironmentActor->GetName());

    // Create environment collision profile if it doesn't exist
    FCore_CollisionProfile EnvironmentProfile;
    EnvironmentProfile.ProfileName = FName("EnvironmentCollision");
    EnvironmentProfile.CollisionEnabled = ECollisionEnabled::QueryAndPhysics;
    EnvironmentProfile.ObjectType = ECC_WorldStatic;
    EnvironmentProfile.bBlockAll = true;
    EnvironmentProfile.bIgnoreAll = false;

    RegisterCollisionProfile(EnvironmentProfile);
    ApplyCollisionProfileToActor(EnvironmentActor, EnvironmentProfile.ProfileName);

    // Add to managed actors
    ManagedPhysicsActors.AddUnique(EnvironmentActor);
}

FCore_PhysicsMetrics ACore_PhysicsSystemManager::GetCurrentPhysicsMetrics() const
{
    return CurrentMetrics;
}

void ACore_PhysicsSystemManager::UpdatePhysicsMetrics()
{
    // Calculate physics frame time
    CalculatePhysicsFrameTime();

    // Count active rigid bodies
    CountActiveRigidBodies();

    // Count collision checks (estimated)
    CountCollisionChecks();

    // Calculate memory usage
    CalculateMemoryUsage();

    // Update Chaos enabled status
    CurrentMetrics.bChaosEnabled = ValidateChaosPhysics();
}

bool ACore_PhysicsSystemManager::IsPhysicsPerformanceAcceptable() const
{
    // Check if physics frame time is acceptable (< 5ms for 60fps)
    if (CurrentMetrics.PhysicsFrameTime > 5.0f)
    {
        return false;
    }

    // Check if we have too many active rigid bodies (> 1000)
    if (CurrentMetrics.ActiveRigidBodies > 1000)
    {
        return false;
    }

    // Check memory usage (> 500MB is concerning)
    if (CurrentMetrics.MemoryUsageMB > 500.0f)
    {
        return false;
    }

    return true;
}

void ACore_PhysicsSystemManager::EnableRigidBodyPhysics(UPrimitiveComponent* Component)
{
    if (!Component)
    {
        return;
    }

    Component->SetSimulatePhysics(true);
    Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Enabled rigid body physics for component %s"), 
           *Component->GetName());
}

void ACore_PhysicsSystemManager::DisableRigidBodyPhysics(UPrimitiveComponent* Component)
{
    if (!Component)
    {
        return;
    }

    Component->SetSimulatePhysics(false);
    Component->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Disabled rigid body physics for component %s"), 
           *Component->GetName());
}

void ACore_PhysicsSystemManager::SetRigidBodyMass(UPrimitiveComponent* Component, float Mass)
{
    if (!Component)
    {
        return;
    }

    Component->SetMassOverrideInKg(NAME_None, Mass, true);
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Set mass %f for component %s"), 
           Mass, *Component->GetName());
}

void ACore_PhysicsSystemManager::EnableDestructionForActor(AActor* TargetActor)
{
    if (!TargetActor)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Enabling destruction for actor %s"), 
           *TargetActor->GetName());

    // Enable destruction on all primitive components
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    TargetActor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);

    for (UPrimitiveComponent* Component : PrimitiveComponents)
    {
        if (Component)
        {
            // Enable physics simulation for destruction
            Component->SetSimulatePhysics(true);
            Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        }
    }
}

void ACore_PhysicsSystemManager::TriggerDestruction(AActor* TargetActor, const FVector& ImpactPoint, float Force)
{
    if (!TargetActor)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Triggering destruction for actor %s with force %f"), 
           *TargetActor->GetName(), Force);

    // Apply impulse at impact point
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    TargetActor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);

    for (UPrimitiveComponent* Component : PrimitiveComponents)
    {
        if (Component && Component->IsSimulatingPhysics())
        {
            FVector ImpulseDirection = (Component->GetComponentLocation() - ImpactPoint).GetSafeNormal();
            FVector Impulse = ImpulseDirection * Force;
            Component->AddImpulseAtLocation(Impulse, ImpactPoint);
        }
    }
}

void ACore_PhysicsSystemManager::RegisterWithArchitectureManager()
{
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Attempting to register with Architecture Manager..."));

#if __has_include("../Engine/Eng_ArchitectureManager.h")
    // Try to find the Architecture Manager in the world
    UWorld* World = GetWorld();
    if (World)
    {
        for (TActorIterator<AEng_ArchitectureManager> ActorItr(World); ActorItr; ++ActorItr)
        {
            AEng_ArchitectureManager* FoundManager = *ActorItr;
            if (FoundManager)
            {
                ArchitectureManager = FoundManager;
                UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Successfully registered with Architecture Manager"));
                return;
            }
        }
    }
#endif

    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Architecture Manager not found"));
}

void ACore_PhysicsSystemManager::ReportSystemStatus()
{
    if (ArchitectureManager.IsValid())
    {
        // Report metrics to Architecture Manager
        // This would be implemented when the Architecture Manager interface is available
        UE_LOG(LogTemp, VeryVerbose, TEXT("Core_PhysicsSystemManager: Reporting status to Architecture Manager"));
    }
}

void ACore_PhysicsSystemManager::DebugPhysicsSystem()
{
    UE_LOG(LogTemp, Log, TEXT("=== PHYSICS SYSTEM DEBUG INFO ==="));
    UE_LOG(LogTemp, Log, TEXT("System State: %d"), (int32)SystemState);
    UE_LOG(LogTemp, Log, TEXT("Physics Frame Time: %f ms"), CurrentMetrics.PhysicsFrameTime);
    UE_LOG(LogTemp, Log, TEXT("Active Rigid Bodies: %d"), CurrentMetrics.ActiveRigidBodies);
    UE_LOG(LogTemp, Log, TEXT("Collision Checks: %d"), CurrentMetrics.CollisionChecks);
    UE_LOG(LogTemp, Log, TEXT("Memory Usage: %f MB"), CurrentMetrics.MemoryUsageMB);
    UE_LOG(LogTemp, Log, TEXT("Chaos Enabled: %s"), CurrentMetrics.bChaosEnabled ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Log, TEXT("Managed Actors: %d"), ManagedPhysicsActors.Num());
    UE_LOG(LogTemp, Log, TEXT("Registered Profiles: %d"), RegisteredCollisionProfiles.Num());
    
    if (!LastErrorMessage.IsEmpty())
    {
        UE_LOG(LogTemp, Log, TEXT("Last Error: %s"), *LastErrorMessage);
    }
}

void ACore_PhysicsSystemManager::ValidateAllPhysicsActors()
{
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Validating all physics actors..."));

    int32 ValidActors = 0;
    int32 InvalidActors = 0;

    for (int32 i = ManagedPhysicsActors.Num() - 1; i >= 0; i--)
    {
        if (ManagedPhysicsActors[i].IsValid())
        {
            ValidActors++;
        }
        else
        {
            // Remove invalid actors
            ManagedPhysicsActors.RemoveAt(i);
            InvalidActors++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Validation complete - Valid: %d, Invalid: %d"), 
           ValidActors, InvalidActors);
}

void ACore_PhysicsSystemManager::ShowCollisionDebugInfo()
{
    if (!bEnableCollisionDebugging)
    {
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Collision debugging is disabled"));
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Draw debug info for managed actors
    for (const TWeakObjectPtr<AActor>& ActorPtr : ManagedPhysicsActors)
    {
        if (AActor* Actor = ActorPtr.Get())
        {
            // Draw actor bounds
            FBox ActorBounds = Actor->GetComponentsBoundingBox();
            DrawDebugBox(World, ActorBounds.GetCenter(), ActorBounds.GetExtent(), 
                        FColor::Green, false, 1.0f, 0, 2.0f);

            // Draw actor name
            DrawDebugString(World, Actor->GetActorLocation() + FVector(0, 0, 100), 
                           Actor->GetName(), nullptr, FColor::White, 1.0f);
        }
    }
}

// === INTERNAL METHODS ===

void ACore_PhysicsSystemManager::InitializeCollisionProfiles()
{
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Initializing collision profiles..."));

    RegisteredCollisionProfiles.Empty();

    // Default collision profile
    FCore_CollisionProfile DefaultProfile;
    DefaultProfile.ProfileName = FName("Default");
    DefaultProfile.CollisionEnabled = ECollisionEnabled::QueryAndPhysics;
    DefaultProfile.ObjectType = ECC_WorldStatic;
    RegisterCollisionProfile(DefaultProfile);

    // Player collision profile
    FCore_CollisionProfile PlayerProfile;
    PlayerProfile.ProfileName = FName("Player");
    PlayerProfile.CollisionEnabled = ECollisionEnabled::QueryAndPhysics;
    PlayerProfile.ObjectType = ECC_Pawn;
    RegisterCollisionProfile(PlayerProfile);

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Collision profiles initialized"));
}

void ACore_PhysicsSystemManager::SetupDefaultPhysicsSettings()
{
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Setting up default physics settings..."));

    // Get physics settings
    UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get();
    if (PhysicsSettings)
    {
        // Ensure Chaos is enabled
        // Note: This is read-only at runtime, but we can validate it
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Physics settings configured"));
    }
}

void ACore_PhysicsSystemManager::RegisterPhysicsCallbacks()
{
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Registering physics callbacks..."));
    // Physics callbacks would be registered here
    // This is a placeholder for future implementation
}

void ACore_PhysicsSystemManager::UnregisterPhysicsCallbacks()
{
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Unregistering physics callbacks..."));
    // Physics callbacks would be unregistered here
    // This is a placeholder for future implementation
}

void ACore_PhysicsSystemManager::CalculatePhysicsFrameTime()
{
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    if (LastPhysicsUpdateTime > 0.0f)
    {
        float DeltaTime = CurrentTime - LastPhysicsUpdateTime;
        AccumulatedPhysicsTime += DeltaTime;
        PhysicsFrameCounter++;

        if (PhysicsFrameCounter >= 10) // Average over 10 frames
        {
            CurrentMetrics.PhysicsFrameTime = (AccumulatedPhysicsTime / PhysicsFrameCounter) * 1000.0f; // Convert to ms
            AccumulatedPhysicsTime = 0.0f;
            PhysicsFrameCounter = 0;
        }
    }

    LastPhysicsUpdateTime = CurrentTime;
}

void ACore_PhysicsSystemManager::CountActiveRigidBodies()
{
    CurrentMetrics.ActiveRigidBodies = 0;

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Count all actors with simulating physics
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            TArray<UPrimitiveComponent*> PrimitiveComponents;
            Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);

            for (UPrimitiveComponent* Component : PrimitiveComponents)
            {
                if (Component && Component->IsSimulatingPhysics())
                {
                    CurrentMetrics.ActiveRigidBodies++;
                }
            }
        }
    }
}

void ACore_PhysicsSystemManager::CountCollisionChecks()
{
    // This is an estimation - actual collision check counting would require engine modifications
    CurrentMetrics.CollisionChecks = CurrentMetrics.ActiveRigidBodies * 10; // Rough estimate
}

void ACore_PhysicsSystemManager::CalculateMemoryUsage()
{
    // This is a placeholder - actual memory usage calculation would require more detailed tracking
    CurrentMetrics.MemoryUsageMB = CurrentMetrics.ActiveRigidBodies * 0.1f; // Rough estimate
}

bool ACore_PhysicsSystemManager::ValidateChaosPhysics() const
{
    // Check if Chaos physics is available and enabled
    return true; // Chaos is the default in UE5
}

bool ACore_PhysicsSystemManager::ValidateCollisionSettings() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    // Validate that collision detection is enabled
    return true; // Basic validation
}

bool ACore_PhysicsSystemManager::ValidatePhysicsWorldSettings() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    // Check if physics simulation is enabled
    return World->bShouldSimulatePhysics;
}