#include "Arch_CaveSystem.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Sound/SoundCue.h"

AArch_CaveSystem::AArch_CaveSystem()
{
    PrimaryActorTick.bCanEverTick = false;

    // Root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Cave entrance mesh
    CaveEntranceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CaveEntranceMesh"));
    CaveEntranceMesh->SetupAttachment(RootComponent);
    CaveEntranceMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    CaveEntranceMesh->SetCollisionResponseToAllChannels(ECR_Block);

    // Cave interior mesh
    CaveInteriorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CaveInteriorMesh"));
    CaveInteriorMesh->SetupAttachment(RootComponent);
    CaveInteriorMesh->SetRelativeLocation(FVector(0, 0, -200));

    // Interior volume for shelter detection
    InteriorVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteriorVolume"));
    InteriorVolume->SetupAttachment(RootComponent);
    InteriorVolume->SetBoxExtent(FVector(400, 400, 200));
    InteriorVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteriorVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteriorVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Ambient audio
    AmbientAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    AmbientAudio->SetupAttachment(RootComponent);
    AmbientAudio->bAutoActivate = true;

    // Default cave properties
    CaveProperties.CaveType = EArch_CaveType::ShallowCave;
    CaveProperties.Temperature = 15.0f;
    CaveProperties.Humidity = 80.0f;
    CaveProperties.bHasWaterSource = false;
    CaveProperties.bHasCaveArt = false;
    CaveProperties.MaxOccupants = 4;
}

void AArch_CaveSystem::BeginPlay()
{
    Super::BeginPlay();
    
    SetupCaveGeometry();
    ConfigureAmbientSounds();
    GenerateShelterSpots();
}

void AArch_CaveSystem::InitializeCave(EArch_CaveType InCaveType)
{
    CaveProperties.CaveType = InCaveType;
    
    switch (InCaveType)
    {
        case EArch_CaveType::ShallowCave:
            CaveProperties.MaxOccupants = 2;
            InteriorVolume->SetBoxExtent(FVector(300, 300, 150));
            break;
            
        case EArch_CaveType::DeepCavern:
            CaveProperties.MaxOccupants = 6;
            CaveProperties.bHasWaterSource = true;
            InteriorVolume->SetBoxExtent(FVector(600, 600, 300));
            break;
            
        case EArch_CaveType::TunnelSystem:
            CaveProperties.MaxOccupants = 8;
            InteriorVolume->SetBoxExtent(FVector(800, 400, 200));
            break;
            
        case EArch_CaveType::UndergroundLake:
            CaveProperties.MaxOccupants = 4;
            CaveProperties.bHasWaterSource = true;
            CaveProperties.Humidity = 95.0f;
            InteriorVolume->SetBoxExtent(FVector(500, 500, 250));
            break;
    }
    
    GenerateShelterSpots();
}

bool AArch_CaveSystem::CanProvideShel ter() const
{
    return ShelterSpots.Num() > 0 && CaveProperties.MaxOccupants > 0;
}

FVector AArch_CaveSystem::GetNearestShelterSpot(FVector PlayerLocation) const
{
    if (ShelterSpots.Num() == 0)
    {
        return GetActorLocation();
    }
    
    FVector NearestSpot = ShelterSpots[0];
    float NearestDistance = FVector::Dist(PlayerLocation, GetActorLocation() + NearestSpot);
    
    for (const FVector& Spot : ShelterSpots)
    {
        FVector WorldSpot = GetActorLocation() + Spot;
        float Distance = FVector::Dist(PlayerLocation, WorldSpot);
        
        if (Distance < NearestDistance)
        {
            NearestDistance = Distance;
            NearestSpot = Spot;
        }
    }
    
    return GetActorLocation() + NearestSpot;
}

void AArch_CaveSystem::AddCaveArt(FVector Location, int32 ArtType)
{
    CaveProperties.bHasCaveArt = true;
    
    // Log cave art addition for debugging
    UE_LOG(LogTemp, Log, TEXT("Cave art added at location: %s, Type: %d"), 
           *Location.ToString(), ArtType);
}

void AArch_CaveSystem::SetupCaveGeometry()
{
    // Configure mesh scaling based on cave type
    FVector Scale = FVector(1.0f);
    
    switch (CaveProperties.CaveType)
    {
        case EArch_CaveType::ShallowCave:
            Scale = FVector(0.8f, 0.8f, 0.6f);
            break;
            
        case EArch_CaveType::DeepCavern:
            Scale = FVector(1.5f, 1.5f, 1.8f);
            break;
            
        case EArch_CaveType::TunnelSystem:
            Scale = FVector(2.0f, 1.0f, 0.8f);
            break;
            
        case EArch_CaveType::UndergroundLake:
            Scale = FVector(1.2f, 1.2f, 1.0f);
            break;
    }
    
    CaveEntranceMesh->SetRelativeScale3D(Scale);
    CaveInteriorMesh->SetRelativeScale3D(Scale);
}

void AArch_CaveSystem::ConfigureAmbientSounds()
{
    if (!AmbientAudio)
        return;
        
    // Configure audio based on cave properties
    AmbientAudio->SetVolumeMultiplier(0.3f);
    
    if (CaveProperties.bHasWaterSource)
    {
        // Water dripping sounds for caves with water
        AmbientAudio->SetVolumeMultiplier(0.5f);
    }
    
    // Set 3D audio properties
    AmbientAudio->bOverrideAttenuation = true;
    AmbientAudio->AttenuationOverrides.bAttenuate = true;
    AmbientAudio->AttenuationOverrides.AttenuationShape = EAttenuationShape::Sphere;
    AmbientAudio->AttenuationOverrides.FalloffDistance = 1000.0f;
}

void AArch_CaveSystem::GenerateShelterSpots()
{
    ShelterSpots.Empty();
    
    // Generate shelter spots based on cave type and size
    FVector Extent = InteriorVolume->GetScaledBoxExtent();
    int32 NumSpots = FMath::Min(CaveProperties.MaxOccupants, 8);
    
    for (int32 i = 0; i < NumSpots; i++)
    {
        float Angle = (2.0f * PI * i) / NumSpots;
        float Radius = Extent.X * 0.6f;
        
        FVector SpotLocation = FVector(
            FMath::Cos(Angle) * Radius,
            FMath::Sin(Angle) * Radius,
            -Extent.Z * 0.8f
        );
        
        ShelterSpots.Add(SpotLocation);
    }
    
    // Add central storage area
    StorageAreas.Empty();
    StorageAreas.Add(FVector(0, 0, -Extent.Z * 0.9f));
}