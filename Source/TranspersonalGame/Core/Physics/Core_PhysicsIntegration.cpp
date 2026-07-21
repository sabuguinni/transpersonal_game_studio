#include "Core_PhysicsIntegration.h"
#include "Core_PhysicsCore.h"
#include "Core/Engine/Eng_SystemRegistry.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Engine/Engine.h"

UCore_PhysicsIntegration::UCore_PhysicsIntegration()
{
    PhysicsCore = nullptr;
    SystemRegistry = nullptr;
    PhysicsStatus = ECore_SystemStatus::Uninitialized;
    PhysicsQualityLevel = 1.0f;
    TimeSinceLastUpdate = 0.0f;
    PhysicsUpdateFrequency = 0.033f; // 30Hz default
    bIsInitialized = false;
    bIsShuttingDown = false;
    PerformanceTimer = 0.0f;
    LastPhysicsTime = 0.0f;

    // Initialize metrics
    CurrentMetrics.PhysicsTime = 0.0f;
    CurrentMetrics.CollisionChecks = 0;
    CurrentMetrics.ActiveRigidBodies = 0;
    CurrentMetrics.MemoryUsage = 0.0f;
    CurrentMetrics.QualityLevel = PhysicsQualityLevel;
}

void UCore_PhysicsIntegration::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegration: Initializing physics integration subsystem"));

    // Get SystemRegistry reference
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        SystemRegistry = GameInstance->GetSubsystem<UEng_SystemRegistry>();
        if (!SystemRegistry)
        {
            UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsIntegration: SystemRegistry not found"));
            PhysicsStatus = ECore_SystemStatus::Error;
            return;
        }
    }

    // Register with SystemRegistry
    if (!RegisterWithSystemRegistry())
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsIntegration: Failed to register with SystemRegistry"));
        PhysicsStatus = ECore_SystemStatus::Error;
        return;
    }

    PhysicsStatus = ECore_SystemStatus::Initializing;
    bIsInitialized = true;

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegration: Physics integration subsystem initialized successfully"));
}

void UCore_PhysicsIntegration::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegration: Deinitializing physics integration subsystem"));

    bIsShuttingDown = true;
    ShutdownPhysicsSystems();

    PhysicsCore = nullptr;
    SystemRegistry = nullptr;
    PhysicsStatus = ECore_SystemStatus::Uninitialized;

    Super::Deinitialize();
}

bool UCore_PhysicsIntegration::InitializePhysicsSystems()
{
    if (!bIsInitialized || bIsShuttingDown)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsIntegration: Cannot initialize - subsystem not ready"));
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegration: Initializing physics systems"));

    PhysicsStatus = ECore_SystemStatus::Initializing;

    // Get or create PhysicsCore
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        PhysicsCore = GameInstance->GetSubsystem<UCore_PhysicsCore>();
        if (!PhysicsCore)
        {
            UE_LOG(LogTemp, Error, TEXT("Core_PhysicsIntegration: PhysicsCore subsystem not found"));
            PhysicsStatus = ECore_SystemStatus::Error;
            return false;
        }
    }

    // Validate system dependencies
    if (!ValidateSystemDependencies())
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsIntegration: System dependency validation failed"));
        PhysicsStatus = ECore_SystemStatus::Error;
        return false;
    }

    // Setup physics profiles
    SetupPhysicsProfiles();

    // Initialize performance monitoring
    PerformanceTimer = 0.0f;
    LastPhysicsTime = 0.0f;
    CurrentMetrics.QualityLevel = PhysicsQualityLevel;

    PhysicsStatus = ECore_SystemStatus::Running;

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegration: Physics systems initialized successfully"));
    return true;
}

void UCore_PhysicsIntegration::ShutdownPhysicsSystems()
{
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegration: Shutting down physics systems"));

    PhysicsStatus = ECore_SystemStatus::ShuttingDown;

    // Reset references
    PhysicsCore = nullptr;

    PhysicsStatus = ECore_SystemStatus::Uninitialized;

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegration: Physics systems shutdown complete"));
}

ECore_SystemStatus UCore_PhysicsIntegration::GetPhysicsSystemStatus() const
{
    return PhysicsStatus;
}

bool UCore_PhysicsIntegration::RegisterWithSystemRegistry()
{
    if (!SystemRegistry)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsIntegration: SystemRegistry not available for registration"));
        return false;
    }

    // Register as a core physics system
    FCore_SystemInfo PhysicsSystemInfo;
    PhysicsSystemInfo.SystemName = TEXT("PhysicsIntegration");
    PhysicsSystemInfo.SystemType = ECore_SystemType::Physics;
    PhysicsSystemInfo.Priority = 100; // High priority for physics
    PhysicsSystemInfo.Status = PhysicsStatus;
    PhysicsSystemInfo.bIsCore = true;

    bool bRegistered = SystemRegistry->RegisterSystem(PhysicsSystemInfo);
    if (bRegistered)
    {
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegration: Successfully registered with SystemRegistry"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsIntegration: Failed to register with SystemRegistry"));
    }

    return bRegistered;
}

void UCore_PhysicsIntegration::ApplySurvivalPhysicsProfile(AActor* Character)
{
    if (!Character || !PhysicsCore)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsIntegration: Invalid character or PhysicsCore for survival profile"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegration: Applying survival physics profile to %s"), *Character->GetName());

    // Apply character physics profile through PhysicsCore
    PhysicsCore->ApplyPhysicsProfile(Character, ECore_PhysicsProfile::Character);

    // Configure survival-specific physics
    if (UPrimitiveComponent* PrimComp = Character->GetRootComponent<UPrimitiveComponent>())
    {
        // Set realistic mass for human character
        PrimComp->SetMassOverrideInKg(NAME_None, 70.0f, true);
        
        // Configure collision for survival interactions
        PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        PrimComp->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
        
        // Enable physics simulation for ragdoll capability
        PrimComp->SetSimulatePhysics(false); // Start with kinematic, enable for ragdoll
    }

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegration: Survival physics profile applied successfully"));
}

void UCore_PhysicsIntegration::ApplyDinosaurPhysicsProfile(AActor* Dinosaur, ECore_DinosaurSize DinosaurSize)
{
    if (!Dinosaur || !PhysicsCore)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsIntegration: Invalid dinosaur or PhysicsCore for dinosaur profile"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegration: Applying dinosaur physics profile to %s"), *Dinosaur->GetName());

    // Apply dinosaur physics profile through PhysicsCore
    PhysicsCore->ApplyPhysicsProfile(Dinosaur, ECore_PhysicsProfile::Dinosaur);

    // Configure size-specific physics
    if (UPrimitiveComponent* PrimComp = Dinosaur->GetRootComponent<UPrimitiveComponent>())
    {
        float Mass = 100.0f; // Default mass
        
        // Adjust mass based on dinosaur size
        switch (DinosaurSize)
        {
            case ECore_DinosaurSize::Small:
                Mass = 50.0f; // Compsognathus, small raptors
                break;
            case ECore_DinosaurSize::Medium:
                Mass = 500.0f; // Velociraptors, Dilophosaurus
                break;
            case ECore_DinosaurSize::Large:
                Mass = 2000.0f; // Allosaurus, Carnotaurus
                break;
            case ECore_DinosaurSize::Massive:
                Mass = 8000.0f; // T-Rex, Brachiosaurus
                break;
        }
        
        PrimComp->SetMassOverrideInKg(NAME_None, Mass, true);
        
        // Configure collision for dinosaur interactions
        PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        PrimComp->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
    }

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegration: Dinosaur physics profile applied successfully"));
}

void UCore_PhysicsIntegration::ConfigureTerrainPhysics(AActor* TerrainActor, ECore_TerrainType TerrainType)
{
    if (!TerrainActor || !PhysicsCore)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsIntegration: Invalid terrain actor or PhysicsCore"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegration: Configuring terrain physics for %s"), *TerrainActor->GetName());

    // Apply environment physics profile
    PhysicsCore->ApplyPhysicsProfile(TerrainActor, ECore_PhysicsProfile::Environment);

    // Configure terrain-specific physics properties
    if (UPrimitiveComponent* PrimComp = TerrainActor->GetRootComponent<UPrimitiveComponent>())
    {
        // Set terrain as static
        PrimComp->SetMobility(EComponentMobility::Static);
        PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        PrimComp->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
        
        // Configure surface properties based on terrain type
        switch (TerrainType)
        {
            case ECore_TerrainType::Grass:
                // Soft, slightly bouncy surface
                break;
            case ECore_TerrainType::Rock:
                // Hard, low friction surface
                break;
            case ECore_TerrainType::Mud:
                // High friction, energy absorbing surface
                break;
            case ECore_TerrainType::Sand:
                // Medium friction, slightly unstable surface
                break;
            case ECore_TerrainType::Water:
                // Special water physics handling
                break;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegration: Terrain physics configured successfully"));
}

void UCore_PhysicsIntegration::EnableSurvivalRagdoll(AActor* Character, float ImpactForce)
{
    if (!Character || !PhysicsCore)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsIntegration: Invalid character or PhysicsCore for ragdoll"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegration: Enabling survival ragdoll for %s with force %f"), *Character->GetName(), ImpactForce);

    // Apply ragdoll physics profile
    PhysicsCore->ApplyPhysicsProfile(Character, ECore_PhysicsProfile::Ragdoll);

    // Enable ragdoll physics on skeletal mesh
    if (ACharacter* CharacterPawn = Cast<ACharacter>(Character))
    {
        if (USkeletalMeshComponent* SkelMesh = CharacterPawn->GetMesh())
        {
            // Enable ragdoll physics
            SkelMesh->SetSimulatePhysics(true);
            SkelMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            
            // Apply impact force if specified
            if (ImpactForce > 0.0f)
            {
                FVector ImpactDirection = FVector(0, 0, 1); // Default upward impact
                SkelMesh->AddImpulse(ImpactDirection * ImpactForce, NAME_None, true);
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegration: Survival ragdoll enabled successfully"));
}

void UCore_PhysicsIntegration::ConfigureProjectilePhysics(AActor* Projectile, float Mass, float Drag)
{
    if (!Projectile || !PhysicsCore)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsIntegration: Invalid projectile or PhysicsCore"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegration: Configuring projectile physics for %s"), *Projectile->GetName());

    // Apply projectile physics profile
    PhysicsCore->ApplyPhysicsProfile(Projectile, ECore_PhysicsProfile::Projectile);

    // Configure projectile-specific physics
    if (UPrimitiveComponent* PrimComp = Projectile->GetRootComponent<UPrimitiveComponent>())
    {
        PrimComp->SetMassOverrideInKg(NAME_None, Mass, true);
        PrimComp->SetSimulatePhysics(true);
        PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        PrimComp->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
        
        // Configure drag for realistic projectile motion
        PrimComp->SetLinearDamping(Drag);
        PrimComp->SetAngularDamping(Drag * 0.5f);
    }

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegration: Projectile physics configured successfully"));
}

void UCore_PhysicsIntegration::UpdatePhysicsQuality(float DeltaTime)
{
    if (!bIsInitialized || bIsShuttingDown)
    {
        return;
    }

    TimeSinceLastUpdate += DeltaTime;
    PerformanceTimer += DeltaTime;

    // Update performance metrics
    if (PerformanceTimer >= PhysicsUpdateFrequency)
    {
        UpdatePerformanceMetrics(DeltaTime);
        PerformanceTimer = 0.0f;
    }

    // Adjust physics quality based on performance
    if (CurrentMetrics.PhysicsTime > 16.0f) // If physics takes more than 16ms
    {
        PhysicsQualityLevel = FMath::Max(0.5f, PhysicsQualityLevel - 0.1f);
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsIntegration: Reducing physics quality to %f"), PhysicsQualityLevel);
    }
    else if (CurrentMetrics.PhysicsTime < 8.0f) // If physics takes less than 8ms
    {
        PhysicsQualityLevel = FMath::Min(1.0f, PhysicsQualityLevel + 0.05f);
    }

    CurrentMetrics.QualityLevel = PhysicsQualityLevel;
}

FCore_PhysicsMetrics UCore_PhysicsIntegration::GetPhysicsMetrics() const
{
    return CurrentMetrics;
}

bool UCore_PhysicsIntegration::ValidatePhysicsIntegrity()
{
    if (!bIsInitialized || bIsShuttingDown)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsIntegration: Cannot validate - subsystem not ready"));
        return false;
    }

    // Validate PhysicsCore availability
    if (!PhysicsCore)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsIntegration: PhysicsCore reference is null"));
        return false;
    }

    // Validate SystemRegistry integration
    if (!SystemRegistry)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsIntegration: SystemRegistry reference is null"));
        return false;
    }

    // Check physics system status
    if (PhysicsStatus != ECore_SystemStatus::Running)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsIntegration: Physics system not running (Status: %d)"), (int32)PhysicsStatus);
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegration: Physics integrity validation passed"));
    return true;
}

void UCore_PhysicsIntegration::SetupPhysicsProfiles()
{
    if (!PhysicsCore)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsIntegration: Cannot setup physics profiles - PhysicsCore not available"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegration: Setting up physics profiles"));

    // Physics profiles are managed by PhysicsCore
    // This method can be extended to configure integration-specific profile settings
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegration: Physics profiles setup complete"));
}

bool UCore_PhysicsIntegration::ValidateSystemDependencies() const
{
    // Check PhysicsCore availability
    if (!PhysicsCore)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsIntegration: PhysicsCore dependency not satisfied"));
        return false;
    }

    // Check SystemRegistry availability
    if (!SystemRegistry)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsIntegration: SystemRegistry dependency not satisfied"));
        return false;
    }

    // Check world availability
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsIntegration: World dependency not satisfied"));
        return false;
    }

    return true;
}

void UCore_PhysicsIntegration::UpdatePerformanceMetrics(float DeltaTime)
{
    // Update physics timing
    CurrentMetrics.PhysicsTime = LastPhysicsTime;
    
    // Count active rigid bodies in the world
    CurrentMetrics.ActiveRigidBodies = 0;
    CurrentMetrics.CollisionChecks = 0;
    
    if (UWorld* World = GetWorld())
    {
        // Count physics actors
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->GetRootComponent())
            {
                if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
                {
                    if (PrimComp->IsSimulatingPhysics())
                    {
                        CurrentMetrics.ActiveRigidBodies++;
                    }
                }
            }
        }
    }

    // Estimate memory usage (simplified)
    CurrentMetrics.MemoryUsage = CurrentMetrics.ActiveRigidBodies * 0.1f; // 0.1MB per rigid body estimate
    
    // Update quality level
    CurrentMetrics.QualityLevel = PhysicsQualityLevel;
}