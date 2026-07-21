#include "Arch_ShelterManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

AArch_ShelterManager::AArch_ShelterManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create shelter mesh component
    ShelterMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShelterMesh"));
    ShelterMesh->SetupAttachment(RootComponent);

    // Create protection zone
    ProtectionZone = CreateDefaultSubobject<UBoxComponent>(TEXT("ProtectionZone"));
    ProtectionZone->SetupAttachment(RootComponent);
    ProtectionZone->SetBoxExtent(FVector(500.0f, 500.0f, 300.0f));

    // Initialize shelter data
    ShelterData = FArch_ShelterData();

    // Initialize components
    InitializeShelterMesh();
    SetupProtectionZone();
}

void AArch_ShelterManager::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    if (ProtectionZone)
    {
        ProtectionZone->OnComponentBeginOverlap.AddDynamic(this, &AArch_ShelterManager::OnProtectionZoneBeginOverlap);
        ProtectionZone->OnComponentEndOverlap.AddDynamic(this, &AArch_ShelterManager::OnProtectionZoneEndOverlap);
    }

    // Clear occupants array
    CurrentOccupants.Empty();
}

void AArch_ShelterManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update shelter effects for current occupants
    for (AActor* Occupant : CurrentOccupants)
    {
        if (IsValid(Occupant))
        {
            // Apply temperature bonus and weather protection
            // This would integrate with the character's survival system
        }
    }
}

void AArch_ShelterManager::InitializeShelterMesh()
{
    if (ShelterMesh)
    {
        // Try to load a basic cube mesh as placeholder
        static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
        if (CubeMeshAsset.Succeeded())
        {
            ShelterMesh->SetStaticMesh(CubeMeshAsset.Object);
            ShelterMesh->SetWorldScale3D(FVector(5.0f, 5.0f, 3.0f)); // Make it shelter-sized
        }

        // Set collision
        ShelterMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        ShelterMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
        ShelterMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
    }
}

void AArch_ShelterManager::SetupProtectionZone()
{
    if (ProtectionZone)
    {
        // Set collision for overlap detection
        ProtectionZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        ProtectionZone->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
        ProtectionZone->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
        ProtectionZone->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

        // Update box extent based on shelter data
        ProtectionZone->SetBoxExtent(FVector(ShelterData.ProtectionRadius, ShelterData.ProtectionRadius, 300.0f));
    }
}

bool AArch_ShelterManager::CanEnterShelter(AActor* Actor)
{
    if (!IsValid(Actor))
    {
        return false;
    }

    // Check if shelter has space
    if (CurrentOccupants.Num() >= ShelterData.MaxOccupants)
    {
        return false;
    }

    // Check if actor is already in shelter
    if (CurrentOccupants.Contains(Actor))
    {
        return false;
    }

    return true;
}

void AArch_ShelterManager::EnterShelter(AActor* Actor)
{
    if (CanEnterShelter(Actor))
    {
        CurrentOccupants.Add(Actor);
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
                FString::Printf(TEXT("Actor %s entered shelter. Occupants: %d/%d"), 
                *Actor->GetName(), CurrentOccupants.Num(), ShelterData.MaxOccupants));
        }
    }
}

void AArch_ShelterManager::ExitShelter(AActor* Actor)
{
    if (CurrentOccupants.Contains(Actor))
    {
        CurrentOccupants.Remove(Actor);
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
                FString::Printf(TEXT("Actor %s exited shelter. Occupants: %d/%d"), 
                *Actor->GetName(), CurrentOccupants.Num(), ShelterData.MaxOccupants));
        }
    }
}

float AArch_ShelterManager::GetTemperatureBonus() const
{
    return ShelterData.TemperatureBonus;
}

bool AArch_ShelterManager::IsWeatherProtected() const
{
    return ShelterData.bProvidesWeatherProtection;
}

int32 AArch_ShelterManager::GetAvailableSpace() const
{
    return FMath::Max(0, ShelterData.MaxOccupants - CurrentOccupants.Num());
}

void AArch_ShelterManager::OnProtectionZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (IsValid(OtherActor) && OtherActor != this)
    {
        // Auto-enter shelter for pawns
        if (OtherActor->IsA<APawn>())
        {
            EnterShelter(OtherActor);
        }
    }
}

void AArch_ShelterManager::OnProtectionZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (IsValid(OtherActor))
    {
        ExitShelter(OtherActor);
    }
}