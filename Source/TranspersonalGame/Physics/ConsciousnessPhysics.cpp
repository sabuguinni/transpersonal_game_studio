/**
 * @file ConsciousnessPhysics.cpp
 * @brief Implementation of consciousness physics system
 * @author Core Systems Programmer
 */

#include "ConsciousnessPhysics.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Engine/GameViewportClient.h"

// Singleton instance
AConsciousnessPhysicsManager* AConsciousnessPhysicsManager::Instance = nullptr;

// Performance constants
static const float CONSCIOUSNESS_PHYSICS_BUDGET_MS = 2.0f;
static const float LAYER_TRANSITION_SMOOTHING = 0.1f;
static const float ASTRAL_SNAP_DISTANCE = 50.0f;

UConsciousnessPhysicsComponent::UConsciousnessPhysicsComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize default physics state
    PhysicsState.ActiveLayer = EConsciousnessLayer::Physical;
    PhysicsState.TransitionProgress = 0.0f;
    PhysicsState.bIsTransitioning = false;
    
    // Initialize layer velocities
    PhysicsState.LayerVelocities.Add(EConsciousnessLayer::Physical, FVector::ZeroVector);
    PhysicsState.LayerVelocities.Add(EConsciousnessLayer::Emotional, FVector::ZeroVector);
    PhysicsState.LayerVelocities.Add(EConsciousnessLayer::Mental, FVector::ZeroVector);
    PhysicsState.LayerVelocities.Add(EConsciousnessLayer::Spiritual, FVector::ZeroVector);
    PhysicsState.LayerVelocities.Add(EConsciousnessLayer::Astral, FVector::ZeroVector);
}

void UConsciousnessPhysicsComponent::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeConsciousnessPhysics();
    
    // Register with the global manager
    if (AConsciousnessPhysicsManager* Manager = AConsciousnessPhysicsManager::GetInstance(this))
    {
        Manager->RegisterConsciousnessComponent(this);
    }
}

void UConsciousnessPhysicsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Early exit if consciousness physics is disabled globally
    if (AConsciousnessPhysicsManager* Manager = AConsciousnessPhysicsManager::GetInstance(this))
    {
        if (Manager->GetConsciousnessPhysicsFrameTime() > CONSCIOUSNESS_PHYSICS_BUDGET_MS)
        {
            return; // Skip this frame to maintain performance
        }
    }
    
    UpdateLayerTransition(DeltaTime);
    ApplyLayerPhysics(DeltaTime);
    UpdateTemporalEffects(DeltaTime);
    
    if (bAstralProjectionActive)
    {
        ValidateAstralDistance();
    }
}

void UConsciousnessPhysicsComponent::InitializeConsciousnessPhysics()
{
    // Cache physical body location
    if (AActor* Owner = GetOwner())
    {
        PhysicalBodyLocation = Owner->GetActorLocation();
    }
    
    // Initialize layer-specific physics properties
    if (UPrimitiveComponent* PrimComp = GetOwner()->FindComponentByClass<UPrimitiveComponent>())
    {
        // Store original physics settings
        OriginalTimeDilation = PrimComp->GetPhysicsLinearVelocity().Size();
    }
}

void UConsciousnessPhysicsComponent::TransitionToLayer(EConsciousnessLayer TargetLayerParam, float TransitionDurationParam)
{
    if (PhysicsState.ActiveLayer == TargetLayerParam)
    {
        return; // Already in target layer
    }
    
    TargetLayer = TargetLayerParam;
    TransitionDuration = FMath::Max(0.1f, TransitionDurationParam);
    TransitionTimer = 0.0f;
    PhysicsState.bIsTransitioning = true;
    
    UE_LOG(LogTemp, Log, TEXT("Consciousness Physics: Transitioning from %d to %d over %.2f seconds"), 
           (int32)PhysicsState.ActiveLayer, (int32)TargetLayer, TransitionDuration);
}

void UConsciousnessPhysicsComponent::ApplyTemporalDilation(float DilationFactor, float Duration)
{
    PhysicsSettings.TimeDilationFactor = FMath::Clamp(DilationFactor, 0.1f, 10.0f);
    TemporalDilationTimer = Duration;
    
    // Apply to world time dilation if this is the player
    if (GetOwner() && GetOwner()->IsA<APawn>())
    {
        if (UWorld* World = GetWorld())
        {
            World->GetWorldSettings()->SetTimeDilation(PhysicsSettings.TimeDilationFactor);
        }
    }
}

void UConsciousnessPhysicsComponent::EnableAstralProjection(const FVector& AstralStartLocation)
{
    if (AActor* Owner = GetOwner())
    {
        PhysicalBodyLocation = Owner->GetActorLocation();
        bAstralProjectionActive = true;
        
        // Transition to astral layer
        TransitionToLayer(EConsciousnessLayer::Astral, 1.0f);
        
        // Move to astral start location
        Owner->SetActorLocation(AstralStartLocation);
        
        UE_LOG(LogTemp, Log, TEXT("Consciousness Physics: Astral projection enabled. Physical body at %s"), 
               *PhysicalBodyLocation.ToString());
    }
}

void UConsciousnessPhysicsComponent::DisableAstralProjection()
{
    if (bAstralProjectionActive && GetOwner())
    {
        bAstralProjectionActive = false;
        
        // Return to physical body location
        GetOwner()->SetActorLocation(PhysicalBodyLocation);
        
        // Transition back to physical layer
        TransitionToLayer(EConsciousnessLayer::Physical, 1.0f);
        
        UE_LOG(LogTemp, Log, TEXT("Consciousness Physics: Astral projection disabled. Returned to physical body"));
    }
}

void UConsciousnessPhysicsComponent::SetPhysicsSettings(const FConsciousnessPhysicsSettings& NewSettings)
{
    PhysicsSettings = NewSettings;
    
    // Validate settings
    PhysicsSettings.TimeDilationFactor = FMath::Clamp(PhysicsSettings.TimeDilationFactor, 0.1f, 10.0f);
    PhysicsSettings.MaxAstralDistance = FMath::Clamp(PhysicsSettings.MaxAstralDistance, 100.0f, 10000.0f);
    PhysicsSettings.ConsciousnessSubsteps = FMath::Clamp(PhysicsSettings.ConsciousnessSubsteps, 1, 8);
}

void UConsciousnessPhysicsComponent::UpdateLayerTransition(float DeltaTime)
{
    if (!PhysicsState.bIsTransitioning)
    {
        return;
    }
    
    TransitionTimer += DeltaTime;
    PhysicsState.TransitionProgress = FMath::Clamp(TransitionTimer / TransitionDuration, 0.0f, 1.0f);
    
    // Smooth transition curve
    float SmoothProgress = FMath::SmoothStep(0.0f, 1.0f, PhysicsState.TransitionProgress);
    
    if (PhysicsState.TransitionProgress >= 1.0f)
    {
        // Transition complete
        PhysicsState.ActiveLayer = TargetLayer;
        PhysicsState.bIsTransitioning = false;
        PhysicsState.TransitionProgress = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("Consciousness Physics: Transition to layer %d complete"), (int32)TargetLayer);
    }
}

void UConsciousnessPhysicsComponent::ApplyLayerPhysics(float DeltaTime)
{
    if (!GetOwner())
    {
        return;
    }
    
    UPrimitiveComponent* PrimComp = GetOwner()->FindComponentByClass<UPrimitiveComponent>();
    if (!PrimComp || !PrimComp->IsSimulatingPhysics())
    {
        return;
    }
    
    // Calculate effective gravity for current layer
    float GravityMultiplier = CalculateLayerGravity(PhysicsState.ActiveLayer);
    
    if (PhysicsState.bIsTransitioning)
    {
        // Interpolate gravity during transition
        float FromGravity = CalculateLayerGravity(PhysicsState.ActiveLayer);
        float ToGravity = CalculateLayerGravity(TargetLayer);
        GravityMultiplier = FMath::Lerp(FromGravity, ToGravity, PhysicsState.TransitionProgress);
    }
    
    // Apply layer-specific gravity
    FVector CurrentVelocity = PrimComp->GetPhysicsLinearVelocity();
    FVector GravityForce = FVector(0.0f, 0.0f, -980.0f) * GravityMultiplier * PrimComp->GetMass();
    
    // Apply consciousness-specific physics modifications
    switch (PhysicsState.ActiveLayer)
    {
        case EConsciousnessLayer::Physical:
            // Standard physics
            break;
            
        case EConsciousnessLayer::Emotional:
            // Emotional layer: affected by emotional state, more fluid movement
            GravityForce *= 0.8f;
            break;
            
        case EConsciousnessLayer::Mental:
            // Mental layer: thought-based movement, reduced gravity
            GravityForce *= 0.6f;
            break;
            
        case EConsciousnessLayer::Spiritual:
            // Spiritual layer: ethereal movement, minimal gravity
            GravityForce *= 0.3f;
            break;
            
        case EConsciousnessLayer::Astral:
            // Astral layer: free movement, almost no gravity
            GravityForce *= 0.1f;
            break;
    }
    
    PrimComp->AddForce(GravityForce, NAME_None, true);
}

void UConsciousnessPhysicsComponent::UpdateTemporalEffects(float DeltaTime)
{
    if (TemporalDilationTimer > 0.0f)
    {
        TemporalDilationTimer -= DeltaTime;
        
        if (TemporalDilationTimer <= 0.0f)
        {
            // Reset time dilation
            PhysicsSettings.TimeDilationFactor = 1.0f;
            
            if (UWorld* World = GetWorld())
            {
                World->GetWorldSettings()->SetTimeDilation(1.0f);
            }
        }
    }
}

void UConsciousnessPhysicsComponent::ValidateAstralDistance()
{
    if (!bAstralProjectionActive || !GetOwner())
    {
        return;
    }
    
    float DistanceFromBody = FVector::Dist(GetOwner()->GetActorLocation(), PhysicalBodyLocation);
    
    if (DistanceFromBody > PhysicsSettings.MaxAstralDistance)
    {
        // Snap back to maximum distance
        FVector Direction = (GetOwner()->GetActorLocation() - PhysicalBodyLocation).GetSafeNormal();
        FVector ClampedLocation = PhysicalBodyLocation + (Direction * PhysicsSettings.MaxAstralDistance);
        GetOwner()->SetActorLocation(ClampedLocation);
        
        UE_LOG(LogTemp, Warning, TEXT("Consciousness Physics: Astral projection distance exceeded. Clamped to %.2f units"), 
               PhysicsSettings.MaxAstralDistance);
    }
}

float UConsciousnessPhysicsComponent::CalculateLayerGravity(EConsciousnessLayer Layer) const
{
    if (const float* GravityMultiplier = PhysicsSettings.LayerGravityMultipliers.Find(Layer))
    {
        return *GravityMultiplier;
    }
    return 1.0f; // Default to physical gravity
}

FVector UConsciousnessPhysicsComponent::InterpolateLayerVelocity(EConsciousnessLayer FromLayer, EConsciousnessLayer ToLayer, float Alpha) const
{
    const FVector* FromVelocity = PhysicsState.LayerVelocities.Find(FromLayer);
    const FVector* ToVelocity = PhysicsState.LayerVelocities.Find(ToLayer);
    
    if (FromVelocity && ToVelocity)
    {
        return FMath::Lerp(*FromVelocity, *ToVelocity, Alpha);
    }
    
    return FVector::ZeroVector;
}

// ============================================================================
// AConsciousnessPhysicsManager Implementation
// ============================================================================

AConsciousnessPhysicsManager::AConsciousnessPhysicsManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PrePhysics;
    
    // Set as singleton
    if (!Instance)
    {
        Instance = this;
    }
}

void AConsciousnessPhysicsManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Consciousness Physics Manager: Initialized"));
}

void AConsciousnessPhysicsManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    PerformanceTimer = FPlatformTime::Seconds();
    
    if (bConsciousnessPhysicsEnabled)
    {
        UpdateRegisteredComponents(DeltaTime);
    }
    
    MonitorPerformance();
    OptimizePerformance();
}

AConsciousnessPhysicsManager* AConsciousnessPhysicsManager::GetInstance(const UObject* WorldContext)
{
    if (!Instance && WorldContext)
    {
        if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull))
        {
            // Try to find existing instance in world
            for (TActorIterator<AConsciousnessPhysicsManager> ActorItr(World); ActorItr; ++ActorItr)
            {
                Instance = *ActorItr;
                break;
            }
            
            // Create new instance if none found
            if (!Instance)
            {
                FActorSpawnParameters SpawnParams;
                SpawnParams.Name = TEXT("ConsciousnessPhysicsManager");
                Instance = World->SpawnActor<AConsciousnessPhysicsManager>(SpawnParams);
            }
        }
    }
    
    return Instance;
}

void AConsciousnessPhysicsManager::RegisterConsciousnessComponent(UConsciousnessPhysicsComponent* Component)
{
    if (Component && !RegisteredComponents.Contains(Component))
    {
        RegisteredComponents.Add(Component);
        UE_LOG(LogTemp, Log, TEXT("Consciousness Physics Manager: Registered component %s"), 
               Component->GetOwner() ? *Component->GetOwner()->GetName() : TEXT("Unknown"));
    }
}

void AConsciousnessPhysicsManager::UnregisterConsciousnessComponent(UConsciousnessPhysicsComponent* Component)
{
    if (Component)
    {
        RegisteredComponents.Remove(Component);
        UE_LOG(LogTemp, Log, TEXT("Consciousness Physics Manager: Unregistered component %s"), 
               Component->GetOwner() ? *Component->GetOwner()->GetName() : TEXT("Unknown"));
    }
}

void AConsciousnessPhysicsManager::SetGlobalConsciousnessLayer(EConsciousnessLayer Layer)
{
    for (UConsciousnessPhysicsComponent* Component : RegisteredComponents)
    {
        if (IsValid(Component))
        {
            Component->TransitionToLayer(Layer, 2.0f);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Consciousness Physics Manager: Set global layer to %d"), (int32)Layer);
}

void AConsciousnessPhysicsManager::SetConsciousnessPhysicsEnabled(bool bEnabled)
{
    bConsciousnessPhysicsEnabled = bEnabled;
    UE_LOG(LogTemp, Log, TEXT("Consciousness Physics Manager: Physics %s"), 
           bEnabled ? TEXT("Enabled") : TEXT("Disabled"));
}

void AConsciousnessPhysicsManager::UpdateRegisteredComponents(float DeltaTime)
{
    // Remove invalid components
    RegisteredComponents.RemoveAll([](UConsciousnessPhysicsComponent* Component)
    {
        return !IsValid(Component) || !IsValid(Component->GetOwner());
    });
}

void AConsciousnessPhysicsManager::MonitorPerformance()
{
    double CurrentTime = FPlatformTime::Seconds();
    FrameTimeMs = (CurrentTime - PerformanceTimer) * 1000.0f;
}

void AConsciousnessPhysicsManager::OptimizePerformance()
{
    if (FrameTimeMs > MaxFrameTimeMs)
    {
        // Performance optimization: reduce update frequency for distant components
        static int32 FrameCounter = 0;
        FrameCounter++;
        
        // Skip every other frame for components beyond a certain distance
        if (FrameCounter % 2 == 0)
        {
            // Implement distance-based LOD for consciousness physics
            // This is a placeholder for more sophisticated optimization
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Consciousness Physics Manager: Performance optimization triggered. Frame time: %.2fms"), FrameTimeMs);
    }
}