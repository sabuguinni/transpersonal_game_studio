#include "Core_BiomePhysicsManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UCore_BiomePhysicsManager::UCore_BiomePhysicsManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update every 100ms for performance
    
    // Initialize default settings
    PhysicsUpdateFrequency = 0.1f;
    MaxPhysicsDistance = 5000.0f;
    bEnablePhysicsOptimization = true;
    
    BiomeManager = nullptr;
    LastCacheUpdate = 0.0f;
    LastPhysicsUpdate = 0.0f;
}

void UCore_BiomePhysicsManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Get BiomeManager reference
    if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this))
    {
        BiomeManager = GameInstance->GetSubsystem<UEng_BiomeManager>();
        if (!BiomeManager)
        {
            UE_LOG(LogTemp, Warning, TEXT("BiomePhysicsManager: BiomeManager subsystem not found"));
        }
    }
    
    // Initialize default biome physics parameters
    InitializeDefaultBiomePhysics();
    
    // Cache nearby actors for performance
    CacheNearbyActors();
}

void UCore_BiomePhysicsManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bEnablePhysicsOptimization)
        return;
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update cache periodically
    if (CurrentTime - LastCacheUpdate > 1.0f) // Update cache every second
    {
        CacheNearbyActors();
        LastCacheUpdate = CurrentTime;
    }
    
    // Update physics for cached actors
    if (CurrentTime - LastPhysicsUpdate > PhysicsUpdateFrequency)
    {
        UpdateCachedActorPhysics();
        LastPhysicsUpdate = CurrentTime;
    }
}

FCore_BiomePhysicsParameters UCore_BiomePhysicsManager::GetPhysicsParametersAtLocation(const FVector& WorldLocation) const
{
    if (!BiomeManager)
    {
        // Return default grassland parameters if BiomeManager not available
        FCore_BiomePhysicsParameters DefaultParams;
        DefaultParams.BiomeType = EEng_BiomeType::Grassland;
        return DefaultParams;
    }
    
    // Get biome type at location
    EEng_BiomeType BiomeType = BiomeManager->GetBiomeAtLocation(WorldLocation);
    
    // Return cached parameters for this biome
    if (const FCore_BiomePhysicsParameters* Params = BiomePhysicsMap.Find(BiomeType))
    {
        return *Params;
    }
    
    // Return default if not found
    FCore_BiomePhysicsParameters DefaultParams;
    DefaultParams.BiomeType = BiomeType;
    return DefaultParams;
}

void UCore_BiomePhysicsManager::ApplyBiomePhysicsToActor(AActor* Actor, const FVector& Location)
{
    if (!Actor || !BiomeManager)
        return;
    
    FCore_BiomePhysicsParameters PhysicsParams = GetPhysicsParametersAtLocation(Location);
    
    // Apply physics parameters to all primitive components
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
    {
        if (!PrimComp)
            continue;
        
        // Set collision channel
        PrimComp->SetCollisionObjectType(PhysicsParams.CollisionChannel);
        
        // Apply physics material properties (if component supports it)
        if (UStaticMeshComponent* StaticMeshComp = Cast<UStaticMeshComponent>(PrimComp))
        {
            // Create or modify physics material
            // Note: In a full implementation, we would create UPhysicalMaterial assets
            // For now, we set basic physics properties
            StaticMeshComp->SetMassOverrideInKg(NAME_None, PhysicsParams.Density * 100.0f);
        }
        
        // Set collision response based on biome
        SetupCollisionLayersForActor(Actor, PhysicsParams.BiomeType);
    }
}

void UCore_BiomePhysicsManager::UpdateActorPhysicsForCurrentBiome(AActor* Actor)
{
    if (!Actor)
        return;
    
    FVector ActorLocation = Actor->GetActorLocation();
    ApplyBiomePhysicsToActor(Actor, ActorLocation);
}

TEnumAsByte<ECollisionChannel> UCore_BiomePhysicsManager::GetCollisionChannelForBiome(EEng_BiomeType BiomeType, const FString& ObjectType) const
{
    // Find collision layer mapping for this biome
    for (const FCore_CollisionLayerMapping& Mapping : CollisionLayerMappings)
    {
        if (Mapping.BiomeType == BiomeType)
        {
            if (ObjectType == TEXT("Ground"))
                return Mapping.GroundChannel;
            else if (ObjectType == TEXT("Vegetation"))
                return Mapping.VegetationChannel;
            else if (ObjectType == TEXT("Rock"))
                return Mapping.RockChannel;
            else if (ObjectType == TEXT("Water"))
                return Mapping.WaterChannel;
        }
    }
    
    // Default to WorldStatic
    return ECC_WorldStatic;
}

void UCore_BiomePhysicsManager::SetupCollisionLayersForActor(AActor* Actor, EEng_BiomeType BiomeType)
{
    if (!Actor)
        return;
    
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
    {
        if (!PrimComp)
            continue;
        
        // Determine object type based on actor name or component type
        FString ObjectType = TEXT("Ground"); // Default
        FString ActorName = Actor->GetName();
        
        if (ActorName.Contains(TEXT("Tree")) || ActorName.Contains(TEXT("Vegetation")))
            ObjectType = TEXT("Vegetation");
        else if (ActorName.Contains(TEXT("Rock")) || ActorName.Contains(TEXT("Stone")))
            ObjectType = TEXT("Rock");
        else if (ActorName.Contains(TEXT("Water")) || ActorName.Contains(TEXT("Lake")) || ActorName.Contains(TEXT("River")))
            ObjectType = TEXT("Water");
        
        // Set appropriate collision channel
        TEnumAsByte<ECollisionChannel> CollisionChannel = GetCollisionChannelForBiome(BiomeType, ObjectType);
        PrimComp->SetCollisionObjectType(CollisionChannel);
    }
}

float UCore_BiomePhysicsManager::GetMovementSpeedMultiplier(const FVector& Location) const
{
    FCore_BiomePhysicsParameters Params = GetPhysicsParametersAtLocation(Location);
    return Params.MovementSpeedMultiplier;
}

float UCore_BiomePhysicsManager::GetStaminaDrainMultiplier(const FVector& Location) const
{
    FCore_BiomePhysicsParameters Params = GetPhysicsParametersAtLocation(Location);
    return Params.StaminaDrainMultiplier;
}

bool UCore_BiomePhysicsManager::CanJumpAtLocation(const FVector& Location) const
{
    FCore_BiomePhysicsParameters Params = GetPhysicsParametersAtLocation(Location);
    return Params.bAllowJumping;
}

bool UCore_BiomePhysicsManager::CanDestroyAtLocation(const FVector& Location, float ImpactForce) const
{
    FCore_BiomePhysicsParameters Params = GetPhysicsParametersAtLocation(Location);
    
    if (!Params.bEnableDestruction)
        return false;
    
    float AdjustedThreshold = Params.DestructionThreshold * Params.RockHardness;
    return ImpactForce >= AdjustedThreshold;
}

void UCore_BiomePhysicsManager::ProcessDestruction(AActor* Actor, const FVector& ImpactLocation, float ImpactForce)
{
    if (!Actor || !CanDestroyAtLocation(ImpactLocation, ImpactForce))
        return;
    
    // Basic destruction implementation
    // In a full implementation, this would spawn debris, particles, sounds, etc.
    
    UE_LOG(LogTemp, Log, TEXT("Destroying actor %s at location %s with force %f"), 
           *Actor->GetName(), *ImpactLocation.ToString(), ImpactForce);
    
    // For now, just destroy the actor
    Actor->Destroy();
}

float UCore_BiomePhysicsManager::GetTemperatureDamageRate(const FVector& Location) const
{
    FCore_BiomePhysicsParameters Params = GetPhysicsParametersAtLocation(Location);
    return Params.TemperatureDamageRate;
}

bool UCore_BiomePhysicsManager::CausesSlowdown(const FVector& Location) const
{
    FCore_BiomePhysicsParameters Params = GetPhysicsParametersAtLocation(Location);
    return Params.bCausesSlowdown;
}

float UCore_BiomePhysicsManager::GetNoiseLevel(const FVector& Location) const
{
    FCore_BiomePhysicsParameters Params = GetPhysicsParametersAtLocation(Location);
    return Params.NoiseLevel;
}

void UCore_BiomePhysicsManager::SetBiomePhysicsParameters(EEng_BiomeType BiomeType, const FCore_BiomePhysicsParameters& Parameters)
{
    BiomePhysicsMap.Add(BiomeType, Parameters);
}

void UCore_BiomePhysicsManager::InitializeDefaultBiomePhysics()
{
    // Clear existing parameters
    BiomePhysicsMap.Empty();
    CollisionLayerMappings.Empty();
    
    // GRASSLAND - Easy movement, low stamina drain
    FCore_BiomePhysicsParameters GrasslandParams;
    GrasslandParams.BiomeType = EEng_BiomeType::Grassland;
    GrasslandParams.Friction = 0.6f;
    GrasslandParams.MovementSpeedMultiplier = 1.0f;
    GrasslandParams.StaminaDrainMultiplier = 1.0f;
    GrasslandParams.DestructionThreshold = 800.0f;
    GrasslandParams.RockHardness = 0.8f;
    BiomePhysicsMap.Add(EEng_BiomeType::Grassland, GrasslandParams);
    
    // FOREST - Slower movement, higher stamina drain due to obstacles
    FCore_BiomePhysicsParameters ForestParams;
    ForestParams.BiomeType = EEng_BiomeType::Forest;
    ForestParams.Friction = 0.8f;
    ForestParams.MovementSpeedMultiplier = 0.7f;
    ForestParams.StaminaDrainMultiplier = 1.3f;
    ForestParams.DestructionThreshold = 1200.0f;
    ForestParams.RockHardness = 1.0f;
    ForestParams.bCausesSlowdown = true;
    BiomePhysicsMap.Add(EEng_BiomeType::Forest, ForestParams);
    
    // DESERT - Fast movement but high stamina drain due to heat
    FCore_BiomePhysicsParameters DesertParams;
    DesertParams.BiomeType = EEng_BiomeType::Desert;
    DesertParams.Friction = 0.4f; // Sand is slippery
    DesertParams.MovementSpeedMultiplier = 1.1f;
    DesertParams.StaminaDrainMultiplier = 1.8f;
    DesertParams.TemperatureDamageRate = 0.5f;
    DesertParams.DestructionThreshold = 600.0f; // Sand is easier to destroy
    DesertParams.RockHardness = 0.6f;
    BiomePhysicsMap.Add(EEng_BiomeType::Desert, DesertParams);
    
    // SWAMP - Very slow movement, high stamina drain, no jumping
    FCore_BiomePhysicsParameters SwampParams;
    SwampParams.BiomeType = EEng_BiomeType::Swamp;
    SwampParams.Friction = 1.2f; // Sticky mud
    SwampParams.MovementSpeedMultiplier = 0.4f;
    SwampParams.StaminaDrainMultiplier = 2.0f;
    SwampParams.bAllowJumping = false;
    SwampParams.bCausesSlowdown = true;
    SwampParams.DestructionThreshold = 400.0f; // Soft ground
    SwampParams.RockHardness = 0.3f;
    SwampParams.NoiseLevel = 1.5f; // Splashing sounds
    BiomePhysicsMap.Add(EEng_BiomeType::Swamp, SwampParams);
    
    // MOUNTAINS - Slow movement, very high stamina drain, hard rocks
    FCore_BiomePhysicsParameters MountainParams;
    MountainParams.BiomeType = EEng_BiomeType::Mountains;
    MountainParams.Friction = 0.9f;
    MountainParams.MovementSpeedMultiplier = 0.6f;
    MountainParams.StaminaDrainMultiplier = 1.6f;
    MountainParams.TemperatureDamageRate = 0.2f; // Cold damage
    MountainParams.DestructionThreshold = 2000.0f; // Very hard rocks
    MountainParams.RockHardness = 2.0f;
    BiomePhysicsMap.Add(EEng_BiomeType::Mountains, MountainParams);
    
    // RIVER - Water physics, no jumping, slow movement
    FCore_BiomePhysicsParameters RiverParams;
    RiverParams.BiomeType = EEng_BiomeType::River;
    RiverParams.Friction = 0.2f; // Slippery when wet
    RiverParams.MovementSpeedMultiplier = 0.3f;
    RiverParams.StaminaDrainMultiplier = 1.5f;
    RiverParams.bAllowJumping = false;
    RiverParams.bCausesSlowdown = true;
    RiverParams.bEnableDestruction = false; // Can't destroy water
    RiverParams.NoiseLevel = 1.2f; // Water sounds
    BiomePhysicsMap.Add(EEng_BiomeType::River, RiverParams);
    
    // Setup collision layer mappings
    for (int32 i = 0; i < (int32)EEng_BiomeType::Count; ++i)
    {
        FCore_CollisionLayerMapping Mapping;
        Mapping.BiomeType = (EEng_BiomeType)i;
        Mapping.GroundChannel = ECC_WorldStatic;
        Mapping.VegetationChannel = ECC_WorldDynamic;
        Mapping.RockChannel = ECC_Destructible;
        Mapping.WaterChannel = ECC_WorldStatic;
        CollisionLayerMappings.Add(Mapping);
    }
    
    UE_LOG(LogTemp, Log, TEXT("BiomePhysicsManager: Initialized default physics parameters for %d biomes"), BiomePhysicsMap.Num());
}

void UCore_BiomePhysicsManager::ValidatePhysicsConfiguration()
{
    UE_LOG(LogTemp, Warning, TEXT("=== BIOME PHYSICS VALIDATION ==="));
    
    // Check if all biome types have physics parameters
    for (int32 i = 0; i < (int32)EEng_BiomeType::Count; ++i)
    {
        EEng_BiomeType BiomeType = (EEng_BiomeType)i;
        if (BiomePhysicsMap.Contains(BiomeType))
        {
            UE_LOG(LogTemp, Log, TEXT("✓ Biome %d has physics parameters"), (int32)BiomeType);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("✗ Biome %d missing physics parameters"), (int32)BiomeType);
        }
    }
    
    // Check collision layer mappings
    UE_LOG(LogTemp, Log, TEXT("Collision layer mappings: %d"), CollisionLayerMappings.Num());
    
    // Check BiomeManager reference
    if (BiomeManager)
    {
        UE_LOG(LogTemp, Log, TEXT("✓ BiomeManager reference valid"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("✗ BiomeManager reference invalid"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== VALIDATION COMPLETE ==="));
}

void UCore_BiomePhysicsManager::DebugDrawPhysicsInfo(const FVector& Location)
{
    if (!GetWorld())
        return;
    
    FCore_BiomePhysicsParameters Params = GetPhysicsParametersAtLocation(Location);
    
    // Draw debug sphere at location
    DrawDebugSphere(GetWorld(), Location, 100.0f, 12, FColor::Red, false, 5.0f);
    
    // Draw debug text with physics info
    FString DebugText = FString::Printf(TEXT("Biome: %d\nSpeed: %.2f\nStamina: %.2f\nFriction: %.2f"), 
                                       (int32)Params.BiomeType, 
                                       Params.MovementSpeedMultiplier,
                                       Params.StaminaDrainMultiplier,
                                       Params.Friction);
    
    DrawDebugString(GetWorld(), Location + FVector(0, 0, 150), DebugText, nullptr, FColor::White, 5.0f);
}

void UCore_BiomePhysicsManager::CacheNearbyActors()
{
    if (!GetOwner())
        return;
    
    CachedNearbyActors.Empty();
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    UWorld* World = GetWorld();
    
    if (!World)
        return;
    
    // Find all actors within MaxPhysicsDistance
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor || Actor == GetOwner())
            continue;
        
        float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
        if (Distance <= MaxPhysicsDistance)
        {
            CachedNearbyActors.Add(Actor);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("BiomePhysicsManager: Cached %d nearby actors"), CachedNearbyActors.Num());
}

void UCore_BiomePhysicsManager::UpdateCachedActorPhysics()
{
    // Update physics for all cached actors
    for (auto& ActorPtr : CachedNearbyActors)
    {
        if (AActor* Actor = ActorPtr.Get())
        {
            UpdateActorPhysicsForCurrentBiome(Actor);
        }
    }
}

FCore_BiomePhysicsParameters UCore_BiomePhysicsManager::BlendPhysicsParameters(const FCore_BiomePhysicsParameters& ParamsA, const FCore_BiomePhysicsParameters& ParamsB, float BlendFactor) const
{
    FCore_BiomePhysicsParameters BlendedParams = ParamsA;
    
    // Blend numeric values
    BlendedParams.Friction = FMath::Lerp(ParamsA.Friction, ParamsB.Friction, BlendFactor);
    BlendedParams.Restitution = FMath::Lerp(ParamsA.Restitution, ParamsB.Restitution, BlendFactor);
    BlendedParams.Density = FMath::Lerp(ParamsA.Density, ParamsB.Density, BlendFactor);
    BlendedParams.MovementSpeedMultiplier = FMath::Lerp(ParamsA.MovementSpeedMultiplier, ParamsB.MovementSpeedMultiplier, BlendFactor);
    BlendedParams.StaminaDrainMultiplier = FMath::Lerp(ParamsA.StaminaDrainMultiplier, ParamsB.StaminaDrainMultiplier, BlendFactor);
    BlendedParams.DestructionThreshold = FMath::Lerp(ParamsA.DestructionThreshold, ParamsB.DestructionThreshold, BlendFactor);
    BlendedParams.RockHardness = FMath::Lerp(ParamsA.RockHardness, ParamsB.RockHardness, BlendFactor);
    BlendedParams.TemperatureDamageRate = FMath::Lerp(ParamsA.TemperatureDamageRate, ParamsB.TemperatureDamageRate, BlendFactor);
    BlendedParams.NoiseLevel = FMath::Lerp(ParamsA.NoiseLevel, ParamsB.NoiseLevel, BlendFactor);
    
    // Boolean values - use threshold
    BlendedParams.bAllowJumping = BlendFactor < 0.5f ? ParamsA.bAllowJumping : ParamsB.bAllowJumping;
    BlendedParams.bEnableDestruction = BlendFactor < 0.5f ? ParamsA.bEnableDestruction : ParamsB.bEnableDestruction;
    BlendedParams.bCausesSlowdown = BlendFactor < 0.5f ? ParamsA.bCausesSlowdown : ParamsB.bCausesSlowdown;
    
    return BlendedParams;
}