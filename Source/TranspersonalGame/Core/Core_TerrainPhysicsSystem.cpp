#include "Core_TerrainPhysicsSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Landscape/Landscape.h"
#include "Landscape/LandscapeComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

UCore_TerrainPhysicsComponent::UCore_TerrainPhysicsComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    
    // Initialize default terrain material (grass)
    CurrentTerrainMaterial.BaseFriction = 0.7f;
    CurrentTerrainMaterial.Restitution = 0.1f;
    CurrentTerrainMaterial.Damping = 0.8f;
    CurrentTerrainMaterial.Density = 1.0f;
    CurrentTerrainMaterial.DeformationResistance = 0.5f;
    CurrentTerrainMaterial.SoundAttenuation = 1.0f;
    CurrentTerrainMaterial.ParticleIntensity = 1.0f;
    
    // Initialize slope physics
    CurrentSlopePhysics.SlopeAngle = 0.0f;
    CurrentSlopePhysics.MaxStableAngle = 45.0f;
    CurrentSlopePhysics.SlopeFrictionMultiplier = 1.0f;
    CurrentSlopePhysics.bIsSliding = false;
    CurrentSlopePhysics.SlidingAcceleration = 9.8f;
    
    // Initialize weather modifier
    WeatherModifier.WeatherIntensity = 0.0f;
    WeatherModifier.WetnessFrictionReduction = 0.3f;
    WeatherModifier.RainDeformationMultiplier = 1.5f;
    WeatherModifier.WindForce = FVector::ZeroVector;
    
    // Component settings
    bAutoUpdateTerrainPhysics = true;
    UpdateFrequency = 0.1f;
    TerrainTraceDistance = 200.0f;
    UpdateTimer = 0.0f;
}

void UCore_TerrainPhysicsComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Find landscape in the world
    FindLandscape();
    
    // Register with terrain physics subsystem
    if (UCore_TerrainPhysicsSubsystem* TerrainSubsystem = GetWorld()->GetSubsystem<UCore_TerrainPhysicsSubsystem>())
    {
        TerrainSubsystem->RegisterActorForTerrainPhysics(GetOwner());
    }
    
    // Initial terrain physics update
    UpdateTerrainPhysics();
}

void UCore_TerrainPhysicsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bAutoUpdateTerrainPhysics)
        return;
    
    UpdateTimer += DeltaTime;
    if (UpdateTimer >= UpdateFrequency)
    {
        UpdateTerrainPhysics();
        UpdateTimer = 0.0f;
    }
}

void UCore_TerrainPhysicsComponent::UpdateTerrainPhysics()
{
    if (!GetOwner())
        return;
    
    const FVector ActorLocation = GetOwner()->GetActorLocation();
    
    // Update terrain material properties
    CurrentTerrainMaterial = GetTerrainMaterialAtLocation(ActorLocation);
    
    // Update slope physics
    CurrentSlopePhysics = CalculateSlopePhysics(ActorLocation);
    
    // Apply terrain forces if needed
    if (CurrentSlopePhysics.bIsSliding || WeatherModifier.WindForce.SizeSquared() > 0.1f)
    {
        ApplyTerrainForces();
    }
}

FCore_TerrainMaterial UCore_TerrainPhysicsComponent::GetTerrainMaterialAtLocation(const FVector& Location)
{
    FCore_TerrainMaterial TerrainMat;
    
    // Get physical material from landscape
    UPhysicalMaterial* PhysMat = GetLandscapePhysicalMaterial(Location);
    if (PhysMat)
    {
        TerrainMat = ConvertPhysicalMaterialToTerrainMaterial(PhysMat);
    }
    else
    {
        // Default terrain material (grass/dirt)
        TerrainMat.BaseFriction = 0.7f;
        TerrainMat.Restitution = 0.1f;
        TerrainMat.Damping = 0.8f;
        TerrainMat.Density = 1.0f;
        TerrainMat.DeformationResistance = 0.5f;
        TerrainMat.SoundAttenuation = 1.0f;
        TerrainMat.ParticleIntensity = 1.0f;
    }
    
    return TerrainMat;
}

FCore_SlopePhysics UCore_TerrainPhysicsComponent::CalculateSlopePhysics(const FVector& Location)
{
    FCore_SlopePhysics SlopePhysics;
    
    FVector HitLocation, SurfaceNormal;
    if (TraceToTerrain(Location, HitLocation, SurfaceNormal))
    {
        // Calculate slope angle from surface normal
        const FVector UpVector = FVector::UpVector;
        const float DotProduct = FVector::DotProduct(SurfaceNormal, UpVector);
        SlopePhysics.SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(DotProduct, -1.0f, 1.0f)));
        
        // Calculate slope friction multiplier
        if (SlopePhysics.SlopeAngle > 0.1f)
        {
            const float SlopeFactor = SlopePhysics.SlopeAngle / 90.0f; // Normalize to 0-1
            SlopePhysics.SlopeFrictionMultiplier = FMath::Lerp(1.0f, 0.1f, SlopeFactor);
        }
        else
        {
            SlopePhysics.SlopeFrictionMultiplier = 1.0f;
        }
        
        // Check if should slide
        SlopePhysics.bIsSliding = SlopePhysics.SlopeAngle > SlopePhysics.MaxStableAngle;
        
        // Calculate sliding acceleration based on slope
        if (SlopePhysics.bIsSliding)
        {
            const float SlopeRadians = FMath::DegreesToRadians(SlopePhysics.SlopeAngle);
            SlopePhysics.SlidingAcceleration = 9.8f * FMath::Sin(SlopeRadians) * GetEffectiveFriction();
        }
    }
    
    return SlopePhysics;
}

void UCore_TerrainPhysicsComponent::ApplyWeatherEffects(float WeatherIntensity, EWeatherType WeatherType)
{
    WeatherModifier.WeatherIntensity = WeatherIntensity;
    
    switch (WeatherType)
    {
        case EWeatherType::Rain:
            WeatherModifier.WetnessFrictionReduction = 0.3f * WeatherIntensity;
            WeatherModifier.RainDeformationMultiplier = 1.0f + (0.5f * WeatherIntensity);
            break;
            
        case EWeatherType::Storm:
            WeatherModifier.WetnessFrictionReduction = 0.5f * WeatherIntensity;
            WeatherModifier.RainDeformationMultiplier = 1.0f + (1.0f * WeatherIntensity);
            WeatherModifier.WindForce = FVector(
                FMath::RandRange(-500.0f, 500.0f) * WeatherIntensity,
                FMath::RandRange(-500.0f, 500.0f) * WeatherIntensity,
                0.0f
            );
            break;
            
        case EWeatherType::Snow:
            WeatherModifier.WetnessFrictionReduction = 0.4f * WeatherIntensity;
            WeatherModifier.RainDeformationMultiplier = 1.0f - (0.2f * WeatherIntensity);
            break;
            
        case EWeatherType::Clear:
        default:
            WeatherModifier.WetnessFrictionReduction = 0.0f;
            WeatherModifier.RainDeformationMultiplier = 1.0f;
            WeatherModifier.WindForce = FVector::ZeroVector;
            break;
    }
}

float UCore_TerrainPhysicsComponent::GetEffectiveFriction() const
{
    float EffectiveFriction = CurrentTerrainMaterial.BaseFriction;
    
    // Apply slope modifier
    EffectiveFriction *= CurrentSlopePhysics.SlopeFrictionMultiplier;
    
    // Apply weather modifier
    EffectiveFriction *= (1.0f - WeatherModifier.WetnessFrictionReduction);
    
    return FMath::Clamp(EffectiveFriction, 0.01f, 2.0f);
}

bool UCore_TerrainPhysicsComponent::ShouldSlideOnTerrain() const
{
    return CurrentSlopePhysics.bIsSliding || GetEffectiveFriction() < 0.1f;
}

void UCore_TerrainPhysicsComponent::ApplyTerrainForces()
{
    if (!GetOwner())
        return;
    
    UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent());
    if (!RootPrimitive || !RootPrimitive->IsSimulatingPhysics())
        return;
    
    // Apply sliding force if on slope
    if (CurrentSlopePhysics.bIsSliding)
    {
        FVector HitLocation, SurfaceNormal;
        if (TraceToTerrain(GetOwner()->GetActorLocation(), HitLocation, SurfaceNormal))
        {
            // Calculate slide direction (perpendicular to surface normal, pointing downhill)
            const FVector UpVector = FVector::UpVector;
            FVector SlideDirection = FVector::CrossProduct(SurfaceNormal, FVector::CrossProduct(UpVector, SurfaceNormal));
            SlideDirection.Normalize();
            
            // Apply sliding force
            const float SlidingForce = CurrentSlopePhysics.SlidingAcceleration * RootPrimitive->GetMass();
            RootPrimitive->AddForce(SlideDirection * SlidingForce);
        }
    }
    
    // Apply wind force
    if (WeatherModifier.WindForce.SizeSquared() > 0.1f)
    {
        RootPrimitive->AddForce(WeatherModifier.WindForce);
    }
}

void UCore_TerrainPhysicsComponent::FindLandscape()
{
    if (GetWorld())
    {
        for (TActorIterator<ALandscape> ActorItr(GetWorld()); ActorItr; ++ActorItr)
        {
            CachedLandscape = *ActorItr;
            break; // Use first landscape found
        }
    }
}

bool UCore_TerrainPhysicsComponent::TraceToTerrain(const FVector& Location, FVector& OutHitLocation, FVector& OutSurfaceNormal)
{
    if (!GetWorld())
        return false;
    
    const FVector TraceStart = Location + FVector(0, 0, 50.0f);
    const FVector TraceEnd = Location - FVector(0, 0, TerrainTraceDistance);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;
    QueryParams.AddIgnoredActor(GetOwner());
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
    {
        OutHitLocation = HitResult.Location;
        OutSurfaceNormal = HitResult.Normal;
        return true;
    }
    
    return false;
}

UPhysicalMaterial* UCore_TerrainPhysicsComponent::GetLandscapePhysicalMaterial(const FVector& Location)
{
    if (!CachedLandscape)
        return nullptr;
    
    FVector HitLocation, SurfaceNormal;
    if (TraceToTerrain(Location, HitLocation, SurfaceNormal))
    {
        // Try to get physical material from hit result
        FHitResult HitResult;
        FCollisionQueryParams QueryParams;
        QueryParams.bTraceComplex = true;
        QueryParams.AddIgnoredActor(GetOwner());
        
        const FVector TraceStart = Location + FVector(0, 0, 50.0f);
        const FVector TraceEnd = Location - FVector(0, 0, TerrainTraceDistance);
        
        if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
        {
            if (HitResult.PhysMaterial.IsValid())
            {
                return HitResult.PhysMaterial.Get();
            }
        }
    }
    
    return nullptr;
}

FCore_TerrainMaterial UCore_TerrainPhysicsComponent::ConvertPhysicalMaterialToTerrainMaterial(UPhysicalMaterial* PhysMat)
{
    FCore_TerrainMaterial TerrainMat;
    
    if (!PhysMat)
        return TerrainMat;
    
    // Convert UE5 physical material properties to our terrain material
    TerrainMat.BaseFriction = PhysMat->Friction;
    TerrainMat.Restitution = PhysMat->Restitution;
    TerrainMat.Damping = 1.0f - PhysMat->Restitution; // Inverse relationship
    TerrainMat.Density = PhysMat->Density;
    
    // Estimate other properties based on physical material name/properties
    const FString MaterialName = PhysMat->GetName().ToLower();
    
    if (MaterialName.Contains(TEXT("mud")) || MaterialName.Contains(TEXT("swamp")))
    {
        TerrainMat.DeformationResistance = 0.2f;
        TerrainMat.SoundAttenuation = 0.7f;
        TerrainMat.ParticleIntensity = 1.5f;
    }
    else if (MaterialName.Contains(TEXT("rock")) || MaterialName.Contains(TEXT("stone")))
    {
        TerrainMat.DeformationResistance = 0.9f;
        TerrainMat.SoundAttenuation = 1.3f;
        TerrainMat.ParticleIntensity = 0.5f;
    }
    else if (MaterialName.Contains(TEXT("sand")) || MaterialName.Contains(TEXT("desert")))
    {
        TerrainMat.DeformationResistance = 0.3f;
        TerrainMat.SoundAttenuation = 0.8f;
        TerrainMat.ParticleIntensity = 2.0f;
    }
    else if (MaterialName.Contains(TEXT("grass")) || MaterialName.Contains(TEXT("forest")))
    {
        TerrainMat.DeformationResistance = 0.5f;
        TerrainMat.SoundAttenuation = 0.9f;
        TerrainMat.ParticleIntensity = 0.8f;
    }
    else // Default
    {
        TerrainMat.DeformationResistance = 0.5f;
        TerrainMat.SoundAttenuation = 1.0f;
        TerrainMat.ParticleIntensity = 1.0f;
    }
    
    return TerrainMat;
}

// Terrain Physics Subsystem Implementation

void UCore_TerrainPhysicsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Initialize global weather state
    GlobalWeatherState.WeatherIntensity = 0.0f;
    GlobalWeatherState.WetnessFrictionReduction = 0.0f;
    GlobalWeatherState.RainDeformationMultiplier = 1.0f;
    GlobalWeatherState.WindForce = FVector::ZeroVector;
    
    // Start batch update timer
    StartBatchUpdateTimer();
}

void UCore_TerrainPhysicsSubsystem::Deinitialize()
{
    // Clear batch update timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(BatchUpdateTimer);
    }
    
    RegisteredActors.Empty();
    
    Super::Deinitialize();
}

void UCore_TerrainPhysicsSubsystem::RegisterActorForTerrainPhysics(AActor* Actor)
{
    if (Actor && !RegisteredActors.Contains(Actor))
    {
        RegisteredActors.Add(Actor);
    }
}

void UCore_TerrainPhysicsSubsystem::UnregisterActorFromTerrainPhysics(AActor* Actor)
{
    if (Actor)
    {
        RegisteredActors.Remove(Actor);
    }
}

void UCore_TerrainPhysicsSubsystem::UpdateGlobalWeatherEffects(float WeatherIntensity, EWeatherType WeatherType)
{
    GlobalWeatherState.WeatherIntensity = WeatherIntensity;
    
    // Update all registered actors with new weather effects
    for (auto ActorPtr : RegisteredActors)
    {
        if (AActor* Actor = ActorPtr.Get())
        {
            if (UCore_TerrainPhysicsComponent* TerrainComp = GetTerrainPhysicsComponent(Actor))
            {
                TerrainComp->ApplyWeatherEffects(WeatherIntensity, WeatherType);
            }
        }
    }
}

UCore_TerrainPhysicsComponent* UCore_TerrainPhysicsSubsystem::GetTerrainPhysicsComponent(AActor* Actor)
{
    if (Actor)
    {
        return Actor->FindComponentByClass<UCore_TerrainPhysicsComponent>();
    }
    return nullptr;
}

void UCore_TerrainPhysicsSubsystem::BatchUpdateTerrainPhysics()
{
    // Clean up invalid actors first
    CleanupInvalidActors();
    
    // Batch update terrain physics for all registered actors
    for (auto ActorPtr : RegisteredActors)
    {
        if (AActor* Actor = ActorPtr.Get())
        {
            if (UCore_TerrainPhysicsComponent* TerrainComp = GetTerrainPhysicsComponent(Actor))
            {
                if (!TerrainComp->bAutoUpdateTerrainPhysics)
                {
                    TerrainComp->UpdateTerrainPhysics();
                }
            }
        }
    }
}

void UCore_TerrainPhysicsSubsystem::StartBatchUpdateTimer()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            BatchUpdateTimer,
            this,
            &UCore_TerrainPhysicsSubsystem::BatchUpdateTerrainPhysics,
            0.5f, // Update every 0.5 seconds
            true  // Loop
        );
    }
}

void UCore_TerrainPhysicsSubsystem::CleanupInvalidActors()
{
    RegisteredActors.RemoveAll([](const TWeakObjectPtr<AActor>& ActorPtr)
    {
        return !ActorPtr.IsValid();
    });
}