#include "Core_TerrainPhysicsManager.h"
#include "Engine/World.h"
#include "Landscape/LandscapeComponent.h"
#include "Landscape/LandscapeInfo.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"

UCore_TerrainPhysicsManager::UCore_TerrainPhysicsManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    TerrainSampleRadius = 100.0f;
    TerrainSamplePoints = 9;
    bEnableTerrainPhysics = true;
    bEnableRealTimeUpdates = true;
    LandscapeQueryRadius = 200.0f;
    bUseLandscapePhysicalMaterials = true;
    UpdateFrequency = 0.1f;
    MaxSimultaneousQueries = 50;
    bEnableLODSystem = true;
    LastUpdateTime = 0.0f;
    CurrentQueryCount = 0;
}

void UCore_TerrainPhysicsManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeTerrainPhysics();
    FindLandscapeActor();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_TerrainPhysicsManager: Initialized terrain physics system"));
}

void UCore_TerrainPhysicsManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bEnableRealTimeUpdates)
        return;
        
    LastUpdateTime += DeltaTime;
    
    if (LastUpdateTime >= UpdateFrequency)
    {
        UpdateTerrainPhysics();
        LastUpdateTime = 0.0f;
    }
}

void UCore_TerrainPhysicsManager::InitializeTerrainPhysics()
{
    InitializeDefaultTerrainProperties();
    InitializeBiomeTerrainMappings();
    
    TrackedActors.Empty();
    ActorTerrainCache.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_TerrainPhysicsManager: Terrain physics initialized with %d terrain types"), TerrainProperties.Num());
}

void UCore_TerrainPhysicsManager::InitializeDefaultTerrainProperties()
{
    TerrainProperties.Empty();
    
    // Grass - Standard terrain
    FCore_TerrainPhysicsProperties GrassProps;
    GrassProps.Friction = 0.8f;
    GrassProps.Restitution = 0.2f;
    GrassProps.Density = 1.0f;
    GrassProps.Stability = 1.0f;
    GrassProps.MovementSpeedMultiplier = 1.0f;
    GrassProps.SoundDampening = 0.6f;
    GrassProps.bCanSink = false;
    TerrainProperties.Add(ECore_TerrainType::Grass, GrassProps);
    
    // Dirt - Slightly less stable
    FCore_TerrainPhysicsProperties DirtProps;
    DirtProps.Friction = 0.7f;
    DirtProps.Restitution = 0.3f;
    DirtProps.Density = 1.1f;
    DirtProps.Stability = 0.9f;
    DirtProps.MovementSpeedMultiplier = 0.95f;
    DirtProps.SoundDampening = 0.7f;
    DirtProps.bCanSink = false;
    TerrainProperties.Add(ECore_TerrainType::Dirt, DirtProps);
    
    // Rock - High friction, stable
    FCore_TerrainPhysicsProperties RockProps;
    RockProps.Friction = 0.9f;
    RockProps.Restitution = 0.1f;
    RockProps.Density = 2.5f;
    RockProps.Stability = 1.2f;
    RockProps.MovementSpeedMultiplier = 1.1f;
    RockProps.SoundDampening = 0.2f;
    RockProps.bCanSink = false;
    TerrainProperties.Add(ECore_TerrainType::Rock, RockProps);
    
    // Sand - Low friction, can sink
    FCore_TerrainPhysicsProperties SandProps;
    SandProps.Friction = 0.4f;
    SandProps.Restitution = 0.5f;
    SandProps.Density = 1.5f;
    SandProps.Stability = 0.6f;
    SandProps.MovementSpeedMultiplier = 0.7f;
    SandProps.SoundDampening = 0.8f;
    SandProps.bCanSink = true;
    SandProps.SinkRate = 0.1f;
    TerrainProperties.Add(ECore_TerrainType::Sand, SandProps);
    
    // Mud - Very low friction, high sink rate
    FCore_TerrainPhysicsProperties MudProps;
    MudProps.Friction = 0.2f;
    MudProps.Restitution = 0.8f;
    MudProps.Density = 1.3f;
    MudProps.Stability = 0.3f;
    MudProps.MovementSpeedMultiplier = 0.5f;
    MudProps.SoundDampening = 0.9f;
    MudProps.bCanSink = true;
    MudProps.SinkRate = 0.3f;
    TerrainProperties.Add(ECore_TerrainType::Mud, MudProps);
    
    // Snow - Moderate properties
    FCore_TerrainPhysicsProperties SnowProps;
    SnowProps.Friction = 0.5f;
    SnowProps.Restitution = 0.4f;
    SnowProps.Density = 0.8f;
    SnowProps.Stability = 0.7f;
    SnowProps.MovementSpeedMultiplier = 0.8f;
    SnowProps.SoundDampening = 0.9f;
    SnowProps.bCanSink = true;
    SnowProps.SinkRate = 0.05f;
    TerrainProperties.Add(ECore_TerrainType::Snow, SnowProps);
    
    // Water - Special case
    FCore_TerrainPhysicsProperties WaterProps;
    WaterProps.Friction = 0.1f;
    WaterProps.Restitution = 0.9f;
    WaterProps.Density = 1.0f;
    WaterProps.Stability = 0.0f;
    WaterProps.MovementSpeedMultiplier = 0.3f;
    WaterProps.SoundDampening = 1.0f;
    WaterProps.bCanSink = true;
    WaterProps.SinkRate = 1.0f;
    TerrainProperties.Add(ECore_TerrainType::Water, WaterProps);
    
    // Swamp - Combination of mud and water
    FCore_TerrainPhysicsProperties SwampProps;
    SwampProps.Friction = 0.3f;
    SwampProps.Restitution = 0.7f;
    SwampProps.Density = 1.2f;
    SwampProps.Stability = 0.4f;
    SwampProps.MovementSpeedMultiplier = 0.4f;
    SwampProps.SoundDampening = 0.95f;
    SwampProps.bCanSink = true;
    SwampProps.SinkRate = 0.2f;
    TerrainProperties.Add(ECore_TerrainType::Swamp, SwampProps);
    
    // Volcanic - Hot, unstable
    FCore_TerrainPhysicsProperties VolcanicProps;
    VolcanicProps.Friction = 0.8f;
    VolcanicProps.Restitution = 0.2f;
    VolcanicProps.Density = 2.0f;
    VolcanicProps.Stability = 0.8f;
    VolcanicProps.MovementSpeedMultiplier = 0.9f;
    VolcanicProps.SoundDampening = 0.3f;
    VolcanicProps.bCanSink = false;
    TerrainProperties.Add(ECore_TerrainType::Volcanic, VolcanicProps);
}

void UCore_TerrainPhysicsManager::InitializeBiomeTerrainMappings()
{
    BiomeTerrainMappings.Empty();
    
    // Forest biome
    FCore_BiomeTerrainMapping ForestMapping;
    ForestMapping.BiomeName = TEXT("Forest");
    ForestMapping.PrimaryTerrainTypes = {ECore_TerrainType::Grass, ECore_TerrainType::Dirt};
    ForestMapping.SecondaryTerrainTypes = {ECore_TerrainType::Rock, ECore_TerrainType::Mud};
    ForestMapping.TerrainVariation = 0.4f;
    BiomeTerrainMappings.Add(ForestMapping);
    
    // Swamp biome
    FCore_BiomeTerrainMapping SwampMapping;
    SwampMapping.BiomeName = TEXT("Swamp");
    SwampMapping.PrimaryTerrainTypes = {ECore_TerrainType::Swamp, ECore_TerrainType::Mud};
    SwampMapping.SecondaryTerrainTypes = {ECore_TerrainType::Water, ECore_TerrainType::Grass};
    SwampMapping.TerrainVariation = 0.6f;
    BiomeTerrainMappings.Add(SwampMapping);
    
    // Desert biome
    FCore_BiomeTerrainMapping DesertMapping;
    DesertMapping.BiomeName = TEXT("Desert");
    DesertMapping.PrimaryTerrainTypes = {ECore_TerrainType::Sand, ECore_TerrainType::Rock};
    DesertMapping.SecondaryTerrainTypes = {ECore_TerrainType::Dirt};
    DesertMapping.TerrainVariation = 0.3f;
    BiomeTerrainMappings.Add(DesertMapping);
    
    // Mountain biome
    FCore_BiomeTerrainMapping MountainMapping;
    MountainMapping.BiomeName = TEXT("Mountain");
    MountainMapping.PrimaryTerrainTypes = {ECore_TerrainType::Rock, ECore_TerrainType::Snow};
    MountainMapping.SecondaryTerrainTypes = {ECore_TerrainType::Dirt, ECore_TerrainType::Grass};
    MountainMapping.TerrainVariation = 0.5f;
    BiomeTerrainMappings.Add(MountainMapping);
    
    // Volcanic biome
    FCore_BiomeTerrainMapping VolcanicMapping;
    VolcanicMapping.BiomeName = TEXT("Volcanic");
    VolcanicMapping.PrimaryTerrainTypes = {ECore_TerrainType::Volcanic, ECore_TerrainType::Rock};
    VolcanicMapping.SecondaryTerrainTypes = {ECore_TerrainType::Dirt};
    VolcanicMapping.TerrainVariation = 0.4f;
    BiomeTerrainMappings.Add(VolcanicMapping);
}

void UCore_TerrainPhysicsManager::FindLandscapeActor()
{
    UWorld* World = GetWorld();
    if (!World)
        return;
        
    for (TActorIterator<ALandscape> ActorItr(World); ActorItr; ++ActorItr)
    {
        ALandscape* Landscape = *ActorItr;
        if (Landscape)
        {
            CachedLandscape = Landscape;
            UE_LOG(LogTemp, Warning, TEXT("Core_TerrainPhysicsManager: Found landscape actor: %s"), *Landscape->GetName());
            break;
        }
    }
}

ECore_TerrainType UCore_TerrainPhysicsManager::GetTerrainTypeAtLocation(const FVector& WorldLocation)
{
    return SampleTerrainAtPoint(WorldLocation);
}

FCore_TerrainPhysicsProperties UCore_TerrainPhysicsManager::GetTerrainPropertiesAtLocation(const FVector& WorldLocation)
{
    ECore_TerrainType TerrainType = GetTerrainTypeAtLocation(WorldLocation);
    
    if (TerrainProperties.Contains(TerrainType))
    {
        return TerrainProperties[TerrainType];
    }
    
    return GetDefaultPropertiesForTerrainType(ECore_TerrainType::Grass);
}

TArray<ECore_TerrainType> UCore_TerrainPhysicsManager::SampleTerrainInArea(const FVector& CenterLocation, float Radius, int32 SampleCount)
{
    TArray<ECore_TerrainType> SampledTypes;
    
    if (SampleCount <= 0)
        SampleCount = TerrainSamplePoints;
        
    if (Radius <= 0.0f)
        Radius = TerrainSampleRadius;
    
    float AngleStep = 360.0f / SampleCount;
    
    for (int32 i = 0; i < SampleCount; i++)
    {
        float Angle = i * AngleStep;
        float RadAngle = FMath::DegreesToRadians(Angle);
        
        FVector SampleLocation = CenterLocation + FVector(
            FMath::Cos(RadAngle) * Radius,
            FMath::Sin(RadAngle) * Radius,
            0.0f
        );
        
        ECore_TerrainType SampledType = SampleTerrainAtPoint(SampleLocation);
        SampledTypes.Add(SampledType);
    }
    
    return SampledTypes;
}

void UCore_TerrainPhysicsManager::ApplyTerrainPhysicsToActor(AActor* Actor, const FVector& Location)
{
    if (!Actor || !bEnableTerrainPhysics)
        return;
        
    FCore_TerrainPhysicsProperties Properties = GetTerrainPropertiesAtLocation(Location);
    
    // Apply to character movement if it's a character
    if (ACharacter* Character = Cast<ACharacter>(Actor))
    {
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            MovementComp->MaxWalkSpeed = MovementComp->MaxWalkSpeed * Properties.MovementSpeedMultiplier;
            MovementComp->GroundFriction = Properties.Friction;
        }
    }
    
    // Apply to physics components
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
    {
        if (PrimComp && PrimComp->IsSimulatingPhysics())
        {
            // Apply physics material properties
            // Note: This would typically involve creating or modifying physical materials
            // For now, we'll cache the properties for later use
        }
    }
    
    // Cache the properties for this actor
    ActorTerrainCache.Add(Actor, Properties);
}

float UCore_TerrainPhysicsManager::GetMovementSpeedMultiplierAtLocation(const FVector& WorldLocation)
{
    FCore_TerrainPhysicsProperties Properties = GetTerrainPropertiesAtLocation(WorldLocation);
    return Properties.MovementSpeedMultiplier;
}

bool UCore_TerrainPhysicsManager::CanSinkAtLocation(const FVector& WorldLocation, float& OutSinkRate)
{
    FCore_TerrainPhysicsProperties Properties = GetTerrainPropertiesAtLocation(WorldLocation);
    OutSinkRate = Properties.SinkRate;
    return Properties.bCanSink;
}

void UCore_TerrainPhysicsManager::UpdateTerrainPhysics()
{
    if (!bEnableTerrainPhysics)
        return;
        
    CleanupInvalidActors();
    
    CurrentQueryCount = 0;
    
    for (auto& ActorPtr : TrackedActors)
    {
        if (CurrentQueryCount >= MaxSimultaneousQueries)
            break;
            
        if (AActor* Actor = ActorPtr.Get())
        {
            UpdateActorTerrainPhysics(Actor);
            CurrentQueryCount++;
        }
    }
}

void UCore_TerrainPhysicsManager::RegisterActorForTerrainPhysics(AActor* Actor)
{
    if (!Actor || !IsValidForTerrainPhysics(Actor))
        return;
        
    TrackedActors.AddUnique(Actor);
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsManager: Registered actor %s for terrain physics"), *Actor->GetName());
}

void UCore_TerrainPhysicsManager::UnregisterActorFromTerrainPhysics(AActor* Actor)
{
    if (!Actor)
        return;
        
    TrackedActors.Remove(Actor);
    ActorTerrainCache.Remove(Actor);
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsManager: Unregistered actor %s from terrain physics"), *Actor->GetName());
}

FCore_BiomeTerrainMapping UCore_TerrainPhysicsManager::GetBiomeTerrainMapping(const FString& BiomeName)
{
    for (const FCore_BiomeTerrainMapping& Mapping : BiomeTerrainMappings)
    {
        if (Mapping.BiomeName == BiomeName)
        {
            return Mapping;
        }
    }
    
    // Return default mapping
    FCore_BiomeTerrainMapping DefaultMapping;
    DefaultMapping.BiomeName = TEXT("Default");
    DefaultMapping.PrimaryTerrainTypes = {ECore_TerrainType::Grass};
    DefaultMapping.SecondaryTerrainTypes = {ECore_TerrainType::Dirt};
    DefaultMapping.TerrainVariation = 0.3f;
    
    return DefaultMapping;
}

void UCore_TerrainPhysicsManager::SetTerrainProperties(ECore_TerrainType TerrainType, const FCore_TerrainPhysicsProperties& Properties)
{
    TerrainProperties.Add(TerrainType, Properties);
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsManager: Updated properties for terrain type %d"), (int32)TerrainType);
}

ECore_TerrainType UCore_TerrainPhysicsManager::SampleTerrainAtPoint(const FVector& WorldLocation)
{
    UWorld* World = GetWorld();
    if (!World)
        return ECore_TerrainType::Grass;
    
    // Simple terrain detection based on height and location
    // In a real implementation, this would query landscape layers or use texture sampling
    
    FVector TraceStart = WorldLocation + FVector(0, 0, 1000);
    FVector TraceEnd = WorldLocation - FVector(0, 0, 1000);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;
    
    if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
    {
        // Basic terrain type detection based on hit surface
        if (HitResult.GetActor())
        {
            FString ActorName = HitResult.GetActor()->GetName().ToLower();
            
            if (ActorName.Contains(TEXT("water")) || ActorName.Contains(TEXT("lake")) || ActorName.Contains(TEXT("river")))
                return ECore_TerrainType::Water;
            if (ActorName.Contains(TEXT("rock")) || ActorName.Contains(TEXT("stone")) || ActorName.Contains(TEXT("cliff")))
                return ECore_TerrainType::Rock;
            if (ActorName.Contains(TEXT("sand")) || ActorName.Contains(TEXT("beach")))
                return ECore_TerrainType::Sand;
            if (ActorName.Contains(TEXT("mud")) || ActorName.Contains(TEXT("swamp")))
                return ECore_TerrainType::Mud;
            if (ActorName.Contains(TEXT("snow")) || ActorName.Contains(TEXT("ice")))
                return ECore_TerrainType::Snow;
        }
        
        // Height-based terrain detection
        float Height = HitResult.Location.Z;
        
        if (Height > 500.0f)
            return ECore_TerrainType::Rock; // High altitude = rock
        else if (Height < -100.0f)
            return ECore_TerrainType::Water; // Below sea level = water
        else if (Height < 50.0f)
            return ECore_TerrainType::Sand; // Near sea level = sand
        else
            return ECore_TerrainType::Grass; // Default = grass
    }
    
    return ECore_TerrainType::Grass;
}

FCore_TerrainPhysicsProperties UCore_TerrainPhysicsManager::GetDefaultPropertiesForTerrainType(ECore_TerrainType TerrainType)
{
    if (TerrainProperties.Contains(TerrainType))
    {
        return TerrainProperties[TerrainType];
    }
    
    // Fallback default properties
    FCore_TerrainPhysicsProperties DefaultProps;
    return DefaultProps;
}

void UCore_TerrainPhysicsManager::UpdateActorTerrainPhysics(AActor* Actor)
{
    if (!Actor)
        return;
        
    FVector ActorLocation = Actor->GetActorLocation();
    ApplyTerrainPhysicsToActor(Actor, ActorLocation);
}

bool UCore_TerrainPhysicsManager::IsValidForTerrainPhysics(AActor* Actor)
{
    if (!Actor)
        return false;
        
    // Check if actor has physics components or is a character
    if (Cast<ACharacter>(Actor))
        return true;
        
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
    {
        if (PrimComp && PrimComp->IsSimulatingPhysics())
            return true;
    }
    
    return false;
}

void UCore_TerrainPhysicsManager::CleanupInvalidActors()
{
    TrackedActors.RemoveAll([](const TWeakObjectPtr<AActor>& ActorPtr)
    {
        return !ActorPtr.IsValid();
    });
    
    TArray<AActor*> InvalidActors;
    for (auto& Pair : ActorTerrainCache)
    {
        if (!IsValid(Pair.Key))
        {
            InvalidActors.Add(Pair.Key);
        }
    }
    
    for (AActor* InvalidActor : InvalidActors)
    {
        ActorTerrainCache.Remove(InvalidActor);
    }
}