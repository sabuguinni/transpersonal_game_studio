#include "Arch_ShelterSystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "TimerManager.h"

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

    // Create protection zone
    ProtectionZone = CreateDefaultSubobject<UBoxComponent>(TEXT("ProtectionZone"));
    ProtectionZone->SetupAttachment(RootComponent);
    ProtectionZone->SetBoxExtent(FVector(500.0f, 500.0f, 300.0f));
    ProtectionZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    ProtectionZone->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
    ProtectionZone->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    ProtectionZone->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    // Initialize default shelter properties
    ShelterConfig.ShelterType = EArch_ShelterType::CaveEntrance;
    ShelterConfig.ProtectionRadius = 500.0f;
    ShelterConfig.WeatherProtection = 0.8f;
    ShelterConfig.TemperatureModifier = 5.0f;
    ShelterConfig.bProvidesSafety = true;

    // Bind overlap events
    ProtectionZone->OnComponentBeginOverlap.AddDynamic(this, &AArch_ShelterSystem::OnProtectionZoneBeginOverlap);
    ProtectionZone->OnComponentEndOverlap.AddDynamic(this, &AArch_ShelterSystem::OnProtectionZoneEndOverlap);
}

void AArch_ShelterSystem::BeginPlay()
{
    Super::BeginPlay();

    // Apply shelter type specific settings
    SetShelterType(ShelterConfig.ShelterType);

    // Update protection zone size based on config
    ProtectionZone->SetBoxExtent(FVector(ShelterConfig.ProtectionRadius, ShelterConfig.ProtectionRadius, 300.0f));
}

void AArch_ShelterSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Periodic weather protection updates
    LastWeatherCheck += DeltaTime;
    if (LastWeatherCheck >= WeatherCheckInterval)
    {
        LastWeatherCheck = 0.0f;
        
        if (bPlayerInShelter)
        {
            // Apply weather protection benefits
            // This would integrate with weather system when available
        }
    }
}

bool AArch_ShelterSystem::IsPlayerInShelter() const
{
    return bPlayerInShelter;
}

float AArch_ShelterSystem::GetWeatherProtectionValue() const
{
    return ShelterConfig.WeatherProtection;
}

void AArch_ShelterSystem::SetShelterType(EArch_ShelterType NewType)
{
    ShelterConfig.ShelterType = NewType;

    // Apply type-specific properties
    switch (NewType)
    {
        case EArch_ShelterType::CaveEntrance:
            ShelterConfig.WeatherProtection = 0.9f;
            ShelterConfig.TemperatureModifier = 8.0f;
            ShelterConfig.bProvidesSafety = true;
            break;

        case EArch_ShelterType::StoneArchway:
            ShelterConfig.WeatherProtection = 0.7f;
            ShelterConfig.TemperatureModifier = 3.0f;
            ShelterConfig.bProvidesSafety = false;
            break;

        case EArch_ShelterType::RockOverhang:
            ShelterConfig.WeatherProtection = 0.6f;
            ShelterConfig.TemperatureModifier = 2.0f;
            ShelterConfig.bProvidesSafety = false;
            break;

        case EArch_ShelterType::NaturalShelter:
            ShelterConfig.WeatherProtection = 0.8f;
            ShelterConfig.TemperatureModifier = 5.0f;
            ShelterConfig.bProvidesSafety = true;
            break;
    }

    // Update protection zone size
    ProtectionZone->SetBoxExtent(FVector(ShelterConfig.ProtectionRadius, ShelterConfig.ProtectionRadius, 300.0f));
}

void AArch_ShelterSystem::OnProtectionZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (ACharacter* Character = Cast<ACharacter>(OtherActor))
    {
        // Check if this is the player character
        if (Character->IsPlayerControlled())
        {
            bPlayerInShelter = true;
            OnPlayerEnterShelter();
            
            UE_LOG(LogTemp, Log, TEXT("Player entered shelter: %s"), *GetName());
        }
    }
}

void AArch_ShelterSystem::OnProtectionZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (ACharacter* Character = Cast<ACharacter>(OtherActor))
    {
        if (Character->IsPlayerControlled())
        {
            bPlayerInShelter = false;
            OnPlayerExitShelter();
            
            UE_LOG(LogTemp, Log, TEXT("Player exited shelter: %s"), *GetName());
        }
    }
}