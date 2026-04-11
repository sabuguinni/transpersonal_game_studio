// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "PhysicsSystemManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "TranspersonalRagdollComponent.h"
#include "TranspersonalDestructionComponent.h"
#include "TranspersonalVehicleComponent.h"

UPhysicsSystemManager::UPhysicsSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize metrics
    CurrentMetrics.ActivePhysicsObjects = 0;
    CurrentMetrics.PhysicsUpdateTime = 0.0f;
    CurrentMetrics.MemoryUsage = 0;
    CurrentMetrics.QualityLevel = PhysicsQualityLevel;
}

void UPhysicsSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Initializing Core Physics Systems"));
    
    // Get world context
    WorldContext = GetWorld();
    if (!WorldContext.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("PhysicsSystemManager: Failed to get valid world context"));
        return;
    }
    
    // Initialize physics system
    if (!InitializePhysicsSystem(WorldContext.Get()))
    {
        UE_LOG(LogTemp, Error, TEXT("PhysicsSystemManager: Failed to initialize physics system"));
        return;
    }
    
    bPhysicsSystemInitialized = true;
    UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Core Physics Systems initialized successfully"));
}

void UPhysicsSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bPhysicsSystemInitialized || !WorldContext.IsValid())
    {
        return;
    }
    
    // Update performance metrics
    UpdatePerformanceMetrics(DeltaTime);
    
    // Apply physics LOD based on performance
    ApplyPhysicsLOD();
    
    // Validate system integrity
    if (!ValidatePhysicsIntegrity())
    {
        UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Physics integrity validation failed"));
    }
    
    // Handle emergency reset cooldown
    if (EmergencyResetCooldown > 0.0f)
    {
        EmergencyResetCooldown -= DeltaTime;
    }
}

void UPhysicsSystemManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Shutting down physics systems"));
    
    // Clear all registered components
    RegisteredRagdolls.Empty();
    RegisteredDestructionComponents.Empty();
    RegisteredVehicles.Empty();
    
    bPhysicsSystemInitialized = false;
    
    Super::EndPlay(EndPlayReason);
}

bool UPhysicsSystemManager::InitializePhysicsSystem(UWorld* InWorld)
{
    if (!InWorld)
    {
        UE_LOG(LogTemp, Error, TEXT("PhysicsSystemManager: Invalid world provided for initialization"));
        return false;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Initializing Chaos Physics integration"));
    
    // Initialize Chaos Physics
    InitializeChaosPhysics();
    
    // Set up physics world
    if (InWorld->GetPhysicsScene())
    {
        UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Physics scene found and ready"));
        
        // Configure physics settings for optimal performance
        UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get();
        if (PhysicsSettings)
        {
            // Optimize for 60fps on PC, 30fps on console
            PhysicsSettings->MaxSubstepDeltaTime = 1.0f / 120.0f; // 120Hz substeps for stability
            PhysicsSettings->MaxSubsteps = 6; // Allow up to 6 substeps
            
            UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Physics settings optimized"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("PhysicsSystemManager: No physics scene available"));
        return false;
    }
    
    // Initialize component arrays
    RegisteredRagdolls.Empty();
    RegisteredDestructionComponents.Empty();
    RegisteredVehicles.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Physics system initialization complete"));
    return true;
}

void UPhysicsSystemManager::RegisterRagdollComponent(UTranspersonalRagdollComponent* RagdollComponent)
{
    if (!RagdollComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Attempted to register null ragdoll component"));
        return;
    }
    
    RegisteredRagdolls.AddUnique(RagdollComponent);
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Registered ragdoll component. Total: %d"), RegisteredRagdolls.Num());
}

void UPhysicsSystemManager::RegisterDestructionComponent(UTranspersonalDestructionComponent* DestructionComponent)
{
    if (!DestructionComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Attempted to register null destruction component"));
        return;
    }
    
    RegisteredDestructionComponents.AddUnique(DestructionComponent);
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Registered destruction component. Total: %d"), RegisteredDestructionComponents.Num());
}

void UPhysicsSystemManager::RegisterVehicleComponent(UTranspersonalVehicleComponent* VehicleComponent)
{
    if (!VehicleComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Attempted to register null vehicle component"));
        return;
    }
    
    RegisteredVehicles.AddUnique(VehicleComponent);
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Registered vehicle component. Total: %d"), RegisteredVehicles.Num());
}

FTranspersonalPhysicsMetrics UPhysicsSystemManager::GetPhysicsMetrics() const
{
    return CurrentMetrics;
}

void UPhysicsSystemManager::SetPhysicsQualityLevel(int32 QualityLevel)
{
    PhysicsQualityLevel = FMath::Clamp(QualityLevel, 0, 3);
    CurrentMetrics.QualityLevel = PhysicsQualityLevel;
    
    // Apply quality settings immediately
    ApplyPhysicsLOD();
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Physics quality level set to %d"), PhysicsQualityLevel);
}

void UPhysicsSystemManager::EmergencyPhysicsReset()
{
    if (EmergencyResetCooldown > 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Emergency reset on cooldown"));
        return;
    }
    
    UE_LOG(LogTemp, Error, TEXT("PhysicsSystemManager: EMERGENCY PHYSICS RESET TRIGGERED"));
    
    if (!WorldContext.IsValid())
    {
        return;
    }
    
    // Reset all physics objects
    for (TActorIterator<AActor> ActorItr(WorldContext.Get()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && !Actor->IsPendingKill())
        {
            TArray<UPrimitiveComponent*> PrimitiveComponents;
            Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
            
            for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
            {
                if (PrimComp && PrimComp->IsSimulatingPhysics())
                {
                    // Reset physics state
                    PrimComp->SetPhysicsLinearVelocity(FVector::ZeroVector);
                    PrimComp->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
                    PrimComp->WakeRigidBody();
                }
            }
        }
    }
    
    // Set cooldown to prevent spam
    EmergencyResetCooldown = 5.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Emergency reset complete"));
}

void UPhysicsSystemManager::InitializeChaosPhysics()
{
    UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Configuring Chaos Physics settings"));
    
    // Get physics settings
    UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get();
    if (!PhysicsSettings)
    {
        UE_LOG(LogTemp, Error, TEXT("PhysicsSystemManager: Could not access physics settings"));
        return;
    }
    
    // Configure Chaos-specific settings for optimal performance
    // These settings are tuned for the prehistoric world with large creatures and destruction
    
    // Set solver iterations for stability vs performance balance
    PhysicsSettings->SolverOptions.SolverIterations = 8; // Higher for stability with large creatures
    PhysicsSettings->SolverOptions.VelocityIterations = 1; // Lower for performance
    
    // Configure collision settings
    PhysicsSettings->DefaultShapeComplexity = ECollisionTraceFlag::CTF_UseSimpleAsComplex;
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Chaos Physics configuration complete"));
}

void UPhysicsSystemManager::UpdatePerformanceMetrics(float DeltaTime)
{
    LastMetricsUpdate += DeltaTime;
    
    if (LastMetricsUpdate < MetricsUpdateInterval)
    {
        return;
    }
    
    LastMetricsUpdate = 0.0f;
    
    // Count active physics objects
    int32 ActiveObjects = 0;
    if (WorldContext.IsValid())
    {
        for (TActorIterator<AActor> ActorItr(WorldContext.Get()); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && !Actor->IsPendingKill())
            {
                TArray<UPrimitiveComponent*> PrimitiveComponents;
                Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
                
                for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
                {
                    if (PrimComp && PrimComp->IsSimulatingPhysics())
                    {
                        ActiveObjects++;
                    }
                }
            }
        }
    }
    
    CurrentMetrics.ActivePhysicsObjects = ActiveObjects;
    CurrentMetrics.PhysicsUpdateTime = DeltaTime * 1000.0f; // Convert to milliseconds
    
    // Estimate memory usage (rough calculation)
    CurrentMetrics.MemoryUsage = ActiveObjects * 1024; // ~1KB per physics object estimate
    
    // Log performance warnings
    if (ActiveObjects > MaxPhysicsObjects)
    {
        UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: High physics object count: %d (max: %d)"), 
               ActiveObjects, MaxPhysicsObjects);
    }
    
    if (CurrentMetrics.PhysicsUpdateTime > 16.67f) // > 60fps threshold
    {
        UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Physics update time high: %.2fms"), 
               CurrentMetrics.PhysicsUpdateTime);
    }
}

void UPhysicsSystemManager::ApplyPhysicsLOD()
{
    if (!WorldContext.IsValid())
    {
        return;
    }
    
    // Get player location for distance-based LOD
    FVector PlayerLocation = FVector::ZeroVector;
    if (APlayerController* PC = WorldContext->GetFirstPlayerController())
    {
        if (APawn* PlayerPawn = PC->GetPawn())
        {
            PlayerLocation = PlayerPawn->GetActorLocation();
        }
    }
    
    // Apply LOD based on quality level and distance
    for (TActorIterator<AActor> ActorItr(WorldContext.Get()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor || Actor->IsPendingKill())
        {
            continue;
        }
        
        float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
        
        // Define LOD distances based on quality level
        float HighDetailDistance = 1000.0f * (PhysicsQualityLevel + 1);
        float MediumDetailDistance = 2000.0f * (PhysicsQualityLevel + 1);
        float LowDetailDistance = 4000.0f * (PhysicsQualityLevel + 1);
        
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
        {
            if (!PrimComp || !PrimComp->IsSimulatingPhysics())
            {
                continue;
            }
            
            // Apply LOD settings based on distance
            if (Distance > LowDetailDistance)
            {
                // Disable physics for very distant objects
                PrimComp->SetSimulatePhysics(false);
            }
            else if (Distance > MediumDetailDistance)
            {
                // Low detail physics
                PrimComp->SetSimulatePhysics(true);
                // Reduce update frequency or complexity here if needed
            }
            else if (Distance > HighDetailDistance)
            {
                // Medium detail physics
                PrimComp->SetSimulatePhysics(true);
            }
            else
            {
                // High detail physics
                PrimComp->SetSimulatePhysics(true);
            }
        }
    }
}

bool UPhysicsSystemManager::ValidatePhysicsIntegrity() const
{
    if (!WorldContext.IsValid())
    {
        return false;
    }
    
    // Check for physics scene
    if (!WorldContext->GetPhysicsScene())
    {
        UE_LOG(LogTemp, Error, TEXT("PhysicsSystemManager: Physics scene is null"));
        return false;
    }
    
    // Validate registered components
    int32 ValidRagdolls = 0;
    for (const TWeakObjectPtr<UTranspersonalRagdollComponent>& RagdollPtr : RegisteredRagdolls)
    {
        if (RagdollPtr.IsValid())
        {
            ValidRagdolls++;
        }
    }
    
    int32 ValidDestruction = 0;
    for (const TWeakObjectPtr<UTranspersonalDestructionComponent>& DestructionPtr : RegisteredDestructionComponents)
    {
        if (DestructionPtr.IsValid())
        {
            ValidDestruction++;
        }
    }
    
    int32 ValidVehicles = 0;
    for (const TWeakObjectPtr<UTranspersonalVehicleComponent>& VehiclePtr : RegisteredVehicles)
    {
        if (VehiclePtr.IsValid())
        {
            ValidVehicles++;
        }
    }
    
    // Log validation results
    if (ValidRagdolls != RegisteredRagdolls.Num() ||
        ValidDestruction != RegisteredDestructionComponents.Num() ||
        ValidVehicles != RegisteredVehicles.Num())
    {
        UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Some registered components are invalid"));
        return false;
    }
    
    return true;
}