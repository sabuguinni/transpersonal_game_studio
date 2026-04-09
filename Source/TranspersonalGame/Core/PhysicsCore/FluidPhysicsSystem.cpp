#include "FluidPhysicsSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Chaos/ChaosEngineInterface.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY(LogFluidPhysics);

UFluidPhysicsSystem::UFluidPhysicsSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    
    // Set default values
    DefaultFluidDensity = 1000.0f; // Water density
    FluidViscosity = 0.001f; // Water viscosity
    FluidCurrentDirection = FVector::ZeroVector;
    FluidCurrentStrength = 0.0f;
    MaxFluidInteractions = 100;
    FluidUpdateFrequency = 30.0f;
    bFluidSimulationEnabled = true;
}

void UFluidPhysicsSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogFluidPhysics, Log, TEXT("FluidPhysicsSystem: Initializing fluid simulation"));
    
    InitializeFluidSimulation();
}

void UFluidPhysicsSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bFluidSimulationEnabled)
    {
        return;
    }
    
    // Update at specified frequency for performance
    LastFluidUpdateTime += DeltaTime;
    if (LastFluidUpdateTime >= (1.0f / FluidUpdateFrequency))
    {
        UpdateFluidInteractions(LastFluidUpdateTime);
        LastFluidUpdateTime = 0.0f;
    }
}

void UFluidPhysicsSystem::InitializeFluidSimulation()
{
    if (!GetWorld())
    {
        UE_LOG(LogFluidPhysics, Error, TEXT("FluidPhysicsSystem: No valid world found"));
        return;
    }
    
    // Clear existing data
    FluidVolumes.Empty();
    FluidInteractingActors.Empty();
    ActiveSplashEffects.Empty();
    
    UE_LOG(LogFluidPhysics, Log, TEXT("FluidPhysicsSystem: Fluid simulation initialized successfully"));
}

void UFluidPhysicsSystem::CreateFluidVolume(const FVector& Location, const FVector& Size, float Density)
{
    FBox FluidVolume(Location - Size * 0.5f, Location + Size * 0.5f);
    FluidVolumes.Add(FluidVolume);
    
    UE_LOG(LogFluidPhysics, Log, TEXT("FluidPhysicsSystem: Created fluid volume at %s with size %s, density %.2f"), 
           *Location.ToString(), *Size.ToString(), Density);
    
    // Debug visualization in development builds
    #if UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG
    if (GetWorld())
    {
        DrawDebugBox(GetWorld(), Location, Size * 0.5f, FColor::Blue, false, 5.0f, 0, 2.0f);
    }
    #endif
}

void UFluidPhysicsSystem::AddFluidInteraction(AActor* Actor, float BuoyancyForce)
{
    if (!Actor)
    {
        UE_LOG(LogFluidPhysics, Warning, TEXT("FluidPhysicsSystem: Attempted to add null actor to fluid interaction"));
        return;
    }
    
    if (FluidInteractingActors.Num() >= MaxFluidInteractions)
    {
        UE_LOG(LogFluidPhysics, Warning, TEXT("FluidPhysicsSystem: Maximum fluid interactions reached (%d)"), MaxFluidInteractions);
        return;
    }
    
    FluidInteractingActors.Add(Actor, BuoyancyForce);
    UE_LOG(LogFluidPhysics, Log, TEXT("FluidPhysicsSystem: Added actor %s to fluid interaction with buoyancy force %.2f"), 
           *Actor->GetName(), BuoyancyForce);
}

void UFluidPhysicsSystem::RemoveFluidInteraction(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    if (FluidInteractingActors.Remove(Actor) > 0)
    {
        UE_LOG(LogFluidPhysics, Log, TEXT("FluidPhysicsSystem: Removed actor %s from fluid interaction"), *Actor->GetName());
    }
}

bool UFluidPhysicsSystem::IsLocationInFluid(const FVector& Location, float& FluidDensity) const
{
    for (const FBox& FluidVolume : FluidVolumes)
    {
        if (FluidVolume.IsInside(Location))
        {
            FluidDensity = DefaultFluidDensity;
            return true;
        }
    }
    
    FluidDensity = 0.0f;
    return false;
}

FVector UFluidPhysicsSystem::GetFluidVelocityAtLocation(const FVector& Location) const
{
    float FluidDensity;
    if (IsLocationInFluid(Location, FluidDensity))
    {
        return FluidCurrentDirection * FluidCurrentStrength;
    }
    
    return FVector::ZeroVector;
}

void UFluidPhysicsSystem::ApplyFluidForces(AActor* Actor, float DeltaTime)
{
    if (!Actor || !bFluidSimulationEnabled)
    {
        return;
    }
    
    UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>();
    if (!PrimComp || !PrimComp->IsSimulatingPhysics())
    {
        return;
    }
    
    FBox FluidVolume;
    if (IsActorInFluidVolume(Actor, FluidVolume))
    {
        FVector FluidVelocity = GetFluidVelocityAtLocation(Actor->GetActorLocation());
        
        // Calculate and apply buoyancy force
        CalculateBuoyancyForce(Actor, FluidVelocity, DefaultFluidDensity, DeltaTime);
        
        // Calculate and apply drag force
        CalculateDragForce(Actor, FluidVelocity, DeltaTime);
    }
}

void UFluidPhysicsSystem::CreateSplashEffect(const FVector& Location, const FVector& Velocity, float Intensity)
{
    if (!SplashEffectTemplate || !GetWorld())
    {
        return;
    }
    
    UNiagaraComponent* SplashEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(), 
        SplashEffectTemplate, 
        Location, 
        FRotator::ZeroRotator,
        FVector(Intensity),
        true,
        true,
        ENCPoolMethod::None,
        true
    );
    
    if (SplashEffect)
    {
        SplashEffect->SetVectorParameter(TEXT("InitialVelocity"), Velocity);
        SplashEffect->SetFloatParameter(TEXT("Intensity"), Intensity);
        ActiveSplashEffects.Add(SplashEffect);
        
        UE_LOG(LogFluidPhysics, Log, TEXT("FluidPhysicsSystem: Created splash effect at %s with intensity %.2f"), 
               *Location.ToString(), Intensity);
    }
}

void UFluidPhysicsSystem::SetFluidCurrent(const FVector& Direction, float Strength)
{
    FluidCurrentDirection = Direction.GetSafeNormal();
    FluidCurrentStrength = Strength;
    
    UE_LOG(LogFluidPhysics, Log, TEXT("FluidPhysicsSystem: Set fluid current - Direction: %s, Strength: %.2f"), 
           *FluidCurrentDirection.ToString(), FluidCurrentStrength);
}

void UFluidPhysicsSystem::SetFluidSimulationEnabled(bool bEnabled)
{
    bFluidSimulationEnabled = bEnabled;
    UE_LOG(LogFluidPhysics, Log, TEXT("FluidPhysicsSystem: Fluid simulation %s"), 
           bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UFluidPhysicsSystem::UpdateFluidInteractions(float DeltaTime)
{
    // Clean up invalid actors
    TArray<AActor*> ActorsToRemove;
    for (auto& Pair : FluidInteractingActors)
    {
        if (!IsValid(Pair.Key))
        {
            ActorsToRemove.Add(Pair.Key);
        }
        else
        {
            ApplyFluidForces(Pair.Key, DeltaTime);
        }
    }
    
    // Remove invalid actors
    for (AActor* Actor : ActorsToRemove)
    {
        FluidInteractingActors.Remove(Actor);
    }
    
    // Clean up finished splash effects
    ActiveSplashEffects.RemoveAll([](const TObjectPtr<UNiagaraComponent>& Effect)
    {
        return !IsValid(Effect) || !Effect->IsActive();
    });
}

void UFluidPhysicsSystem::CalculateBuoyancyForce(AActor* Actor, const FVector& FluidVelocity, float FluidDensity, float DeltaTime)
{
    UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>();
    if (!PrimComp)
    {
        return;
    }
    
    // Get actor volume and mass
    float ActorMass = PrimComp->GetMass();
    FVector ActorBounds = Actor->GetActorBounds(false).BoxExtent;
    float ActorVolume = ActorBounds.X * ActorBounds.Y * ActorBounds.Z * 8.0f; // Approximate volume
    
    // Calculate buoyancy force (Archimedes' principle)
    float BuoyancyMagnitude = FluidDensity * ActorVolume * 9.81f; // ρ * V * g
    FVector BuoyancyForce = FVector::UpVector * BuoyancyMagnitude;
    
    // Apply buoyancy force
    PrimComp->AddForce(BuoyancyForce);
    
    // Get buoyancy multiplier from actor registration
    float* BuoyancyMultiplier = FluidInteractingActors.Find(Actor);
    if (BuoyancyMultiplier)
    {
        BuoyancyForce *= (*BuoyancyMultiplier / 9800.0f); // Normalize to default
    }
}

void UFluidPhysicsSystem::CalculateDragForce(AActor* Actor, const FVector& FluidVelocity, float DeltaTime)
{
    UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>();
    if (!PrimComp)
    {
        return;
    }
    
    // Get actor velocity relative to fluid
    FVector ActorVelocity = PrimComp->GetPhysicsLinearVelocity();
    FVector RelativeVelocity = ActorVelocity - FluidVelocity;
    
    if (RelativeVelocity.IsNearlyZero())
    {
        return;
    }
    
    // Calculate drag force: F = 0.5 * ρ * v² * Cd * A
    float DragCoefficient = 0.47f; // Sphere approximation
    FVector ActorBounds = Actor->GetActorBounds(false).BoxExtent;
    float CrossSectionalArea = FMath::Pi * FMath::Max(ActorBounds.X, ActorBounds.Y) * FMath::Max(ActorBounds.X, ActorBounds.Y);
    
    float DragMagnitude = 0.5f * DefaultFluidDensity * RelativeVelocity.SizeSquared() * DragCoefficient * CrossSectionalArea;
    FVector DragForce = -RelativeVelocity.GetSafeNormal() * DragMagnitude;
    
    // Apply drag force
    PrimComp->AddForce(DragForce);
}

bool UFluidPhysicsSystem::IsActorInFluidVolume(AActor* Actor, FBox& OutFluidVolume) const
{
    if (!Actor)
    {
        return false;
    }
    
    FVector ActorLocation = Actor->GetActorLocation();
    
    for (const FBox& FluidVolume : FluidVolumes)
    {
        if (FluidVolume.IsInside(ActorLocation))
        {
            OutFluidVolume = FluidVolume;
            return true;
        }
    }
    
    return false;
}