// Copyright Transpersonal Game Studio. All Rights Reserved.
// Jurassic Architecture Core System Implementation
// Agent #07 - Architecture & Interior Agent
// CYCLE_ID: IMPLEMENTATION_001

#include "JurassicArchitectureCore.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialParameterCollection.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "PCGComponent.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogJurassicArchitecture, Log, All);

// ========================================
// FArchitecturalMaterial Implementation
// ========================================

FArchitecturalMaterial::FArchitecturalMaterial()
{
    PrimaryMaterial = EConstructionMaterial::Wood;
    MaterialQuality = 0.5f;
    ProcessingLevel = 0.3f;
    WeatheringLevel = 0.2f;
    MossGrowth = 0.1f;
    WaterDamage = 0.1f;
    InsectDamage = 0.05f;
    BaseMaterial = nullptr;
    WeatheredVariations.Empty();
    DamagedVariations.Empty();
}

void FArchitecturalMaterial::ApplyWeathering(float DeltaTime, float EnvironmentalFactor)
{
    // Simulate natural weathering over time
    float WeatheringRate = 0.001f * EnvironmentalFactor; // Base weathering rate
    
    WeatheringLevel = FMath::Clamp(WeatheringLevel + (WeatheringRate * DeltaTime), 0.0f, 1.0f);
    
    // Moss growth increases with moisture and time
    if (EnvironmentalFactor > 0.6f) // High moisture environment
    {
        MossGrowth = FMath::Clamp(MossGrowth + (WeatheringRate * 0.5f * DeltaTime), 0.0f, 1.0f);
    }
    
    // Water damage from rain and humidity
    WaterDamage = FMath::Clamp(WaterDamage + (WeatheringRate * 0.3f * DeltaTime), 0.0f, 1.0f);
    
    // Insect damage for organic materials
    if (PrimaryMaterial == EConstructionMaterial::Wood || PrimaryMaterial == EConstructionMaterial::Thatch)
    {
        InsectDamage = FMath::Clamp(InsectDamage + (WeatheringRate * 0.2f * DeltaTime), 0.0f, 1.0f);
    }
}

UMaterialInterface* FArchitecturalMaterial::GetCurrentMaterial() const
{
    // Return appropriate material based on weathering level
    if (WeatheringLevel < 0.3f && BaseMaterial)
    {
        return BaseMaterial;
    }
    else if (WeatheringLevel < 0.7f && WeatheredVariations.Num() > 0)
    {
        int32 Index = FMath::RandRange(0, WeatheredVariations.Num() - 1);
        return WeatheredVariations[Index];
    }
    else if (DamagedVariations.Num() > 0)
    {
        int32 Index = FMath::RandRange(0, DamagedVariations.Num() - 1);
        return DamagedVariations[Index];
    }
    
    return BaseMaterial; // Fallback
}

// ========================================
// FInteriorProp Implementation
// ========================================

FInteriorProp::FInteriorProp()
{
    PropName = TEXT("Generic Prop");
    PropMesh = nullptr;
    RelativeTransform = FTransform::Identity;
    bAttachToWall = false;
    bAttachToFloor = true;
    bAttachToCeiling = false;
    StoryContext = TEXT("A simple object left by the inhabitants");
    bIsClue = false;
    ClueText = TEXT("");
    ConditionLevel = 1.0f;
    bIsUsable = true;
    bIsMoveable = false;
    PropMaterial = FArchitecturalMaterial();
    SpawnProbability = 1.0f;
    Priority = 1;
}

bool FInteriorProp::ShouldSpawn(const FRandomStream& RandomStream) const
{
    return RandomStream.FRand() <= SpawnProbability;
}

FVector FInteriorProp::GetPlacementPosition(const FVector& RoomBounds, const FRandomStream& RandomStream) const
{
    FVector Position = FVector::ZeroVector;
    
    if (bAttachToFloor)
    {
        // Place on floor with some random variation
        Position.X = RandomStream.FRandRange(-RoomBounds.X * 0.4f, RoomBounds.X * 0.4f);
        Position.Y = RandomStream.FRandRange(-RoomBounds.Y * 0.4f, RoomBounds.Y * 0.4f);
        Position.Z = 0.0f;
    }
    else if (bAttachToWall)
    {
        // Place against a wall
        float WallChoice = RandomStream.FRand();
        if (WallChoice < 0.25f) // North wall
        {
            Position = FVector(RandomStream.FRandRange(-RoomBounds.X * 0.4f, RoomBounds.X * 0.4f), RoomBounds.Y * 0.45f, RandomStream.FRandRange(0.0f, RoomBounds.Z * 0.8f));
        }
        else if (WallChoice < 0.5f) // South wall
        {
            Position = FVector(RandomStream.FRandRange(-RoomBounds.X * 0.4f, RoomBounds.X * 0.4f), -RoomBounds.Y * 0.45f, RandomStream.FRandRange(0.0f, RoomBounds.Z * 0.8f));
        }
        else if (WallChoice < 0.75f) // East wall
        {
            Position = FVector(RoomBounds.X * 0.45f, RandomStream.FRandRange(-RoomBounds.Y * 0.4f, RoomBounds.Y * 0.4f), RandomStream.FRandRange(0.0f, RoomBounds.Z * 0.8f));
        }
        else // West wall
        {
            Position = FVector(-RoomBounds.X * 0.45f, RandomStream.FRandRange(-RoomBounds.Y * 0.4f, RoomBounds.Y * 0.4f), RandomStream.FRandRange(0.0f, RoomBounds.Z * 0.8f));
        }
    }
    else if (bAttachToCeiling)
    {
        // Hang from ceiling
        Position.X = RandomStream.FRandRange(-RoomBounds.X * 0.4f, RoomBounds.X * 0.4f);
        Position.Y = RandomStream.FRandRange(-RoomBounds.Y * 0.4f, RoomBounds.Y * 0.4f);
        Position.Z = RoomBounds.Z * 0.9f;
    }
    
    return Position + RelativeTransform.GetLocation();
}

// ========================================
// FInteriorLayout Implementation
// ========================================

FInteriorLayout::FInteriorLayout()
{
    LayoutName = TEXT("Basic Layout");
    StoryType = EInhabitationStory::ActiveFamily;
    InteriorBounds = FVector(400.0f, 400.0f, 250.0f);
    Density = EInteriorDensity::Functional;
    EssentialProps.Empty();
    OptionalProps.Empty();
    StoryProps.Empty();
    
    // Default storytelling elements
    bHasFireplace = false;
    bHasSleepingArea = true;
    bHasWorkArea = false;
    bHasStorageArea = true;
    bHasCookingArea = false;
    bHasBloodStains = false;
    bHasStruggleSigns = false;
    bHasPersonalItems = true;
}

void FInteriorLayout::GenerateLayout(UWorld* World, AActor* OwnerActor, const FVector& SpawnLocation, const FRandomStream& RandomStream)
{
    if (!World || !OwnerActor)
    {
        UE_LOG(LogJurassicArchitecture, Warning, TEXT("Cannot generate layout: Invalid World or OwnerActor"));
        return;
    }
    
    UE_LOG(LogJurassicArchitecture, Log, TEXT("Generating interior layout '%s' at location %s"), *LayoutName, *SpawnLocation.ToString());
    
    // Spawn essential props first
    for (const FInteriorProp& Prop : EssentialProps)
    {
        SpawnProp(World, OwnerActor, Prop, SpawnLocation, RandomStream);
    }
    
    // Spawn optional props based on probability
    for (const FInteriorProp& Prop : OptionalProps)
    {
        if (Prop.ShouldSpawn(RandomStream))
        {
            SpawnProp(World, OwnerActor, Prop, SpawnLocation, RandomStream);
        }
    }
    
    // Spawn story-specific props
    for (const FInteriorProp& Prop : StoryProps)
    {
        if (IsStoryPropRelevant(Prop) && Prop.ShouldSpawn(RandomStream))
        {
            SpawnProp(World, OwnerActor, Prop, SpawnLocation, RandomStream);
        }
    }
    
    // Add atmospheric storytelling elements
    GenerateStorytellingElements(World, OwnerActor, SpawnLocation, RandomStream);
}

void FInteriorLayout::SpawnProp(UWorld* World, AActor* OwnerActor, const FInteriorProp& Prop, const FVector& RoomCenter, const FRandomStream& RandomStream)
{
    if (!Prop.PropMesh)
    {
        UE_LOG(LogJurassicArchitecture, Warning, TEXT("Cannot spawn prop '%s': No mesh assigned"), *Prop.PropName);
        return;
    }
    
    // Calculate spawn position
    FVector SpawnPosition = RoomCenter + Prop.GetPlacementPosition(InteriorBounds, RandomStream);
    
    // Calculate spawn rotation with some random variation
    FRotator SpawnRotation = Prop.RelativeTransform.GetRotation().Rotator();
    SpawnRotation.Yaw += RandomStream.FRandRange(-15.0f, 15.0f); // Small random rotation
    
    // Create static mesh component
    UStaticMeshComponent* PropComponent = NewObject<UStaticMeshComponent>(OwnerActor);
    if (PropComponent)
    {
        PropComponent->SetStaticMesh(Prop.PropMesh);
        PropComponent->SetWorldLocation(SpawnPosition);
        PropComponent->SetWorldRotation(SpawnRotation);
        PropComponent->SetWorldScale3D(Prop.RelativeTransform.GetScale3D());
        PropComponent->AttachToComponent(OwnerActor->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
        
        // Apply material if specified
        if (UMaterialInterface* PropMaterial = Prop.PropMaterial.GetCurrentMaterial())
        {
            PropComponent->SetMaterial(0, PropMaterial);
        }
        
        // Set collision and physics properties
        PropComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        PropComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
        PropComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
        
        // Enable Nanite if supported
        PropComponent->SetForcedLodModel(0);
        
        UE_LOG(LogJurassicArchitecture, Log, TEXT("Spawned prop '%s' at location %s"), *Prop.PropName, *SpawnPosition.ToString());
    }
}

bool FInteriorLayout::IsStoryPropRelevant(const FInteriorProp& Prop) const
{
    // Check if this prop is relevant to the current story type
    switch (StoryType)
    {
        case EInhabitationStory::ActiveFamily:
            return Prop.StoryContext.Contains(TEXT("family")) || Prop.StoryContext.Contains(TEXT("children"));
            
        case EInhabitationStory::SingleSurvivor:
            return Prop.StoryContext.Contains(TEXT("lone")) || Prop.StoryContext.Contains(TEXT("survival"));
            
        case EInhabitationStory::AbandonedHurried:
            return Prop.StoryContext.Contains(TEXT("abandoned")) || Prop.StoryContext.Contains(TEXT("hasty"));
            
        case EInhabitationStory::TragedyScene:
            return Prop.StoryContext.Contains(TEXT("blood")) || Prop.StoryContext.Contains(TEXT("struggle"));
            
        default:
            return true; // Default to relevant
    }
}

void FInteriorLayout::GenerateStorytellingElements(UWorld* World, AActor* OwnerActor, const FVector& RoomCenter, const FRandomStream& RandomStream)
{
    // Generate atmospheric elements based on story flags
    if (bHasFireplace)
    {
        // TODO: Spawn fireplace with ash, charcoal, cooking implements
        UE_LOG(LogJurassicArchitecture, Log, TEXT("Generated fireplace storytelling elements"));
    }
    
    if (bHasBloodStains && StoryType == EInhabitationStory::TragedyScene)
    {
        // TODO: Spawn blood decals and struggle evidence
        UE_LOG(LogJurassicArchitecture, Log, TEXT("Generated tragedy storytelling elements"));
    }
    
    if (bHasPersonalItems)
    {
        // TODO: Spawn personal belongings that tell the inhabitant's story
        UE_LOG(LogJurassicArchitecture, Log, TEXT("Generated personal item storytelling elements"));
    }
    
    // Add wear patterns and lived-in details based on density
    switch (Density)
    {
        case EInteriorDensity::Cluttered:
            // TODO: Add extra clutter and accumulated possessions
            break;
            
        case EInteriorDensity::Abandoned:
            // TODO: Add dust, cobwebs, scattered items
            break;
            
        case EInteriorDensity::Minimal:
            // TODO: Keep only essential items
            break;
            
        default:
            break;
    }
}

int32 FInteriorLayout::GetPropCount() const
{
    return EssentialProps.Num() + OptionalProps.Num() + StoryProps.Num();
}

float FInteriorLayout::GetLayoutComplexity() const
{
    float Complexity = 0.0f;
    
    // Base complexity from prop count
    Complexity += GetPropCount() * 0.1f;
    
    // Density modifier
    switch (Density)
    {
        case EInteriorDensity::Minimal:
            Complexity *= 0.5f;
            break;
        case EInteriorDensity::Cluttered:
            Complexity *= 1.5f;
            break;
        default:
            break;
    }
    
    // Story complexity modifier
    if (bHasFireplace) Complexity += 0.2f;
    if (bHasWorkArea) Complexity += 0.3f;
    if (bHasBloodStains) Complexity += 0.4f;
    if (bHasStruggleSigns) Complexity += 0.3f;
    
    return FMath::Clamp(Complexity, 0.1f, 2.0f);
}

// ========================================
// Utility Functions
// ========================================

FString GetArchitecturalPeriodString(EArchitecturalPeriod Period)
{
    switch (Period)
    {
        case EArchitecturalPeriod::Emergency:
            return TEXT("Emergency Construction");
        case EArchitecturalPeriod::Survival:
            return TEXT("Survival Phase");
        case EArchitecturalPeriod::Settlement:
            return TEXT("Settlement Phase");
        case EArchitecturalPeriod::Established:
            return TEXT("Established Community");
        case EArchitecturalPeriod::Abandoned:
            return TEXT("Abandoned");
        case EArchitecturalPeriod::Ruins:
            return TEXT("Ruins");
        default:
            return TEXT("Unknown");
    }
}

FString GetStructuralIntegrityString(EStructuralIntegrity Integrity)
{
    switch (Integrity)
    {
        case EStructuralIntegrity::Perfect:
            return TEXT("Perfect");
        case EStructuralIntegrity::Excellent:
            return TEXT("Excellent");
        case EStructuralIntegrity::Good:
            return TEXT("Good");
        case EStructuralIntegrity::Fair:
            return TEXT("Fair");
        case EStructuralIntegrity::Poor:
            return TEXT("Poor");
        case EStructuralIntegrity::Failing:
            return TEXT("Failing");
        case EStructuralIntegrity::Collapsed:
            return TEXT("Collapsed");
        case EStructuralIntegrity::Destroyed:
            return TEXT("Destroyed");
        default:
            return TEXT("Unknown");
    }
}

FLinearColor GetIntegrityColor(EStructuralIntegrity Integrity)
{
    switch (Integrity)
    {
        case EStructuralIntegrity::Perfect:
        case EStructuralIntegrity::Excellent:
            return FLinearColor::Green;
        case EStructuralIntegrity::Good:
        case EStructuralIntegrity::Fair:
            return FLinearColor::Yellow;
        case EStructuralIntegrity::Poor:
        case EStructuralIntegrity::Failing:
            return FLinearColor(1.0f, 0.5f, 0.0f); // Orange
        case EStructuralIntegrity::Collapsed:
        case EStructuralIntegrity::Destroyed:
            return FLinearColor::Red;
        default:
            return FLinearColor::Gray;
    }
}

float CalculateEnvironmentalFactor(EJurassicBiomeType BiomeType, float Humidity, float Temperature)
{
    float Factor = 0.5f; // Base factor
    
    // Biome-specific modifiers
    switch (BiomeType)
    {
        case EJurassicBiomeType::TropicalRainforest:
            Factor += 0.4f; // High weathering
            break;
        case EJurassicBiomeType::SwampWetlands:
            Factor += 0.3f; // High moisture
            break;
        case EJurassicBiomeType::TemperateForest:
            Factor += 0.2f; // Moderate weathering
            break;
        case EJurassicBiomeType::Savanna:
            Factor += 0.1f; // Lower weathering
            break;
        case EJurassicBiomeType::Desert:
            Factor -= 0.1f; // Dry preservation
            break;
        case EJurassicBiomeType::Tundra:
            Factor -= 0.2f; // Cold preservation
            break;
        default:
            break;
    }
    
    // Humidity and temperature modifiers
    Factor += (Humidity - 0.5f) * 0.3f;
    Factor += (Temperature - 0.5f) * 0.2f;
    
    return FMath::Clamp(Factor, 0.1f, 1.0f);
}