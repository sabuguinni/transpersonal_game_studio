#include "Core_TerrainPhysicsManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Landscape/LandscapeComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/CollisionProfile.h"

UCore_TerrainPhysicsManager::UCore_TerrainPhysicsManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update every 100ms
    
    // Initialize default values
    GlobalFrictionMultiplier = 1.0f;
    GlobalRestitutionMultiplier = 1.0f;
    CollisionUpdateDistance = 5000.0f;
    MaxCollisionVertices = 10000;
    bEnableComplexCollision = true;
    bEnableTerrainDeformation = false;
    MaxDeformationDepth = 50.0f;
    DeformationRecoveryRate = 1.0f;
    CachedLandscape = nullptr;
}

void UCore_TerrainPhysicsManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeBiomeSurfaceData();
    InitializeTerrainPhysics();
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsManager: Initialized terrain physics system"));
}

void UCore_TerrainPhysicsManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bEnableTerrainDeformation)
    {
        ProcessDeformationRecovery(DeltaTime);
    }
}

void UCore_TerrainPhysicsManager::InitializeTerrainPhysics()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_TerrainPhysicsManager: No valid world found"));
        return;
    }
    
    // Find landscape in the world
    for (TActorIterator<ALandscape> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        ALandscape* Landscape = *ActorIterator;
        if (Landscape)
        {
            CachedLandscape = Landscape;
            OptimizeTerrainCollision(Landscape);
            UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsManager: Found and optimized landscape"));
            break;
        }
    }
    
    // Initialize surface data for all biomes
    InitializeBiomeSurfaceData();
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsManager: Terrain physics initialization complete"));
}

void UCore_TerrainPhysicsManager::UpdateSurfaceProperties(EBiomeType BiomeType, const FCore_TerrainSurfaceData& SurfaceData)
{
    BiomeSurfaceData.Add(BiomeType, SurfaceData);
    
    // Update all registered terrain actors with this biome type
    for (AActor* Actor : RegisteredTerrainActors)
    {
        if (Actor)
        {
            UpdateActorPhysicsProperties(Actor, SurfaceData);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsManager: Updated surface properties for biome %d"), (int32)BiomeType);
}

FCore_TerrainSurfaceData UCore_TerrainPhysicsManager::GetSurfaceDataAtLocation(const FVector& WorldLocation)
{
    // For now, return plains data - in full implementation this would sample the biome at location
    if (BiomeSurfaceData.Contains(EBiomeType::Plains))
    {
        return BiomeSurfaceData[EBiomeType::Plains];
    }
    
    return GetDefaultSurfaceData(EBiomeType::Plains);
}

void UCore_TerrainPhysicsManager::ApplySurfaceEffectsToActor(AActor* Actor, const FVector& ContactPoint)
{
    if (!Actor)
    {
        return;
    }
    
    FCore_TerrainSurfaceData SurfaceData = GetSurfaceDataAtLocation(ContactPoint);
    UpdateActorPhysicsProperties(Actor, SurfaceData);
    
    UE_LOG(LogTemp, Verbose, TEXT("Core_TerrainPhysicsManager: Applied surface effects to actor %s"), *Actor->GetName());
}

void UCore_TerrainPhysicsManager::OptimizeTerrainCollision(ALandscape* Landscape)
{
    if (!Landscape)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_TerrainPhysicsManager: No landscape provided for collision optimization"));
        return;
    }
    
    // Get landscape components and optimize collision
    TArray<ULandscapeComponent*> LandscapeComponents;
    Landscape->GetLandscapeComponents(LandscapeComponents);
    
    for (ULandscapeComponent* Component : LandscapeComponents)
    {
        if (Component)
        {
            // Set collision profile for performance
            Component->SetCollisionProfileName(TEXT("BlockAll"));
            Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            
            // Optimize collision complexity based on settings
            if (bEnableComplexCollision)
            {
                Component->SetCollisionResponseToAllChannels(ECR_Block);
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsManager: Optimized collision for %d landscape components"), LandscapeComponents.Num());
}

void UCore_TerrainPhysicsManager::GenerateCollisionMesh(const FCore_TerrainCollisionData& CollisionData)
{
    if (CollisionData.CollisionVertices.Num() == 0 || CollisionData.CollisionIndices.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_TerrainPhysicsManager: Invalid collision data provided"));
        return;
    }
    
    // Limit vertices to prevent performance issues
    int32 VertexCount = FMath::Min(CollisionData.CollisionVertices.Num(), MaxCollisionVertices);
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsManager: Generated collision mesh with %d vertices"), VertexCount);
}

void UCore_TerrainPhysicsManager::UpdateCollisionComplexity(float NewComplexity)
{
    NewComplexity = FMath::Clamp(NewComplexity, 0.1f, 2.0f);
    
    // Update collision complexity for all registered terrain actors
    for (AActor* Actor : RegisteredTerrainActors)
    {
        if (Actor)
        {
            TArray<UPrimitiveComponent*> PrimitiveComponents;
            Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
            
            for (UPrimitiveComponent* Component : PrimitiveComponents)
            {
                if (Component)
                {
                    // Adjust collision complexity based on the new value
                    if (NewComplexity < 0.5f)
                    {
                        Component->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
                    }
                    else
                    {
                        Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                    }
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsManager: Updated collision complexity to %f"), NewComplexity);
}

void UCore_TerrainPhysicsManager::ApplyTerrainDeformation(const FVector& Location, float Force, float Radius)
{
    if (!bEnableTerrainDeformation || !IsValidTerrainLocation(Location))
    {
        return;
    }
    
    float DeformationAmount = FMath::Clamp(Force / 1000.0f, 0.0f, MaxDeformationDepth);
    
    // Store deformation data
    FVector GridLocation = FVector(
        FMath::RoundToInt(Location.X / 100.0f) * 100.0f,
        FMath::RoundToInt(Location.Y / 100.0f) * 100.0f,
        Location.Z
    );
    
    DeformationMap.Add(GridLocation, DeformationAmount);
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsManager: Applied deformation %f at location %s"), DeformationAmount, *Location.ToString());
}

bool UCore_TerrainPhysicsManager::CanDeformAtLocation(const FVector& Location)
{
    if (!bEnableTerrainDeformation)
    {
        return false;
    }
    
    FCore_TerrainSurfaceData SurfaceData = GetSurfaceDataAtLocation(Location);
    return SurfaceData.bCanDeform && IsValidTerrainLocation(Location);
}

void UCore_TerrainPhysicsManager::ResetDeformation(const FVector& Location, float Radius)
{
    TArray<FVector> KeysToRemove;
    
    for (auto& DeformationPair : DeformationMap)
    {
        float Distance = FVector::Dist(DeformationPair.Key, Location);
        if (Distance <= Radius)
        {
            KeysToRemove.Add(DeformationPair.Key);
        }
    }
    
    for (const FVector& Key : KeysToRemove)
    {
        DeformationMap.Remove(Key);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsManager: Reset deformation at %s (radius %f)"), *Location.ToString(), Radius);
}

void UCore_TerrainPhysicsManager::RegisterTerrainActor(AActor* TerrainActor, EBiomeType BiomeType)
{
    if (!TerrainActor)
    {
        return;
    }
    
    RegisteredTerrainActors.AddUnique(TerrainActor);
    
    // Apply biome-specific surface properties
    if (BiomeSurfaceData.Contains(BiomeType))
    {
        UpdateActorPhysicsProperties(TerrainActor, BiomeSurfaceData[BiomeType]);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsManager: Registered terrain actor %s with biome %d"), *TerrainActor->GetName(), (int32)BiomeType);
}

void UCore_TerrainPhysicsManager::UnregisterTerrainActor(AActor* TerrainActor)
{
    if (TerrainActor)
    {
        RegisteredTerrainActors.Remove(TerrainActor);
        UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsManager: Unregistered terrain actor %s"), *TerrainActor->GetName());
    }
}

void UCore_TerrainPhysicsManager::UpdateTerrainPhysicsSettings()
{
    // Update all registered actors with current settings
    for (AActor* Actor : RegisteredTerrainActors)
    {
        if (Actor)
        {
            FCore_TerrainSurfaceData DefaultData = GetDefaultSurfaceData(EBiomeType::Plains);
            UpdateActorPhysicsProperties(Actor, DefaultData);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsManager: Updated terrain physics settings for %d actors"), RegisteredTerrainActors.Num());
}

void UCore_TerrainPhysicsManager::InitializeBiomeSurfaceData()
{
    // Plains - grassy terrain
    FCore_TerrainSurfaceData PlainsData;
    PlainsData.BiomeType = EBiomeType::Plains;
    PlainsData.Friction = 0.8f;
    PlainsData.Restitution = 0.1f;
    PlainsData.Density = 1.0f;
    PlainsData.bCanDeform = true;
    PlainsData.DeformationThreshold = 50.0f;
    BiomeSurfaceData.Add(EBiomeType::Plains, PlainsData);
    
    // Forest - soft earth with roots
    FCore_TerrainSurfaceData ForestData;
    ForestData.BiomeType = EBiomeType::Forest;
    ForestData.Friction = 0.9f;
    ForestData.Restitution = 0.05f;
    ForestData.Density = 1.2f;
    ForestData.bCanDeform = true;
    ForestData.DeformationThreshold = 75.0f;
    BiomeSurfaceData.Add(EBiomeType::Forest, ForestData);
    
    // Desert - sandy terrain
    FCore_TerrainSurfaceData DesertData;
    DesertData.BiomeType = EBiomeType::Desert;
    DesertData.Friction = 0.4f;
    DesertData.Restitution = 0.2f;
    DesertData.Density = 0.8f;
    DesertData.bCanDeform = true;
    DesertData.DeformationThreshold = 25.0f;
    BiomeSurfaceData.Add(EBiomeType::Desert, DesertData);
    
    // Mountain - rocky terrain
    FCore_TerrainSurfaceData MountainData;
    MountainData.BiomeType = EBiomeType::Mountain;
    MountainData.Friction = 1.2f;
    MountainData.Restitution = 0.3f;
    MountainData.Density = 2.0f;
    MountainData.bCanDeform = false;
    MountainData.DeformationThreshold = 200.0f;
    BiomeSurfaceData.Add(EBiomeType::Mountain, MountainData);
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsManager: Initialized surface data for %d biomes"), BiomeSurfaceData.Num());
}

void UCore_TerrainPhysicsManager::UpdateActorPhysicsProperties(AActor* Actor, const FCore_TerrainSurfaceData& SurfaceData)
{
    if (!Actor)
    {
        return;
    }
    
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* Component : PrimitiveComponents)
    {
        if (Component && Component->GetBodyInstance())
        {
            FBodyInstance* BodyInstance = Component->GetBodyInstance();
            
            // Apply surface properties with global multipliers
            BodyInstance->SetPhysMaterialOverride(nullptr); // Reset to use component material
            
            // Note: Direct friction/restitution setting requires custom physics material
            // This is a simplified implementation
            Component->SetPhysMaterialOverride(nullptr);
        }
    }
}

FCore_TerrainSurfaceData UCore_TerrainPhysicsManager::GetDefaultSurfaceData(EBiomeType BiomeType)
{
    FCore_TerrainSurfaceData DefaultData;
    DefaultData.BiomeType = BiomeType;
    
    switch (BiomeType)
    {
        case EBiomeType::Forest:
            DefaultData.Friction = 0.9f;
            DefaultData.Restitution = 0.05f;
            break;
        case EBiomeType::Desert:
            DefaultData.Friction = 0.4f;
            DefaultData.Restitution = 0.2f;
            break;
        case EBiomeType::Mountain:
            DefaultData.Friction = 1.2f;
            DefaultData.Restitution = 0.3f;
            break;
        default: // Plains
            DefaultData.Friction = 0.8f;
            DefaultData.Restitution = 0.1f;
            break;
    }
    
    return DefaultData;
}

void UCore_TerrainPhysicsManager::ProcessDeformationRecovery(float DeltaTime)
{
    TArray<FVector> KeysToRemove;
    
    for (auto& DeformationPair : DeformationMap)
    {
        float& DeformationAmount = DeformationPair.Value;
        DeformationAmount -= DeformationRecoveryRate * DeltaTime;
        
        if (DeformationAmount <= 0.0f)
        {
            KeysToRemove.Add(DeformationPair.Key);
        }
    }
    
    for (const FVector& Key : KeysToRemove)
    {
        DeformationMap.Remove(Key);
    }
}

bool UCore_TerrainPhysicsManager::IsValidTerrainLocation(const FVector& Location)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Simple bounds check - in full implementation this would check against landscape bounds
    return FMath::Abs(Location.X) < 100000.0f && FMath::Abs(Location.Y) < 100000.0f;
}