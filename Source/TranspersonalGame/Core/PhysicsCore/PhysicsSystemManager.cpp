#include "PhysicsSystemManager.h"
#include "AdvancedCollisionComponent.h"
#include "RagdollPhysicsComponent.h"
#include "DestructionComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Kismet/GameplayStatics.h"

// Initialize static instance
UPhysicsSystemManager* UPhysicsSystemManager::Instance = nullptr;

UPhysicsSystemManager::UPhysicsSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize default settings
    bPhysicsOptimizationsEnabled = true;
    MaxPhysicsObjectsPerFrame = 500;
    TargetPhysicsFrameRate = 60.0f;
    CurrentPhysicsFrameTime = 0.0f;
    
    // Reserve space for performance history
    PhysicsFrameTimeHistory.Reserve(60); // 1 second of history at 60fps
}

void UPhysicsSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Set singleton instance
    Instance = this;
    
    // Initialize physics systems
    InitializePhysicsSystems();
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Systems initialized successfully"));
}

void UPhysicsSystemManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Shutdown physics systems
    ShutdownPhysicsSystems();
    
    // Clear singleton instance
    if (Instance == this)
    {
        Instance = nullptr;
    }
    
    Super::EndPlay(EndPlayReason);
}

void UPhysicsSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update performance metrics
    UpdatePerformanceMetrics(DeltaTime);
    
    // Apply optimizations if enabled
    if (bPhysicsOptimizationsEnabled)
    {
        ApplyPhysicsOptimizations();
    }
}

void UPhysicsSystemManager::InitializePhysicsSystems()
{
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Initializing physics systems..."));
    
    // Clear existing registrations
    RegisteredCollisionComponents.Empty();
    RegisteredRagdollComponents.Empty();
    RegisteredDestructionComponents.Empty();
    
    // Configure physics settings for optimal performance
    if (UPhysicsSettings* PhysicsSettings = GetMutableDefault<UPhysicsSettings>())
    {
        // Set up physics solver settings for better performance
        PhysicsSettings->DefaultGravityZ = -980.0f; // Realistic gravity
        PhysicsSettings->DefaultTerminalVelocity = 4000.0f;
        
        // Optimize collision detection
        PhysicsSettings->bEnableShapeSharing = true;
        PhysicsSettings->bEnablePCM = true; // Persistent Contact Manifolds
        PhysicsSettings->bEnableStabilization = true;
        
        // Set reasonable physics substep settings
        PhysicsSettings->bSubstepping = true;
        PhysicsSettings->bSubsteppingAsync = true;
        PhysicsSettings->MaxSubstepDeltaTime = 0.016667f; // 60fps
        PhysicsSettings->MaxSubsteps = 6;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Physics systems initialized"));
}

void UPhysicsSystemManager::ShutdownPhysicsSystems()
{
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Shutting down physics systems..."));
    
    // Unregister all components
    RegisteredCollisionComponents.Empty();
    RegisteredRagdollComponents.Empty();
    RegisteredDestructionComponents.Empty();
    
    // Clear performance history
    PhysicsFrameTimeHistory.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Physics systems shutdown complete"));
}

void UPhysicsSystemManager::RegisterCollisionComponent(UAdvancedCollisionComponent* CollisionComponent)
{
    if (CollisionComponent && !RegisteredCollisionComponents.Contains(CollisionComponent))
    {
        RegisteredCollisionComponents.Add(CollisionComponent);
        UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Registered collision component: %s"), 
               *CollisionComponent->GetName());
    }
}

void UPhysicsSystemManager::UnregisterCollisionComponent(UAdvancedCollisionComponent* CollisionComponent)
{
    if (CollisionComponent)
    {
        RegisteredCollisionComponents.Remove(CollisionComponent);
        UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Unregistered collision component: %s"), 
               *CollisionComponent->GetName());
    }
}

void UPhysicsSystemManager::RegisterRagdollComponent(URagdollPhysicsComponent* RagdollComponent)
{
    if (RagdollComponent && !RegisteredRagdollComponents.Contains(RagdollComponent))
    {
        RegisteredRagdollComponents.Add(RagdollComponent);
        UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Registered ragdoll component: %s"), 
               *RagdollComponent->GetName());
    }
}

void UPhysicsSystemManager::UnregisterRagdollComponent(URagdollPhysicsComponent* RagdollComponent)
{
    if (RagdollComponent)
    {
        RegisteredRagdollComponents.Remove(RagdollComponent);
        UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Unregistered ragdoll component: %s"), 
               *RagdollComponent->GetName());
    }
}

void UPhysicsSystemManager::RegisterDestructionComponent(UDestructionComponent* DestructionComponent)
{
    if (DestructionComponent && !RegisteredDestructionComponents.Contains(DestructionComponent))
    {
        RegisteredDestructionComponents.Add(DestructionComponent);
        UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Registered destruction component: %s"), 
               *DestructionComponent->GetName());
    }
}

void UPhysicsSystemManager::UnregisterDestructionComponent(UDestructionComponent* DestructionComponent)
{
    if (DestructionComponent)
    {
        RegisteredDestructionComponents.Remove(DestructionComponent);
        UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Unregistered destruction component: %s"), 
               *DestructionComponent->GetName());
    }
}

FString UPhysicsSystemManager::GetPhysicsPerformanceMetrics() const
{
    float AverageFrameTime = 0.0f;
    if (PhysicsFrameTimeHistory.Num() > 0)
    {
        for (float FrameTime : PhysicsFrameTimeHistory)
        {
            AverageFrameTime += FrameTime;
        }
        AverageFrameTime /= PhysicsFrameTimeHistory.Num();
    }
    
    float AveragePhysicsFPS = AverageFrameTime > 0.0f ? 1.0f / AverageFrameTime : 0.0f;
    
    return FString::Printf(TEXT("Physics Performance:\n")
                          TEXT("  Collision Components: %d\n")
                          TEXT("  Ragdoll Components: %d\n")
                          TEXT("  Destruction Components: %d\n")
                          TEXT("  Average Physics FPS: %.1f\n")
                          TEXT("  Current Frame Time: %.3fms\n")
                          TEXT("  Optimizations Enabled: %s"),
                          RegisteredCollisionComponents.Num(),
                          RegisteredRagdollComponents.Num(),
                          RegisteredDestructionComponents.Num(),
                          AveragePhysicsFPS,
                          CurrentPhysicsFrameTime * 1000.0f,
                          bPhysicsOptimizationsEnabled ? TEXT("Yes") : TEXT("No"));
}

void UPhysicsSystemManager::SetPhysicsOptimizationsEnabled(bool bEnable)
{
    bPhysicsOptimizationsEnabled = bEnable;
    UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Physics optimizations %s"), 
           bEnable ? TEXT("enabled") : TEXT("disabled"));
}

UPhysicsSystemManager* UPhysicsSystemManager::GetPhysicsSystemManager(UWorld* World)
{
    if (!World)
    {
        return nullptr;
    }
    
    // Return cached instance if available
    if (Instance && IsValid(Instance))
    {
        return Instance;
    }
    
    // Find existing instance in world
    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (UPhysicsSystemManager* PhysicsManager = Actor->FindComponentByClass<UPhysicsSystemManager>())
        {
            Instance = PhysicsManager;
            return Instance;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: No instance found in world"));
    return nullptr;
}

void UPhysicsSystemManager::UpdatePerformanceMetrics(float DeltaTime)
{
    CurrentPhysicsFrameTime = DeltaTime;
    
    // Add to history
    PhysicsFrameTimeHistory.Add(DeltaTime);
    
    // Keep only last 60 frames (1 second at 60fps)
    if (PhysicsFrameTimeHistory.Num() > 60)
    {
        PhysicsFrameTimeHistory.RemoveAt(0);
    }
}

void UPhysicsSystemManager::ApplyPhysicsOptimizations()
{
    // Calculate current load
    int32 TotalPhysicsObjects = RegisteredCollisionComponents.Num() + 
                               RegisteredRagdollComponents.Num() + 
                               RegisteredDestructionComponents.Num();
    
    // If we're over the limit, apply optimizations
    if (TotalPhysicsObjects > MaxPhysicsObjectsPerFrame)
    {
        // Reduce physics update frequency for distant objects
        float DistanceThreshold = 2000.0f; // 20 meters
        
        // Get player location for distance calculations
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (PlayerPawn)
        {
            FVector PlayerLocation = PlayerPawn->GetActorLocation();
            
            // Optimize collision components
            for (UAdvancedCollisionComponent* CollisionComp : RegisteredCollisionComponents)
            {
                if (CollisionComp && CollisionComp->GetOwner())
                {
                    float Distance = FVector::Dist(PlayerLocation, CollisionComp->GetOwner()->GetActorLocation());
                    if (Distance > DistanceThreshold)
                    {
                        // Reduce update frequency for distant objects
                        CollisionComp->SetComponentTickEnabled(false);
                    }
                    else
                    {
                        CollisionComp->SetComponentTickEnabled(true);
                    }
                }
            }
        }
    }
}