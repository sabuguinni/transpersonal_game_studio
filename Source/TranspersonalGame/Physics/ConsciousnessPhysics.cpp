/**
 * @file ConsciousnessPhysics.cpp
 * @brief Implementation of consciousness physics system
 * @author Core Systems Programmer
 * @date 2024
 */

#include "ConsciousnessPhysics.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

// Performance profiling macros
#define CONSCIOUSNESS_PHYSICS_SCOPE_CYCLE_COUNTER(Name) SCOPE_CYCLE_COUNTER(STAT_##Name)
DECLARE_CYCLE_STAT(TEXT("Consciousness Physics Update"), STAT_ConsciousnessPhysicsUpdate, STATGROUP_Game);
DECLARE_CYCLE_STAT(TEXT("Layer Transition"), STAT_LayerTransition, STATGROUP_Game);
DECLARE_CYCLE_STAT(TEXT("Reality Distortion"), STAT_RealityDistortion, STATGROUP_Game);

UConsciousnessPhysicsComponent::UConsciousnessPhysicsComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics; // Update before physics simulation
    
    // Initialize default settings
    PhysicsSettings = FConsciousnessPhysicsSettings();
}

void UConsciousnessPhysicsComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache primitive component reference
    PrimitiveComponent = GetOwner()->FindComponentByClass<UPrimitiveComponent>();
    
    if (!PrimitiveComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("ConsciousnessPhysicsComponent: No PrimitiveComponent found on %s"), 
               *GetOwner()->GetName());
        return;
    }

    // Register with world manager if it exists
    if (UWorld* World = GetWorld())
    {
        if (AConsciousnessPhysicsManager* Manager = World->GetSubsystem<AConsciousnessPhysicsManager>())
        {
            Manager->RegisterComponent(this);
        }
    }

    // Apply initial physics properties
    UpdatePhysicsProperties();
}

void UConsciousnessPhysicsComponent::TickComponent(float DeltaTime, ELevelTick TickType, 
                                                  FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    CONSCIOUSNESS_PHYSICS_SCOPE_CYCLE_COUNTER(ConsciousnessPhysicsUpdate);

    // Process layer transition if active
    if (bIsTransitioning)
    {
        ProcessLayerTransition(DeltaTime);
    }

    // Process reality distortion effects
    if (DistortionTimeRemaining > 0.0f)
    {
        ProcessDistortionEffect(DeltaTime);
    }
}

void UConsciousnessPhysicsComponent::TransitionToLayer(EConsciousnessLayer NewLayer, float TransitionDuration)
{
    if (NewLayer == CurrentLayer)
    {
        return; // Already in target layer
    }

    // Set up transition
    PreviousLayer = CurrentLayer;
    TargetLayer = NewLayer;
    TransitionProgress = 0.0f;
    this->TransitionDuration = (TransitionDuration > 0.0f) ? TransitionDuration : PhysicsSettings.LayerTransitionTime;
    bIsTransitioning = true;

    UE_LOG(LogTemp, Log, TEXT("ConsciousnessPhysics: Transitioning from %d to %d over %.2f seconds"), 
           static_cast<int32>(PreviousLayer), static_cast<int32>(TargetLayer), this->TransitionDuration);
}

void UConsciousnessPhysicsComponent::ApplyRealityDistortion(float DistortionStrength, float Duration)
{
    CONSCIOUSNESS_PHYSICS_SCOPE_CYCLE_COUNTER(RealityDistortion);

    // Clamp distortion strength to valid range
    TargetDistortionStrength = FMath::Clamp(DistortionStrength, 0.0f, PhysicsSettings.MaxDistortionStrength);
    DistortionTimeRemaining = FMath::Max(Duration, 0.1f);

    OnDistortionApplied.Broadcast(TargetDistortionStrength);

    UE_LOG(LogTemp, Log, TEXT("ConsciousnessPhysics: Applying distortion %.2f for %.2f seconds"), 
           TargetDistortionStrength, Duration);
}

float UConsciousnessPhysicsComponent::GetCurrentGravityMultiplier() const
{
    if (const float* Multiplier = PhysicsSettings.GravityMultipliers.Find(CurrentLayer))
    {
        return *Multiplier;
    }
    return 1.0f; // Default to normal gravity
}

float UConsciousnessPhysicsComponent::GetCurrentTimeDilation() const
{
    if (const float* Dilation = PhysicsSettings.TimeDilationFactors.Find(CurrentLayer))
    {
        return *Dilation;
    }
    return 1.0f; // Default to normal time
}

bool UConsciousnessPhysicsComponent::CanPhaseThrough() const
{
    // Spiritual and Mental layers allow phasing
    return CurrentLayer == EConsciousnessLayer::Spiritual || CurrentLayer == EConsciousnessLayer::Mental;
}

UPhysicalMaterial* UConsciousnessPhysicsComponent::GetCurrentLayerMaterial() const
{
    if (UPhysicalMaterial* const* Material = PhysicsSettings.LayerMaterials.Find(CurrentLayer))
    {
        return *Material;
    }
    return nullptr;
}

void UConsciousnessPhysicsComponent::UpdatePhysicsProperties()
{
    if (!PrimitiveComponent)
    {
        return;
    }

    UpdateGravity();
    UpdateTimeDilation();
    UpdateMaterialProperties();
}

void UConsciousnessPhysicsComponent::UpdateGravity()
{
    if (!PrimitiveComponent)
    {
        return;
    }

    float GravityMultiplier = GetCurrentGravityMultiplier();
    
    // Apply distortion effect to gravity
    if (CurrentDistortionStrength > 0.0f)
    {
        float DistortionFactor = 1.0f + (CurrentDistortionStrength * 0.5f);
        GravityMultiplier *= DistortionFactor;
    }

    // Set gravity scale on physics body
    if (UBodyInstance* BodyInstance = PrimitiveComponent->GetBodyInstance())
    {
        BodyInstance->SetGravityScale(GravityMultiplier);
    }
}

void UConsciousnessPhysicsComponent::UpdateTimeDilation()
{
    if (UWorld* World = GetWorld())
    {
        float TimeDilation = GetCurrentTimeDilation();
        
        // Apply distortion effect to time dilation
        if (CurrentDistortionStrength > 0.0f)
        {
            float DistortionFactor = 1.0f - (CurrentDistortionStrength * 0.3f);
            TimeDilation *= FMath::Max(DistortionFactor, 0.1f);
        }

        // Apply time dilation to actor (affects all components)
        GetOwner()->CustomTimeDilation = TimeDilation;
    }
}

void UConsciousnessPhysicsComponent::UpdateMaterialProperties()
{
    if (!PrimitiveComponent)
    {
        return;
    }

    // Apply layer-specific physics material
    if (UPhysicalMaterial* LayerMaterial = GetCurrentLayerMaterial())
    {
        PrimitiveComponent->SetPhysMaterialOverride(LayerMaterial);
    }

    // Handle phasing for spiritual/mental layers
    if (CanPhaseThrough())
    {
        // Enable collision response modification for phasing
        PrimitiveComponent->SetCollisionResponseToAllChannels(ECR_Overlap);
        PrimitiveComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);
    }
    else
    {
        // Restore normal collision
        PrimitiveComponent->SetCollisionResponseToAllChannels(ECR_Block);
    }
}

void UConsciousnessPhysicsComponent::ProcessLayerTransition(float DeltaTime)
{
    CONSCIOUSNESS_PHYSICS_SCOPE_CYCLE_COUNTER(LayerTransition);

    TransitionProgress += DeltaTime / TransitionDuration;
    
    if (TransitionProgress >= 1.0f)
    {
        // Transition complete
        TransitionProgress = 1.0f;
        CurrentLayer = TargetLayer;
        bIsTransitioning = false;
        
        OnLayerChanged.Broadcast(static_cast<int32>(CurrentLayer), TransitionDuration);
        
        UE_LOG(LogTemp, Log, TEXT("ConsciousnessPhysics: Transition to layer %d complete"), 
               static_cast<int32>(CurrentLayer));
    }

    // Update physics properties with interpolated values during transition
    UpdatePhysicsProperties();
}

void UConsciousnessPhysicsComponent::ProcessDistortionEffect(float DeltaTime)
{
    DistortionTimeRemaining -= DeltaTime;
    
    if (DistortionTimeRemaining <= 0.0f)
    {
        // Effect ended
        CurrentDistortionStrength = 0.0f;
        DistortionTimeRemaining = 0.0f;
    }
    else
    {
        // Interpolate distortion strength (fade in/out)
        float EffectProgress = 1.0f - (DistortionTimeRemaining / 3.0f); // Assuming 3s duration
        CurrentDistortionStrength = TargetDistortionStrength * FMath::Sin(EffectProgress * PI);
    }

    // Update physics with current distortion
    UpdatePhysicsProperties();
}

float UConsciousnessPhysicsComponent::InterpolateLayerValues(float PhysicalValue, float TargetValue, float Progress) const
{
    return FMath::Lerp(PhysicalValue, TargetValue, Progress);
}

// Manager Implementation
AConsciousnessPhysicsManager::AConsciousnessPhysicsManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PrePhysics;
    
    // Initialize default settings
    GlobalSettings = FConsciousnessPhysicsSettings();
}

void AConsciousnessPhysicsManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("ConsciousnessPhysicsManager: Initialized with budget %.2fms"), PerformanceBudgetMS);
}

void AConsciousnessPhysicsManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update components in batches for performance
    UpdateComponentsBatched(DeltaTime);
}

void AConsciousnessPhysicsManager::RegisterComponent(UConsciousnessPhysicsComponent* Component)
{
    if (Component && !RegisteredComponents.Contains(Component))
    {
        RegisteredComponents.Add(Component);
        UE_LOG(LogTemp, Log, TEXT("ConsciousnessPhysicsManager: Registered component %s (Total: %d)"), 
               *Component->GetOwner()->GetName(), RegisteredComponents.Num());
    }
}

void AConsciousnessPhysicsManager::UnregisterComponent(UConsciousnessPhysicsComponent* Component)
{
    if (Component)
    {
        RegisteredComponents.Remove(Component);
        UE_LOG(LogTemp, Log, TEXT("ConsciousnessPhysicsManager: Unregistered component %s (Total: %d)"), 
               *Component->GetOwner()->GetName(), RegisteredComponents.Num());
    }
}

void AConsciousnessPhysicsManager::ApplyGlobalDistortion(float DistortionStrength, float Duration)
{
    for (UConsciousnessPhysicsComponent* Component : RegisteredComponents)
    {
        if (IsValid(Component))
        {
            Component->ApplyRealityDistortion(DistortionStrength, Duration);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("ConsciousnessPhysicsManager: Applied global distortion %.2f to %d components"), 
           DistortionStrength, RegisteredComponents.Num());
}

void AConsciousnessPhysicsManager::GetPerformanceMetrics(float& OutUpdateTimeMS, int32& OutRegisteredComponents) const
{
    OutUpdateTimeMS = LastUpdateTimeMS;
    OutRegisteredComponents = RegisteredComponents.Num();
}

void AConsciousnessPhysicsManager::UpdateComponentsBatched(float DeltaTime)
{
    if (RegisteredComponents.Num() == 0)
    {
        return;
    }

    double StartTime = FPlatformTime::Seconds();
    int32 ComponentsProcessed = 0;
    int32 MaxComponents = FMath::Min(MaxComponentsPerFrame, RegisteredComponents.Num());

    // Process components in round-robin fashion
    for (int32 i = 0; i < MaxComponents; ++i)
    {
        int32 Index = (CurrentUpdateIndex + i) % RegisteredComponents.Num();
        
        if (UConsciousnessPhysicsComponent* Component = RegisteredComponents[Index])
        {
            if (IsValid(Component))
            {
                // Component updates are handled in their own TickComponent
                ComponentsProcessed++;
            }
            else
            {
                // Remove invalid components
                RegisteredComponents.RemoveAt(Index);
                if (Index <= CurrentUpdateIndex)
                {
                    CurrentUpdateIndex--;
                }
            }
        }

        // Check performance budget
        double CurrentTime = FPlatformTime::Seconds();
        float ElapsedMS = (CurrentTime - StartTime) * 1000.0f;
        if (ElapsedMS >= PerformanceBudgetMS)
        {
            break;
        }
    }

    // Update index for next frame
    CurrentUpdateIndex = (CurrentUpdateIndex + ComponentsProcessed) % FMath::Max(RegisteredComponents.Num(), 1);
    
    // Track performance
    double EndTime = FPlatformTime::Seconds();
    LastUpdateTimeMS = (EndTime - StartTime) * 1000.0f;
}