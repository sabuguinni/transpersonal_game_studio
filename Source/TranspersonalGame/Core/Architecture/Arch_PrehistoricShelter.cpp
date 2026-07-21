#include "Arch_PrehistoricShelter.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "UObject/ConstructorHelpers.h"

AArch_PrehistoricShelter::AArch_PrehistoricShelter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize components
    ShelterMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShelterMesh"));
    RootComponent = ShelterMesh;

    InteriorVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteriorVolume"));
    InteriorVolume->SetupAttachment(RootComponent);
    InteriorVolume->SetBoxExtent(FVector(200.0f, 200.0f, 150.0f));

    EntranceVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("EntranceVolume"));
    EntranceVolume->SetupAttachment(RootComponent);
    EntranceVolume->SetBoxExtent(FVector(150.0f, 150.0f, 100.0f));
    EntranceVolume->SetRelativeLocation(FVector(180.0f, 0.0f, 0.0f));

    InteriorLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("InteriorLight"));
    InteriorLight->SetupAttachment(RootComponent);
    InteriorLight->SetRelativeLocation(FVector(-100.0f, 0.0f, 100.0f));
    InteriorLight->SetIntensity(500.0f);
    InteriorLight->SetLightColor(FLinearColor(1.0f, 0.8f, 0.6f, 1.0f));
    InteriorLight->SetAttenuationRadius(400.0f);

    // Default configuration
    ShelterType = EArch_ShelterType::CaveEntrance;
    ShelterScale = 1.0f;
    bAutoConfigureForBiome = true;
    bPlayerInShelter = false;
    CurrentWeatherProtection = 0.8f;
    CurrentTemperatureBonus = 5.0f;

    // Initialize default shelter properties
    ShelterProperties.WeatherProtection = 0.8f;
    ShelterProperties.TemperatureModifier = 5.0f;
    ShelterProperties.InteriorSpace = 100.0f;
    ShelterProperties.bHasFireplace = false;
    ShelterProperties.bProvidesSafety = true;
}

void AArch_PrehistoricShelter::BeginPlay()
{
    Super::BeginPlay();

    InitializeShelterMesh();
    ConfigureLighting();
    SetupCollisionVolumes();

    // Bind overlap events
    if (EntranceVolume)
    {
        EntranceVolume->OnComponentBeginOverlap.AddDynamic(this, &AArch_PrehistoricShelter::OnEntranceVolumeBeginOverlap);
        EntranceVolume->OnComponentEndOverlap.AddDynamic(this, &AArch_PrehistoricShelter::OnEntranceVolumeEndOverlap);
    }

    // Auto-configure for biome if enabled
    if (bAutoConfigureForBiome)
    {
        ConfigureShelterType(ShelterType);
    }
}

void AArch_PrehistoricShelter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update lighting based on time of day
    if (InteriorLight && IsValid(InteriorLight))
    {
        // Simulate firelight flickering if fireplace is enabled
        if (ShelterProperties.bHasFireplace)
        {
            float FlickerIntensity = FMath::RandRange(450.0f, 550.0f);
            InteriorLight->SetIntensity(FlickerIntensity);
        }
    }
}

void AArch_PrehistoricShelter::ConfigureShelterType(EArch_ShelterType NewType)
{
    ShelterType = NewType;

    switch (ShelterType)
    {
    case EArch_ShelterType::CaveEntrance:
        ShelterProperties.WeatherProtection = 0.9f;
        ShelterProperties.TemperatureModifier = 8.0f;
        ShelterProperties.InteriorSpace = 150.0f;
        ShelterProperties.bProvidesSafety = true;
        CurrentWeatherProtection = 0.9f;
        CurrentTemperatureBonus = 8.0f;
        break;

    case EArch_ShelterType::RockOverhang:
        ShelterProperties.WeatherProtection = 0.7f;
        ShelterProperties.TemperatureModifier = 3.0f;
        ShelterProperties.InteriorSpace = 80.0f;
        ShelterProperties.bProvidesSafety = false;
        CurrentWeatherProtection = 0.7f;
        CurrentTemperatureBonus = 3.0f;
        break;

    case EArch_ShelterType::NaturalArch:
        ShelterProperties.WeatherProtection = 0.5f;
        ShelterProperties.TemperatureModifier = 2.0f;
        ShelterProperties.InteriorSpace = 60.0f;
        ShelterProperties.bProvidesSafety = false;
        CurrentWeatherProtection = 0.5f;
        CurrentTemperatureBonus = 2.0f;
        break;

    case EArch_ShelterType::StoneLedge:
        ShelterProperties.WeatherProtection = 0.6f;
        ShelterProperties.TemperatureModifier = 2.5f;
        ShelterProperties.InteriorSpace = 70.0f;
        ShelterProperties.bProvidesSafety = false;
        CurrentWeatherProtection = 0.6f;
        CurrentTemperatureBonus = 2.5f;
        break;
    }

    ConfigureLighting();
}

float AArch_PrehistoricShelter::GetWeatherProtection() const
{
    return CurrentWeatherProtection;
}

float AArch_PrehistoricShelter::GetTemperatureBonus() const
{
    return CurrentTemperatureBonus;
}

bool AArch_PrehistoricShelter::IsPlayerInShelter() const
{
    return bPlayerInShelter;
}

void AArch_PrehistoricShelter::SetupFireplace(bool bEnable)
{
    ShelterProperties.bHasFireplace = bEnable;
    
    if (bEnable)
    {
        CurrentTemperatureBonus += 5.0f;
        if (InteriorLight)
        {
            InteriorLight->SetIntensity(700.0f);
            InteriorLight->SetLightColor(FLinearColor(1.0f, 0.6f, 0.3f, 1.0f));
        }
    }
    else
    {
        CurrentTemperatureBonus = ShelterProperties.TemperatureModifier;
        if (InteriorLight)
        {
            InteriorLight->SetIntensity(500.0f);
            InteriorLight->SetLightColor(FLinearColor(1.0f, 0.8f, 0.6f, 1.0f));
        }
    }
}

void AArch_PrehistoricShelter::OnEntranceVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (ACharacter* PlayerCharacter = Cast<ACharacter>(OtherActor))
    {
        bPlayerInShelter = true;
        OnPlayerEnterShelter();
        
        if (GEngine)
        {
            FString ShelterTypeName = UEnum::GetValueAsString(ShelterType);
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                FString::Printf(TEXT("Entered %s - Weather Protection: %.1f%%, Temperature Bonus: +%.1f°C"), 
                    *ShelterTypeName, CurrentWeatherProtection * 100.0f, CurrentTemperatureBonus));
        }
    }
}

void AArch_PrehistoricShelter::OnEntranceVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (ACharacter* PlayerCharacter = Cast<ACharacter>(OtherActor))
    {
        bPlayerInShelter = false;
        OnPlayerExitShelter();
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Left shelter"));
        }
    }
}

void AArch_PrehistoricShelter::InitializeShelterMesh()
{
    if (ShelterMesh)
    {
        // Try to load appropriate mesh based on shelter type
        UStaticMesh* MeshToUse = nullptr;
        
        switch (ShelterType)
        {
        case EArch_ShelterType::CaveEntrance:
            // Try to load cave entrance mesh
            MeshToUse = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
            break;
        case EArch_ShelterType::RockOverhang:
            // Try to load rock overhang mesh
            MeshToUse = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
            break;
        default:
            MeshToUse = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
            break;
        }

        if (MeshToUse)
        {
            ShelterMesh->SetStaticMesh(MeshToUse);
            ShelterMesh->SetWorldScale3D(FVector(3.0f, 3.0f, 2.0f) * ShelterScale);
        }
    }
}

void AArch_PrehistoricShelter::ConfigureLighting()
{
    if (InteriorLight)
    {
        switch (ShelterType)
        {
        case EArch_ShelterType::CaveEntrance:
            InteriorLight->SetIntensity(600.0f);
            InteriorLight->SetAttenuationRadius(500.0f);
            InteriorLight->SetLightColor(FLinearColor(0.8f, 0.7f, 0.5f, 1.0f));
            break;
        case EArch_ShelterType::RockOverhang:
            InteriorLight->SetIntensity(300.0f);
            InteriorLight->SetAttenuationRadius(300.0f);
            InteriorLight->SetLightColor(FLinearColor(0.9f, 0.9f, 0.8f, 1.0f));
            break;
        default:
            InteriorLight->SetIntensity(400.0f);
            InteriorLight->SetAttenuationRadius(350.0f);
            InteriorLight->SetLightColor(FLinearColor(0.85f, 0.8f, 0.7f, 1.0f));
            break;
        }
    }
}

void AArch_PrehistoricShelter::SetupCollisionVolumes()
{
    if (InteriorVolume)
    {
        InteriorVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        InteriorVolume->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
        InteriorVolume->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
        InteriorVolume->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
    }

    if (EntranceVolume)
    {
        EntranceVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        EntranceVolume->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
        EntranceVolume->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
        EntranceVolume->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
    }
}