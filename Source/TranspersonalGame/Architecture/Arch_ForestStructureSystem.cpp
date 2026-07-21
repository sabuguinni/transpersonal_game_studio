#include "Arch_ForestStructureSystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"

AArch_ForestStructureSystem::AArch_ForestStructureSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize components
    InitializeStructureComponents();
    SetupInteractionVolume();

    // Set default structure data
    StructureData.StructureType = EArch_ForestStructureType::LogShelter;
    StructureData.StructuralIntegrity = 100.0f;
    StructureData.bIsWeatherproof = false;
    StructureData.MaxOccupants = 1;
    StructureData.CamouflageLevel = 0.7f;

    WeatheringTimer = 0.0f;
}

void AArch_ForestStructureSystem::InitializeStructureComponents()
{
    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create main structure mesh
    StructureMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StructureMesh"));
    StructureMesh->SetupAttachment(RootComponent);
    StructureMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    StructureMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
}

void AArch_ForestStructureSystem::SetupInteractionVolume()
{
    // Create interaction volume
    InteractionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionVolume"));
    InteractionVolume->SetupAttachment(RootComponent);
    InteractionVolume->SetBoxExtent(FVector(200.0f, 200.0f, 150.0f));
    InteractionVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionVolume->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    InteractionVolume->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    InteractionVolume->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    // Bind overlap events
    InteractionVolume->OnComponentBeginOverlap.AddDynamic(this, &AArch_ForestStructureSystem::OnInteractionVolumeBeginOverlap);
    InteractionVolume->OnComponentEndOverlap.AddDynamic(this, &AArch_ForestStructureSystem::OnInteractionVolumeEndOverlap);
}

void AArch_ForestStructureSystem::BeginPlay()
{
    Super::BeginPlay();

    if (bAutoGenerateStructure)
    {
        GenerateStructureComponents();
    }

    ApplyMossAndWeathering();
}

void AArch_ForestStructureSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update weathering over time
    WeatheringTimer += DeltaTime;
    if (WeatheringTimer >= 60.0f) // Weather damage every minute
    {
        ApplyWeatherDamage(0.1f);
        WeatheringTimer = 0.0f;
    }
}

void AArch_ForestStructureSystem::SetStructureType(EArch_ForestStructureType NewType)
{
    StructureData.StructureType = NewType;

    // Update structure properties based on type
    switch (NewType)
    {
        case EArch_ForestStructureType::LogShelter:
            StructureData.bIsWeatherproof = true;
            StructureData.MaxOccupants = 2;
            StructureData.CamouflageLevel = 0.8f;
            break;
        case EArch_ForestStructureType::TreePlatform:
            StructureData.bIsWeatherproof = false;
            StructureData.MaxOccupants = 1;
            StructureData.CamouflageLevel = 0.9f;
            break;
        case EArch_ForestStructureType::CaveEntrance:
            StructureData.bIsWeatherproof = true;
            StructureData.MaxOccupants = 4;
            StructureData.CamouflageLevel = 0.6f;
            break;
        case EArch_ForestStructureType::WoodBridge:
            StructureData.bIsWeatherproof = false;
            StructureData.MaxOccupants = 0;
            StructureData.CamouflageLevel = 0.3f;
            break;
        case EArch_ForestStructureType::StorageCache:
            StructureData.bIsWeatherproof = true;
            StructureData.MaxOccupants = 0;
            StructureData.CamouflageLevel = 0.95f;
            break;
    }

    GenerateStructureComponents();
}

void AArch_ForestStructureSystem::ApplyWeatherDamage(float DamageAmount)
{
    StructureData.StructuralIntegrity = FMath::Max(0.0f, StructureData.StructuralIntegrity - DamageAmount);

    if (StructureData.StructuralIntegrity <= 0.0f)
    {
        // Structure collapsed
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, 
                FString::Printf(TEXT("Forest structure %s has collapsed!"), *GetName()));
        }
    }
}

bool AArch_ForestStructureSystem::CanProvideShelte() const
{
    return StructureData.bIsWeatherproof && 
           StructureData.StructuralIntegrity > 25.0f &&
           CurrentOccupants.Num() < StructureData.MaxOccupants;
}

void AArch_ForestStructureSystem::GenerateStructureComponents()
{
    if (!StructureMesh)
        return;

    // Apply different configurations based on structure type
    switch (StructureData.StructureType)
    {
        case EArch_ForestStructureType::LogShelter:
            StructureMesh->SetRelativeScale3D(FVector(1.2f, 1.0f, 0.8f));
            break;
        case EArch_ForestStructureType::TreePlatform:
            StructureMesh->SetRelativeScale3D(FVector(0.8f, 0.8f, 0.3f));
            StructureMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 300.0f));
            break;
        case EArch_ForestStructureType::CaveEntrance:
            StructureMesh->SetRelativeScale3D(FVector(1.5f, 1.5f, 1.2f));
            break;
        case EArch_ForestStructureType::WoodBridge:
            StructureMesh->SetRelativeScale3D(FVector(3.0f, 0.5f, 0.2f));
            break;
        case EArch_ForestStructureType::StorageCache:
            StructureMesh->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.4f));
            break;
    }

    ApplyMossAndWeathering();
}

float AArch_ForestStructureSystem::GetCamouflageEffectiveness() const
{
    float baseEffectiveness = StructureData.CamouflageLevel;
    float integrityModifier = StructureData.StructuralIntegrity / 100.0f;
    return baseEffectiveness * integrityModifier;
}

void AArch_ForestStructureSystem::OnInteractionVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor != this)
    {
        CurrentOccupants.AddUnique(OtherActor);
        OnStructureEntered(OtherActor);

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                FString::Printf(TEXT("%s entered forest structure"), *OtherActor->GetName()));
        }
    }
}

void AArch_ForestStructureSystem::OnInteractionVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor && OtherActor != this)
    {
        CurrentOccupants.Remove(OtherActor);
        OnStructureExited(OtherActor);

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
                FString::Printf(TEXT("%s left forest structure"), *OtherActor->GetName()));
        }
    }
}

void AArch_ForestStructureSystem::ApplyMossAndWeathering()
{
    if (!StructureMesh)
        return;

    // Create dynamic material instance for weathering effects
    if (WeatheredMaterials.Num() > 0)
    {
        UMaterialInterface* BaseMaterial = WeatheredMaterials[0];
        if (BaseMaterial)
        {
            UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
            if (DynamicMaterial)
            {
                // Apply weathering parameters
                float weatheringAmount = 1.0f - (StructureData.StructuralIntegrity / 100.0f);
                DynamicMaterial->SetScalarParameterValue(TEXT("WeatheringAmount"), weatheringAmount);
                DynamicMaterial->SetScalarParameterValue(TEXT("MossGrowth"), StructureData.CamouflageLevel);
                
                StructureMesh->SetMaterial(0, DynamicMaterial);
            }
        }
    }
}