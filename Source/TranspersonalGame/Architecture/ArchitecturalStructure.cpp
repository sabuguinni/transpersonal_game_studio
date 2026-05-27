#include "ArchitecturalStructure.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

AArchitecturalStructure::AArchitecturalStructure()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create structure mesh component
    StructureMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StructureMesh"));
    StructureMesh->SetupAttachment(RootComponent);
    StructureMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    StructureMesh->SetCollisionResponseToAllChannels(ECR_Block);

    // Create interaction volume
    InteractionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionVolume"));
    InteractionVolume->SetupAttachment(RootComponent);
    InteractionVolume->SetBoxExtent(FVector(600.0f, 600.0f, 300.0f));
    InteractionVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Initialize structure data
    StructureData.StructureType = EArch_StructureType::Shelter;
    StructureData.DurabilityMax = 1000.0f;
    StructureData.DurabilityCurrent = 1000.0f;
    StructureData.bCanProvideShade = true;
    StructureData.bCanProvideShelter = true;
    StructureData.ShelterRadius = 500.0f;

    BiomeType = EBiomeType::Savana;
    bPlayerInShelter = false;
    WeatherDecayRate = 1.0f;
}

void AArchitecturalStructure::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    if (InteractionVolume)
    {
        InteractionVolume->OnComponentBeginOverlap.AddDynamic(this, &AArchitecturalStructure::OnInteractionVolumeBeginOverlap);
        InteractionVolume->OnComponentEndOverlap.AddDynamic(this, &AArchitecturalStructure::OnInteractionVolumeEndOverlap);
    }

    // Adjust decay rate based on biome
    switch (BiomeType)
    {
        case EBiomeType::Pantano:
            WeatherDecayRate = 2.0f; // Faster decay in swamp
            break;
        case EBiomeType::Deserto:
            WeatherDecayRate = 1.5f; // Sand erosion
            break;
        case EBiomeType::Montanha:
            WeatherDecayRate = 0.5f; // Slower decay in mountains
            break;
        default:
            WeatherDecayRate = 1.0f;
            break;
    }
}

void AArchitecturalStructure::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Apply weather decay over time
    if (StructureData.DurabilityCurrent > 0.0f)
    {
        float DecayAmount = WeatherDecayRate * DeltaTime * 0.1f; // Very slow decay
        StructureData.DurabilityCurrent = FMath::Max(0.0f, StructureData.DurabilityCurrent - DecayAmount);

        // Destroy structure if durability reaches zero
        if (StructureData.DurabilityCurrent <= 0.0f)
        {
            OnStructureDestroyed();
        }
    }
}

void AArchitecturalStructure::TakeDamage(float DamageAmount)
{
    if (DamageAmount <= 0.0f) return;

    StructureData.DurabilityCurrent = FMath::Max(0.0f, StructureData.DurabilityCurrent - DamageAmount);

    if (StructureData.DurabilityCurrent <= 0.0f)
    {
        OnStructureDestroyed();
    }
}

void AArchitecturalStructure::RepairStructure(float RepairAmount)
{
    if (RepairAmount <= 0.0f) return;

    StructureData.DurabilityCurrent = FMath::Min(StructureData.DurabilityMax, StructureData.DurabilityCurrent + RepairAmount);
}

float AArchitecturalStructure::GetDurabilityPercentage() const
{
    if (StructureData.DurabilityMax <= 0.0f) return 0.0f;
    return (StructureData.DurabilityCurrent / StructureData.DurabilityMax) * 100.0f;
}

bool AArchitecturalStructure::IsInShelterRange(FVector TestLocation) const
{
    if (!StructureData.bCanProvideShelter) return false;

    float Distance = FVector::Dist(GetActorLocation(), TestLocation);
    return Distance <= StructureData.ShelterRadius;
}

void AArchitecturalStructure::SetStructureType(EArch_StructureType NewType)
{
    StructureData.StructureType = NewType;

    // Adjust properties based on structure type
    switch (NewType)
    {
        case EArch_StructureType::Shelter:
            StructureData.bCanProvideShelter = true;
            StructureData.bCanProvideShade = true;
            StructureData.ShelterRadius = 500.0f;
            break;
        case EArch_StructureType::Monument:
            StructureData.bCanProvideShelter = false;
            StructureData.bCanProvideShade = true;
            StructureData.ShelterRadius = 200.0f;
            StructureData.DurabilityMax = 2000.0f; // Monuments are more durable
            break;
        case EArch_StructureType::Wall:
            StructureData.bCanProvideShelter = false;
            StructureData.bCanProvideShade = true;
            StructureData.ShelterRadius = 100.0f;
            break;
        case EArch_StructureType::Pillar:
            StructureData.bCanProvideShelter = false;
            StructureData.bCanProvideShade = false;
            StructureData.ShelterRadius = 50.0f;
            break;
        case EArch_StructureType::Platform:
            StructureData.bCanProvideShelter = false;
            StructureData.bCanProvideShade = false;
            StructureData.ShelterRadius = 300.0f;
            break;
        case EArch_StructureType::Bridge:
            StructureData.bCanProvideShelter = false;
            StructureData.bCanProvideShade = true;
            StructureData.ShelterRadius = 200.0f;
            break;
    }

    StructureData.DurabilityCurrent = StructureData.DurabilityMax;
}

void AArchitecturalStructure::OnInteractionVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor) return;

    // Check if it's the player character
    if (OtherActor->IsA<APawn>() && OtherActor->ActorHasTag(TEXT("Player")))
    {
        if (StructureData.bCanProvideShelter && !bPlayerInShelter)
        {
            bPlayerInShelter = true;
            OnPlayerEnterShelter();
        }
    }
}

void AArchitecturalStructure::OnInteractionVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor) return;

    // Check if it's the player character
    if (OtherActor->IsA<APawn>() && OtherActor->ActorHasTag(TEXT("Player")))
    {
        if (bPlayerInShelter)
        {
            bPlayerInShelter = false;
            OnPlayerExitShelter();
        }
    }
}