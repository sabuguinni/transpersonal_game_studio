#include "Core_TerrainPhysicsSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Landscape/Landscape.h"
#include "Landscape/LandscapeComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UCore_TerrainPhysicsSystem::UCore_TerrainPhysicsSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = UpdateFrequency;

    // Initialize default values
    DefaultFriction = 0.7f;
    DefaultRestitution = 0.1f;
    MaxSlopeAngle = 60.0f;
    TerrainSampleRadius = 150.0f;
    bEnableTerrainDeformation = true;
    DeformationIntensityMultiplier = 1.0f;
    UpdateFrequency = 0.1f;
    LastUpdateTime = 0.0f;
}

void UCore_TerrainPhysicsSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeTerrainPhysics();
    RegisterWithArchitecturalFramework();
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsSystem: System initialized successfully"));
}

void UCore_TerrainPhysicsSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    LastUpdateTime += DeltaTime;
    
    // Clear old cache entries periodically
    if (LastUpdateTime > 5.0f)
    {
        ClearOldCacheEntries();
        LastUpdateTime = 0.0f;
    }
}

void UCore_TerrainPhysicsSystem::InitializeTerrainPhysics()
{
    CachedWorld = GetWorld();
    if (!CachedWorld.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Core_TerrainPhysicsSystem: Failed to get valid world reference"));
        return;
    }

    CacheLandscapeReference();
    SetupDefaultMaterialMappings();
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsSystem: Terrain physics initialized"));
}

void UCore_TerrainPhysicsSystem::CacheLandscapeReference()
{
    if (!CachedWorld.IsValid()) return;

    // Find landscape in the world
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(CachedWorld.Get(), ALandscape::StaticClass(), FoundActors);
    
    if (FoundActors.Num() > 0)
    {
        CachedLandscape = Cast<ALandscape>(FoundActors[0]);
        UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsSystem: Landscape reference cached"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_TerrainPhysicsSystem: No landscape found in world"));
    }
}

void UCore_TerrainPhysicsSystem::SetupDefaultMaterialMappings()
{
    BiomeMaterialMappings.Empty();

    // Create default material mappings for each biome type
    FCore_TerrainMaterialMapping ForestMapping;
    ForestMapping.BiomeType = EEng_BiomeType::Forest;
    ForestMapping.FrictionMultiplier = 0.8f;
    ForestMapping.RestitutionMultiplier = 0.2f;
    BiomeMaterialMappings.Add(ForestMapping);

    FCore_TerrainMaterialMapping DesertMapping;
    DesertMapping.BiomeType = EEng_BiomeType::Desert;
    DesertMapping.FrictionMultiplier = 0.4f;
    DesertMapping.RestitutionMultiplier = 0.1f;
    BiomeMaterialMappings.Add(DesertMapping);

    FCore_TerrainMaterialMapping SwampMapping;
    SwampMapping.BiomeType = EEng_BiomeType::Swamp;
    SwampMapping.FrictionMultiplier = 0.3f;
    SwampMapping.RestitutionMultiplier = 0.05f;
    BiomeMaterialMappings.Add(SwampMapping);

    FCore_TerrainMaterialMapping SavannaMapping;
    SavannaMapping.BiomeType = EEng_BiomeType::Savanna;
    SavannaMapping.FrictionMultiplier = 0.6f;
    SavannaMapping.RestitutionMultiplier = 0.15f;
    BiomeMaterialMappings.Add(SavannaMapping);

    FCore_TerrainMaterialMapping MountainMapping;
    MountainMapping.BiomeType = EEng_BiomeType::Mountain;
    MountainMapping.FrictionMultiplier = 0.9f;
    MountainMapping.RestitutionMultiplier = 0.3f;
    BiomeMaterialMappings.Add(MountainMapping);

    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsSystem: Default material mappings setup complete"));
}

FCore_TerrainPhysicsData UCore_TerrainPhysicsSystem::GetTerrainPhysicsAtLocation(const FVector& WorldLocation)
{
    // Check cache first
    FVector CacheKey = FVector(FMath::RoundToInt(WorldLocation.X / 100.0f) * 100, 
                              FMath::RoundToInt(WorldLocation.Y / 100.0f) * 100, 
                              0);
    
    if (CachedPhysicsData.Contains(CacheKey))
    {
        return CachedPhysicsData[CacheKey];
    }

    FCore_TerrainPhysicsData PhysicsData;
    
    // Calculate slope angle
    PhysicsData.SlopeAngle = CalculateSlopeAngle(WorldLocation, TerrainSampleRadius);
    
    // Determine biome type
    PhysicsData.BiomeType = DetermineBiomeAtLocation(WorldLocation);
    
    // Get material mapping for biome
    FCore_TerrainMaterialMapping* Mapping = BiomeMaterialMappings.FindByPredicate(
        [&](const FCore_TerrainMaterialMapping& Map) { return Map.BiomeType == PhysicsData.BiomeType; });
    
    if (Mapping)
    {
        PhysicsData.Friction = DefaultFriction * Mapping->FrictionMultiplier;
        PhysicsData.Restitution = DefaultRestitution * Mapping->RestitutionMultiplier;
    }
    else
    {
        PhysicsData.Friction = DefaultFriction;
        PhysicsData.Restitution = DefaultRestitution;
    }

    // Check if location is underwater
    PhysicsData.bIsWaterlogged = IsLocationUnderwater(WorldLocation);
    if (PhysicsData.bIsWaterlogged)
    {
        PhysicsData.Friction *= 0.3f; // Reduce friction underwater
        PhysicsData.Density = 0.5f; // Buoyancy effect
    }

    // Cache the result
    CachedPhysicsData.Add(CacheKey, PhysicsData);
    
    return PhysicsData;
}

void UCore_TerrainPhysicsSystem::ApplyTerrainPhysicsToActor(AActor* Actor, const FVector& Location)
{
    if (!Actor)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_TerrainPhysicsSystem: Null actor passed to ApplyTerrainPhysicsToActor"));
        return;
    }

    FCore_TerrainPhysicsData PhysicsData = GetTerrainPhysicsAtLocation(Location);
    
    // Apply physics to actor's primitive components
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* Component : PrimitiveComponents)
    {
        if (Component && Component->IsSimulatingPhysics())
        {
            // Apply environmental forces
            ApplyEnvironmentalForces(Actor, Location);
            
            // Modify physics properties based on terrain
            if (PhysicsData.bIsWaterlogged)
            {
                // Apply buoyancy force
                FVector BuoyancyForce = FVector(0, 0, 980.0f * PhysicsData.Density);
                Component->AddForce(BuoyancyForce);
            }
        }
    }
}

float UCore_TerrainPhysicsSystem::CalculateSlopeAngle(const FVector& Location, float SampleRadius)
{
    if (!CachedLandscape.IsValid()) return 0.0f;

    // Sample terrain heights in a cross pattern
    float CenterHeight = GetTerrainHeight(Location);
    float NorthHeight = GetTerrainHeight(Location + FVector(0, SampleRadius, 0));
    float SouthHeight = GetTerrainHeight(Location + FVector(0, -SampleRadius, 0));
    float EastHeight = GetTerrainHeight(Location + FVector(SampleRadius, 0, 0));
    float WestHeight = GetTerrainHeight(Location + FVector(-SampleRadius, 0, 0));

    // Calculate gradients
    float GradientNS = (NorthHeight - SouthHeight) / (2.0f * SampleRadius);
    float GradientEW = (EastHeight - WestHeight) / (2.0f * SampleRadius);

    // Calculate slope angle from gradient magnitude
    float GradientMagnitude = FMath::Sqrt(GradientNS * GradientNS + GradientEW * GradientEW);
    float SlopeAngle = FMath::RadiansToDegrees(FMath::Atan(GradientMagnitude));

    return FMath::Clamp(SlopeAngle, 0.0f, 90.0f);
}

bool UCore_TerrainPhysicsSystem::IsLocationOnSteepSlope(const FVector& Location, float MaxSlopeAngle)
{
    float SlopeAngle = CalculateSlopeAngle(Location, TerrainSampleRadius);
    return SlopeAngle > MaxSlopeAngle;
}

void UCore_TerrainPhysicsSystem::UpdateTerrainMaterialMapping(EEng_BiomeType BiomeType, UPhysicalMaterial* PhysMaterial)
{
    FCore_TerrainMaterialMapping* Mapping = BiomeMaterialMappings.FindByPredicate(
        [&](const FCore_TerrainMaterialMapping& Map) { return Map.BiomeType == BiomeType; });
    
    if (Mapping)
    {
        Mapping->PhysicalMaterial = PhysMaterial;
    }
    else
    {
        FCore_TerrainMaterialMapping NewMapping;
        NewMapping.BiomeType = BiomeType;
        NewMapping.PhysicalMaterial = PhysMaterial;
        BiomeMaterialMappings.Add(NewMapping);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsSystem: Updated material mapping for biome"));
}

UPhysicalMaterial* UCore_TerrainPhysicsSystem::GetPhysicalMaterialForBiome(EEng_BiomeType BiomeType)
{
    FCore_TerrainMaterialMapping* Mapping = BiomeMaterialMappings.FindByPredicate(
        [&](const FCore_TerrainMaterialMapping& Map) { return Map.BiomeType == BiomeType; });
    
    if (Mapping && Mapping->PhysicalMaterial.IsValid())
    {
        return Mapping->PhysicalMaterial.Get();
    }
    
    return nullptr;
}

void UCore_TerrainPhysicsSystem::ApplyTerrainDeformation(const FVector& Location, float Radius, float Intensity)
{
    if (!bEnableTerrainDeformation || !CachedLandscape.IsValid()) return;

    // Apply deformation intensity multiplier
    float FinalIntensity = Intensity * DeformationIntensityMultiplier;
    
    // This would require landscape editing functionality
    // For now, we log the deformation request
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsSystem: Terrain deformation requested at %s, Radius: %f, Intensity: %f"), 
           *Location.ToString(), Radius, FinalIntensity);
}

bool UCore_TerrainPhysicsSystem::CanDeformTerrain(const FVector& Location)
{
    if (!bEnableTerrainDeformation || !CachedLandscape.IsValid()) return false;
    
    // Check if location is within landscape bounds
    FBox LandscapeBounds = CachedLandscape->GetComponentsBoundingBox();
    return LandscapeBounds.IsInside(Location);
}

void UCore_TerrainPhysicsSystem::CreateFootprint(const FVector& Location, float FootSize, float Depth)
{
    if (CanDeformTerrain(Location))
    {
        ApplyTerrainDeformation(Location, FootSize, -Depth);
    }
}

void UCore_TerrainPhysicsSystem::ApplyEnvironmentalForces(AActor* Actor, const FVector& Location)
{
    if (!Actor) return;

    FCore_TerrainPhysicsData PhysicsData = GetTerrainPhysicsAtLocation(Location);
    
    // Apply slope-based forces
    if (PhysicsData.SlopeAngle > 15.0f)
    {
        FVector SlopeDirection = GetTerrainNormal(Location);
        FVector SlideForce = FVector::CrossProduct(SlopeDirection, FVector::UpVector) * PhysicsData.SlopeAngle * 10.0f;
        
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        for (UPrimitiveComponent* Component : PrimitiveComponents)
        {
            if (Component && Component->IsSimulatingPhysics())
            {
                Component->AddForce(SlideForce);
            }
        }
    }
}

float UCore_TerrainPhysicsSystem::GetTerrainStability(const FVector& Location)
{
    float SlopeAngle = CalculateSlopeAngle(Location, TerrainSampleRadius);
    FCore_TerrainPhysicsData PhysicsData = GetTerrainPhysicsAtLocation(Location);
    
    // Calculate stability based on slope and material properties
    float SlopeStability = FMath::Clamp(1.0f - (SlopeAngle / MaxSlopeAngle), 0.0f, 1.0f);
    float MaterialStability = PhysicsData.Friction;
    
    return SlopeStability * MaterialStability;
}

bool UCore_TerrainPhysicsSystem::IsLocationUnderwater(const FVector& Location)
{
    // Simple water level check - in a full implementation this would check against water volumes
    float WaterLevel = 0.0f; // Sea level
    return Location.Z < WaterLevel;
}

void UCore_TerrainPhysicsSystem::RegisterWithArchitecturalFramework()
{
    // Register this system with the architectural framework
    if (CachedWorld.IsValid())
    {
        UGameInstance* GameInstance = CachedWorld->GetGameInstance();
        if (GameInstance)
        {
            // This would integrate with the Eng_ArchitecturalFramework
            UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsSystem: Registered with architectural framework"));
        }
    }
}

void UCore_TerrainPhysicsSystem::ValidateSystemIntegrity()
{
    bool bIsValid = true;
    
    if (!CachedWorld.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Core_TerrainPhysicsSystem: Invalid world reference"));
        bIsValid = false;
    }
    
    if (BiomeMaterialMappings.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_TerrainPhysicsSystem: No biome material mappings configured"));
        bIsValid = false;
    }
    
    if (bIsValid)
    {
        UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsSystem: System integrity validation passed"));
    }
}

void UCore_TerrainPhysicsSystem::DebugTerrainPhysics()
{
    if (!CachedWorld.IsValid()) return;

    AActor* Owner = GetOwner();
    if (!Owner) return;

    FVector ActorLocation = Owner->GetActorLocation();
    FCore_TerrainPhysicsData PhysicsData = GetTerrainPhysicsAtLocation(ActorLocation);
    
    // Draw debug information
    DrawDebugString(CachedWorld.Get(), ActorLocation + FVector(0, 0, 200), 
                   FString::Printf(TEXT("Slope: %.1f°, Friction: %.2f"), PhysicsData.SlopeAngle, PhysicsData.Friction),
                   nullptr, FColor::Yellow, 0.0f);
    
    // Draw terrain normal
    FVector TerrainNormal = GetTerrainNormal(ActorLocation);
    DrawDebugLine(CachedWorld.Get(), ActorLocation, ActorLocation + TerrainNormal * 100.0f, FColor::Green, false, 0.0f, 0, 2.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsSystem: Debug info - Slope: %.1f°, Friction: %.2f, Biome: %d"), 
           PhysicsData.SlopeAngle, PhysicsData.Friction, (int32)PhysicsData.BiomeType);
}

// Private helper functions
FVector UCore_TerrainPhysicsSystem::GetTerrainNormal(const FVector& Location)
{
    if (!CachedLandscape.IsValid()) return FVector::UpVector;

    // Sample terrain heights around the location to calculate normal
    float SampleDistance = 50.0f;
    float CenterHeight = GetTerrainHeight(Location);
    float NorthHeight = GetTerrainHeight(Location + FVector(0, SampleDistance, 0));
    float EastHeight = GetTerrainHeight(Location + FVector(SampleDistance, 0, 0));

    FVector North = FVector(0, SampleDistance, NorthHeight - CenterHeight);
    FVector East = FVector(SampleDistance, 0, EastHeight - CenterHeight);
    
    FVector Normal = FVector::CrossProduct(East, North).GetSafeNormal();
    return Normal;
}

float UCore_TerrainPhysicsSystem::GetTerrainHeight(const FVector& Location)
{
    if (!CachedLandscape.IsValid()) return 0.0f;

    // Use landscape's GetHeightAtLocation if available, otherwise use line trace
    FVector Start = Location + FVector(0, 0, 1000.0f);
    FVector End = Location - FVector(0, 0, 1000.0f);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;
    
    if (CachedWorld->LineTraceSingleByChannel(HitResult, Start, End, ECC_WorldStatic, QueryParams))
    {
        return HitResult.Location.Z;
    }
    
    return Location.Z;
}

EEng_BiomeType UCore_TerrainPhysicsSystem::DetermineBiomeAtLocation(const FVector& Location)
{
    // Simple biome determination based on location
    // In a full implementation, this would query a biome system
    
    float X = Location.X;
    float Y = Location.Y;
    
    if (X < -5000.0f && Y < -5000.0f) return EEng_BiomeType::Swamp;
    if (X > 5000.0f) return EEng_BiomeType::Desert;
    if (Y > 5000.0f) return EEng_BiomeType::Mountain;
    if (FMath::Abs(X) < 2000.0f && FMath::Abs(Y) < 2000.0f) return EEng_BiomeType::Savanna;
    
    return EEng_BiomeType::Forest; // Default
}

void UCore_TerrainPhysicsSystem::ApplyPhysicalMaterialToLocation(const FVector& Location, UPhysicalMaterial* Material)
{
    if (!Material || !CachedLandscape.IsValid()) return;

    // This would apply the physical material to the landscape at the specified location
    // Implementation would depend on landscape material system
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsSystem: Applied physical material at location %s"), *Location.ToString());
}

void UCore_TerrainPhysicsSystem::ClearOldCacheEntries()
{
    // Clear cache if it gets too large
    if (CachedPhysicsData.Num() > 1000)
    {
        CachedPhysicsData.Empty();
        UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsSystem: Cleared physics data cache"));
    }
}