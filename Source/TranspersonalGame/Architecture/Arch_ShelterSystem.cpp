#include "Arch_ShelterSystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "../TranspersonalCharacter.h"

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

    // Create interior volume for shelter effects
    InteriorVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteriorVolume"));
    InteriorVolume->SetupAttachment(RootComponent);
    InteriorVolume->SetBoxExtent(FVector(300.0f, 300.0f, 200.0f));
    InteriorVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteriorVolume->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
    InteriorVolume->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    InteriorVolume->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    // Create entrance volume for detection
    EntranceVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("EntranceVolume"));
    EntranceVolume->SetupAttachment(RootComponent);
    EntranceVolume->SetBoxExtent(FVector(150.0f, 150.0f, 150.0f));
    EntranceVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    EntranceVolume->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
    EntranceVolume->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    EntranceVolume->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    // Initialize default shelter properties
    ShelterConfig.ShelterType = EArch_ShelterType::CaveEntrance;
    ShelterConfig.WeatherProtection = 0.8f;
    ShelterConfig.TemperatureModifier = 5.0f;
    ShelterConfig.MaxOccupants = 4;
    ShelterConfig.bProvidesWarmth = true;
    ShelterConfig.bBlocksWind = true;

    CurrentOccupants.Empty();
}

void AArch_ShelterSystem::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    if (EntranceVolume)
    {
        EntranceVolume->OnComponentBeginOverlap.AddDynamic(this, &AArch_ShelterSystem::OnEntranceOverlapBegin);
        EntranceVolume->OnComponentEndOverlap.AddDynamic(this, &AArch_ShelterSystem::OnEntranceOverlapEnd);
    }

    // Set up mesh based on shelter type
    if (ShelterMesh)
    {
        switch (ShelterConfig.ShelterType)
        {
            case EArch_ShelterType::CaveEntrance:
                // Load cave entrance mesh when available
                break;
            case EArch_ShelterType::RockOverhang:
                // Load rock overhang mesh when available
                break;
            case EArch_ShelterType::StoneArchway:
                // Load stone archway mesh when available
                break;
            case EArch_ShelterType::NaturalAlcove:
                // Load natural alcove mesh when available
                break;
        }
    }
}

void AArch_ShelterSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateShelterEffects();
}

bool AArch_ShelterSystem::CanEnterShelter(AActor* Actor)
{
    if (!Actor || !IsValid(Actor))
    {
        return false;
    }

    // Check if shelter has space
    if (CurrentOccupants.Num() >= ShelterConfig.MaxOccupants)
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

void AArch_ShelterSystem::EnterShelter(AActor* Actor)
{
    if (!CanEnterShelter(Actor))
    {
        return;
    }

    CurrentOccupants.AddUnique(Actor);
    ApplyShelterBenefits(Actor);
    OnActorEnteredShelter(Actor);

    UE_LOG(LogTemp, Log, TEXT("Actor %s entered shelter %s"), 
        *Actor->GetName(), *GetName());
}

void AArch_ShelterSystem::ExitShelter(AActor* Actor)
{
    if (!Actor || !CurrentOccupants.Contains(Actor))
    {
        return;
    }

    CurrentOccupants.Remove(Actor);
    RemoveShelterBenefits(Actor);
    OnActorExitedShelter(Actor);

    UE_LOG(LogTemp, Log, TEXT("Actor %s exited shelter %s"), 
        *Actor->GetName(), *GetName());
}

float AArch_ShelterSystem::GetWeatherProtection() const
{
    return ShelterConfig.WeatherProtection;
}

float AArch_ShelterSystem::GetTemperatureBonus() const
{
    return ShelterConfig.TemperatureModifier;
}

bool AArch_ShelterSystem::IsOccupied() const
{
    return CurrentOccupants.Num() > 0;
}

int32 AArch_ShelterSystem::GetAvailableSpace() const
{
    return FMath::Max(0, ShelterConfig.MaxOccupants - CurrentOccupants.Num());
}

void AArch_ShelterSystem::OnEntranceOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor->IsA<ACharacter>())
    {
        EnterShelter(OtherActor);
    }
}

void AArch_ShelterSystem::OnEntranceOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor && CurrentOccupants.Contains(OtherActor))
    {
        ExitShelter(OtherActor);
    }
}

void AArch_ShelterSystem::UpdateShelterEffects()
{
    // Clean up invalid occupants
    CurrentOccupants.RemoveAll([](AActor* Actor) {
        return !IsValid(Actor);
    });

    // Apply continuous shelter benefits
    for (AActor* Occupant : CurrentOccupants)
    {
        if (IsValid(Occupant))
        {
            ApplyShelterBenefits(Occupant);
        }
    }
}

void AArch_ShelterSystem::ApplyShelterBenefits(AActor* Actor)
{
    if (!IsValid(Actor))
    {
        return;
    }

    // Apply benefits to TranspersonalCharacter if applicable
    if (ATranspersonalCharacter* Character = Cast<ATranspersonalCharacter>(Actor))
    {
        // Temperature regulation
        if (ShelterConfig.bProvidesWarmth)
        {
            // Character->ModifyTemperature(ShelterConfig.TemperatureModifier * GetWorld()->GetDeltaSeconds());
        }

        // Weather protection reduces exposure effects
        if (ShelterConfig.WeatherProtection > 0.0f)
        {
            // Character->SetWeatherProtection(ShelterConfig.WeatherProtection);
        }

        // Stamina regeneration bonus in shelter
        // Character->ModifyStamina(10.0f * GetWorld()->GetDeltaSeconds());
    }
}

void AArch_ShelterSystem::RemoveShelterBenefits(AActor* Actor)
{
    if (!IsValid(Actor))
    {
        return;
    }

    // Remove benefits from TranspersonalCharacter if applicable
    if (ATranspersonalCharacter* Character = Cast<ATranspersonalCharacter>(Actor))
    {
        // Remove weather protection
        // Character->SetWeatherProtection(0.0f);
    }
}