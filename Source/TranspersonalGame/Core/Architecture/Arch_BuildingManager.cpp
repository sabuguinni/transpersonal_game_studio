#include "Arch_BuildingManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMesh.h"

AArch_BuildingManager::AArch_BuildingManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create building mesh component
    BuildingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BuildingMesh"));
    BuildingMesh->SetupAttachment(RootComponent);
    BuildingMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    BuildingMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);

    // Create interior volume for occupancy detection
    InteriorVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteriorVolume"));
    InteriorVolume->SetupAttachment(RootComponent);
    InteriorVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteriorVolume->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    InteriorVolume->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    InteriorVolume->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
    InteriorVolume->SetBoxExtent(FVector(400.0f, 400.0f, 250.0f));

    // Create exterior volume for approach detection
    ExteriorVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("ExteriorVolume"));
    ExteriorVolume->SetupAttachment(RootComponent);
    ExteriorVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    ExteriorVolume->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    ExteriorVolume->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    ExteriorVolume->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
    ExteriorVolume->SetBoxExtent(FVector(600.0f, 600.0f, 350.0f));

    // Initialize building data
    BuildingData.BuildingType = EArch_BuildingType::StoneRuin;
    BuildingData.Dimensions = FVector(500.0f, 500.0f, 300.0f);
    BuildingData.StructuralIntegrity = 100.0f;
    BuildingData.bIsHabitable = true;
    BuildingData.MaxOccupants = 4;
    BuildingData.WeatherProtection = 0.8f;
    BuildingData.bHasFirePit = false;

    // Setup default interior spawn points
    BuildingData.InteriorSpawnPoints.Add(FVector(0.0f, 0.0f, 0.0f));
    BuildingData.InteriorSpawnPoints.Add(FVector(200.0f, 200.0f, 0.0f));
    BuildingData.InteriorSpawnPoints.Add(FVector(-200.0f, 200.0f, 0.0f));
    BuildingData.InteriorSpawnPoints.Add(FVector(0.0f, -200.0f, 0.0f));

    // Initialize state
    bIsPlayerInside = false;
    LastOccupancyCheck = 0.0f;
}

void AArch_BuildingManager::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    if (InteriorVolume)
    {
        InteriorVolume->OnComponentBeginOverlap.AddDynamic(this, &AArch_BuildingManager::OnInteriorVolumeBeginOverlap);
        InteriorVolume->OnComponentEndOverlap.AddDynamic(this, &AArch_BuildingManager::OnInteriorVolumeEndOverlap);
    }

    // Configure building based on type
    ConfigureBuildingData(BuildingData.BuildingType);
    SetupBuildingMeshForType(BuildingData.BuildingType);
    SetupCollisionVolumes();

    UE_LOG(LogTemp, Warning, TEXT("Arch_BuildingManager: Building initialized - Type: %d, Integrity: %f"), 
           (int32)BuildingData.BuildingType, BuildingData.StructuralIntegrity);
}

void AArch_BuildingManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Periodic occupancy check
    LastOccupancyCheck += DeltaTime;
    if (LastOccupancyCheck >= OccupancyCheckInterval)
    {
        LastOccupancyCheck = 0.0f;

        // Clean up null actors from occupants list
        CurrentOccupants.RemoveAll([](AActor* Actor) { return !IsValid(Actor); });

        // Check if player is still inside
        bool bPlayerCurrentlyInside = false;
        for (AActor* Occupant : CurrentOccupants)
        {
            if (ACharacter* Character = Cast<ACharacter>(Occupant))
            {
                if (Character->IsPlayerControlled())
                {
                    bPlayerCurrentlyInside = true;
                    break;
                }
            }
        }

        if (bPlayerCurrentlyInside != bIsPlayerInside)
        {
            bIsPlayerInside = bPlayerCurrentlyInside;
            if (bIsPlayerInside)
            {
                OnPlayerEntered();
            }
            else
            {
                OnPlayerExited();
            }
        }
    }
}

void AArch_BuildingManager::InitializeBuilding(EArch_BuildingType Type, FVector Location, FRotator Rotation)
{
    BuildingData.BuildingType = Type;
    SetActorLocation(Location);
    SetActorRotation(Rotation);

    ConfigureBuildingData(Type);
    SetupBuildingMeshForType(Type);
    SetupCollisionVolumes();

    UE_LOG(LogTemp, Warning, TEXT("Arch_BuildingManager: Building initialized at location %s with type %d"), 
           *Location.ToString(), (int32)Type);
}

bool AArch_BuildingManager::CanEnterBuilding(AActor* Actor)
{
    if (!IsValid(Actor) || !BuildingData.bIsHabitable)
    {
        return false;
    }

    if (BuildingData.StructuralIntegrity < 30.0f)
    {
        return false;
    }

    if (CurrentOccupants.Num() >= BuildingData.MaxOccupants)
    {
        return false;
    }

    return true;
}

void AArch_BuildingManager::OnActorEnterBuilding(AActor* Actor)
{
    if (!IsValid(Actor) || CurrentOccupants.Contains(Actor))
    {
        return;
    }

    if (CanEnterBuilding(Actor))
    {
        CurrentOccupants.Add(Actor);
        UE_LOG(LogTemp, Warning, TEXT("Arch_BuildingManager: Actor %s entered building. Occupants: %d/%d"), 
               *Actor->GetName(), CurrentOccupants.Num(), BuildingData.MaxOccupants);
    }
}

void AArch_BuildingManager::OnActorExitBuilding(AActor* Actor)
{
    if (IsValid(Actor) && CurrentOccupants.Contains(Actor))
    {
        CurrentOccupants.Remove(Actor);
        UE_LOG(LogTemp, Warning, TEXT("Arch_BuildingManager: Actor %s exited building. Occupants: %d/%d"), 
               *Actor->GetName(), CurrentOccupants.Num(), BuildingData.MaxOccupants);
    }
}

float AArch_BuildingManager::GetProtectionLevel() const
{
    float BaseProtection = BuildingData.WeatherProtection;
    float IntegrityModifier = BuildingData.StructuralIntegrity / 100.0f;
    return BaseProtection * IntegrityModifier * WindProtection;
}

FVector AArch_BuildingManager::GetRandomInteriorPoint() const
{
    if (BuildingData.InteriorSpawnPoints.Num() == 0)
    {
        return GetActorLocation();
    }

    int32 RandomIndex = FMath::RandRange(0, BuildingData.InteriorSpawnPoints.Num() - 1);
    FVector LocalPoint = BuildingData.InteriorSpawnPoints[RandomIndex];
    return GetActorTransform().TransformPosition(LocalPoint);
}

void AArch_BuildingManager::SetBuildingMesh(UStaticMesh* NewMesh)
{
    if (BuildingMesh && NewMesh)
    {
        BuildingMesh->SetStaticMesh(NewMesh);
        UE_LOG(LogTemp, Warning, TEXT("Arch_BuildingManager: Building mesh updated"));
    }
}

void AArch_BuildingManager::UpdateStructuralIntegrity(float DamageAmount)
{
    float OldIntegrity = BuildingData.StructuralIntegrity;
    BuildingData.StructuralIntegrity = FMath::Clamp(BuildingData.StructuralIntegrity - DamageAmount, 0.0f, 100.0f);

    if (BuildingData.StructuralIntegrity != OldIntegrity)
    {
        OnStructuralDamage(BuildingData.StructuralIntegrity);
        UE_LOG(LogTemp, Warning, TEXT("Arch_BuildingManager: Structural integrity updated: %f -> %f"), 
               OldIntegrity, BuildingData.StructuralIntegrity);

        // If building becomes unsafe, evacuate occupants
        if (BuildingData.StructuralIntegrity < 30.0f && CurrentOccupants.Num() > 0)
        {
            BuildingData.bIsHabitable = false;
            UE_LOG(LogTemp, Warning, TEXT("Arch_BuildingManager: Building marked as uninhabitable due to structural damage"));
        }
    }
}

bool AArch_BuildingManager::IsStructurallySafe() const
{
    return BuildingData.StructuralIntegrity >= 30.0f && BuildingData.bIsHabitable;
}

void AArch_BuildingManager::OnInteriorVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (IsValid(OtherActor))
    {
        OnActorEnterBuilding(OtherActor);
    }
}

void AArch_BuildingManager::OnInteriorVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (IsValid(OtherActor))
    {
        OnActorExitBuilding(OtherActor);
    }
}

void AArch_BuildingManager::SetupBuildingMeshForType(EArch_BuildingType Type)
{
    if (!BuildingMesh)
    {
        return;
    }

    // Set different materials/meshes based on building type
    switch (Type)
    {
        case EArch_BuildingType::StoneRuin:
            // Use weathered stone material
            break;
        case EArch_BuildingType::CaveEntrance:
            // Use rock/cave material
            break;
        case EArch_BuildingType::RockOverhang:
            // Use natural rock material
            break;
        case EArch_BuildingType::WoodPlatform:
            // Use wood material
            break;
        case EArch_BuildingType::BuriedShelter:
            // Use earth/dirt material
            break;
        case EArch_BuildingType::AncientPillar:
            // Use carved stone material
            break;
        case EArch_BuildingType::StoneCircle:
            // Use ritual stone material
            break;
    }
}

void AArch_BuildingManager::ConfigureBuildingData(EArch_BuildingType Type)
{
    switch (Type)
    {
        case EArch_BuildingType::StoneRuin:
            BuildingData.WeatherProtection = 0.8f;
            BuildingData.MaxOccupants = 6;
            BuildingData.bHasFirePit = true;
            TemperatureModifier = 8.0f;
            WindProtection = 0.9f;
            break;
        case EArch_BuildingType::CaveEntrance:
            BuildingData.WeatherProtection = 0.95f;
            BuildingData.MaxOccupants = 8;
            BuildingData.bHasFirePit = false;
            TemperatureModifier = 12.0f;
            WindProtection = 0.95f;
            break;
        case EArch_BuildingType::RockOverhang:
            BuildingData.WeatherProtection = 0.6f;
            BuildingData.MaxOccupants = 3;
            BuildingData.bHasFirePit = false;
            TemperatureModifier = 4.0f;
            WindProtection = 0.7f;
            break;
        case EArch_BuildingType::WoodPlatform:
            BuildingData.WeatherProtection = 0.3f;
            BuildingData.MaxOccupants = 4;
            BuildingData.bHasFirePit = true;
            TemperatureModifier = 2.0f;
            WindProtection = 0.4f;
            break;
        case EArch_BuildingType::BuriedShelter:
            BuildingData.WeatherProtection = 0.9f;
            BuildingData.MaxOccupants = 2;
            BuildingData.bHasFirePit = false;
            TemperatureModifier = 15.0f;
            WindProtection = 0.98f;
            break;
        case EArch_BuildingType::AncientPillar:
            BuildingData.WeatherProtection = 0.2f;
            BuildingData.MaxOccupants = 1;
            BuildingData.bHasFirePit = false;
            TemperatureModifier = 0.0f;
            WindProtection = 0.1f;
            break;
        case EArch_BuildingType::StoneCircle:
            BuildingData.WeatherProtection = 0.1f;
            BuildingData.MaxOccupants = 12;
            BuildingData.bHasFirePit = true;
            TemperatureModifier = 0.0f;
            WindProtection = 0.1f;
            break;
    }
}

void AArch_BuildingManager::SetupCollisionVolumes()
{
    if (InteriorVolume)
    {
        FVector Extents = BuildingData.Dimensions * 0.8f;
        InteriorVolume->SetBoxExtent(Extents);
    }

    if (ExteriorVolume)
    {
        FVector Extents = BuildingData.Dimensions * 1.2f;
        ExteriorVolume->SetBoxExtent(Extents);
    }
}