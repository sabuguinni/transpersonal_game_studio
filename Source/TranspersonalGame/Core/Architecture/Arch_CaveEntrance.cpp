#include "Arch_CaveEntrance.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"
#include "Math/UnrealMathUtility.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

AArch_CaveEntrance::AArch_CaveEntrance()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create cave entrance mesh
    CaveEntranceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CaveEntranceMesh"));
    CaveEntranceMesh->SetupAttachment(RootComponent);
    CaveEntranceMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    CaveEntranceMesh->SetCollisionResponseToAllChannels(ECR_Block);

    // Create interior walls mesh
    InteriorWallsMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("InteriorWallsMesh"));
    InteriorWallsMesh->SetupAttachment(RootComponent);
    InteriorWallsMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    InteriorWallsMesh->SetCollisionResponseToAllChannels(ECR_Block);

    // Create entrance volume for player detection
    EntranceVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("EntranceVolume"));
    EntranceVolume->SetupAttachment(RootComponent);
    EntranceVolume->SetBoxExtent(FVector(200.0f, 200.0f, 150.0f));
    EntranceVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    EntranceVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
    EntranceVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Create interior volume for environmental effects
    InteriorVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteriorVolume"));
    InteriorVolume->SetupAttachment(RootComponent);
    InteriorVolume->SetBoxExtent(FVector(400.0f, 400.0f, 200.0f));
    InteriorVolume->SetRelativeLocation(FVector(-300.0f, 0.0f, 0.0f));
    InteriorVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteriorVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteriorVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Create interior lighting
    InteriorLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("InteriorLight"));
    InteriorLight->SetupAttachment(RootComponent);
    InteriorLight->SetRelativeLocation(FVector(-200.0f, 0.0f, 100.0f));
    InteriorLight->SetIntensity(500.0f);
    InteriorLight->SetLightColor(FLinearColor(1.0f, 0.8f, 0.6f, 1.0f)); // Warm cave light
    InteriorLight->SetAttenuationRadius(800.0f);
    InteriorLight->SetSourceRadius(50.0f);
    InteriorLight->SetSoftSourceRadius(100.0f);
    InteriorLight->SetCastShadows(true);

    // Initialize cave properties
    CaveProperties.CaveType = EArch_CaveType::Natural_Limestone;
    CaveProperties.EntranceWidth = 400.0f;
    CaveProperties.EntranceHeight = 300.0f;
    CaveProperties.InteriorDepth = 800.0f;
    CaveProperties.TemperatureModifier = -5.0f;
    CaveProperties.bHasAncientCarvings = false;
    CaveProperties.bHasNaturalLight = false;
    CaveProperties.HumidityLevel = 0.8f;
    CaveProperties.bIsShelterSafe = true;

    BaseTemperature = 20.0f;
    bPlayerInside = false;
}

void AArch_CaveEntrance::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    if (EntranceVolume)
    {
        EntranceVolume->OnComponentBeginOverlap.AddDynamic(this, &AArch_CaveEntrance::OnEntranceVolumeBeginOverlap);
        EntranceVolume->OnComponentEndOverlap.AddDynamic(this, &AArch_CaveEntrance::OnEntranceVolumeEndOverlap);
    }

    // Setup cave appearance
    SetupCaveMesh();
    SetupInteriorVolume();
    ApplyCaveTypeProperties();

    if (bRandomizeOnSpawn)
    {
        RandomizeCaveAppearance();
    }

    UpdateInteriorLighting();
}

void AArch_CaveEntrance::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update environmental effects for actors inside
    if (bPlayerInside && InteriorVolume)
    {
        // Apply temperature effects, humidity, etc.
        // This would integrate with survival systems
    }
}

void AArch_CaveEntrance::SetCaveType(EArch_CaveType NewType)
{
    CaveProperties.CaveType = NewType;
    ApplyCaveTypeProperties();
    UpdateCaveMaterials();
    UpdateInteriorLighting();
}

void AArch_CaveEntrance::RandomizeCaveAppearance()
{
    // Randomize cave type
    int32 RandomType = FMath::RandRange(0, 4);
    CaveProperties.CaveType = static_cast<EArch_CaveType>(RandomType);

    // Randomize dimensions slightly
    float WidthVariation = FMath::RandRange(-50.0f, 50.0f);
    float HeightVariation = FMath::RandRange(-30.0f, 30.0f);
    float DepthVariation = FMath::RandRange(-100.0f, 200.0f);

    CaveProperties.EntranceWidth += WidthVariation;
    CaveProperties.EntranceHeight += HeightVariation;
    CaveProperties.InteriorDepth += DepthVariation;

    // Random chance for ancient carvings
    CaveProperties.bHasAncientCarvings = FMath::RandBool();

    // Random chance for natural light
    CaveProperties.bHasNaturalLight = FMath::RandRange(0.0f, 1.0f) < 0.3f;

    // Apply changes
    SetupCaveMesh();
    SetupInteriorVolume();
    ApplyCaveTypeProperties();
}

void AArch_CaveEntrance::SetCaveDimensions(float Width, float Height, float Depth)
{
    CaveProperties.EntranceWidth = Width;
    CaveProperties.EntranceHeight = Height;
    CaveProperties.InteriorDepth = Depth;

    SetupCaveMesh();
    SetupInteriorVolume();
}

bool AArch_CaveEntrance::IsPlayerInCave() const
{
    return bPlayerInside;
}

float AArch_CaveEntrance::GetInteriorTemperature() const
{
    return BaseTemperature + CaveProperties.TemperatureModifier;
}

void AArch_CaveEntrance::SetAncientCarvings(bool bEnabled)
{
    CaveProperties.bHasAncientCarvings = bEnabled;
    UpdateCaveMaterials();
}

void AArch_CaveEntrance::UpdateInteriorLighting()
{
    if (!InteriorLight) return;

    if (CaveProperties.bHasNaturalLight)
    {
        // Natural light from cave opening
        InteriorLight->SetIntensity(800.0f);
        InteriorLight->SetLightColor(FLinearColor(0.9f, 0.95f, 1.0f, 1.0f)); // Cooler natural light
    }
    else
    {
        // Dim cave lighting
        InteriorLight->SetIntensity(300.0f);
        InteriorLight->SetLightColor(FLinearColor(1.0f, 0.7f, 0.5f, 1.0f)); // Warm cave light
    }

    // Adjust based on cave type
    switch (CaveProperties.CaveType)
    {
        case EArch_CaveType::Volcanic_Basalt:
            InteriorLight->SetLightColor(FLinearColor(1.0f, 0.6f, 0.4f, 1.0f)); // Reddish volcanic light
            break;
        case EArch_CaveType::Ancient_Ruins:
            InteriorLight->SetIntensity(InteriorLight->Intensity * 1.2f); // Brighter for ruins
            break;
        default:
            break;
    }
}

void AArch_CaveEntrance::OnEntranceVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (ACharacter* Character = Cast<ACharacter>(OtherActor))
    {
        if (Character->IsPlayerControlled())
        {
            bPlayerInside = true;
            OnPlayerEnterCave();
            
            UE_LOG(LogTemp, Log, TEXT("Player entered cave: %s"), *GetName());
        }
    }

    ActorsInside.AddUnique(OtherActor);
}

void AArch_CaveEntrance::OnEntranceVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
    if (ACharacter* Character = Cast<ACharacter>(OtherActor))
    {
        if (Character->IsPlayerControlled())
        {
            bPlayerInside = false;
            OnPlayerExitCave();
            
            UE_LOG(LogTemp, Log, TEXT("Player exited cave: %s"), *GetName());
        }
    }

    ActorsInside.Remove(OtherActor);
}

void AArch_CaveEntrance::SetupCaveMesh()
{
    if (!CaveEntranceMesh) return;

    // Scale entrance mesh based on properties
    float WidthScale = CaveProperties.EntranceWidth / 400.0f;
    float HeightScale = CaveProperties.EntranceHeight / 300.0f;
    
    CaveEntranceMesh->SetRelativeScale3D(FVector(1.0f, WidthScale, HeightScale));

    // Setup interior walls
    if (InteriorWallsMesh)
    {
        float DepthScale = CaveProperties.InteriorDepth / 800.0f;
        InteriorWallsMesh->SetRelativeScale3D(FVector(DepthScale, WidthScale, HeightScale));
        InteriorWallsMesh->SetRelativeLocation(FVector(-CaveProperties.InteriorDepth * 0.5f, 0.0f, 0.0f));
    }
}

void AArch_CaveEntrance::SetupInteriorVolume()
{
    if (!InteriorVolume) return;

    // Adjust interior volume based on cave dimensions
    FVector NewExtent(
        CaveProperties.InteriorDepth * 0.5f,
        CaveProperties.EntranceWidth * 0.5f,
        CaveProperties.EntranceHeight * 0.5f
    );
    
    InteriorVolume->SetBoxExtent(NewExtent);
    InteriorVolume->SetRelativeLocation(FVector(-CaveProperties.InteriorDepth * 0.5f, 0.0f, 0.0f));

    // Adjust entrance volume
    if (EntranceVolume)
    {
        FVector EntranceExtent(
            100.0f,
            CaveProperties.EntranceWidth * 0.6f,
            CaveProperties.EntranceHeight * 0.6f
        );
        EntranceVolume->SetBoxExtent(EntranceExtent);
    }
}

void AArch_CaveEntrance::ApplyCaveTypeProperties()
{
    switch (CaveProperties.CaveType)
    {
        case EArch_CaveType::Natural_Limestone:
            CaveProperties.TemperatureModifier = -5.0f;
            CaveProperties.HumidityLevel = 0.8f;
            break;
        case EArch_CaveType::Carved_Sandstone:
            CaveProperties.TemperatureModifier = -3.0f;
            CaveProperties.HumidityLevel = 0.6f;
            CaveProperties.bHasAncientCarvings = true;
            break;
        case EArch_CaveType::Volcanic_Basalt:
            CaveProperties.TemperatureModifier = 2.0f;
            CaveProperties.HumidityLevel = 0.4f;
            break;
        case EArch_CaveType::Weathered_Granite:
            CaveProperties.TemperatureModifier = -4.0f;
            CaveProperties.HumidityLevel = 0.7f;
            break;
        case EArch_CaveType::Ancient_Ruins:
            CaveProperties.TemperatureModifier = -2.0f;
            CaveProperties.HumidityLevel = 0.5f;
            CaveProperties.bHasAncientCarvings = true;
            CaveProperties.bHasNaturalLight = true;
            break;
    }
}

void AArch_CaveEntrance::UpdateCaveMaterials()
{
    if (CaveMaterials.Num() == 0 || !CaveEntranceMesh) return;

    // Select material based on cave type
    int32 MaterialIndex = static_cast<int32>(CaveProperties.CaveType);
    if (CaveMaterials.IsValidIndex(MaterialIndex))
    {
        CaveEntranceMesh->SetMaterial(0, CaveMaterials[MaterialIndex]);
        
        if (InteriorWallsMesh)
        {
            InteriorWallsMesh->SetMaterial(0, CaveMaterials[MaterialIndex]);
        }
    }
}