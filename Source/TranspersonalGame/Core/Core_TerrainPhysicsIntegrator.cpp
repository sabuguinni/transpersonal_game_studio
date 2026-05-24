#include "Core_TerrainPhysicsIntegrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "PhysicsEngine/PhysicalMaterial.h"
#include "Landscape/LandscapeComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/StaticMeshActor.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

UCore_TerrainPhysicsIntegrator::UCore_TerrainPhysicsIntegrator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    bEnableTerrainPhysics = true;
    bEnableDynamicFriction = true;
    PhysicsUpdateInterval = 0.1f;
    LastPhysicsUpdateTime = 0.0f;
}

void UCore_TerrainPhysicsIntegrator::BeginPlay()
{
    Super::BeginPlay();
    
    if (bEnableTerrainPhysics)
    {
        InitializeDefaultBiomeConfigurations();
        InitializeTerrainPhysics();
        
        UE_LOG(LogTemp, Warning, TEXT("Core_TerrainPhysicsIntegrator: Initialized with %d biome configurations"), 
               BiomeConfigurations.Num());
    }
}

void UCore_TerrainPhysicsIntegrator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bEnableTerrainPhysics)
        return;
        
    LastPhysicsUpdateTime += DeltaTime;
    
    if (LastPhysicsUpdateTime >= PhysicsUpdateInterval)
    {
        // Update physics for tracked actors
        for (int32 i = TrackedPhysicsActors.Num() - 1; i >= 0; --i)
        {
            if (TrackedPhysicsActors[i].IsValid())
            {
                UpdateActorPhysicsProperties(TrackedPhysicsActors[i].Get());
            }
            else
            {
                TrackedPhysicsActors.RemoveAt(i);
            }
        }
        
        LastPhysicsUpdateTime = 0.0f;
    }
}

void UCore_TerrainPhysicsIntegrator::InitializeTerrainPhysics()
{
    if (BiomeConfigurations.Num() == 0)
    {
        InitializeDefaultBiomeConfigurations();
    }
    
    SetupBiomePhysicsMaterials();
    
    // Find and configure landscape actors
    UWorld* World = GetWorld();
    if (World)
    {
        for (TActorIterator<ALandscape> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            ALandscape* LandscapeActor = *ActorIterator;
            if (LandscapeActor)
            {
                ConfigureLandscapePhysics(LandscapeActor);
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_TerrainPhysicsIntegrator: Physics initialization complete"));
}

void UCore_TerrainPhysicsIntegrator::InitializeDefaultBiomeConfigurations()
{
    BiomeConfigurations.Empty();
    
    // Savanna (center)
    FCore_BiomePhysicsConfig SavannaConfig;
    SavannaConfig.TerrainType = ECore_TerrainType::Savanna;
    SavannaConfig.BiomeCenter = FVector(0.0f, 0.0f, 0.0f);
    SavannaConfig.BiomeRadius = 30000.0f;
    SavannaConfig.PhysicsProperties.Friction = 0.7f;
    SavannaConfig.PhysicsProperties.Restitution = 0.3f;
    SavannaConfig.PhysicsProperties.MovementSpeedMultiplier = 1.0f;
    BiomeConfigurations.Add(SavannaConfig);
    
    // Swamp (southwest)
    FCore_BiomePhysicsConfig SwampConfig;
    SwampConfig.TerrainType = ECore_TerrainType::Swamp;
    SwampConfig.BiomeCenter = FVector(-50000.0f, -45000.0f, 0.0f);
    SwampConfig.BiomeRadius = 25000.0f;
    SwampConfig.PhysicsProperties.Friction = 0.3f;
    SwampConfig.PhysicsProperties.Restitution = 0.1f;
    SwampConfig.PhysicsProperties.MovementSpeedMultiplier = 0.6f;
    BiomeConfigurations.Add(SwampConfig);
    
    // Forest (northwest)
    FCore_BiomePhysicsConfig ForestConfig;
    ForestConfig.TerrainType = ECore_TerrainType::Forest;
    ForestConfig.BiomeCenter = FVector(-45000.0f, 40000.0f, 0.0f);
    ForestConfig.BiomeRadius = 28000.0f;
    ForestConfig.PhysicsProperties.Friction = 0.8f;
    ForestConfig.PhysicsProperties.Restitution = 0.4f;
    ForestConfig.PhysicsProperties.MovementSpeedMultiplier = 0.8f;
    BiomeConfigurations.Add(ForestConfig);
    
    // Desert (east)
    FCore_BiomePhysicsConfig DesertConfig;
    DesertConfig.TerrainType = ECore_TerrainType::Desert;
    DesertConfig.BiomeCenter = FVector(55000.0f, 0.0f, 0.0f);
    DesertConfig.BiomeRadius = 32000.0f;
    DesertConfig.PhysicsProperties.Friction = 0.4f;
    DesertConfig.PhysicsProperties.Restitution = 0.2f;
    DesertConfig.PhysicsProperties.MovementSpeedMultiplier = 0.7f;
    BiomeConfigurations.Add(DesertConfig);
    
    // Mountain (northeast)
    FCore_BiomePhysicsConfig MountainConfig;
    MountainConfig.TerrainType = ECore_TerrainType::Mountain;
    MountainConfig.BiomeCenter = FVector(40000.0f, 50000.0f, 0.0f);
    MountainConfig.BiomeRadius = 30000.0f;
    MountainConfig.PhysicsProperties.Friction = 0.9f;
    MountainConfig.PhysicsProperties.Restitution = 0.6f;
    MountainConfig.PhysicsProperties.MovementSpeedMultiplier = 0.5f;
    BiomeConfigurations.Add(MountainConfig);
}

void UCore_TerrainPhysicsIntegrator::SetupBiomePhysicsMaterials()
{
    BiomePhysicsMaterials.Empty();
    
    for (const FCore_BiomePhysicsConfig& BiomeConfig : BiomeConfigurations)
    {
        CreatePhysicsMaterialForBiome(BiomeConfig.TerrainType, BiomeConfig.PhysicsProperties);
    }
}

void UCore_TerrainPhysicsIntegrator::CreatePhysicsMaterialForBiome(ECore_TerrainType BiomeType, const FCore_TerrainPhysicsProperties& Properties)
{
    FString MaterialName = FString::Printf(TEXT("PhysMat_%s"), 
        *UEnum::GetValueAsString(BiomeType));
    
    UPhysicalMaterial* PhysMaterial = NewObject<UPhysicalMaterial>(this, *MaterialName);
    if (PhysMaterial)
    {
        PhysMaterial->Friction = Properties.Friction;
        PhysMaterial->Restitution = Properties.Restitution;
        PhysMaterial->Density = Properties.Density;
        
        BiomePhysicsMaterials.Add(BiomeType, PhysMaterial);
        
        UE_LOG(LogTemp, Log, TEXT("Created physics material for biome: %s"), *MaterialName);
    }
}

FCore_TerrainPhysicsProperties UCore_TerrainPhysicsIntegrator::GetTerrainPropertiesAtLocation(const FVector& WorldLocation) const
{
    ECore_TerrainType BiomeType = GetBiomeTypeAtLocation(WorldLocation);
    
    for (const FCore_BiomePhysicsConfig& BiomeConfig : BiomeConfigurations)
    {
        if (BiomeConfig.TerrainType == BiomeType)
        {
            return BiomeConfig.PhysicsProperties;
        }
    }
    
    // Return default properties if no biome found
    return FCore_TerrainPhysicsProperties();
}

ECore_TerrainType UCore_TerrainPhysicsIntegrator::GetBiomeTypeAtLocation(const FVector& WorldLocation) const
{
    for (const FCore_BiomePhysicsConfig& BiomeConfig : BiomeConfigurations)
    {
        if (IsLocationInBiome(WorldLocation, BiomeConfig))
        {
            return BiomeConfig.TerrainType;
        }
    }
    
    return ECore_TerrainType::Savanna; // Default fallback
}

bool UCore_TerrainPhysicsIntegrator::IsLocationInBiome(const FVector& Location, const FCore_BiomePhysicsConfig& BiomeConfig) const
{
    float DistanceSquared = FVector::DistSquared(Location, BiomeConfig.BiomeCenter);
    float RadiusSquared = BiomeConfig.BiomeRadius * BiomeConfig.BiomeRadius;
    
    return DistanceSquared <= RadiusSquared;
}

void UCore_TerrainPhysicsIntegrator::ApplyPhysicsPropertiesToActor(AActor* TargetActor, const FCore_TerrainPhysicsProperties& Properties)
{
    if (!TargetActor)
        return;
        
    UPrimitiveComponent* PrimComp = TargetActor->FindComponentByClass<UPrimitiveComponent>();
    if (PrimComp)
    {
        // Apply physics properties
        PrimComp->SetUseCCD(Properties.bEnableComplexCollision);
        
        // Track this actor for future updates
        TrackedPhysicsActors.AddUnique(TargetActor);
    }
}

void UCore_TerrainPhysicsIntegrator::UpdateActorPhysicsProperties(AActor* Actor)
{
    if (!Actor || !bEnableDynamicFriction)
        return;
        
    FVector ActorLocation = Actor->GetActorLocation();
    FCore_TerrainPhysicsProperties TerrainProps = GetTerrainPropertiesAtLocation(ActorLocation);
    
    ApplyPhysicsPropertiesToActor(Actor, TerrainProps);
}

void UCore_TerrainPhysicsIntegrator::ConfigureLandscapePhysics(ALandscape* LandscapeActor)
{
    if (!LandscapeActor)
        return;
        
    // Configure landscape collision and physics
    ULandscapeComponent* LandscapeComp = LandscapeActor->FindComponentByClass<ULandscapeComponent>();
    if (LandscapeComp)
    {
        LandscapeComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        LandscapeComp->SetCollisionResponseToAllChannels(ECR_Block);
        
        UE_LOG(LogTemp, Log, TEXT("Configured landscape physics for: %s"), 
               *LandscapeActor->GetName());
    }
}

void UCore_TerrainPhysicsIntegrator::UpdateTerrainPhysicsForBiome(ECore_TerrainType BiomeType)
{
    UWorld* World = GetWorld();
    if (!World)
        return;
        
    // Find biome configuration
    const FCore_BiomePhysicsConfig* BiomeConfig = nullptr;
    for (const FCore_BiomePhysicsConfig& Config : BiomeConfigurations)
    {
        if (Config.TerrainType == BiomeType)
        {
            BiomeConfig = &Config;
            break;
        }
    }
    
    if (!BiomeConfig)
        return;
        
    // Update all actors in this biome
    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (Actor && IsLocationInBiome(Actor->GetActorLocation(), *BiomeConfig))
        {
            ApplyPhysicsPropertiesToActor(Actor, BiomeConfig->PhysicsProperties);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Updated terrain physics for biome: %s"), 
           *UEnum::GetValueAsString(BiomeType));
}

void UCore_TerrainPhysicsIntegrator::ValidateTerrainPhysicsSetup()
{
    UE_LOG(LogTemp, Warning, TEXT("=== Terrain Physics Validation ==="));
    UE_LOG(LogTemp, Warning, TEXT("Biome Configurations: %d"), BiomeConfigurations.Num());
    UE_LOG(LogTemp, Warning, TEXT("Physics Materials: %d"), BiomePhysicsMaterials.Num());
    UE_LOG(LogTemp, Warning, TEXT("Tracked Actors: %d"), TrackedPhysicsActors.Num());
    UE_LOG(LogTemp, Warning, TEXT("Terrain Physics Enabled: %s"), bEnableTerrainPhysics ? TEXT("Yes") : TEXT("No"));
    
    for (const FCore_BiomePhysicsConfig& Config : BiomeConfigurations)
    {
        UE_LOG(LogTemp, Warning, TEXT("Biome %s: Center(%s), Radius(%.1f)"), 
               *UEnum::GetValueAsString(Config.TerrainType),
               *Config.BiomeCenter.ToString(),
               Config.BiomeRadius);
    }
}

void UCore_TerrainPhysicsIntegrator::DebugDrawBiomeBoundaries()
{
    UWorld* World = GetWorld();
    if (!World)
        return;
        
    for (const FCore_BiomePhysicsConfig& Config : BiomeConfigurations)
    {
        FColor BiomeColor = FColor::Red;
        switch (Config.TerrainType)
        {
            case ECore_TerrainType::Savanna: BiomeColor = FColor::Yellow; break;
            case ECore_TerrainType::Swamp: BiomeColor = FColor::Green; break;
            case ECore_TerrainType::Forest: BiomeColor = FColor::Emerald; break;
            case ECore_TerrainType::Desert: BiomeColor = FColor::Orange; break;
            case ECore_TerrainType::Mountain: BiomeColor = FColor::Blue; break;
        }
        
        DrawDebugCircle(World, Config.BiomeCenter, Config.BiomeRadius, 32, BiomeColor, false, 5.0f);
    }
}

void UCore_TerrainPhysicsIntegrator::OptimizeTerrainPhysicsForPerformance()
{
    // Remove invalid tracked actors
    for (int32 i = TrackedPhysicsActors.Num() - 1; i >= 0; --i)
    {
        if (!TrackedPhysicsActors[i].IsValid())
        {
            TrackedPhysicsActors.RemoveAt(i);
        }
    }
    
    // Increase update interval if too many actors
    if (TrackedPhysicsActors.Num() > 1000)
    {
        PhysicsUpdateInterval = FMath::Max(0.2f, PhysicsUpdateInterval * 1.1f);
    }
    else if (TrackedPhysicsActors.Num() < 100)
    {
        PhysicsUpdateInterval = FMath::Max(0.05f, PhysicsUpdateInterval * 0.9f);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Optimized terrain physics: %d actors, %.3fs interval"), 
           TrackedPhysicsActors.Num(), PhysicsUpdateInterval);
}

int32 UCore_TerrainPhysicsIntegrator::GetActivePhysicsActorCount() const
{
    int32 ValidCount = 0;
    for (const TWeakObjectPtr<AActor>& ActorPtr : TrackedPhysicsActors)
    {
        if (ActorPtr.IsValid())
        {
            ValidCount++;
        }
    }
    return ValidCount;
}