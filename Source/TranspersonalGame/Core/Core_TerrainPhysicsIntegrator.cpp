#include "Core_TerrainPhysicsIntegrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Landscape/LandscapeComponent.h"
#include "PhysicsEngine/PhysicalMaterial.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"

UCore_TerrainPhysicsIntegrator::UCore_TerrainPhysicsIntegrator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS for physics updates
    
    // Initialize terrain physics properties
    MaxDeformationRadius = 500.0f;
    DeformationStrengthMultiplier = 1.0f;
    bEnableTerrainDeformation = true;
    
    CollisionUpdateFrequency = 0.1f;
    MaxCollisionLODLevel = 3;
    CollisionCullingDistance = 5000.0f;
    
    LastCollisionUpdate = 0.0f;
    CurrentLODLevel = 0;
    CachedLandscape = nullptr;
    
    // Setup default terrain friction values
    TerrainFrictionValues.Add(ECore_TerrainType::Grassland, 0.8f);
    TerrainFrictionValues.Add(ECore_TerrainType::Forest, 0.6f);
    TerrainFrictionValues.Add(ECore_TerrainType::Desert, 0.4f);
    TerrainFrictionValues.Add(ECore_TerrainType::Swamp, 0.3f);
    TerrainFrictionValues.Add(ECore_TerrainType::Mountain, 0.9f);
    
    // Setup default terrain bounciness values
    TerrainBouncinessValues.Add(ECore_TerrainType::Grassland, 0.2f);
    TerrainBouncinessValues.Add(ECore_TerrainType::Forest, 0.1f);
    TerrainBouncinessValues.Add(ECore_TerrainType::Desert, 0.0f);
    TerrainBouncinessValues.Add(ECore_TerrainType::Swamp, 0.0f);
    TerrainBouncinessValues.Add(ECore_TerrainType::Mountain, 0.3f);
}

void UCore_TerrainPhysicsIntegrator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsIntegrator: Initializing terrain physics integration"));
    
    InitializeTerrainPhysics();
    CacheLandscapeReference();
    SetupTerrainPhysicsMaterials();
}

void UCore_TerrainPhysicsIntegrator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    LastCollisionUpdate += DeltaTime;
    
    if (LastCollisionUpdate >= CollisionUpdateFrequency)
    {
        if (CachedLandscape)
        {
            UpdateTerrainCollision(CachedLandscape);
        }
        
        UpdatePhysicsActorsList();
        LastCollisionUpdate = 0.0f;
    }
}

void UCore_TerrainPhysicsIntegrator::InitializeTerrainPhysics()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_TerrainPhysicsIntegrator: No valid world found"));
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsIntegrator: Initializing terrain physics for world: %s"), *World->GetName());
    
    // Find and cache landscape
    CacheLandscapeReference();
    
    // Setup physics materials for different terrain types
    SetupTerrainPhysicsMaterials();
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsIntegrator: Terrain physics initialization complete"));
}

void UCore_TerrainPhysicsIntegrator::UpdateTerrainCollision(ALandscape* Landscape)
{
    if (!Landscape)
    {
        return;
    }
    
    // Update collision for landscape components based on player proximity
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    UpdateCollisionLOD(PlayerLocation);
    
    // Apply physics materials based on terrain type
    TArray<ULandscapeComponent*> LandscapeComponents;
    Landscape->GetLandscapeComponents(LandscapeComponents);
    
    for (ULandscapeComponent* Component : LandscapeComponents)
    {
        if (!Component)
        {
            continue;
        }
        
        FVector ComponentLocation = Component->GetComponentLocation();
        float DistanceToPlayer = FVector::Dist(PlayerLocation, ComponentLocation);
        
        if (DistanceToPlayer <= CollisionCullingDistance)
        {
            ECore_TerrainType TerrainType = GetTerrainTypeAtLocation(ComponentLocation);
            ApplyTerrainMaterialPhysics(ComponentLocation, TerrainType);
        }
    }
}

void UCore_TerrainPhysicsIntegrator::ApplyTerrainMaterialPhysics(const FVector& Location, ECore_TerrainType TerrainType)
{
    UPhysicalMaterial* PhysicsMaterial = GetPhysicsMaterialForTerrain(TerrainType);
    if (!PhysicsMaterial)
    {
        return;
    }
    
    // Apply physics material to nearby actors
    TArray<AActor*> OverlappingActors;
    UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        Location,
        100.0f, // Small radius for local application
        TArray<TEnumAsByte<EObjectTypeQuery>>(),
        AActor::StaticClass(),
        TArray<AActor*>(),
        OverlappingActors
    );
    
    for (AActor* Actor : OverlappingActors)
    {
        if (Actor && Actor->IsA<AStaticMeshActor>())
        {
            ApplyPhysicsMaterialToActor(Actor, PhysicsMaterial);
        }
    }
}

float UCore_TerrainPhysicsIntegrator::GetTerrainFriction(const FVector& Location) const
{
    ECore_TerrainType TerrainType = GetTerrainTypeAtLocation(Location);
    
    if (const float* FrictionValue = TerrainFrictionValues.Find(TerrainType))
    {
        return *FrictionValue;
    }
    
    return 0.7f; // Default friction
}

float UCore_TerrainPhysicsIntegrator::GetTerrainBounciness(const FVector& Location) const
{
    ECore_TerrainType TerrainType = GetTerrainTypeAtLocation(Location);
    
    if (const float* BouncinessValue = TerrainBouncinessValues.Find(TerrainType))
    {
        return *BouncinessValue;
    }
    
    return 0.1f; // Default bounciness
}

FVector UCore_TerrainPhysicsIntegrator::GetTerrainNormal(const FVector& Location) const
{
    if (!CachedLandscape)
    {
        return FVector::UpVector;
    }
    
    // Perform line trace to get terrain normal
    FHitResult HitResult;
    FVector TraceStart = Location + FVector(0, 0, 1000);
    FVector TraceEnd = Location - FVector(0, 0, 1000);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_WorldStatic
    );
    
    if (bHit && HitResult.Component.IsValid())
    {
        return HitResult.Normal;
    }
    
    return FVector::UpVector;
}

void UCore_TerrainPhysicsIntegrator::ApplyTerrainDeformation(const FVector& Location, float Radius, float Strength)
{
    if (!bEnableTerrainDeformation || !CachedLandscape)
    {
        return;
    }
    
    if (Radius > MaxDeformationRadius)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_TerrainPhysicsIntegrator: Deformation radius %f exceeds maximum %f"), Radius, MaxDeformationRadius);
        return;
    }
    
    // Apply deformation to landscape
    float AdjustedStrength = Strength * DeformationStrengthMultiplier;
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsIntegrator: Applying terrain deformation at %s with radius %f and strength %f"), 
           *Location.ToString(), Radius, AdjustedStrength);
    
    // Note: Actual landscape deformation would require landscape editing tools
    // This is a placeholder for the deformation logic
}

bool UCore_TerrainPhysicsIntegrator::CanDeformTerrain(const FVector& Location) const
{
    if (!bEnableTerrainDeformation)
    {
        return false;
    }
    
    ECore_TerrainType TerrainType = GetTerrainTypeAtLocation(Location);
    
    // Some terrain types are more deformable than others
    switch (TerrainType)
    {
        case ECore_TerrainType::Desert:
        case ECore_TerrainType::Swamp:
            return true;
        case ECore_TerrainType::Grassland:
        case ECore_TerrainType::Forest:
            return true;
        case ECore_TerrainType::Mountain:
            return false; // Rocky terrain is harder to deform
        default:
            return true;
    }
}

void UCore_TerrainPhysicsIntegrator::SetupTerrainPhysicsMaterials()
{
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsIntegrator: Setting up terrain physics materials"));
    
    // Create or load physics materials for each terrain type
    // In a real implementation, these would be loaded from assets
    // For now, we'll create basic physics materials
    
    for (auto& TerrainTypePair : TerrainFrictionValues)
    {
        ECore_TerrainType TerrainType = TerrainTypePair.Key;
        float Friction = TerrainTypePair.Value;
        float Bounciness = TerrainBouncinessValues.FindRef(TerrainType);
        
        // Create a basic physics material
        UPhysicalMaterial* PhysicsMaterial = NewObject<UPhysicalMaterial>();
        if (PhysicsMaterial)
        {
            PhysicsMaterial->Friction = Friction;
            PhysicsMaterial->Restitution = Bounciness;
            TerrainPhysicsMaterials.Add(TerrainType, PhysicsMaterial);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsIntegrator: Created %d terrain physics materials"), TerrainPhysicsMaterials.Num());
}

UPhysicalMaterial* UCore_TerrainPhysicsIntegrator::GetPhysicsMaterialForTerrain(ECore_TerrainType TerrainType) const
{
    if (UPhysicalMaterial* const* PhysicsMaterial = TerrainPhysicsMaterials.Find(TerrainType))
    {
        return *PhysicsMaterial;
    }
    
    return nullptr;
}

void UCore_TerrainPhysicsIntegrator::OptimizeTerrainCollision(float ViewDistance)
{
    CollisionCullingDistance = ViewDistance;
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsIntegrator: Optimizing terrain collision for view distance: %f"), ViewDistance);
}

void UCore_TerrainPhysicsIntegrator::UpdateCollisionLOD(const FVector& PlayerLocation)
{
    if (!CachedLandscape)
    {
        return;
    }
    
    // Calculate appropriate LOD level based on distance and performance
    float DistanceToLandscape = FVector::Dist(PlayerLocation, CachedLandscape->GetActorLocation());
    
    if (DistanceToLandscape < 1000.0f)
    {
        CurrentLODLevel = 0; // Highest detail
    }
    else if (DistanceToLandscape < 3000.0f)
    {
        CurrentLODLevel = 1; // Medium detail
    }
    else if (DistanceToLandscape < 5000.0f)
    {
        CurrentLODLevel = 2; // Low detail
    }
    else
    {
        CurrentLODLevel = MaxCollisionLODLevel; // Lowest detail
    }
}

void UCore_TerrainPhysicsIntegrator::CacheLandscapeReference()
{
    if (CachedLandscape)
    {
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Find landscape in the world
    for (TActorIterator<ALandscape> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        ALandscape* Landscape = *ActorIterator;
        if (Landscape)
        {
            CachedLandscape = Landscape;
            UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsIntegrator: Cached landscape reference: %s"), *Landscape->GetName());
            break;
        }
    }
    
    if (!CachedLandscape)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_TerrainPhysicsIntegrator: No landscape found in world"));
    }
}

void UCore_TerrainPhysicsIntegrator::UpdatePhysicsActorsList()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    PhysicsEnabledActors.Empty();
    
    // Find all physics-enabled actors
    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (!Actor)
        {
            continue;
        }
        
        UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>();
        if (PrimComp && PrimComp->IsSimulatingPhysics())
        {
            PhysicsEnabledActors.Add(Actor);
        }
    }
}

ECore_TerrainType UCore_TerrainPhysicsIntegrator::GetTerrainTypeAtLocation(const FVector& Location) const
{
    // Simple terrain type determination based on location
    // In a real implementation, this would query the biome system or terrain data
    
    float X = Location.X;
    float Y = Location.Y;
    
    // Basic biome distribution based on coordinates
    if (X < -2000 && Y < -2000)
    {
        return ECore_TerrainType::Swamp;
    }
    else if (X < -2000 && Y > 2000)
    {
        return ECore_TerrainType::Forest;
    }
    else if (X > 2000)
    {
        return ECore_TerrainType::Desert;
    }
    else if (Y > 2000)
    {
        return ECore_TerrainType::Mountain;
    }
    else
    {
        return ECore_TerrainType::Grassland;
    }
}

void UCore_TerrainPhysicsIntegrator::ApplyPhysicsMaterialToActor(AActor* Actor, UPhysicalMaterial* PhysicsMaterial)
{
    if (!Actor || !PhysicsMaterial)
    {
        return;
    }
    
    UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>();
    if (PrimComp)
    {
        PrimComp->SetPhysMaterialOverride(PhysicsMaterial);
    }
}