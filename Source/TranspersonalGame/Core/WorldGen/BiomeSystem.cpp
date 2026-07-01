#include "BiomeSystem.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

// ============================================================
// UBiomeDetectorComponent
// ============================================================

UBiomeDetectorComponent::UBiomeDetectorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f;  // Check every 0.5s for performance
}

void UBiomeDetectorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Initial biome detection
    DetectCurrentBiome();
}

void UBiomeDetectorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    TimeSinceLastCheck += DeltaTime;
    if (TimeSinceLastCheck >= BiomeCheckInterval)
    {
        TimeSinceLastCheck = 0.0f;
        DetectCurrentBiome();
    }
}

void UBiomeDetectorComponent::DetectCurrentBiome()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    FVector OwnerLocation = Owner->GetActorLocation();
    EWorld_BiomeType DetectedBiome = SampleBiomeAtLocation(OwnerLocation);

    if (DetectedBiome != CurrentBiome)
    {
        PreviousBiome = CurrentBiome;
        CurrentBiome = DetectedBiome;

        // Set up transition data
        ActiveTransition.FromBiome = PreviousBiome;
        ActiveTransition.ToBiome = CurrentBiome;
        ActiveTransition.BlendAlpha = 1.0f;

        // Broadcast biome change event
        OnBiomeChanged.Broadcast(PreviousBiome, CurrentBiome);
    }
}

EWorld_BiomeType UBiomeDetectorComponent::SampleBiomeAtLocation(const FVector& Location) const
{
    UWorld* World = GetWorld();
    if (!World) return EWorld_BiomeType::None;

    // Find all ABiomeWorldActor in the level and check which one contains this location
    TArray<AActor*> BiomeActors;
    UGameplayStatics::GetAllActorsOfClass(World, ABiomeWorldActor::StaticClass(), BiomeActors);

    float ClosestDistance = FLT_MAX;
    EWorld_BiomeType ClosestBiome = EWorld_BiomeType::OpenSavanna;  // Default to savanna

    for (AActor* Actor : BiomeActors)
    {
        ABiomeWorldActor* BiomeActor = Cast<ABiomeWorldActor>(Actor);
        if (!BiomeActor) continue;

        if (BiomeActor->IsLocationInBiome(Location))
        {
            float Dist = FVector::Dist(Location, BiomeActor->GetActorLocation());
            if (Dist < ClosestDistance)
            {
                ClosestDistance = Dist;
                ClosestBiome = BiomeActor->BiomeType;
            }
        }
    }

    return ClosestBiome;
}

FWorld_BiomeParameters UBiomeDetectorComponent::GetCurrentBiomeParameters() const
{
    return GetBiomeParametersForType(CurrentBiome);
}

FWorld_BiomeParameters UBiomeDetectorComponent::GetBiomeParametersForType(EWorld_BiomeType BiomeType)
{
    FWorld_BiomeParameters Params;
    Params.BiomeType = BiomeType;

    switch (BiomeType)
    {
    case EWorld_BiomeType::JungleForest:
        Params.Temperature = 32.0f;
        Params.Humidity = 0.90f;
        Params.VegetationDensity = 0.95f;
        Params.DangerLevel = 0.70f;
        Params.FogColor = FLinearColor(0.3f, 0.5f, 0.3f, 1.0f);
        Params.FogDensity = 0.04f;
        Params.AmbientColor = FLinearColor(0.2f, 0.4f, 0.2f, 1.0f);
        Params.PlayerStaminaDrainMultiplier = 1.4f;
        Params.PlayerHungerDrainMultiplier = 1.2f;
        Params.PlayerThirstDrainMultiplier = 1.5f;
        Params.DinosaurSpawnWeight = 1.8f;
        break;

    case EWorld_BiomeType::VolcanicPlains:
        Params.Temperature = 55.0f;
        Params.Humidity = 0.10f;
        Params.VegetationDensity = 0.10f;
        Params.DangerLevel = 0.90f;
        Params.FogColor = FLinearColor(0.6f, 0.3f, 0.1f, 1.0f);
        Params.FogDensity = 0.06f;
        Params.AmbientColor = FLinearColor(0.5f, 0.2f, 0.05f, 1.0f);
        Params.PlayerStaminaDrainMultiplier = 1.8f;
        Params.PlayerHungerDrainMultiplier = 1.3f;
        Params.PlayerThirstDrainMultiplier = 2.0f;
        Params.DinosaurSpawnWeight = 0.6f;
        break;

    case EWorld_BiomeType::RiverWetlands:
        Params.Temperature = 28.0f;
        Params.Humidity = 0.95f;
        Params.VegetationDensity = 0.75f;
        Params.DangerLevel = 0.60f;
        Params.FogColor = FLinearColor(0.4f, 0.5f, 0.6f, 1.0f);
        Params.FogDensity = 0.05f;
        Params.AmbientColor = FLinearColor(0.3f, 0.4f, 0.5f, 1.0f);
        Params.PlayerStaminaDrainMultiplier = 1.2f;
        Params.PlayerHungerDrainMultiplier = 0.9f;
        Params.PlayerThirstDrainMultiplier = 0.5f;  // Water available
        Params.DinosaurSpawnWeight = 1.5f;
        break;

    case EWorld_BiomeType::RockyHighlands:
        Params.Temperature = 15.0f;
        Params.Humidity = 0.30f;
        Params.VegetationDensity = 0.20f;
        Params.DangerLevel = 0.50f;
        Params.FogColor = FLinearColor(0.5f, 0.5f, 0.55f, 1.0f);
        Params.FogDensity = 0.02f;
        Params.AmbientColor = FLinearColor(0.45f, 0.45f, 0.50f, 1.0f);
        Params.PlayerStaminaDrainMultiplier = 1.6f;  // Climbing is hard
        Params.PlayerHungerDrainMultiplier = 1.4f;
        Params.PlayerThirstDrainMultiplier = 1.3f;
        Params.DinosaurSpawnWeight = 0.8f;
        break;

    case EWorld_BiomeType::OpenSavanna:
    default:
        Params.Temperature = 30.0f;
        Params.Humidity = 0.35f;
        Params.VegetationDensity = 0.40f;
        Params.DangerLevel = 0.65f;
        Params.FogColor = FLinearColor(0.7f, 0.65f, 0.5f, 1.0f);
        Params.FogDensity = 0.01f;
        Params.AmbientColor = FLinearColor(0.65f, 0.60f, 0.45f, 1.0f);
        Params.PlayerStaminaDrainMultiplier = 1.0f;
        Params.PlayerHungerDrainMultiplier = 1.1f;
        Params.PlayerThirstDrainMultiplier = 1.2f;
        Params.DinosaurSpawnWeight = 1.2f;
        break;
    }

    return Params;
}

void UBiomeDetectorComponent::SetCurrentBiome(EWorld_BiomeType NewBiome)
{
    if (NewBiome != CurrentBiome)
    {
        PreviousBiome = CurrentBiome;
        CurrentBiome = NewBiome;
        OnBiomeChanged.Broadcast(PreviousBiome, CurrentBiome);
    }
}

// ============================================================
// ABiomeWorldActor
// ============================================================

ABiomeWorldActor::ABiomeWorldActor()
{
    PrimaryActorTick.bCanEverTick = false;

    BiomeBoundsComponent = CreateDefaultSubobject<USphereComponent>(TEXT("BiomeBounds"));
    BiomeBoundsComponent->SetSphereRadius(3000.0f);
    BiomeBoundsComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    BiomeBoundsComponent->SetVisibility(false);
    RootComponent = BiomeBoundsComponent;
}

void ABiomeWorldActor::BeginPlay()
{
    Super::BeginPlay();

    // Sync sphere radius with BiomeRadius property
    if (BiomeBoundsComponent)
    {
        BiomeBoundsComponent->SetSphereRadius(BiomeRadius);
    }
}

bool ABiomeWorldActor::IsLocationInBiome(const FVector& WorldLocation) const
{
    float Dist = FVector::Dist2D(WorldLocation, GetActorLocation());
    return Dist <= BiomeRadius;
}

float ABiomeWorldActor::GetBlendAlphaAtLocation(const FVector& WorldLocation) const
{
    float Dist = FVector::Dist2D(WorldLocation, GetActorLocation());
    if (Dist >= BiomeRadius) return 0.0f;

    float InnerRadius = BiomeRadius - TransitionBlendDistance;
    if (Dist <= InnerRadius) return 1.0f;

    // Blend zone
    float BlendDist = Dist - InnerRadius;
    return 1.0f - (BlendDist / TransitionBlendDistance);
}

void ABiomeWorldActor::ApplyBiomeDefaults()
{
    // Apply default parameters based on BiomeType
    BiomeParameters = UBiomeDetectorComponent::GetBiomeParametersForType(BiomeType);

    // Sync sphere radius
    if (BiomeBoundsComponent)
    {
        BiomeBoundsComponent->SetSphereRadius(BiomeRadius);
    }

    UE_LOG(LogTemp, Log, TEXT("ABiomeWorldActor: Applied defaults for biome type %d"), (int32)BiomeType);
}
