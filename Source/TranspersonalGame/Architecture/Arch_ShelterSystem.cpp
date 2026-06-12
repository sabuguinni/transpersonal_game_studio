#include "Arch_ShelterSystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"

AArch_ShelterSystem::AArch_ShelterSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create shelter mesh component
    ShelterMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShelterMesh"));
    ShelterMesh->SetupAttachment(RootComponent);
    ShelterMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    ShelterMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
    ShelterMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);

    // Create interior volume for occupancy detection
    InteriorVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteriorVolume"));
    InteriorVolume->SetupAttachment(RootComponent);
    InteriorVolume->SetBoxExtent(FVector(300.0f, 300.0f, 200.0f));
    InteriorVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteriorVolume->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    InteriorVolume->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    InteriorVolume->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    // Create entrance collision for interaction
    EntranceCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("EntranceCollision"));
    EntranceCollision->SetupAttachment(RootComponent);
    EntranceCollision->SetBoxExtent(FVector(150.0f, 150.0f, 200.0f));
    EntranceCollision->SetRelativeLocation(FVector(200.0f, 0.0f, 0.0f));
    EntranceCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    EntranceCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    EntranceCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    EntranceCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    // Initialize shelter properties
    ShelterData = FArch_ShelterProperties();
    CurrentOccupants.Empty();
    bIsOccupied = false;
}

void AArch_ShelterSystem::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    if (EntranceCollision)
    {
        EntranceCollision->OnComponentBeginOverlap.AddDynamic(this, &AArch_ShelterSystem::OnEntranceBeginOverlap);
        EntranceCollision->OnComponentEndOverlap.AddDynamic(this, &AArch_ShelterSystem::OnEntranceEndOverlap);
    }

    // Set initial shelter state
    UpdateShelterStatus();
}

void AArch_ShelterSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Apply weather damage over time
    ApplyWeatherDamage(DeltaTime);

    // Update occupancy status
    UpdateShelterStatus();
}

bool AArch_ShelterSystem::CanEnterShelter(AActor* Actor)
{
    if (!Actor || !ShelterData.bProvidesSafety)
    {
        return false;
    }

    // Check if shelter has available space
    if (CurrentOccupants.Num() >= ShelterData.MaxOccupants)
    {
        return false;
    }

    // Check if actor is already inside
    if (CurrentOccupants.Contains(Actor))
    {
        return false;
    }

    // Check structural integrity
    if (ShelterData.StructuralIntegrity < 0.3f)
    {
        return false;
    }

    return true;
}

bool AArch_ShelterSystem::EnterShelter(AActor* Actor)
{
    if (!CanEnterShelter(Actor))
    {
        return false;
    }

    CurrentOccupants.Add(Actor);
    bIsOccupied = (CurrentOccupants.Num() > 0);

    // Notify blueprint
    OnOccupantEntered(Actor);

    UE_LOG(LogTemp, Log, TEXT("Actor %s entered shelter %s"), 
           Actor ? *Actor->GetName() : TEXT("NULL"), 
           *GetName());

    return true;
}

bool AArch_ShelterSystem::ExitShelter(AActor* Actor)
{
    if (!Actor || !CurrentOccupants.Contains(Actor))
    {
        return false;
    }

    CurrentOccupants.Remove(Actor);
    bIsOccupied = (CurrentOccupants.Num() > 0);

    // Notify blueprint
    OnOccupantExited(Actor);

    UE_LOG(LogTemp, Log, TEXT("Actor %s exited shelter %s"), 
           Actor ? *Actor->GetName() : TEXT("NULL"), 
           *GetName());

    return true;
}

int32 AArch_ShelterSystem::GetAvailableSpace() const
{
    return FMath::Max(0, ShelterData.MaxOccupants - CurrentOccupants.Num());
}

void AArch_ShelterSystem::OnEntranceBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this)
    {
        return;
    }

    // Check if it's a character
    if (ACharacter* Character = Cast<ACharacter>(OtherActor))
    {
        if (CanEnterShelter(Character))
        {
            EnterShelter(Character);
        }
    }
}

void AArch_ShelterSystem::OnEntranceEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor || OtherActor == this)
    {
        return;
    }

    // Check if character is leaving
    if (ACharacter* Character = Cast<ACharacter>(OtherActor))
    {
        ExitShelter(Character);
    }
}

void AArch_ShelterSystem::UpdateShelterStatus()
{
    // Remove null or invalid occupants
    CurrentOccupants.RemoveAll([](AActor* Actor) {
        return !IsValid(Actor);
    });

    bIsOccupied = (CurrentOccupants.Num() > 0);
}

void AArch_ShelterSystem::ApplyWeatherDamage(float DeltaTime)
{
    // Gradual structural degradation from weather exposure
    if (ShelterData.StructuralIntegrity > 0.0f)
    {
        float DamageRate = 0.001f; // Very slow degradation
        
        // Different shelter types have different durability
        switch (ShelterData.ShelterType)
        {
            case EArch_ShelterType::CaveEntrance:
                DamageRate *= 0.1f; // Caves are very durable
                break;
            case EArch_ShelterType::RockOverhang:
                DamageRate *= 0.3f; // Rock overhangs are quite durable
                break;
            case EArch_ShelterType::StoneShelter:
                DamageRate *= 0.5f; // Stone shelters degrade moderately
                break;
            case EArch_ShelterType::CliffDwelling:
                DamageRate *= 0.2f; // Cliff dwellings are protected
                break;
        }

        ShelterData.StructuralIntegrity = FMath::Max(0.0f, 
            ShelterData.StructuralIntegrity - (DamageRate * DeltaTime));

        // Reduce weather protection as structure degrades
        if (ShelterData.StructuralIntegrity < 0.5f)
        {
            float IntegrityRatio = ShelterData.StructuralIntegrity / 0.5f;
            ShelterData.WeatherProtection = FMath::Max(0.1f, 
                ShelterData.WeatherProtection * IntegrityRatio);
        }
    }
}