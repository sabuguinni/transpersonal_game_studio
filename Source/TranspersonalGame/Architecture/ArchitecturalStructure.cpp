#include "ArchitecturalStructure.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"

AArchitecturalStructure::AArchitecturalStructure()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create main structure mesh
    MainStructureMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainStructureMesh"));
    MainStructureMesh->SetupAttachment(RootComponent);
    MainStructureMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MainStructureMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
    MainStructureMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);

    // Create interaction volume
    InteractionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionVolume"));
    InteractionVolume->SetupAttachment(RootComponent);
    InteractionVolume->SetBoxExtent(FVector(800.0f, 800.0f, 400.0f));
    InteractionVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionVolume->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    InteractionVolume->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    InteractionVolume->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    // Bind overlap events
    InteractionVolume->OnComponentBeginOverlap.AddDynamic(this, &AArchitecturalStructure::OnInteractionVolumeBeginOverlap);
    InteractionVolume->OnComponentEndOverlap.AddDynamic(this, &AArchitecturalStructure::OnInteractionVolumeEndOverlap);

    // Initialize structure data
    StructureData.StructureType = EArch_StructureType::Ruins;
    StructureData.Condition = EArch_StructureCondition::Weathered;
    StructureData.Age = 1000.0f;
    StructureData.bIsExplorable = true;
    StructureData.bHasInterior = false;
    StructureData.HistoricalDescription = TEXT("Ancient stone structure from the Cretaceous period, weathered by millennia of exposure to the elements.");

    // Initialize gameplay properties
    bCanProvideShelte = true;
    ShelterRadius = 500.0f;
    bHasHiddenAreas = false;
    bPlayerInside = false;
    LastWeatheringTime = 0.0f;
}

void AArchitecturalStructure::BeginPlay()
{
    Super::BeginPlay();
    
    // Apply initial visual condition based on structure data
    UpdateVisualCondition();
    
    // Log structure creation
    UE_LOG(LogTemp, Warning, TEXT("ArchitecturalStructure spawned: %s, Age: %.1f years"), 
           *UEnum::GetValueAsString(StructureData.StructureType), StructureData.Age);
}

void AArchitecturalStructure::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Gradual weathering over time (very slow)
    LastWeatheringTime += DeltaTime;
    if (LastWeatheringTime >= 60.0f) // Every minute
    {
        ApplyWeathering(0.001f); // Very small weathering amount
        LastWeatheringTime = 0.0f;
    }
}

void AArchitecturalStructure::SetStructureType(EArch_StructureType NewType)
{
    StructureData.StructureType = NewType;
    UpdateVisualCondition();
    
    // Update properties based on structure type
    switch (NewType)
    {
        case EArch_StructureType::Temple:
            StructureData.bHasInterior = true;
            bCanProvideShelte = true;
            ShelterRadius = 800.0f;
            break;
        case EArch_StructureType::Ruins:
            StructureData.bHasInterior = false;
            bCanProvideShelte = false;
            ShelterRadius = 300.0f;
            break;
        case EArch_StructureType::Shelter:
            StructureData.bHasInterior = true;
            bCanProvideShelte = true;
            ShelterRadius = 400.0f;
            break;
        case EArch_StructureType::Bridge:
            StructureData.bHasInterior = false;
            bCanProvideShelte = false;
            ShelterRadius = 0.0f;
            break;
        default:
            bCanProvideShelte = false;
            ShelterRadius = 200.0f;
            break;
    }
}

void AArchitecturalStructure::SetCondition(EArch_StructureCondition NewCondition)
{
    StructureData.Condition = NewCondition;
    UpdateVisualCondition();
}

bool AArchitecturalStructure::IsPlayerInShelter(AActor* Player)
{
    if (!Player || !bCanProvideShelte)
    {
        return false;
    }

    float Distance = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
    return Distance <= ShelterRadius;
}

void AArchitecturalStructure::ApplyWeathering(float WeatheringAmount)
{
    StructureData.Age += WeatheringAmount * 10.0f; // Age increases with weathering

    // Gradually worsen condition based on age
    if (StructureData.Age > 2000.0f && StructureData.Condition == EArch_StructureCondition::Pristine)
    {
        SetCondition(EArch_StructureCondition::Weathered);
    }
    else if (StructureData.Age > 5000.0f && StructureData.Condition == EArch_StructureCondition::Weathered)
    {
        SetCondition(EArch_StructureCondition::Damaged);
    }
    else if (StructureData.Age > 10000.0f && StructureData.Condition == EArch_StructureCondition::Damaged)
    {
        SetCondition(EArch_StructureCondition::Ruined);
    }
}

FString AArchitecturalStructure::GetStructureDescription() const
{
    FString TypeString = UEnum::GetValueAsString(StructureData.StructureType);
    FString ConditionString = UEnum::GetValueAsString(StructureData.Condition);
    
    return FString::Printf(TEXT("%s %s (Age: %.0f years) - %s"), 
                          *ConditionString, 
                          *TypeString, 
                          StructureData.Age, 
                          *StructureData.HistoricalDescription);
}

void AArchitecturalStructure::OnInteractionVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor->IsA<ACharacter>())
    {
        bPlayerInside = true;
        OnPlayerEnterStructure(OtherActor);
        
        // Log interaction
        UE_LOG(LogTemp, Log, TEXT("Player entered %s"), *GetStructureDescription());
    }
}

void AArchitecturalStructure::OnInteractionVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor && OtherActor->IsA<ACharacter>())
    {
        bPlayerInside = false;
        OnPlayerExitStructure(OtherActor);
        
        // Log interaction
        UE_LOG(LogTemp, Log, TEXT("Player exited %s"), *GetStructureDescription());
    }
}

void AArchitecturalStructure::UpdateVisualCondition()
{
    if (!MainStructureMesh)
    {
        return;
    }

    // Apply visual changes based on condition
    // This would typically involve material parameter changes, mesh swapping, etc.
    // For now, we'll just log the condition change
    UE_LOG(LogTemp, Warning, TEXT("Structure visual condition updated: %s"), 
           *UEnum::GetValueAsString(StructureData.Condition));
    
    // In a full implementation, you would:
    // 1. Change material parameters (rust, moss, damage)
    // 2. Swap meshes for different damage states
    // 3. Add/remove particle effects (dust, vegetation)
    // 4. Adjust collision based on structural integrity
}