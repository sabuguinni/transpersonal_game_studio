#include "Core_BiomePhysicsSystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

UCore_BiomePhysicsSystem::UCore_BiomePhysicsSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize default properties
    InitializeBiomeProperties();
}

void UCore_BiomePhysicsSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Set initial biome based on spawn location
    if (AActor* Owner = GetOwner())
    {
        UpdatePhysicsForLocation(Owner->GetActorLocation());
    }
    
    LastUpdateTime = GetWorld()->GetTimeSeconds();
}

void UCore_BiomePhysicsSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bEnableDetailedPhysics)
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastUpdateTime < UpdateFrequency)
    {
        return;
    }
    
    // Get player location for distance checks
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC || !PC->GetPawn())
    {
        return;
    }
    
    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
    float DistanceToPlayer = FVector::Dist(GetOwner()->GetActorLocation(), PlayerLocation);
    
    // Only update physics for objects near the player
    if (DistanceToPlayer <= MaxUpdateDistance)
    {
        UpdatePhysicsForLocation(GetOwner()->GetActorLocation());
        LastPlayerLocation = PlayerLocation;
    }
    
    LastUpdateTime = CurrentTime;
}

void UCore_BiomePhysicsSystem::SetBiomePhysics(ECore_BiomePhysicsType BiomeType)
{
    CurrentBiome = BiomeType;
    
    if (AActor* Owner = GetOwner())
    {
        ApplyBiomePhysicsToActor(Owner);
    }
}

FCore_BiomePhysicsProperties UCore_BiomePhysicsSystem::GetCurrentBiomeProperties() const
{
    if (const FCore_BiomePhysicsProperties* Properties = BiomeProperties.Find(CurrentBiome))
    {
        return *Properties;
    }
    
    // Return default properties if not found
    return FCore_BiomePhysicsProperties();
}

void UCore_BiomePhysicsSystem::ApplyBiomePhysicsToActor(AActor* TargetActor)
{
    if (!TargetActor)
    {
        return;
    }
    
    FCore_BiomePhysicsProperties Properties = GetCurrentBiomeProperties();
    
    // Apply physics properties to all primitive components
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    TargetActor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
    {
        if (PrimComp && PrimComp->GetBodyInstance())
        {
            FBodyInstance* BodyInstance = PrimComp->GetBodyInstance();
            
            // Apply friction
            if (BodyInstance->GetSimulatePhysics())
            {
                PrimComp->SetPhysMaterialOverride(nullptr); // Reset first
                
                // Create dynamic physics material
                // Note: In a full implementation, we'd create and cache UPhysicalMaterial assets
                // For now, we'll modify the body instance directly
                
                // Apply density changes through mass scaling
                float MassScale = Properties.Density;
                BodyInstance->SetMassScale(MassScale);
                
                // Apply restitution and friction would require a proper PhysicalMaterial
                // This is a simplified implementation for demonstration
            }
        }
    }
}

void UCore_BiomePhysicsSystem::UpdatePhysicsForLocation(const FVector& WorldLocation)
{
    ECore_BiomePhysicsType DetectedBiome = DetectBiomeAtLocation(WorldLocation);
    
    if (DetectedBiome != CurrentBiome)
    {
        SetBiomePhysics(DetectedBiome);
    }
}

void UCore_BiomePhysicsSystem::ApplyWindEffect(const FVector& WindDirection, float WindStrength)
{
    if (!GetOwner())
    {
        return;
    }
    
    FCore_BiomePhysicsProperties Properties = GetCurrentBiomeProperties();
    float EffectiveWindForce = WindStrength * (1.0f - Properties.WindResistance);
    
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    GetOwner()->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
    {
        if (PrimComp && PrimComp->IsSimulatingPhysics())
        {
            FVector WindForce = WindDirection.GetSafeNormal() * EffectiveWindForce;
            PrimComp->AddForce(WindForce);
        }
    }
}

void UCore_BiomePhysicsSystem::ApplyTemperatureEffect(float Temperature)
{
    FCore_BiomePhysicsProperties Properties = GetCurrentBiomeProperties();
    
    // Temperature affects material properties
    float TemperatureModifier = 1.0f + (Temperature * Properties.TemperatureEffect);
    
    // Apply temperature-based modifications
    // In a full implementation, this would affect material brittleness, expansion, etc.
}

void UCore_BiomePhysicsSystem::ApplyMoistureEffect(float MoistureLevel)
{
    // Moisture affects friction and stability
    FCore_BiomePhysicsProperties Properties = GetCurrentBiomeProperties();
    
    if (MoistureLevel > 0.7f) // High moisture
    {
        // Reduce friction and stability
        Properties.Friction *= 0.8f;
        Properties.GroundStability *= 0.9f;
    }
}

float UCore_BiomePhysicsSystem::CalculateGroundFriction(const FVector& Location) const
{
    FCore_BiomePhysicsProperties Properties = GetCurrentBiomeProperties();
    
    // Base friction from biome
    float BaseFriction = Properties.Friction;
    
    // Modify based on local conditions
    if (IsLocationInWater(Location))
    {
        BaseFriction *= 0.3f; // Slippery when wet
    }
    
    return FMath::Clamp(BaseFriction, 0.1f, 2.0f);
}

float UCore_BiomePhysicsSystem::CalculateGroundStability(const FVector& Location) const
{
    FCore_BiomePhysicsProperties Properties = GetCurrentBiomeProperties();
    
    float BaseStability = Properties.GroundStability;
    
    // Modify based on biome-specific conditions
    switch (CurrentBiome)
    {
        case ECore_BiomePhysicsType::Swamp:
            BaseStability *= 0.6f; // Unstable swamp ground
            break;
        case ECore_BiomePhysicsType::Desert:
            BaseStability *= 0.8f; // Shifting sand
            break;
        case ECore_BiomePhysicsType::Mountain:
            BaseStability *= 1.2f; // Solid rock
            break;
        default:
            break;
    }
    
    return FMath::Clamp(BaseStability, 0.1f, 2.0f);
}

bool UCore_BiomePhysicsSystem::IsLocationInWater(const FVector& Location) const
{
    // Simple water detection - in a full implementation this would check water volumes
    // For now, assume swamp areas have water
    return CurrentBiome == ECore_BiomePhysicsType::Swamp && Location.Z < 100.0f;
}

ECore_BiomePhysicsType UCore_BiomePhysicsSystem::DetectBiomeAtLocation(const FVector& Location) const
{
    // Biome detection based on world coordinates
    // This matches the biome layout from the memories
    
    float X = Location.X;
    float Y = Location.Y;
    
    // Swamp: X=-50000, Y=-45000
    if (X < -25000.0f && Y < -20000.0f)
    {
        return ECore_BiomePhysicsType::Swamp;
    }
    
    // Forest: X=-45000, Y=40000
    if (X < -20000.0f && Y > 20000.0f)
    {
        return ECore_BiomePhysicsType::Forest;
    }
    
    // Desert: X=55000, Y=0
    if (X > 30000.0f && FMath::Abs(Y) < 25000.0f)
    {
        return ECore_BiomePhysicsType::Desert;
    }
    
    // Mountain: X=40000, Y=50000
    if (X > 20000.0f && Y > 25000.0f)
    {
        return ECore_BiomePhysicsType::Mountain;
    }
    
    // Default to Savanna (center: X=0, Y=0)
    return ECore_BiomePhysicsType::Savanna;
}

float UCore_BiomePhysicsSystem::CalculateBiomeTransition(const FVector& Location, ECore_BiomePhysicsType& OutPrimaryBiome, ECore_BiomePhysicsType& OutSecondaryBiome, float& OutBlendFactor) const
{
    // Detect primary biome
    OutPrimaryBiome = DetectBiomeAtLocation(Location);
    
    // Calculate distance to biome centers for blending
    FVector BiomeCenters[5] = {
        FVector(0.0f, 0.0f, 0.0f),           // Savanna
        FVector(-50000.0f, -45000.0f, 0.0f), // Swamp
        FVector(-45000.0f, 40000.0f, 0.0f),  // Forest
        FVector(55000.0f, 0.0f, 0.0f),       // Desert
        FVector(40000.0f, 50000.0f, 0.0f)    // Mountain
    };
    
    float MinDistance = FLT_MAX;
    float SecondMinDistance = FLT_MAX;
    int32 PrimaryIndex = 0;
    int32 SecondaryIndex = 0;
    
    for (int32 i = 0; i < 5; i++)
    {
        float Distance = FVector::Dist2D(Location, BiomeCenters[i]);
        if (Distance < MinDistance)
        {
            SecondMinDistance = MinDistance;
            SecondaryIndex = PrimaryIndex;
            MinDistance = Distance;
            PrimaryIndex = i;
        }
        else if (Distance < SecondMinDistance)
        {
            SecondMinDistance = Distance;
            SecondaryIndex = i;
        }
    }
    
    OutSecondaryBiome = static_cast<ECore_BiomePhysicsType>(SecondaryIndex);
    
    // Calculate blend factor based on distance
    float BlendDistance = 10000.0f; // 10km transition zone
    OutBlendFactor = FMath::Clamp((SecondMinDistance - MinDistance) / BlendDistance, 0.0f, 1.0f);
    
    return MinDistance;
}

void UCore_BiomePhysicsSystem::InitializeBiomeProperties()
{
    // Savanna - balanced properties
    FCore_BiomePhysicsProperties SavannaProps;
    SavannaProps.Friction = 0.7f;
    SavannaProps.Restitution = 0.3f;
    SavannaProps.Density = 1.0f;
    SavannaProps.WindResistance = 0.2f;
    SavannaProps.GroundStability = 1.0f;
    SavannaProps.TemperatureEffect = 0.01f;
    BiomeProperties.Add(ECore_BiomePhysicsType::Savanna, SavannaProps);
    
    // Swamp - low friction, unstable
    FCore_BiomePhysicsProperties SwampProps;
    SwampProps.Friction = 0.3f;
    SwampProps.Restitution = 0.1f;
    SwampProps.Density = 1.2f;
    SwampProps.WindResistance = 0.4f;
    SwampProps.GroundStability = 0.6f;
    SwampProps.TemperatureEffect = 0.005f;
    BiomeProperties.Add(ECore_BiomePhysicsType::Swamp, SwampProps);
    
    // Forest - high friction, stable
    FCore_BiomePhysicsProperties ForestProps;
    ForestProps.Friction = 0.9f;
    ForestProps.Restitution = 0.4f;
    ForestProps.Density = 0.9f;
    ForestProps.WindResistance = 0.6f;
    ForestProps.GroundStability = 1.1f;
    ForestProps.TemperatureEffect = 0.002f;
    BiomeProperties.Add(ECore_BiomePhysicsType::Forest, ForestProps);
    
    // Desert - shifting, hot
    FCore_BiomePhysicsProperties DesertProps;
    DesertProps.Friction = 0.5f;
    DesertProps.Restitution = 0.2f;
    DesertProps.Density = 0.8f;
    DesertProps.WindResistance = 0.1f;
    DesertProps.GroundStability = 0.8f;
    DesertProps.TemperatureEffect = 0.02f;
    BiomeProperties.Add(ECore_BiomePhysicsType::Desert, DesertProps);
    
    // Mountain - solid, cold
    FCore_BiomePhysicsProperties MountainProps;
    MountainProps.Friction = 1.0f;
    MountainProps.Restitution = 0.5f;
    MountainProps.Density = 1.3f;
    MountainProps.WindResistance = 0.3f;
    MountainProps.GroundStability = 1.4f;
    MountainProps.TemperatureEffect = -0.01f;
    BiomeProperties.Add(ECore_BiomePhysicsType::Mountain, MountainProps);
}