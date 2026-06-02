#include "Arch_VolcanicStructureSystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

AArch_VolcanicStructureSystem::AArch_VolcanicStructureSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create structure mesh component
    StructureMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StructureMesh"));
    StructureMesh->SetupAttachment(RootComponent);

    // Create heat zone collision
    HeatZone = CreateDefaultSubobject<UBoxComponent>(TEXT("HeatZone"));
    HeatZone->SetupAttachment(RootComponent);
    HeatZone->SetBoxExtent(FVector(500.0f, 500.0f, 300.0f));
    HeatZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    HeatZone->SetCollisionResponseToAllChannels(ECR_Ignore);
    HeatZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Initialize structure data
    StructureData.StructureType = EArch_VolcanicStructureType::LavaRock;
    StructureData.Temperature = 800.0f;
    StructureData.bIsActiveFlow = false;
    StructureData.DamageRadius = 500.0f;

    CurrentHeatIntensity = 0.5f;
}

void AArch_VolcanicStructureSystem::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    HeatZone->OnComponentBeginOverlap.AddDynamic(this, &AArch_VolcanicStructureSystem::OnHeatZoneBeginOverlap);
    HeatZone->OnComponentEndOverlap.AddDynamic(this, &AArch_VolcanicStructureSystem::OnHeatZoneEndOverlap);

    // Initialize structure appearance
    UpdateStructureAppearance();
}

void AArch_VolcanicStructureSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (StructureData.bIsActiveFlow)
    {
        UpdateHeatEffects();
    }
}

void AArch_VolcanicStructureSystem::SetStructureType(EArch_VolcanicStructureType NewType)
{
    StructureData.StructureType = NewType;
    UpdateStructureAppearance();

    // Adjust properties based on type
    switch (NewType)
    {
        case EArch_VolcanicStructureType::LavaRock:
            StructureData.Temperature = 600.0f;
            StructureData.DamageRadius = 300.0f;
            break;
        case EArch_VolcanicStructureType::VolcanicPillar:
            StructureData.Temperature = 400.0f;
            StructureData.DamageRadius = 200.0f;
            break;
        case EArch_VolcanicStructureType::LavaFlow:
            StructureData.Temperature = 1200.0f;
            StructureData.DamageRadius = 800.0f;
            StructureData.bIsActiveFlow = true;
            break;
        case EArch_VolcanicStructureType::VolcanicCave:
            StructureData.Temperature = 300.0f;
            StructureData.DamageRadius = 150.0f;
            break;
        case EArch_VolcanicStructureType::ObsidianFormation:
            StructureData.Temperature = 200.0f;
            StructureData.DamageRadius = 100.0f;
            break;
    }

    // Update heat zone size
    HeatZone->SetBoxExtent(FVector(StructureData.DamageRadius, StructureData.DamageRadius, 300.0f));
}

void AArch_VolcanicStructureSystem::ActivateLavaFlow()
{
    StructureData.bIsActiveFlow = true;
    StructureData.Temperature = FMath::Max(StructureData.Temperature, 1000.0f);

    // Start lava flow timer
    GetWorldTimerManager().SetTimer(LavaFlowTimer, this, &AArch_VolcanicStructureSystem::SpawnVolcanicDebris, 5.0f, true);

    UE_LOG(LogTemp, Warning, TEXT("Lava flow activated at %s"), *GetActorLocation().ToString());
}

void AArch_VolcanicStructureSystem::DeactivateLavaFlow()
{
    StructureData.bIsActiveFlow = false;
    StructureData.Temperature *= 0.7f; // Cool down

    // Clear lava flow timer
    GetWorldTimerManager().ClearTimer(LavaFlowTimer);

    UE_LOG(LogTemp, Warning, TEXT("Lava flow deactivated at %s"), *GetActorLocation().ToString());
}

float AArch_VolcanicStructureSystem::GetTemperature() const
{
    return StructureData.Temperature;
}

void AArch_VolcanicStructureSystem::SpawnVolcanicDebris()
{
    if (!StructureData.bIsActiveFlow) return;

    // Spawn volcanic debris around the structure
    for (int32 i = 0; i < 3; i++)
    {
        FVector SpawnLocation = GetActorLocation() + FVector(
            FMath::RandRange(-200.0f, 200.0f),
            FMath::RandRange(-200.0f, 200.0f),
            FMath::RandRange(0.0f, 100.0f)
        );

        // TODO: Spawn actual volcanic debris actors when available
        UE_LOG(LogTemp, Log, TEXT("Volcanic debris spawned at %s"), *SpawnLocation.ToString());
    }
}

void AArch_VolcanicStructureSystem::OnHeatZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (ACharacter* Character = Cast<ACharacter>(OtherActor))
    {
        // Apply heat damage to character
        float DamageAmount = StructureData.Temperature / 100.0f; // Scale damage based on temperature
        
        // TODO: Apply actual damage when damage system is available
        UE_LOG(LogTemp, Warning, TEXT("Character %s entered heat zone - Temperature: %f"), 
               *Character->GetName(), StructureData.Temperature);
    }
}

void AArch_VolcanicStructureSystem::OnHeatZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (ACharacter* Character = Cast<ACharacter>(OtherActor))
    {
        UE_LOG(LogTemp, Log, TEXT("Character %s left heat zone"), *Character->GetName());
    }
}

void AArch_VolcanicStructureSystem::UpdateStructureAppearance()
{
    // Update mesh based on structure type
    if (VolcanicMeshes.Num() > 0)
    {
        int32 MeshIndex = static_cast<int32>(StructureData.StructureType);
        if (VolcanicMeshes.IsValidIndex(MeshIndex) && VolcanicMeshes[MeshIndex])
        {
            StructureMesh->SetStaticMesh(VolcanicMeshes[MeshIndex]);
        }
    }

    // Update scale based on type
    FVector NewScale = FVector(1.0f);
    switch (StructureData.StructureType)
    {
        case EArch_VolcanicStructureType::LavaRock:
            NewScale = FVector(1.5f, 1.5f, 1.0f);
            break;
        case EArch_VolcanicStructureType::VolcanicPillar:
            NewScale = FVector(0.8f, 0.8f, 3.0f);
            break;
        case EArch_VolcanicStructureType::LavaFlow:
            NewScale = FVector(3.0f, 2.0f, 0.5f);
            break;
        case EArch_VolcanicStructureType::VolcanicCave:
            NewScale = FVector(2.5f, 2.5f, 1.5f);
            break;
        case EArch_VolcanicStructureType::ObsidianFormation:
            NewScale = FVector(1.2f, 1.2f, 2.0f);
            break;
    }
    SetActorScale3D(NewScale);
}

void AArch_VolcanicStructureSystem::UpdateHeatEffects()
{
    // Oscillate heat intensity for visual effects
    CurrentHeatIntensity = 0.5f + 0.3f * FMath::Sin(GetWorld()->GetTimeSeconds() * 2.0f);
    
    // TODO: Update particle effects and material parameters when available
    if (StructureData.bIsActiveFlow && FMath::RandFloat() < 0.1f)
    {
        // Randomly spawn heat effects
        UE_LOG(LogTemp, VeryVerbose, TEXT("Heat effect intensity: %f"), CurrentHeatIntensity);
    }
}