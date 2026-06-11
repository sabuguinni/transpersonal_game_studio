#include "Arch_StructuralSystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

AArch_StructuralSystem::AArch_StructuralSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create structure mesh component
    StructureMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StructureMesh"));
    StructureMesh->SetupAttachment(RootComponent);
    StructureMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    StructureMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);

    // Create interaction volume
    InteractionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionVolume"));
    InteractionVolume->SetupAttachment(RootComponent);
    InteractionVolume->SetBoxExtent(FVector(200.0f, 200.0f, 200.0f));
    InteractionVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionVolume->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
    InteractionVolume->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    InteractionVolume->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    // Create discovery trigger
    DiscoveryTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("DiscoveryTrigger"));
    DiscoveryTrigger->SetupAttachment(RootComponent);
    DiscoveryTrigger->SetBoxExtent(FVector(500.0f, 500.0f, 300.0f));
    DiscoveryTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    DiscoveryTrigger->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
    DiscoveryTrigger->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    DiscoveryTrigger->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    // Bind overlap events
    DiscoveryTrigger->OnComponentBeginOverlap.AddDynamic(this, &AArch_StructuralSystem::OnDiscoveryTriggerBeginOverlap);

    // Initialize default structure data
    StructureData.StructureType = EArch_StructureType::Pillar;
    StructureData.WeatherState = EArch_WeatherState::Weathered;
    StructureData.StructuralIntegrity = 0.75f;
    StructureData.Age = 1000.0f;
    StructureData.bIsDiscovered = false;
    StructureData.HistoricalContext = TEXT("An ancient pillar from a forgotten civilization, weathered by countless seasons.");

    // Initialize discovery properties
    DiscoveryRadius = 1000.0f;
    bRequiresExamination = true;
    DiscoveryText = TEXT("You discover an ancient structure, its purpose lost to time...");

    // Initialize environmental storytelling
    EnvironmentalClues.Add(TEXT("Carved symbols suggest this was once part of a larger complex"));
    EnvironmentalClues.Add(TEXT("Wear patterns indicate heavy use in ancient times"));
    EnvironmentalClues.Add(TEXT("The stone shows signs of careful craftsmanship"));
    
    bHasHiddenSecrets = false;
    SecretLocation = FVector::ZeroVector;
}

void AArch_StructuralSystem::BeginPlay()
{
    Super::BeginPlay();
    
    SetupStructureMesh();
    ApplyWeatherEffects();
    ConfigureDiscoverySystem();
}

void AArch_StructuralSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update structural integrity over time (very slow degradation)
    if (StructureData.StructuralIntegrity > 0.0f)
    {
        float DegradationRate = 0.0001f; // Very slow degradation
        StructureData.StructuralIntegrity = FMath::Max(0.0f, StructureData.StructuralIntegrity - (DegradationRate * DeltaTime));
    }

    // Age the structure
    StructureData.Age += DeltaTime;
}

void AArch_StructuralSystem::InitializeStructure(EArch_StructureType Type, EArch_WeatherState Weather)
{
    StructureData.StructureType = Type;
    StructureData.WeatherState = Weather;
    
    SetupStructureMesh();
    ApplyWeatherEffects();
    
    // Set appropriate historical context based on type
    switch (Type)
    {
        case EArch_StructureType::Pillar:
            StructureData.HistoricalContext = TEXT("A weathered stone pillar, possibly marking an important location or serving as a structural support.");
            break;
        case EArch_StructureType::Archway:
            StructureData.HistoricalContext = TEXT("An ancient archway that once served as an entrance to a larger structure, now standing alone.");
            break;
        case EArch_StructureType::Platform:
            StructureData.HistoricalContext = TEXT("A raised stone platform where ceremonies or gatherings may have taken place in ancient times.");
            break;
        case EArch_StructureType::Ruin:
            StructureData.HistoricalContext = TEXT("The remains of a once-grand structure, now partially collapsed and reclaimed by nature.");
            break;
        case EArch_StructureType::Monument:
            StructureData.HistoricalContext = TEXT("A monumental structure built to commemorate something significant, its meaning lost to time.");
            break;
        case EArch_StructureType::Foundation:
            StructureData.HistoricalContext = TEXT("The foundation stones of a building, all that remains of what was once here.");
            break;
        default:
            StructureData.HistoricalContext = TEXT("An ancient structure of unknown purpose and origin.");
            break;
    }
}

void AArch_StructuralSystem::UpdateWeatherState(EArch_WeatherState NewState)
{
    StructureData.WeatherState = NewState;
    ApplyWeatherEffects();
}

void AArch_StructuralSystem::TriggerDiscovery(AActor* DiscoveringActor)
{
    if (!StructureData.bIsDiscovered && DiscoveringActor)
    {
        StructureData.bIsDiscovered = true;
        
        // Log discovery
        UE_LOG(LogTemp, Log, TEXT("Structure discovered by %s"), *DiscoveringActor->GetName());
        
        // Display discovery text
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, DiscoveryText);
        }
        
        // Trigger Blueprint event
        OnStructureDiscovered(DiscoveringActor);
        
        // Reveal environmental clues
        RevealEnvironmentalClues();
    }
}

void AArch_StructuralSystem::RevealEnvironmentalClues()
{
    for (const FString& Clue : EnvironmentalClues)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, Clue);
        }
        UE_LOG(LogTemp, Log, TEXT("Environmental Clue: %s"), *Clue);
    }
}

void AArch_StructuralSystem::OnDiscoveryTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // Check if the overlapping actor is a character (player)
    if (ACharacter* Character = Cast<ACharacter>(OtherActor))
    {
        TriggerDiscovery(Character);
    }
}

void AArch_StructuralSystem::SetupStructureMesh()
{
    if (StructureMesh && StructureMeshes.Num() > 0)
    {
        int32 MeshIndex = static_cast<int32>(StructureData.StructureType);
        if (StructureMeshes.IsValidIndex(MeshIndex) && StructureMeshes[MeshIndex])
        {
            StructureMesh->SetStaticMesh(StructureMeshes[MeshIndex]);
        }
    }
}

void AArch_StructuralSystem::ApplyWeatherEffects()
{
    if (StructureMesh && WeatherMaterials.Num() > 0)
    {
        int32 MaterialIndex = static_cast<int32>(StructureData.WeatherState);
        if (WeatherMaterials.IsValidIndex(MaterialIndex) && WeatherMaterials[MaterialIndex])
        {
            StructureMesh->SetMaterial(0, WeatherMaterials[MaterialIndex]);
        }
    }
    
    // Adjust structural integrity based on weather state
    switch (StructureData.WeatherState)
    {
        case EArch_WeatherState::Pristine:
            StructureData.StructuralIntegrity = FMath::Min(1.0f, StructureData.StructuralIntegrity + 0.1f);
            break;
        case EArch_WeatherState::Weathered:
            // No change - this is the default state
            break;
        case EArch_WeatherState::Overgrown:
            StructureData.StructuralIntegrity = FMath::Max(0.3f, StructureData.StructuralIntegrity - 0.1f);
            break;
        case EArch_WeatherState::Ruined:
            StructureData.StructuralIntegrity = FMath::Max(0.1f, StructureData.StructuralIntegrity - 0.2f);
            break;
        case EArch_WeatherState::Collapsed:
            StructureData.StructuralIntegrity = 0.0f;
            break;
    }
}

void AArch_StructuralSystem::ConfigureDiscoverySystem()
{
    if (DiscoveryTrigger)
    {
        DiscoveryTrigger->SetBoxExtent(FVector(DiscoveryRadius, DiscoveryRadius, DiscoveryRadius * 0.5f));
    }
}