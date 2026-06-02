#include "Core_PhysicsIntegrationSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Chaos/ChaosEngineInterface.h"

DEFINE_LOG_CATEGORY_STATIC(LogPhysicsIntegration, Log, All);

UCore_PhysicsIntegrationSystem::UCore_PhysicsIntegrationSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.016f; // 60 FPS
    
    // Initialize physics integration settings
    bEnableAdvancedPhysics = true;
    bEnableFluidDynamics = false;
    bEnableDestructibleMeshes = true;
    bEnableRagdollPhysics = true;
    bEnableVehiclePhysics = false;
    
    // Performance settings
    MaxPhysicsObjects = 5000;
    PhysicsTickRate = 60.0f;
    CollisionComplexity = ECollisionEnabled::QueryAndPhysics;
    
    // Integration thresholds
    VelocityThreshold = 1000.0f;
    AngularVelocityThreshold = 50.0f;
    ForceThreshold = 10000.0f;
    
    // Initialize counters
    ActivePhysicsObjects = 0;
    PhysicsCalculationsPerSecond = 0;
    LastCalculationCount = 0;
    LastCalculationTime = 0.0f;
}

void UCore_PhysicsIntegrationSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogPhysicsIntegration, Log, TEXT("Physics Integration System initialized"));
    
    // Initialize physics subsystems
    InitializePhysicsSubsystems();
    
    // Start performance monitoring
    GetWorld()->GetTimerManager().SetTimer(
        PerformanceTimerHandle,
        this,
        &UCore_PhysicsIntegrationSystem::UpdatePerformanceMetrics,
        1.0f,
        true
    );
}

void UCore_PhysicsIntegrationSystem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clean up timers
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(PerformanceTimerHandle);
    }
    
    // Shutdown physics subsystems
    ShutdownPhysicsSubsystems();
    
    Super::EndPlay(EndPlayReason);
}

void UCore_PhysicsIntegrationSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update physics integration
    UpdatePhysicsIntegration(DeltaTime);
    
    // Monitor physics performance
    MonitorPhysicsPerformance(DeltaTime);
    
    // Validate physics constraints
    ValidatePhysicsConstraints();
}

void UCore_PhysicsIntegrationSystem::InitializePhysicsSubsystems()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogPhysicsIntegration, Error, TEXT("Failed to get world for physics initialization"));
        return;
    }
    
    // Initialize Chaos Physics
    if (bEnableAdvancedPhysics)
    {
        InitializeChaosPhysics();
    }
    
    // Initialize fluid dynamics if enabled
    if (bEnableFluidDynamics)
    {
        InitializeFluidDynamics();
    }
    
    // Initialize destructible mesh system
    if (bEnableDestructibleMeshes)
    {
        InitializeDestructibleSystem();
    }
    
    // Initialize ragdoll physics
    if (bEnableRagdollPhysics)
    {
        InitializeRagdollSystem();
    }
    
    // Initialize vehicle physics
    if (bEnableVehiclePhysics)
    {
        InitializeVehiclePhysics();
    }
    
    UE_LOG(LogPhysicsIntegration, Log, TEXT("Physics subsystems initialized successfully"));
}

void UCore_PhysicsIntegrationSystem::InitializeChaosPhysics()
{
    // Configure Chaos Physics settings
    UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get();
    if (PhysicsSettings)
    {
        // Set physics tick rate
        PhysicsSettings->MaxPhysicsDeltaTime = 1.0f / PhysicsTickRate;
        PhysicsSettings->bSubstepping = true;
        PhysicsSettings->MaxSubstepDeltaTime = 0.016f;
        PhysicsSettings->MaxSubsteps = 6;
        
        UE_LOG(LogPhysicsIntegration, Log, TEXT("Chaos Physics configured: TickRate=%.1f"), PhysicsTickRate);
    }
}

void UCore_PhysicsIntegrationSystem::InitializeFluidDynamics()
{
    // Initialize fluid simulation parameters
    FluidDensity = 1000.0f; // Water density
    FluidViscosity = 0.001f; // Water viscosity
    FluidPressure = 101325.0f; // Atmospheric pressure
    
    UE_LOG(LogPhysicsIntegration, Log, TEXT("Fluid dynamics initialized"));
}

void UCore_PhysicsIntegrationSystem::InitializeDestructibleSystem()
{
    // Configure destructible mesh settings
    DestructionImpulseThreshold = 1000.0f;
    MaxDestructibleChunks = 100;
    DestructionLifetime = 30.0f;
    
    UE_LOG(LogPhysicsIntegration, Log, TEXT("Destructible system initialized"));
}

void UCore_PhysicsIntegrationSystem::InitializeRagdollSystem()
{
    // Configure ragdoll physics settings
    RagdollBlendWeight = 1.0f;
    RagdollStiffness = 0.8f;
    RagdollDamping = 0.1f;
    
    UE_LOG(LogPhysicsIntegration, Log, TEXT("Ragdoll system initialized"));
}

void UCore_PhysicsIntegrationSystem::InitializeVehiclePhysics()
{
    // Configure vehicle physics settings
    VehicleMass = 1500.0f; // kg
    VehicleDragCoefficient = 0.3f;
    VehicleDownforce = 100.0f;
    
    UE_LOG(LogPhysicsIntegration, Log, TEXT("Vehicle physics initialized"));
}

void UCore_PhysicsIntegrationSystem::UpdatePhysicsIntegration(float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Count active physics objects
    ActivePhysicsObjects = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor || Actor->IsPendingKill())
        {
            continue;
        }
        
        // Check for physics components
        TArray<UPrimitiveComponent*> PhysicsComponents;
        Actor->GetComponents<UPrimitiveComponent>(PhysicsComponents);
        
        for (UPrimitiveComponent* Component : PhysicsComponents)
        {
            if (Component && Component->IsSimulatingPhysics())
            {
                ActivePhysicsObjects++;
                
                // Apply physics constraints
                ApplyPhysicsConstraints(Component);
                
                // Update physics calculations counter
                PhysicsCalculationsPerSecond++;
            }
        }
    }
    
    // Check performance limits
    if (ActivePhysicsObjects > MaxPhysicsObjects)
    {
        UE_LOG(LogPhysicsIntegration, Warning, 
            TEXT("Physics object limit exceeded: %d/%d"), 
            ActivePhysicsObjects, MaxPhysicsObjects);
    }
}

void UCore_PhysicsIntegrationSystem::ApplyPhysicsConstraints(UPrimitiveComponent* Component)
{
    if (!Component || !Component->GetBodyInstance())
    {
        return;
    }
    
    FBodyInstance* BodyInstance = Component->GetBodyInstance();
    
    // Get current velocity
    FVector LinearVelocity = BodyInstance->GetUnrealWorldVelocity();
    FVector AngularVelocity = BodyInstance->GetUnrealWorldAngularVelocityInRadians();
    
    // Apply velocity constraints
    if (LinearVelocity.Size() > VelocityThreshold)
    {
        FVector ClampedVelocity = LinearVelocity.GetClampedToMaxSize(VelocityThreshold);
        BodyInstance->SetLinearVelocity(ClampedVelocity, false);
        
        UE_LOG(LogPhysicsIntegration, VeryVerbose, 
            TEXT("Velocity clamped for %s: %.1f -> %.1f"), 
            *Component->GetName(), LinearVelocity.Size(), ClampedVelocity.Size());
    }
    
    // Apply angular velocity constraints
    if (AngularVelocity.Size() > AngularVelocityThreshold)
    {
        FVector ClampedAngularVelocity = AngularVelocity.GetClampedToMaxSize(AngularVelocityThreshold);
        BodyInstance->SetAngularVelocityInRadians(ClampedAngularVelocity, false);
    }
}

void UCore_PhysicsIntegrationSystem::MonitorPhysicsPerformance(float DeltaTime)
{
    // Update frame time tracking
    CurrentFrameTime = DeltaTime;
    AverageFrameTime = (AverageFrameTime * 0.9f) + (DeltaTime * 0.1f);
    
    // Check for performance issues
    if (DeltaTime > 0.033f) // 30 FPS threshold
    {
        PerformanceWarningCount++;
        
        if (PerformanceWarningCount > 60) // 1 second of poor performance
        {
            UE_LOG(LogPhysicsIntegration, Warning, 
                TEXT("Physics performance degraded: %.3fms frame time, %d active objects"), 
                DeltaTime * 1000.0f, ActivePhysicsObjects);
            
            // Reset counter
            PerformanceWarningCount = 0;
        }
    }
    else
    {
        PerformanceWarningCount = 0;
    }
}

void UCore_PhysicsIntegrationSystem::ValidatePhysicsConstraints()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Validate physics objects every 60 frames
    static int32 ValidationCounter = 0;
    ValidationCounter++;
    
    if (ValidationCounter >= 60)
    {
        ValidationCounter = 0;
        
        // Check for physics anomalies
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (!Actor || Actor->IsPendingKill())
            {
                continue;
            }
            
            // Check actor location validity
            FVector Location = Actor->GetActorLocation();
            if (!Location.IsNearlyZero() && (FMath::Abs(Location.X) > 1000000.0f || 
                FMath::Abs(Location.Y) > 1000000.0f || 
                FMath::Abs(Location.Z) > 1000000.0f))
            {
                UE_LOG(LogPhysicsIntegration, Warning, 
                    TEXT("Actor %s has extreme location: %s"), 
                    *Actor->GetName(), *Location.ToString());
                
                // Reset to safe location
                Actor->SetActorLocation(FVector::ZeroVector);
            }
        }
    }
}

void UCore_PhysicsIntegrationSystem::UpdatePerformanceMetrics()
{
    // Calculate physics calculations per second
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float TimeDelta = CurrentTime - LastCalculationTime;
    
    if (TimeDelta > 0.0f)
    {
        int32 CalculationsDelta = PhysicsCalculationsPerSecond - LastCalculationCount;
        float CalculationsRate = CalculationsDelta / TimeDelta;
        
        UE_LOG(LogPhysicsIntegration, VeryVerbose, 
            TEXT("Physics Performance: %d objects, %.1f calc/s, %.2fms avg frame"), 
            ActivePhysicsObjects, CalculationsRate, AverageFrameTime * 1000.0f);
        
        LastCalculationCount = PhysicsCalculationsPerSecond;
        LastCalculationTime = CurrentTime;
    }
}

void UCore_PhysicsIntegrationSystem::ShutdownPhysicsSubsystems()
{
    UE_LOG(LogPhysicsIntegration, Log, TEXT("Shutting down physics subsystems"));
    
    // Reset all physics settings to defaults
    bEnableAdvancedPhysics = false;
    bEnableFluidDynamics = false;
    bEnableDestructibleMeshes = false;
    bEnableRagdollPhysics = false;
    bEnableVehiclePhysics = false;
    
    // Clear performance counters
    ActivePhysicsObjects = 0;
    PhysicsCalculationsPerSecond = 0;
    PerformanceWarningCount = 0;
}

bool UCore_PhysicsIntegrationSystem::IsPhysicsSystemHealthy() const
{
    // Check if physics system is operating within normal parameters
    return (ActivePhysicsObjects <= MaxPhysicsObjects) && 
           (AverageFrameTime < 0.033f) && 
           (PerformanceWarningCount < 30);
}

void UCore_PhysicsIntegrationSystem::SetPhysicsQuality(ECore_PhysicsQuality Quality)
{
    PhysicsQuality = Quality;
    
    switch (Quality)
    {
        case ECore_PhysicsQuality::Low:
            PhysicsTickRate = 30.0f;
            MaxPhysicsObjects = 2000;
            VelocityThreshold = 500.0f;
            break;
            
        case ECore_PhysicsQuality::Medium:
            PhysicsTickRate = 60.0f;
            MaxPhysicsObjects = 5000;
            VelocityThreshold = 1000.0f;
            break;
            
        case ECore_PhysicsQuality::High:
            PhysicsTickRate = 120.0f;
            MaxPhysicsObjects = 10000;
            VelocityThreshold = 2000.0f;
            break;
            
        case ECore_PhysicsQuality::Ultra:
            PhysicsTickRate = 240.0f;
            MaxPhysicsObjects = 20000;
            VelocityThreshold = 5000.0f;
            break;
    }
    
    UE_LOG(LogPhysicsIntegration, Log, 
        TEXT("Physics quality set to %d: TickRate=%.1f, MaxObjects=%d"), 
        (int32)Quality, PhysicsTickRate, MaxPhysicsObjects);
}

FCore_PhysicsStats UCore_PhysicsIntegrationSystem::GetPhysicsStats() const
{
    FCore_PhysicsStats Stats;
    Stats.ActivePhysicsObjects = ActivePhysicsObjects;
    Stats.PhysicsCalculationsPerSecond = PhysicsCalculationsPerSecond;
    Stats.AverageFrameTime = AverageFrameTime;
    Stats.CurrentFrameTime = CurrentFrameTime;
    Stats.bIsSystemHealthy = IsPhysicsSystemHealthy();
    Stats.PhysicsQuality = PhysicsQuality;
    
    return Stats;
}