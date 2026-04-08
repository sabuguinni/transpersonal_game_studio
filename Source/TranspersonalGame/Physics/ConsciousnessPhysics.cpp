/**
 * @file ConsciousnessPhysics.cpp
 * @brief Implementation of consciousness-based physics system
 * @author Core Systems Programmer
 * @version 1.0
 */

#include "ConsciousnessPhysics.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/WorldSettings.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// Performance profiling macros
#define CONSCIOUSNESS_PHYSICS_SCOPE() SCOPE_CYCLE_COUNTER(STAT_ConsciousnessPhysics)
DECLARE_CYCLE_STAT(TEXT("Consciousness Physics"), STAT_ConsciousnessPhysics, STATGROUP_Game);

UConsciousnessPhysicsComponent::UConsciousnessPhysicsComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics; // Tick before physics update
    
    // Initialize default layer parameters
    InitializeLayerDefaults();
}

void UConsciousnessPhysicsComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache the primitive component for physics manipulation
    CachedPrimitiveComponent = GetOwner()->FindComponentByClass<UPrimitiveComponent>();
    
    if (CachedPrimitiveComponent)
    {
        // Store original physics settings
        if (CachedPrimitiveComponent->GetBodyInstance())
        {
            OriginalGravityScale = CachedPrimitiveComponent->GetBodyInstance()->GetGravityScale();
        }
        OriginalCollisionResponse = CachedPrimitiveComponent->GetCollisionResponseToChannel(ECC_WorldStatic);
        
        // Apply initial layer settings
        UpdatePhysicsForCurrentLayer();
    }
    
    // Register with global manager
    if (AConsciousnessPhysicsManager* Manager = Cast<AConsciousnessPhysicsManager>(
        UGameplayStatics::GetActorOfClass(GetWorld(), AConsciousnessPhysicsManager::StaticClass())))
    {
        Manager->RegisterComponent(this);
    }
}

void UConsciousnessPhysicsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    CONSCIOUSNESS_PHYSICS_SCOPE();
    
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Process layer transitions
    if (bIsTransitioning)
    {
        ProcessLayerTransition(DeltaTime);
    }
    
    // Debug visualization in development builds
    #if WITH_EDITOR
    if (CVarShowConsciousnessDebug.GetValueOnGameThread())
    {
        FVector ActorLocation = GetOwner()->GetActorLocation();
        FColor LayerColor = GetLayerDebugColor(CurrentLayer);
        DrawDebugSphere(GetWorld(), ActorLocation, 50.0f, 12, LayerColor, false, 0.0f, 0, 2.0f);
    }
    #endif
}

bool UConsciousnessPhysicsComponent::TransitionToLayer(EConsciousnessLayer TargetLayer, bool bInstant)
{
    // Validate transition
    if (!bCanTransitionLayers || TargetLayer == CurrentLayer)
    {
        return false;
    }
    
    // Check if we can transition to target layer
    if (!CanInteractWithLayer(TargetLayer))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot transition to layer %d - interaction not allowed"), (int32)TargetLayer);
        return false;
    }
    
    EConsciousnessLayer PreviousLayer = CurrentLayer;
    
    if (bInstant)
    {
        // Instant transition
        CurrentLayer = TargetLayer;
        UpdatePhysicsForCurrentLayer();
        OnLayerTransition.Broadcast(PreviousLayer, CurrentLayer);
    }
    else
    {
        // Gradual transition
        bIsTransitioning = true;
        TransitionTarget = TargetLayer;
        TransitionProgress = 0.0f;
    }
    
    return true;
}

FVector UConsciousnessPhysicsComponent::GetEffectiveGravity() const
{
    if (LayerParams.Contains(CurrentLayer))
    {
        const FConsciousnessPhysicsParams& Params = LayerParams[CurrentLayer];
        FVector WorldGravity = GetWorld()->GetGravityZ() * FVector::UpVector;
        return WorldGravity * Params.GravityMultiplier;
    }
    
    return GetWorld()->GetGravityZ() * FVector::UpVector;
}

float UConsciousnessPhysicsComponent::GetCurrentTimeDilation() const
{
    if (LayerParams.Contains(CurrentLayer))
    {
        return LayerParams[CurrentLayer].TimeDilation;
    }
    return 1.0f;
}

bool UConsciousnessPhysicsComponent::CanInteractWithLayer(EConsciousnessLayer TargetLayer) const
{
    // Physical layer can interact with all layers
    if (CurrentLayer == EConsciousnessLayer::Physical)
    {
        return true;
    }
    
    // Adjacent layers can interact
    int32 CurrentLayerIndex = (int32)CurrentLayer;
    int32 TargetLayerIndex = (int32)TargetLayer;
    
    return FMath::Abs(CurrentLayerIndex - TargetLayerIndex) <= 1;
}

void UConsciousnessPhysicsComponent::ApplyConsciousnessForce(const FVector& Force, bool bAccelChange)
{
    if (!CachedPrimitiveComponent || !CachedPrimitiveComponent->GetBodyInstance())
    {
        return;
    }
    
    // Modify force based on current consciousness layer
    FVector ModifiedForce = Force;
    if (LayerParams.Contains(CurrentLayer))
    {
        const FConsciousnessPhysicsParams& Params = LayerParams[CurrentLayer];
        ModifiedForce *= Params.GravityMultiplier; // Use gravity multiplier as force modifier
    }
    
    // Apply the force
    CachedPrimitiveComponent->AddForce(ModifiedForce, NAME_None, bAccelChange);
}

void UConsciousnessPhysicsComponent::UpdatePhysicsForCurrentLayer()
{
    if (!CachedPrimitiveComponent)
    {
        return;
    }
    
    if (LayerParams.Contains(CurrentLayer))
    {
        ApplyLayerPhysicsSettings(LayerParams[CurrentLayer]);
    }
}

void UConsciousnessPhysicsComponent::ProcessLayerTransition(float DeltaTime)
{
    TransitionProgress += DeltaTime / TransitionDuration;
    
    if (TransitionProgress >= 1.0f)
    {
        // Transition complete
        EConsciousnessLayer PreviousLayer = CurrentLayer;
        CurrentLayer = TransitionTarget;
        bIsTransitioning = false;
        TransitionProgress = 0.0f;
        
        UpdatePhysicsForCurrentLayer();
        OnLayerTransition.Broadcast(PreviousLayer, CurrentLayer);
    }
    else
    {
        // Interpolate physics properties during transition
        if (LayerParams.Contains(CurrentLayer) && LayerParams.Contains(TransitionTarget))
        {
            const FConsciousnessPhysicsParams& FromParams = LayerParams[CurrentLayer];
            const FConsciousnessPhysicsParams& ToParams = LayerParams[TransitionTarget];
            
            FConsciousnessPhysicsParams InterpolatedParams;
            InterpolatedParams.GravityMultiplier = FMath::Lerp(FromParams.GravityMultiplier, ToParams.GravityMultiplier, TransitionProgress);
            InterpolatedParams.TimeDilation = FMath::Lerp(FromParams.TimeDilation, ToParams.TimeDilation, TransitionProgress);
            
            ApplyLayerPhysicsSettings(InterpolatedParams);
        }
    }
}

void UConsciousnessPhysicsComponent::ApplyLayerPhysicsSettings(const FConsciousnessPhysicsParams& Params)
{
    if (!CachedPrimitiveComponent)
    {
        return;
    }
    
    // Apply gravity scaling
    if (CachedPrimitiveComponent->GetBodyInstance())
    {
        CachedPrimitiveComponent->GetBodyInstance()->SetGravityScale(OriginalGravityScale * Params.GravityMultiplier);
    }
    
    // Apply collision response
    CachedPrimitiveComponent->SetCollisionResponseToChannel(ECC_WorldStatic, Params.CollisionResponse);
    
    // Apply physics material if specified
    if (Params.LayerPhysicsMaterial)
    {
        CachedPrimitiveComponent->SetPhysMaterialOverride(Params.LayerPhysicsMaterial);
    }
    
    // Broadcast time dilation change
    OnTemporalShift.Broadcast(Params.TimeDilation);
}

void UConsciousnessPhysicsComponent::InitializeLayerDefaults()
{
    // Physical layer - normal physics
    FConsciousnessPhysicsParams PhysicalParams;
    PhysicalParams.GravityMultiplier = 1.0f;
    PhysicalParams.TimeDilation = 1.0f;
    PhysicalParams.CollisionResponse = ECR_Block;
    LayerParams.Add(EConsciousnessLayer::Physical, PhysicalParams);
    
    // Etheric layer - reduced gravity, slight time dilation
    FConsciousnessPhysicsParams EthericParams;
    EthericParams.GravityMultiplier = 0.7f;
    EthericParams.TimeDilation = 0.9f;
    EthericParams.CollisionResponse = ECR_Block;
    LayerParams.Add(EConsciousnessLayer::Etheric, EthericParams);
    
    // Astral layer - very low gravity, significant time dilation
    FConsciousnessPhysicsParams AstralParams;
    AstralParams.GravityMultiplier = 0.3f;
    AstralParams.TimeDilation = 0.5f;
    AstralParams.CollisionResponse = ECR_Overlap;
    LayerParams.Add(EConsciousnessLayer::Astral, AstralParams);
    
    // Mental layer - no gravity, extreme time dilation
    FConsciousnessPhysicsParams MentalParams;
    MentalParams.GravityMultiplier = 0.0f;
    MentalParams.TimeDilation = 0.2f;
    MentalParams.CollisionResponse = ECR_Ignore;
    LayerParams.Add(EConsciousnessLayer::Mental, MentalParams);
    
    // Causal layer - inverted gravity, time acceleration
    FConsciousnessPhysicsParams CausalParams;
    CausalParams.GravityMultiplier = -0.5f;
    CausalParams.TimeDilation = 2.0f;
    CausalParams.CollisionResponse = ECR_Ignore;
    LayerParams.Add(EConsciousnessLayer::Causal, CausalParams);
}

// Manager Implementation
AConsciousnessPhysicsManager::AConsciousnessPhysicsManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update every 100ms for performance
    
    // Initialize layer visibility
    LayerVisibility.Add(EConsciousnessLayer::Physical, true);
    LayerVisibility.Add(EConsciousnessLayer::Etheric, true);
    LayerVisibility.Add(EConsciousnessLayer::Astral, true);
    LayerVisibility.Add(EConsciousnessLayer::Mental, false); // Hidden by default
    LayerVisibility.Add(EConsciousnessLayer::Causal, false); // Hidden by default
}

void AConsciousnessPhysicsManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Consciousness Physics Manager initialized"));
}

void AConsciousnessPhysicsManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update performance tracking
    LastFrameTime = DeltaTime * 1000.0f; // Convert to milliseconds
    
    // Update tracked components list
    UpdateTrackedComponents();
    
    // Optimize performance if needed
    if (LastFrameTime > PerformanceBudgetMS)
    {
        OptimizePerformance();
    }
}

TArray<AActor*> AConsciousnessPhysicsManager::GetActorsInLayer(EConsciousnessLayer Layer) const
{
    TArray<AActor*> ActorsInLayer;
    
    for (const UConsciousnessPhysicsComponent* Component : TrackedComponents)
    {
        if (Component && Component->CurrentLayer == Layer)
        {
            ActorsInLayer.Add(Component->GetOwner());
        }
    }
    
    return ActorsInLayer;
}

void AConsciousnessPhysicsManager::ForceAllToPhysicalLayer()
{
    UE_LOG(LogTemp, Warning, TEXT("Emergency: Forcing all actors to physical layer"));
    
    for (UConsciousnessPhysicsComponent* Component : TrackedComponents)
    {
        if (Component && Component->CurrentLayer != EConsciousnessLayer::Physical)
        {
            Component->TransitionToLayer(EConsciousnessLayer::Physical, true);
        }
    }
}

void AConsciousnessPhysicsManager::GetPerformanceMetrics(float& OutFrameTimeMS, int32& OutActorCount, int32& OutTransitionCount) const
{
    OutFrameTimeMS = LastFrameTime;
    OutActorCount = TrackedComponents.Num();
    
    // Count active transitions
    OutTransitionCount = 0;
    for (const UConsciousnessPhysicsComponent* Component : TrackedComponents)
    {
        if (Component && Component->bIsTransitioning)
        {
            OutTransitionCount++;
        }
    }
}

void AConsciousnessPhysicsManager::SetLayerVisibility(EConsciousnessLayer Layer, bool bVisible)
{
    LayerVisibility.FindOrAdd(Layer) = bVisible;
    
    // Apply visibility to all actors in layer
    TArray<AActor*> ActorsInLayer = GetActorsInLayer(Layer);
    for (AActor* Actor : ActorsInLayer)
    {
        if (Actor)
        {
            Actor->SetActorHiddenInGame(!bVisible);
        }
    }
}

void AConsciousnessPhysicsManager::UpdateTrackedComponents()
{
    // Remove invalid components
    TrackedComponents.RemoveAll([](const UConsciousnessPhysicsComponent* Component)
    {
        return !IsValid(Component) || !IsValid(Component->GetOwner());
    });
}

void AConsciousnessPhysicsManager::OptimizePerformance()
{
    // Count non-physical actors
    int32 NonPhysicalCount = 0;
    for (const UConsciousnessPhysicsComponent* Component : TrackedComponents)
    {
        if (Component && Component->CurrentLayer != EConsciousnessLayer::Physical)
        {
            NonPhysicalCount++;
        }
    }
    
    // If we exceed the limit, force some actors back to physical layer
    if (NonPhysicalCount > MaxNonPhysicalActors)
    {
        int32 ToForceBack = NonPhysicalCount - MaxNonPhysicalActors;
        
        for (UConsciousnessPhysicsComponent* Component : TrackedComponents)
        {
            if (Component && Component->CurrentLayer != EConsciousnessLayer::Physical && ToForceBack > 0)
            {
                Component->TransitionToLayer(EConsciousnessLayer::Physical, true);
                ToForceBack--;
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Performance optimization: Forced %d actors back to physical layer"), 
               NonPhysicalCount - MaxNonPhysicalActors);
    }
}

void AConsciousnessPhysicsManager::RegisterComponent(UConsciousnessPhysicsComponent* Component)
{
    if (Component && !TrackedComponents.Contains(Component))
    {
        TrackedComponents.Add(Component);
    }
}

void AConsciousnessPhysicsManager::UnregisterComponent(UConsciousnessPhysicsComponent* Component)
{
    TrackedComponents.Remove(Component);
}