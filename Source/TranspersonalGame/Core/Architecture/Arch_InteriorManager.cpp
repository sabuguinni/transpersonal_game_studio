#include "Arch_InteriorManager.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"

AArch_InteriorManager::AArch_InteriorManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root scene component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootSceneComponent;

    // Create interior mesh component
    InteriorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("InteriorMesh"));
    InteriorMesh->SetupAttachment(RootComponent);

    // Create trigger volume for interior detection
    InteriorTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("InteriorTrigger"));
    InteriorTrigger->SetupAttachment(RootComponent);
    InteriorTrigger->SetBoxExtent(FVector(300.0f, 300.0f, 200.0f));
    InteriorTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteriorTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteriorTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Create fire pit mesh
    FirePitMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FirePitMesh"));
    FirePitMesh->SetupAttachment(RootComponent);
    FirePitMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -50.0f));
    FirePitMesh->SetVisibility(false);

    // Create fire light
    FireLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("FireLight"));
    FireLight->SetupAttachment(FirePitMesh);
    FireLight->SetRelativeLocation(FVector(0.0f, 0.0f, 20.0f));
    FireLight->SetLightColor(FLinearColor(1.0f, 0.6f, 0.2f, 1.0f));
    FireLight->SetIntensity(1000.0f);
    FireLight->SetAttenuationRadius(500.0f);
    FireLight->SetVisibility(false);

    // Initialize default interior data
    InteriorData.InteriorType = EArch_InteriorType::Cave;
    InteriorData.ComfortLevel = 50.0f;
    InteriorData.WarmthBonus = 25.0f;
    InteriorData.bHasFirePit = false;
    InteriorData.bHasSleepingArea = false;
    InteriorData.MaxOccupants = 2;

    bIsOccupied = false;
}

void AArch_InteriorManager::BeginPlay()
{
    Super::BeginPlay();

    // Bind trigger events
    if (InteriorTrigger)
    {
        InteriorTrigger->OnComponentBeginOverlap.AddDynamic(this, &AArch_InteriorManager::OnInteriorEnter);
        InteriorTrigger->OnComponentEndOverlap.AddDynamic(this, &AArch_InteriorManager::OnInteriorExit);
    }

    SetupInteriorMesh();
    ConfigureInteriorSettings();
}

void AArch_InteriorManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update fire pit effects if active
    if (InteriorData.bHasFirePit && FireLight->IsVisible())
    {
        UpdateFirePitVisuals();
    }

    // Update occupancy status
    bIsOccupied = CurrentOccupants.Num() > 0;
}

void AArch_InteriorManager::SetupInteriorType(EArch_InteriorType NewType)
{
    InteriorData.InteriorType = NewType;

    switch (NewType)
    {
        case EArch_InteriorType::Cave:
            InteriorData.ComfortLevel = 70.0f;
            InteriorData.WarmthBonus = 35.0f;
            InteriorData.MaxOccupants = 3;
            break;

        case EArch_InteriorType::StoneShelter:
            InteriorData.ComfortLevel = 60.0f;
            InteriorData.WarmthBonus = 30.0f;
            InteriorData.MaxOccupants = 2;
            break;

        case EArch_InteriorType::LogHut:
            InteriorData.ComfortLevel = 55.0f;
            InteriorData.WarmthBonus = 25.0f;
            InteriorData.MaxOccupants = 2;
            break;

        case EArch_InteriorType::RockOverhang:
            InteriorData.ComfortLevel = 40.0f;
            InteriorData.WarmthBonus = 15.0f;
            InteriorData.MaxOccupants = 1;
            break;

        case EArch_InteriorType::BuriedDwelling:
            InteriorData.ComfortLevel = 80.0f;
            InteriorData.WarmthBonus = 45.0f;
            InteriorData.MaxOccupants = 4;
            break;
    }

    ConfigureInteriorSettings();
}

void AArch_InteriorManager::AddFurniture(UStaticMesh* FurnitureMesh, FVector RelativeLocation, FRotator RelativeRotation)
{
    if (!FurnitureMesh)
    {
        return;
    }

    UStaticMeshComponent* NewFurniture = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("Furniture_%d"), FurnitureComponents.Num()));
    if (NewFurniture)
    {
        NewFurniture->SetupAttachment(RootComponent);
        NewFurniture->SetStaticMesh(FurnitureMesh);
        NewFurniture->SetRelativeLocation(RelativeLocation);
        NewFurniture->SetRelativeRotation(RelativeRotation);
        FurnitureComponents.Add(NewFurniture);

        // Increase comfort level based on furniture
        InteriorData.ComfortLevel += 10.0f;
    }
}

void AArch_InteriorManager::LightFirePit()
{
    if (FirePitMesh && FireLight)
    {
        InteriorData.bHasFirePit = true;
        FirePitMesh->SetVisibility(true);
        FireLight->SetVisibility(true);
        InteriorData.WarmthBonus += 20.0f;
        InteriorData.ComfortLevel += 15.0f;

        UE_LOG(LogTemp, Log, TEXT("Fire pit lit in interior %s"), *GetName());
    }
}

void AArch_InteriorManager::ExtinguishFirePit()
{
    if (FirePitMesh && FireLight)
    {
        InteriorData.bHasFirePit = false;
        FirePitMesh->SetVisibility(false);
        FireLight->SetVisibility(false);
        InteriorData.WarmthBonus -= 20.0f;
        InteriorData.ComfortLevel -= 15.0f;

        UE_LOG(LogTemp, Log, TEXT("Fire pit extinguished in interior %s"), *GetName());
    }
}

bool AArch_InteriorManager::CanAccommodateOccupant() const
{
    return CurrentOccupants.Num() < InteriorData.MaxOccupants;
}

void AArch_InteriorManager::AddOccupant(AActor* NewOccupant)
{
    if (NewOccupant && CanAccommodateOccupant())
    {
        CurrentOccupants.AddUnique(NewOccupant);
        UE_LOG(LogTemp, Log, TEXT("Occupant %s added to interior %s"), *NewOccupant->GetName(), *GetName());
    }
}

void AArch_InteriorManager::RemoveOccupant(AActor* OccupantToRemove)
{
    if (OccupantToRemove)
    {
        CurrentOccupants.Remove(OccupantToRemove);
        UE_LOG(LogTemp, Log, TEXT("Occupant %s removed from interior %s"), *OccupantToRemove->GetName(), *GetName());
    }
}

float AArch_InteriorManager::GetComfortLevel() const
{
    return InteriorData.ComfortLevel;
}

float AArch_InteriorManager::GetWarmthBonus() const
{
    return InteriorData.WarmthBonus;
}

void AArch_InteriorManager::OnInteriorEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor->IsA<APawn>())
    {
        AddOccupant(OtherActor);
        
        // Notify player of interior benefits
        if (GEngine)
        {
            FString InteriorTypeStr;
            switch (InteriorData.InteriorType)
            {
                case EArch_InteriorType::Cave: InteriorTypeStr = TEXT("Cave"); break;
                case EArch_InteriorType::StoneShelter: InteriorTypeStr = TEXT("Stone Shelter"); break;
                case EArch_InteriorType::LogHut: InteriorTypeStr = TEXT("Log Hut"); break;
                case EArch_InteriorType::RockOverhang: InteriorTypeStr = TEXT("Rock Overhang"); break;
                case EArch_InteriorType::BuriedDwelling: InteriorTypeStr = TEXT("Buried Dwelling"); break;
            }

            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                FString::Printf(TEXT("Entered %s - Comfort: %.0f%%, Warmth: +%.0f"), 
                    *InteriorTypeStr, InteriorData.ComfortLevel, InteriorData.WarmthBonus));
        }
    }
}

void AArch_InteriorManager::OnInteriorExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor && OtherActor->IsA<APawn>())
    {
        RemoveOccupant(OtherActor);

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Left shelter"));
        }
    }
}

void AArch_InteriorManager::UpdateFirePitVisuals()
{
    if (FireLight)
    {
        // Create flickering fire effect
        float FlickerIntensity = 1000.0f + FMath::Sin(GetWorld()->GetTimeSeconds() * 8.0f) * 200.0f;
        FireLight->SetIntensity(FlickerIntensity);

        // Subtle color variation
        float ColorVariation = 0.8f + FMath::Sin(GetWorld()->GetTimeSeconds() * 12.0f) * 0.2f;
        FireLight->SetLightColor(FLinearColor(1.0f, 0.6f * ColorVariation, 0.2f, 1.0f));
    }
}

void AArch_InteriorManager::SetupInteriorMesh()
{
    // This would normally load appropriate meshes based on interior type
    // For now, we'll use basic shapes as placeholders
    if (InteriorMesh)
    {
        // Set basic collision and rendering properties
        InteriorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        InteriorMesh->SetCollisionResponseToAllChannels(ECR_Block);
        InteriorMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
    }
}

void AArch_InteriorManager::ConfigureInteriorSettings()
{
    // Adjust trigger size based on interior type
    if (InteriorTrigger)
    {
        FVector TriggerExtent;
        switch (InteriorData.InteriorType)
        {
            case EArch_InteriorType::Cave:
                TriggerExtent = FVector(400.0f, 400.0f, 250.0f);
                break;
            case EArch_InteriorType::StoneShelter:
                TriggerExtent = FVector(300.0f, 300.0f, 200.0f);
                break;
            case EArch_InteriorType::LogHut:
                TriggerExtent = FVector(250.0f, 250.0f, 180.0f);
                break;
            case EArch_InteriorType::RockOverhang:
                TriggerExtent = FVector(200.0f, 150.0f, 120.0f);
                break;
            case EArch_InteriorType::BuriedDwelling:
                TriggerExtent = FVector(350.0f, 350.0f, 150.0f);
                break;
        }
        InteriorTrigger->SetBoxExtent(TriggerExtent);
    }
}