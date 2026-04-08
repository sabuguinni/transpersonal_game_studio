/**
 * @file ConsciousnessPhysics.cpp
 * @brief Implementation of consciousness expansion physics system
 */

#include "ConsciousnessPhysics.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/PhysicsSettings.h"

UConsciousnessPhysicsComponent::UConsciousnessPhysicsComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics; // Tick before physics update
    
    // Set reasonable tick interval for performance
    PrimaryComponentTick.TickInterval = 0.016f; // ~60 FPS
}

void UConsciousnessPhysicsComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache the primitive component for physics modifications
    AActor* Owner = GetOwner();
    if (Owner)
    {
        CachedPrimitiveComponent = Owner->FindComponentByClass<UPrimitiveComponent>();
        if (!CachedPrimitiveComponent)
        {
            // Try to get from character
            if (ACharacter* Character = Cast<ACharacter>(Owner))
            {
                CachedPrimitiveComponent = Character->GetMesh();
            }
        }
    }
    
    // Store original physics settings
    if (CachedPrimitiveComponent)
    {
        OriginalPhysicsSettings.Add(TEXT("Mass"), CachedPrimitiveComponent->GetMass());
        OriginalPhysicsSettings.Add(TEXT("LinearDamping"), CachedPrimitiveComponent->GetLinearDamping());
        OriginalPhysicsSettings.Add(TEXT("AngularDamping"), CachedPrimitiveComponent->GetAngularDamping());
    }
    
    // Apply initial layer physics
    ApplyLayerPhysics();
}

void UConsciousnessPhysicsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update layer transition if in progress
    if (TransitionProgress > 0.0f && TransitionProgress < 1.0f)
    {
        UpdateLayerTransition(DeltaTime);
    }
    
    // Update astral projection physics
    if (bIsAstralProjecting)
    {
        UpdateAstralProjection(DeltaTime);
    }
}

void UConsciousnessPhysicsComponent::TransitionToLayer(EConsciousnessLayer TargetLayer, bool bInstant)
{
    if (TargetLayer == CurrentLayer)
    {
        return; // Already in target layer
    }
    
    EConsciousnessLayer PreviousLayer = CurrentLayer;
    TargetTransitionLayer = TargetLayer;
    
    if (bInstant)
    {
        CurrentLayer = TargetLayer;
        TransitionProgress = 0.0f;
        ApplyLayerPhysics();
        OnLayerTransition.Broadcast(PreviousLayer, CurrentLayer);
    }
    else
    {
        // Start gradual transition
        TransitionProgress = 0.001f; // Start transition
        
        // Clear any existing timer
        if (GetWorld())
        {
            GetWorld()->GetTimerManager().ClearTimer(LayerTransitionTimer);
        }
    }
}

void UConsciousnessPhysicsComponent::BeginAstralProjection(const FVector& ProjectionDirection)
{
    if (bIsAstralProjecting)
    {
        return; // Already projecting
    }
    
    bIsAstralProjecting = true;
    
    // Transition to astral layer if not already there
    if (CurrentLayer != EConsciousnessLayer::Astral)
    {
        TransitionToLayer(EConsciousnessLayer::Astral, false);
    }
    
    // Disable collision with physical objects
    if (CachedPrimitiveComponent)
    {
        CachedPrimitiveComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);
        CachedPrimitiveComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);
        CachedPrimitiveComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
    }
    
    // Apply initial projection velocity if specified
    if (!ProjectionDirection.IsZero() && CachedPrimitiveComponent)
    {
        FVector ProjectionVelocity = ProjectionDirection.GetSafeNormal() * PhysicsSettings.AstralMovementSpeed;
        CachedPrimitiveComponent->SetPhysicsLinearVelocity(ProjectionVelocity);
    }
}

void UConsciousnessPhysicsComponent::EndAstralProjection()
{
    if (!bIsAstralProjecting)
    {
        return; // Not projecting
    }
    
    bIsAstralProjecting = false;
    
    // Restore collision with physical objects
    if (CachedPrimitiveComponent)
    {
        CachedPrimitiveComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
        CachedPrimitiveComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
        CachedPrimitiveComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
    }
    
    // Return to physical layer
    TransitionToLayer(EConsciousnessLayer::Physical, false);
}

void UConsciousnessPhysicsComponent::SetTemporalState(ETemporalState NewTemporalState, float Duration)
{
    if (NewTemporalState == CurrentTemporalState)
    {
        return; // Already in target state
    }
    
    ETemporalState PreviousState = CurrentTemporalState;
    CurrentTemporalState = NewTemporalState;
    
    // Apply temporal effects to world
    UWorld* World = GetWorld();
    if (World)
    {
        float TimeDilation = GetTimeDilationFactor();
        UGameplayStatics::SetGlobalTimeDilation(World, TimeDilation);
    }
    
    // Set timer to reset if duration specified
    if (Duration > 0.0f && World)
    {
        World->GetTimerManager().SetTimer(TemporalEffectTimer, this, &UConsciousnessPhysicsComponent::ResetTemporalState, Duration, false);
    }
    
    OnTemporalStateChanged.Broadcast(PreviousState, CurrentTemporalState);
}

float UConsciousnessPhysicsComponent::GetEffectiveGravity() const
{
    const float* GravityMultiplier = PhysicsSettings.GravityMultipliers.Find(CurrentLayer);
    if (GravityMultiplier)
    {
        UWorld* World = GetWorld();
        if (World)
        {
            float WorldGravity = FMath::Abs(World->GetGravityZ());
            return WorldGravity * (*GravityMultiplier);
        }
    }
    
    return 980.0f; // Default gravity
}

float UConsciousnessPhysicsComponent::GetTimeDilationFactor() const
{
    switch (CurrentTemporalState)
    {
        case ETemporalState::Accelerated:
            return 2.0f * PhysicsSettings.TemporalDistortionStrength;
        case ETemporalState::Decelerated:
            return 0.5f / PhysicsSettings.TemporalDistortionStrength;
        case ETemporalState::Frozen:
            return 0.001f; // Nearly frozen
        case ETemporalState::Reversed:
            return -0.5f; // Negative time (experimental)
        case ETemporalState::Normal:
        default:
            return 1.0f;
    }
}

bool UConsciousnessPhysicsComponent::CanInteractWithPhysicalObjects() const
{
    // Can only interact with physical objects in physical and etheric layers
    return CurrentLayer == EConsciousnessLayer::Physical || 
           CurrentLayer == EConsciousnessLayer::Etheric;
}

void UConsciousnessPhysicsComponent::ApplyLayerPhysics()
{
    if (!CachedPrimitiveComponent)
    {
        return;
    }
    
    // Apply gravity multiplier
    float EffectiveGravity = GetEffectiveGravity();
    CachedPrimitiveComponent->SetEnableGravity(EffectiveGravity > 0.1f);
    
    // Apply air resistance
    const float* AirResistanceValue = PhysicsSettings.AirResistance.Find(CurrentLayer);
    if (AirResistanceValue)
    {
        float OriginalDamping = *OriginalPhysicsSettings.Find(TEXT("LinearDamping"));
        CachedPrimitiveComponent->SetLinearDamping(OriginalDamping * (*AirResistanceValue));
    }
    
    // Apply mass modifications for different layers
    float MassMultiplier = 1.0f;
    switch (CurrentLayer)
    {
        case EConsciousnessLayer::Etheric:
            MassMultiplier = 0.8f;
            break;
        case EConsciousnessLayer::Astral:
            MassMultiplier = 0.3f;
            break;
        case EConsciousnessLayer::Mental:
            MassMultiplier = 0.1f;
            break;
        case EConsciousnessLayer::Causal:
            MassMultiplier = 0.01f;
            break;
        default:
            MassMultiplier = 1.0f;
            break;
    }
    
    float OriginalMass = *OriginalPhysicsSettings.Find(TEXT("Mass"));
    CachedPrimitiveComponent->SetMassOverrideInKg(NAME_None, OriginalMass * MassMultiplier, true);
}

void UConsciousnessPhysicsComponent::UpdateLayerTransition(float DeltaTime)
{
    // Update transition progress
    float TransitionSpeed = 1.0f / PhysicsSettings.PhaseTransitionTime;
    TransitionProgress += DeltaTime * TransitionSpeed;
    
    if (TransitionProgress >= 1.0f)
    {
        CompleteLayerTransition();
    }
    else
    {
        // Interpolate physics properties during transition
        // This creates a smooth transition effect
        ApplyLayerPhysics();
    }
}

void UConsciousnessPhysicsComponent::CompleteLayerTransition()
{
    EConsciousnessLayer PreviousLayer = CurrentLayer;
    CurrentLayer = TargetTransitionLayer;
    TransitionProgress = 0.0f;
    
    ApplyLayerPhysics();
    OnLayerTransition.Broadcast(PreviousLayer, CurrentLayer);
}

void UConsciousnessPhysicsComponent::ResetTemporalState()
{
    SetTemporalState(ETemporalState::Normal, -1.0f);
}

void UConsciousnessPhysicsComponent::UpdateAstralProjection(float DeltaTime)
{
    if (!CachedPrimitiveComponent)
    {
        return;
    }
    
    // Apply astral movement physics
    FVector CurrentVelocity = CachedPrimitiveComponent->GetPhysicsLinearVelocity();
    
    // Clamp velocity to astral maximum
    const float* MaxVelocity = PhysicsSettings.MaxVelocities.Find(EConsciousnessLayer::Astral);
    if (MaxVelocity && CurrentVelocity.Size() > *MaxVelocity)
    {
        FVector ClampedVelocity = CurrentVelocity.GetSafeNormal() * (*MaxVelocity);
        CachedPrimitiveComponent->SetPhysicsLinearVelocity(ClampedVelocity);
    }
    
    // Apply reduced gravity in astral state
    if (CurrentLayer == EConsciousnessLayer::Astral)
    {
        FVector AntiGravityForce = FVector(0, 0, GetEffectiveGravity() * 0.7f * CachedPrimitiveComponent->GetMass());
        CachedPrimitiveComponent->AddForce(AntiGravityForce);
    }
}