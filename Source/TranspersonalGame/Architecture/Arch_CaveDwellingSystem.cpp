#include "Arch_CaveDwellingSystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "UObject/ConstructorHelpers.h"

AArch_CaveDwellingSystem::AArch_CaveDwellingSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize components
    InitializeCaveComponents();
    
    // Set default cave properties
    CaveProperties.CaveType = EArch_CaveType::ShallowShelter;
    CaveProperties.Temperature = 18.0f;
    CaveProperties.Humidity = 0.8f;
    CaveProperties.bHasWaterSource = false;
    CaveProperties.MaxOccupants = 4;
    CaveProperties.ShelterValue = 0.7f;
    
    bIsOccupied = false;
    CurrentOccupants = 0;
}

void AArch_CaveDwellingSystem::InitializeCaveComponents()
{
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
    CaveInteriorMesh->SetRelativeLocation(FVector(-500.0f, 0.0f, 0.0f));
    CaveInteriorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    // Interior trigger volume
    InteriorTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("InteriorTrigger"));
    InteriorTrigger->SetupAttachment(RootComponent);
    InteriorTrigger->SetBoxExtent(FVector(300.0f, 300.0f, 200.0f));
    InteriorTrigger->SetRelativeLocation(FVector(-400.0f, 0.0f, 100.0f));
    InteriorTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteriorTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteriorTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Ambient audio component
    AmbientAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    AmbientAudio->SetupAttachment(RootComponent);
    AmbientAudio->bAutoActivate = false;
}

void AArch_CaveDwellingSystem::BeginPlay()
{
    Super::BeginPlay();
    
    SetupTriggerEvents();
    LoadCaveMeshes();
    SetupCaveInterior();
    ConfigureAmbientAudio();
}

void AArch_CaveDwellingSystem::SetupTriggerEvents()
{
    if (InteriorTrigger)
    {
        InteriorTrigger->OnComponentBeginOverlap.AddDynamic(this, &AArch_CaveDwellingSystem::OnInteriorTriggerBeginOverlap);
        InteriorTrigger->OnComponentEndOverlap.AddDynamic(this, &AArch_CaveDwellingSystem::OnInteriorTriggerEndOverlap);
    }
}

void AArch_CaveDwellingSystem::LoadCaveMeshes()
{
    // Load default cave meshes - using engine primitives for now
    if (CaveEntranceMesh)
    {
        // Set a basic mesh for the cave entrance
        UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
        if (CubeMesh)
        {
            CaveEntranceMesh->SetStaticMesh(CubeMesh);
            CaveEntranceMesh->SetRelativeScale3D(FVector(2.0f, 3.0f, 2.5f));
        }
    }

    if (CaveInteriorMesh)
    {
        // Set a basic mesh for the cave interior
        UStaticMesh* SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere"));
        if (SphereMesh)
        {
            CaveInteriorMesh->SetStaticMesh(SphereMesh);
            CaveInteriorMesh->SetRelativeScale3D(FVector(4.0f, 4.0f, 3.0f));
        }
    }
}

void AArch_CaveDwellingSystem::ConfigureAmbientAudio()
{
    if (AmbientAudio)
    {
        // Configure cave ambient sound settings
        AmbientAudio->SetVolumeMultiplier(0.3f);
        AmbientAudio->SetPitchMultiplier(0.8f);
        
        // Set 3D audio properties for cave acoustics
        AmbientAudio->bAllowSpatialization = true;
        AmbientAudio->AttenuationSettings = nullptr; // Use default attenuation
    }
}

void AArch_CaveDwellingSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update ambient effects based on occupancy
    UpdateAmbientEffects();
}

void AArch_CaveDwellingSystem::EnterCave(AActor* Character)
{
    if (!Character || !CanEnterCave())
    {
        return;
    }

    CurrentOccupants++;
    bIsOccupied = (CurrentOccupants > 0);

    // Log cave entry
    UE_LOG(LogTemp, Log, TEXT("Character %s entered cave. Current occupants: %d"), 
           *Character->GetName(), CurrentOccupants);

    // Start ambient audio
    if (AmbientAudio && !AmbientAudio->IsPlaying())
    {
        AmbientAudio->Play();
    }
}

void AArch_CaveDwellingSystem::ExitCave(AActor* Character)
{
    if (!Character || CurrentOccupants <= 0)
    {
        return;
    }

    CurrentOccupants--;
    bIsOccupied = (CurrentOccupants > 0);

    // Log cave exit
    UE_LOG(LogTemp, Log, TEXT("Character %s exited cave. Current occupants: %d"), 
           *Character->GetName(), CurrentOccupants);

    // Stop ambient audio if empty
    if (!bIsOccupied && AmbientAudio && AmbientAudio->IsPlaying())
    {
        AmbientAudio->Stop();
    }
}

bool AArch_CaveDwellingSystem::CanEnterCave() const
{
    return CurrentOccupants < CaveProperties.MaxOccupants;
}

float AArch_CaveDwellingSystem::GetShelterValue() const
{
    // Calculate shelter value based on cave properties
    float ShelterValue = CaveProperties.ShelterValue;
    
    // Bonus for water source
    if (CaveProperties.bHasWaterSource)
    {
        ShelterValue += 0.1f;
    }
    
    // Penalty for overcrowding
    if (CurrentOccupants > CaveProperties.MaxOccupants)
    {
        ShelterValue *= 0.5f;
    }
    
    return FMath::Clamp(ShelterValue, 0.0f, 1.0f);
}

void AArch_CaveDwellingSystem::SetupCaveInterior()
{
    // Clear existing props
    InteriorProps.Empty();
    
    // Spawn basic interior props based on cave type
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    FVector InteriorCenter = GetActorLocation() + FVector(-400.0f, 0.0f, 0.0f);
    
    // Add basic props for different cave types
    switch (CaveProperties.CaveType)
    {
        case EArch_CaveType::ShallowShelter:
            // Simple shelter setup
            break;
            
        case EArch_CaveType::DeepCave:
            // More complex cave setup
            break;
            
        case EArch_CaveType::TunnelSystem:
            // Tunnel network setup
            break;
            
        case EArch_CaveType::UndergroundChamber:
            // Large chamber setup
            break;
    }
}

void AArch_CaveDwellingSystem::UpdateAmbientEffects()
{
    if (!AmbientAudio)
    {
        return;
    }

    // Adjust audio based on occupancy and cave properties
    float VolumeMultiplier = bIsOccupied ? 0.5f : 0.2f;
    float PitchMultiplier = 0.8f + (CurrentOccupants * 0.05f);
    
    AmbientAudio->SetVolumeMultiplier(VolumeMultiplier);
    AmbientAudio->SetPitchMultiplier(FMath::Clamp(PitchMultiplier, 0.5f, 1.2f));
}

void AArch_CaveDwellingSystem::OnInteriorTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (ACharacter* Character = Cast<ACharacter>(OtherActor))
    {
        EnterCave(Character);
    }
}

void AArch_CaveDwellingSystem::OnInteriorTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (ACharacter* Character = Cast<ACharacter>(OtherActor))
    {
        ExitCave(Character);
    }
}