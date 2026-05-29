#include "Arch_ShelterManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"

AArch_ShelterManager::AArch_ShelterManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create shelter mesh component
    ShelterMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShelterMesh"));
    ShelterMesh->SetupAttachment(RootComponent);
    ShelterMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    ShelterMesh->SetCollisionResponseToAllChannels(ECR_Block);

    // Create interior volume for occupancy detection
    InteriorVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteriorVolume"));
    InteriorVolume->SetupAttachment(RootComponent);
    InteriorVolume->SetBoxExtent(FVector(300.0f, 300.0f, 200.0f));
    InteriorVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteriorVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteriorVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Create fire pit component
    FirePit = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FirePit"));
    FirePit->SetupAttachment(RootComponent);
    FirePit->SetRelativeLocation(FVector(0.0f, 0.0f, -50.0f));
    FirePit->SetVisibility(false);

    // Initialize shelter properties
    ShelterData.ShelterType = EArch_ShelterType::Cave;
    ShelterData.WeatherProtection = 0.8f;
    ShelterData.TemperatureModifier = 5.0f;
    ShelterData.MaxOccupants = 4;
    ShelterData.bHasFire = false;
    ShelterData.bIsOccupied = false;

    InteriorTemperature = 20.0f;
    ShelterIntegrity = 100.0f;
}

void AArch_ShelterManager::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    if (InteriorVolume)
    {
        InteriorVolume->OnComponentBeginOverlap.AddDynamic(this, &AArch_ShelterManager::OnInteriorVolumeBeginOverlap);
        InteriorVolume->OnComponentEndOverlap.AddDynamic(this, &AArch_ShelterManager::OnInteriorVolumeEndOverlap);
    }

    // Set initial fire state
    if (FirePit)
    {
        FirePit->SetVisibility(ShelterData.bHasFire);
    }

    UE_LOG(LogTemp, Log, TEXT("Shelter Manager initialized: Type=%d, Protection=%.2f"), 
           (int32)ShelterData.ShelterType, ShelterData.WeatherProtection);
}

void AArch_ShelterManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateShelterCondition(DeltaTime);

    // Update fire duration
    if (ShelterData.bHasFire)
    {
        FireBurnTime += DeltaTime;
        if (FireBurnTime >= MaxFireDuration)
        {
            ExtinguishFire();
        }
    }

    // Update interior temperature based on fire and occupancy
    float TargetTemperature = 20.0f; // Base temperature
    if (ShelterData.bHasFire)
    {
        TargetTemperature += 15.0f; // Fire adds warmth
    }
    if (CurrentOccupants.Num() > 0)
    {
        TargetTemperature += CurrentOccupants.Num() * 2.0f; // Body heat
    }

    InteriorTemperature = FMath::FInterpTo(InteriorTemperature, TargetTemperature, DeltaTime, 2.0f);
}

bool AArch_ShelterManager::EnterShelter(AActor* Occupant)
{
    if (!Occupant || CurrentOccupants.Contains(Occupant))
    {
        return false;
    }

    if (CurrentOccupants.Num() >= ShelterData.MaxOccupants)
    {
        UE_LOG(LogTemp, Warning, TEXT("Shelter is at maximum capacity"));
        return false;
    }

    CurrentOccupants.Add(Occupant);
    ShelterData.bIsOccupied = true;

    OnOccupantEntered(Occupant);
    UE_LOG(LogTemp, Log, TEXT("Occupant entered shelter: %s"), *Occupant->GetName());

    return true;
}

bool AArch_ShelterManager::ExitShelter(AActor* Occupant)
{
    if (!Occupant || !CurrentOccupants.Contains(Occupant))
    {
        return false;
    }

    CurrentOccupants.Remove(Occupant);
    ShelterData.bIsOccupied = (CurrentOccupants.Num() > 0);

    OnOccupantExited(Occupant);
    UE_LOG(LogTemp, Log, TEXT("Occupant exited shelter: %s"), *Occupant->GetName());

    return true;
}

void AArch_ShelterManager::LightFire()
{
    if (ShelterData.bHasFire)
    {
        UE_LOG(LogTemp, Warning, TEXT("Fire is already lit"));
        return;
    }

    ShelterData.bHasFire = true;
    FireBurnTime = 0.0f;

    if (FirePit)
    {
        FirePit->SetVisibility(true);
    }

    OnFireLit();
    UE_LOG(LogTemp, Log, TEXT("Fire lit in shelter"));
}

void AArch_ShelterManager::ExtinguishFire()
{
    if (!ShelterData.bHasFire)
    {
        return;
    }

    ShelterData.bHasFire = false;
    FireBurnTime = 0.0f;

    if (FirePit)
    {
        FirePit->SetVisibility(false);
    }

    OnFireExtinguished();
    UE_LOG(LogTemp, Log, TEXT("Fire extinguished in shelter"));
}

float AArch_ShelterManager::GetWeatherProtection() const
{
    return ShelterData.WeatherProtection * (ShelterIntegrity / 100.0f);
}

float AArch_ShelterManager::GetTemperatureModifier() const
{
    float Modifier = ShelterData.TemperatureModifier;
    if (ShelterData.bHasFire)
    {
        Modifier += 10.0f;
    }
    return Modifier * (ShelterIntegrity / 100.0f);
}

bool AArch_ShelterManager::CanAccommodate(int32 AdditionalOccupants) const
{
    return (CurrentOccupants.Num() + AdditionalOccupants) <= ShelterData.MaxOccupants;
}

void AArch_ShelterManager::UpdateShelterCondition(float DeltaTime)
{
    // Gradual weather damage
    if (ShelterIntegrity > 0.0f)
    {
        ShelterIntegrity -= WeatherDamageRate * DeltaTime;
        ShelterIntegrity = FMath::Clamp(ShelterIntegrity, 0.0f, 100.0f);
    }

    // Repair when occupied (maintenance)
    if (ShelterData.bIsOccupied && ShelterIntegrity < 100.0f)
    {
        ShelterIntegrity += RepairRate * DeltaTime;
        ShelterIntegrity = FMath::Clamp(ShelterIntegrity, 0.0f, 100.0f);
    }
}

void AArch_ShelterManager::OnInteriorVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor->IsA<ACharacter>())
    {
        EnterShelter(OtherActor);
    }
}

void AArch_ShelterManager::OnInteriorVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor && OtherActor->IsA<ACharacter>())
    {
        ExitShelter(OtherActor);
    }
}