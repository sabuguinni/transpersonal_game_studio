#include "ArchitecturalStructure.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "UObject/ConstructorHelpers.h"

AArchitecturalStructure::AArchitecturalStructure()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create structure mesh component
    StructureMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StructureMesh"));
    StructureMesh->SetupAttachment(RootComponent);
    StructureMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    StructureMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
    StructureMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);

    // Create interaction volume
    InteractionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionVolume"));
    InteractionVolume->SetupAttachment(RootComponent);
    InteractionVolume->SetBoxExtent(FVector(300.0f, 300.0f, 200.0f));
    InteractionVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionVolume->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    InteractionVolume->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    InteractionVolume->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    // Initialize default properties
    Properties.StructureType = EArch_StructureType::Pillar;
    Properties.Condition = EArch_StructureCondition::Weathered;
    Properties.StructuralIntegrity = 75.0f;
    Properties.bCanClimb = false;
    Properties.bProvidesShade = true;
    Properties.bCanHideInside = false;

    StructureName = TEXT("Ancient Stone Pillar");
    StructureDescription = FText::FromString(TEXT("A weathered stone pillar from the Cretaceous period, covered in moss and vines."));
    InteractionRange = 300.0f;

    bPlayerInRange = false;
    TimeInCurrentCondition = 0.0f;
}

void AArchitecturalStructure::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    if (InteractionVolume)
    {
        InteractionVolume->OnComponentBeginOverlap.AddDynamic(this, &AArchitecturalStructure::OnInteractionVolumeBeginOverlap);
        InteractionVolume->OnComponentEndOverlap.AddDynamic(this, &AArchitecturalStructure::OnInteractionVolumeEndOverlap);
    }

    SetupStructureMesh();
    ApplyWeatheringEffects();
}

void AArchitecturalStructure::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TimeInCurrentCondition += DeltaTime;

    // Gradual weathering over time
    if (TimeInCurrentCondition > 60.0f) // Every minute of game time
    {
        UpdateStructureCondition();
        TimeInCurrentCondition = 0.0f;
    }
}

void AArchitecturalStructure::InitializeStructure(EArch_StructureType Type, EArch_StructureCondition InitialCondition)
{
    Properties.StructureType = Type;
    Properties.Condition = InitialCondition;

    switch (Type)
    {
        case EArch_StructureType::Pillar:
            StructureName = TEXT("Stone Pillar");
            Properties.bCanClimb = true;
            Properties.bProvidesShade = true;
            break;

        case EArch_StructureType::Archway:
            StructureName = TEXT("Stone Archway");
            Properties.bCanClimb = false;
            Properties.bProvidesShade = true;
            break;

        case EArch_StructureType::Wall:
            StructureName = TEXT("Stone Wall");
            Properties.bCanClimb = true;
            Properties.bProvidesShade = true;
            break;

        case EArch_StructureType::Ruins:
            StructureName = TEXT("Ancient Ruins");
            Properties.bCanClimb = true;
            Properties.bCanHideInside = true;
            Properties.StructuralIntegrity = 45.0f;
            break;

        case EArch_StructureType::Shelter:
            StructureName = TEXT("Primitive Shelter");
            Properties.bCanHideInside = true;
            Properties.bProvidesShade = true;
            Properties.StructuralIntegrity = 60.0f;
            break;

        case EArch_StructureType::Platform:
            StructureName = TEXT("Stone Platform");
            Properties.bCanClimb = true;
            Properties.bProvidesShade = false;
            break;
    }

    SetupStructureMesh();
    ApplyWeatheringEffects();
}

bool AArchitecturalStructure::CanPlayerInteract() const
{
    return bPlayerInRange && Properties.StructuralIntegrity > 10.0f;
}

void AArchitecturalStructure::OnPlayerEnterRange()
{
    bPlayerInRange = true;
    
    if (GEngine)
    {
        FString Message = FString::Printf(TEXT("Discovered: %s"), *StructureName);
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, Message);
    }
}

void AArchitecturalStructure::OnPlayerExitRange()
{
    bPlayerInRange = false;
}

void AArchitecturalStructure::OnInteractionVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (ACharacter* PlayerCharacter = Cast<ACharacter>(OtherActor))
    {
        OnPlayerEnterRange();
    }
}

void AArchitecturalStructure::OnInteractionVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (ACharacter* PlayerCharacter = Cast<ACharacter>(OtherActor))
    {
        OnPlayerExitRange();
    }
}

void AArchitecturalStructure::SetupStructureMesh()
{
    if (!StructureMesh)
        return;

    // Set appropriate collision based on structure type
    switch (Properties.StructureType)
    {
        case EArch_StructureType::Pillar:
        case EArch_StructureType::Wall:
            StructureMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
            break;

        case EArch_StructureType::Archway:
            StructureMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
            break;

        case EArch_StructureType::Platform:
            StructureMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
            break;

        case EArch_StructureType::Ruins:
        case EArch_StructureType::Shelter:
            StructureMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
            break;
    }
}

void AArchitecturalStructure::ApplyWeatheringEffects()
{
    // Adjust structural integrity based on condition
    switch (Properties.Condition)
    {
        case EArch_StructureCondition::Pristine:
            Properties.StructuralIntegrity = FMath::Clamp(Properties.StructuralIntegrity, 90.0f, 100.0f);
            break;

        case EArch_StructureCondition::Weathered:
            Properties.StructuralIntegrity = FMath::Clamp(Properties.StructuralIntegrity, 60.0f, 89.0f);
            break;

        case EArch_StructureCondition::Damaged:
            Properties.StructuralIntegrity = FMath::Clamp(Properties.StructuralIntegrity, 30.0f, 59.0f);
            break;

        case EArch_StructureCondition::Ruined:
            Properties.StructuralIntegrity = FMath::Clamp(Properties.StructuralIntegrity, 10.0f, 29.0f);
            break;

        case EArch_StructureCondition::Overgrown:
            Properties.StructuralIntegrity = FMath::Clamp(Properties.StructuralIntegrity, 40.0f, 70.0f);
            Properties.bCanHideInside = true; // Overgrowth provides cover
            break;
    }
}

void AArchitecturalStructure::UpdateStructureCondition()
{
    // Gradual degradation over time
    Properties.StructuralIntegrity = FMath::Max(Properties.StructuralIntegrity - 0.1f, 0.0f);

    // Update condition based on integrity
    if (Properties.StructuralIntegrity > 90.0f)
    {
        Properties.Condition = EArch_StructureCondition::Pristine;
    }
    else if (Properties.StructuralIntegrity > 60.0f)
    {
        Properties.Condition = EArch_StructureCondition::Weathered;
    }
    else if (Properties.StructuralIntegrity > 30.0f)
    {
        Properties.Condition = EArch_StructureCondition::Damaged;
    }
    else if (Properties.StructuralIntegrity > 10.0f)
    {
        Properties.Condition = EArch_StructureCondition::Ruined;
    }

    // Chance for overgrowth in humid environments
    if (FMath::RandRange(0.0f, 1.0f) < 0.05f && Properties.StructuralIntegrity > 40.0f)
    {
        Properties.Condition = EArch_StructureCondition::Overgrown;
    }
}