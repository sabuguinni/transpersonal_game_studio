#include "Core_AdvancedPhysicsIntegrator.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Materials/MaterialInterface.h"
#include "Engine/TriggerVolume.h"
#include "Landscape/Landscape.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

UCore_AdvancedPhysicsIntegrator::UCore_AdvancedPhysicsIntegrator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    
    bEnableAdvancedPhysics = true;
    PhysicsUpdateFrequency = 60.0f;
    TerrainDeformationThreshold = 1000.0f;
    EnvironmentalForceMultiplier = 1.0f;
    
    LastPhysicsUpdateTime = 0.0f;
    PhysicsFrameCounter = 0;
}

void UCore_AdvancedPhysicsIntegrator::BeginPlay()
{
    Super::BeginPlay();
    
    InitializePhysicsIntegration();
}

void UCore_AdvancedPhysicsIntegrator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bEnableAdvancedPhysics)
    {
        return;
    }
    
    PhysicsFrameCounter++;
    LastPhysicsUpdateTime += DeltaTime;
    
    // Update physics at specified frequency
    if (LastPhysicsUpdateTime >= (1.0f / PhysicsUpdateFrequency))
    {
        UpdateTerrainPhysics(LastPhysicsUpdateTime);
        ProcessPhysicsZoneOverlaps();
        LastPhysicsUpdateTime = 0.0f;
    }
}

void UCore_AdvancedPhysicsIntegrator::InitializePhysicsIntegration()
{
    UE_LOG(LogTemp, Warning, TEXT("Core_AdvancedPhysicsIntegrator: Initializing Advanced Physics Integration"));
    
    CacheWorldReferences();
    InitializeBiomePhysicsZones();
    
    // Validate physics settings
    ValidatePhysicsIntegration();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_AdvancedPhysicsIntegrator: Physics Integration Initialized"));
}

void UCore_AdvancedPhysicsIntegrator::UpdateTerrainPhysics(float DeltaTime)
{
    if (!CachedWorld.IsValid())
    {
        return;
    }
    
    // Process all physics-enabled actors in the world
    for (TActorIterator<AActor> ActorIterator(CachedWorld.Get()); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (!Actor || !Actor->GetRootComponent())
        {
            continue;
        }
        
        UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
        if (PrimComp && PrimComp->IsSimulatingPhysics())
        {
            FVector ActorLocation = Actor->GetActorLocation();
            ECore_BiomePhysicsType BiomeType = GetBiomeTypeAtLocation(ActorLocation);
            
            ApplyBiomePhysics(Actor, BiomeType);
            
            // Apply environmental forces (wind simulation)
            FVector WindDirection = FVector(1.0f, 0.0f, 0.0f); // Simple east wind
            float WindStrength = 50.0f;
            ApplyEnvironmentalForces(Actor, WindDirection, WindStrength);
        }
    }
}

void UCore_AdvancedPhysicsIntegrator::ApplyBiomePhysics(AActor* Actor, ECore_BiomePhysicsType BiomeType)
{
    if (!Actor)
    {
        return;
    }
    
    // Find the physics properties for this biome
    FCore_BiomePhysicsProperties Properties;
    for (const FCore_TerrainPhysicsZone& Zone : PhysicsZones)
    {
        if (Zone.BiomeType == BiomeType)
        {
            Properties = Zone.PhysicsProperties;
            break;
        }
    }
    
    UpdateObjectPhysicsProperties(Actor, Properties);
}

ECore_BiomePhysicsType UCore_AdvancedPhysicsIntegrator::GetBiomeTypeAtLocation(const FVector& Location) const
{
    // Determine biome based on location (simplified grid system)
    if (Location.X < -2500.0f && Location.Y < -2500.0f)
    {
        return ECore_BiomePhysicsType::Swamp;
    }
    else if (Location.X < -2500.0f && Location.Y > 2500.0f)
    {
        return ECore_BiomePhysicsType::Forest;
    }
    else if (Location.X > 2500.0f && Location.Y < -2500.0f)
    {
        return ECore_BiomePhysicsType::Desert;
    }
    else if (Location.X > 2500.0f && Location.Y > 2500.0f)
    {
        return ECore_BiomePhysicsType::Mountain;
    }
    else
    {
        return ECore_BiomePhysicsType::Savanna; // Central area
    }
}

void UCore_AdvancedPhysicsIntegrator::CreatePhysicsZone(const FCore_TerrainPhysicsZone& ZoneData)
{
    // Remove existing zone of the same type
    RemovePhysicsZone(ZoneData.BiomeType);
    
    // Add new zone
    PhysicsZones.Add(ZoneData);
    
    UE_LOG(LogTemp, Warning, TEXT("Core_AdvancedPhysicsIntegrator: Created physics zone for biome type: %d"), (int32)ZoneData.BiomeType);
}

void UCore_AdvancedPhysicsIntegrator::RemovePhysicsZone(ECore_BiomePhysicsType BiomeType)
{
    PhysicsZones.RemoveAll([BiomeType](const FCore_TerrainPhysicsZone& Zone)
    {
        return Zone.BiomeType == BiomeType;
    });
}

TArray<FCore_TerrainPhysicsZone> UCore_AdvancedPhysicsIntegrator::GetAllPhysicsZones() const
{
    return PhysicsZones;
}

void UCore_AdvancedPhysicsIntegrator::SimulateTerrainDeformation(const FVector& ImpactLocation, float Force, float Radius)
{
    if (Force < TerrainDeformationThreshold)
    {
        return;
    }
    
    // Find nearby physics objects and apply impulse
    if (!CachedWorld.IsValid())
    {
        return;
    }
    
    TArray<FOverlapResult> OverlapResults;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;
    
    bool bHit = CachedWorld->OverlapMultiByChannel(
        OverlapResults,
        ImpactLocation,
        FQuat::Identity,
        ECollisionChannel::ECC_WorldDynamic,
        FCollisionShape::MakeSphere(Radius),
        QueryParams
    );
    
    if (bHit)
    {
        for (const FOverlapResult& Result : OverlapResults)
        {
            if (UPrimitiveComponent* PrimComp = Result.GetComponent())
            {
                if (PrimComp->IsSimulatingPhysics())
                {
                    FVector Direction = (Result.GetActor()->GetActorLocation() - ImpactLocation).GetSafeNormal();
                    FVector Impulse = Direction * Force * EnvironmentalForceMultiplier;
                    PrimComp->AddImpulseAtLocation(Impulse, ImpactLocation);
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_AdvancedPhysicsIntegrator: Simulated terrain deformation at %s with force %f"), *ImpactLocation.ToString(), Force);
}

void UCore_AdvancedPhysicsIntegrator::ApplyEnvironmentalForces(AActor* Actor, const FVector& WindDirection, float WindStrength)
{
    if (!Actor)
    {
        return;
    }
    
    UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
    if (!PrimComp || !PrimComp->IsSimulatingPhysics())
    {
        return;
    }
    
    // Calculate wind force based on object surface area (simplified)
    FVector ObjectBounds = Actor->GetActorBounds(false).BoxExtent;
    float SurfaceArea = ObjectBounds.X * ObjectBounds.Y; // Simplified surface area
    
    FVector WindForce = WindDirection.GetSafeNormal() * WindStrength * SurfaceArea * 0.01f; // Scale factor
    PrimComp->AddForce(WindForce, NAME_None, false);
}

void UCore_AdvancedPhysicsIntegrator::ProcessObjectTerrainInteraction(AActor* Object, const FVector& ContactPoint)
{
    if (!Object)
    {
        return;
    }
    
    ECore_BiomePhysicsType BiomeType = GetBiomeTypeAtLocation(ContactPoint);
    ApplyBiomePhysics(Object, BiomeType);
    
    // Log interaction for debugging
    UE_LOG(LogTemp, Log, TEXT("Core_AdvancedPhysicsIntegrator: Object %s interacting with terrain at %s (Biome: %d)"), 
           *Object->GetName(), *ContactPoint.ToString(), (int32)BiomeType);
}

UPhysicalMaterial* UCore_AdvancedPhysicsIntegrator::GetPhysicalMaterialForBiome(ECore_BiomePhysicsType BiomeType) const
{
    for (const FCore_TerrainPhysicsZone& Zone : PhysicsZones)
    {
        if (Zone.BiomeType == BiomeType && Zone.PhysicalMaterial.IsValid())
        {
            return Zone.PhysicalMaterial.LoadSynchronous();
        }
    }
    
    return nullptr;
}

void UCore_AdvancedPhysicsIntegrator::UpdatePhysicalMaterialProperties(ECore_BiomePhysicsType BiomeType, const FCore_BiomePhysicsProperties& Properties)
{
    for (FCore_TerrainPhysicsZone& Zone : PhysicsZones)
    {
        if (Zone.BiomeType == BiomeType)
        {
            Zone.PhysicsProperties = Properties;
            UE_LOG(LogTemp, Warning, TEXT("Core_AdvancedPhysicsIntegrator: Updated physics properties for biome type: %d"), (int32)BiomeType);
            break;
        }
    }
}

void UCore_AdvancedPhysicsIntegrator::ValidatePhysicsIntegration()
{
    UE_LOG(LogTemp, Warning, TEXT("Core_AdvancedPhysicsIntegrator: Validating Physics Integration..."));
    
    // Check world reference
    if (!CachedWorld.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Core_AdvancedPhysicsIntegrator: Invalid world reference!"));
        return;
    }
    
    // Check physics zones
    if (PhysicsZones.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_AdvancedPhysicsIntegrator: No physics zones configured"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_AdvancedPhysicsIntegrator: %d physics zones configured"), PhysicsZones.Num());
    }
    
    // Check landscape actors
    UE_LOG(LogTemp, Warning, TEXT("Core_AdvancedPhysicsIntegrator: %d landscape actors cached"), LandscapeActors.Num());
    
    UE_LOG(LogTemp, Warning, TEXT("Core_AdvancedPhysicsIntegrator: Physics Integration Validation Complete"));
}

void UCore_AdvancedPhysicsIntegrator::DrawPhysicsZoneDebug(bool bShowZones)
{
    if (!bShowZones || !CachedWorld.IsValid())
    {
        return;
    }
    
    for (const FCore_TerrainPhysicsZone& Zone : PhysicsZones)
    {
        FColor ZoneColor = FColor::Green;
        switch (Zone.BiomeType)
        {
            case ECore_BiomePhysicsType::Swamp:
                ZoneColor = FColor::Blue;
                break;
            case ECore_BiomePhysicsType::Forest:
                ZoneColor = FColor::Green;
                break;
            case ECore_BiomePhysicsType::Savanna:
                ZoneColor = FColor::Yellow;
                break;
            case ECore_BiomePhysicsType::Desert:
                ZoneColor = FColor::Orange;
                break;
            case ECore_BiomePhysicsType::Mountain:
                ZoneColor = FColor::Purple;
                break;
        }
        
        DrawDebugBox(CachedWorld.Get(), Zone.ZoneCenter, Zone.ZoneExtent, ZoneColor, false, 1.0f, 0, 5.0f);
    }
}

void UCore_AdvancedPhysicsIntegrator::ProcessPhysicsZoneOverlaps()
{
    // This would be called by trigger volume overlaps in a full implementation
    // For now, we use the location-based biome detection system
}

void UCore_AdvancedPhysicsIntegrator::UpdateObjectPhysicsProperties(AActor* Actor, const FCore_BiomePhysicsProperties& Properties)
{
    if (!Actor)
    {
        return;
    }
    
    UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
    if (!PrimComp)
    {
        return;
    }
    
    // Update physics properties based on biome
    if (UStaticMeshComponent* StaticMeshComp = Cast<UStaticMeshComponent>(PrimComp))
    {
        StaticMeshComp->SetLinearDamping(Properties.LinearDamping);
        StaticMeshComp->SetAngularDamping(Properties.AngularDamping);
        StaticMeshComp->SetEnableGravity(Properties.bEnableGravity);
    }
}

void UCore_AdvancedPhysicsIntegrator::CacheWorldReferences()
{
    CachedWorld = GetWorld();
    
    if (CachedWorld.IsValid())
    {
        // Cache landscape actors
        LandscapeActors.Empty();
        for (TActorIterator<ALandscape> LandscapeIterator(CachedWorld.Get()); LandscapeIterator; ++LandscapeIterator)
        {
            LandscapeActors.Add(*LandscapeIterator);
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Core_AdvancedPhysicsIntegrator: Cached %d landscape actors"), LandscapeActors.Num());
    }
}

void UCore_AdvancedPhysicsIntegrator::InitializeBiomePhysicsZones()
{
    // Initialize default physics zones for each biome
    PhysicsZones.Empty();
    
    // Swamp Zone (SW)
    FCore_TerrainPhysicsZone SwampZone;
    SwampZone.BiomeType = ECore_BiomePhysicsType::Swamp;
    SwampZone.ZoneCenter = FVector(-5000.0f, -5000.0f, 100.0f);
    SwampZone.ZoneExtent = FVector(2500.0f, 2500.0f, 500.0f);
    SwampZone.PhysicsProperties.Friction = 0.3f;
    SwampZone.PhysicsProperties.Restitution = 0.1f;
    SwampZone.PhysicsProperties.LinearDamping = 0.8f; // High damping for muddy terrain
    PhysicsZones.Add(SwampZone);
    
    // Forest Zone (NW)
    FCore_TerrainPhysicsZone ForestZone;
    ForestZone.BiomeType = ECore_BiomePhysicsType::Forest;
    ForestZone.ZoneCenter = FVector(-5000.0f, 5000.0f, 200.0f);
    ForestZone.ZoneExtent = FVector(2500.0f, 2500.0f, 500.0f);
    ForestZone.PhysicsProperties.Friction = 0.7f;
    ForestZone.PhysicsProperties.Restitution = 0.2f;
    ForestZone.PhysicsProperties.LinearDamping = 0.3f; // Moderate damping for forest floor
    PhysicsZones.Add(ForestZone);
    
    // Savanna Zone (Center)
    FCore_TerrainPhysicsZone SavannaZone;
    SavannaZone.BiomeType = ECore_BiomePhysicsType::Savanna;
    SavannaZone.ZoneCenter = FVector(0.0f, 0.0f, 150.0f);
    SavannaZone.ZoneExtent = FVector(2500.0f, 2500.0f, 500.0f);
    SavannaZone.PhysicsProperties.Friction = 0.8f;
    SavannaZone.PhysicsProperties.Restitution = 0.3f;
    SavannaZone.PhysicsProperties.LinearDamping = 0.1f; // Low damping for grassland
    PhysicsZones.Add(SavannaZone);
    
    // Desert Zone (E)
    FCore_TerrainPhysicsZone DesertZone;
    DesertZone.BiomeType = ECore_BiomePhysicsType::Desert;
    DesertZone.ZoneCenter = FVector(5000.0f, 0.0f, 120.0f);
    DesertZone.ZoneExtent = FVector(2500.0f, 2500.0f, 500.0f);
    DesertZone.PhysicsProperties.Friction = 0.4f;
    DesertZone.PhysicsProperties.Restitution = 0.1f;
    DesertZone.PhysicsProperties.LinearDamping = 0.5f; // Moderate damping for sand
    PhysicsZones.Add(DesertZone);
    
    // Mountain Zone (NE)
    FCore_TerrainPhysicsZone MountainZone;
    MountainZone.BiomeType = ECore_BiomePhysicsType::Mountain;
    MountainZone.ZoneCenter = FVector(5000.0f, 5000.0f, 500.0f);
    MountainZone.ZoneExtent = FVector(2500.0f, 2500.0f, 1000.0f);
    MountainZone.PhysicsProperties.Friction = 0.9f;
    MountainZone.PhysicsProperties.Restitution = 0.4f;
    MountainZone.PhysicsProperties.LinearDamping = 0.05f; // Very low damping for rocky terrain
    PhysicsZones.Add(MountainZone);
    
    UE_LOG(LogTemp, Warning, TEXT("Core_AdvancedPhysicsIntegrator: Initialized %d biome physics zones"), PhysicsZones.Num());
}