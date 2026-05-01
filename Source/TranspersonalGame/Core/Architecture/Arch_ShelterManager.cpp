#include "Arch_ShelterManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"

AArch_ShelterManager::AArch_ShelterManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create shelter mesh component
    ShelterMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShelterMesh"));
    ShelterMesh->SetupAttachment(RootComponent);
    ShelterMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    ShelterMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);

    // Create interaction volume
    InteractionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionVolume"));
    InteractionVolume->SetupAttachment(RootComponent);
    InteractionVolume->SetBoxExtent(FVector(300.0f, 300.0f, 200.0f));
    InteractionVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionVolume->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    InteractionVolume->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    InteractionVolume->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    // Initialize shelter properties
    ShelterProperties.ShelterType = EArch_ShelterType::Cave;
    ShelterProperties.ProtectionLevel = 0.7f;
    ShelterProperties.TemperatureBonus = 15.0f;
    ShelterProperties.MaxOccupants = 2;
    ShelterProperties.bHasFirePit = true;
    ShelterProperties.bHasStorage = true;
    ShelterProperties.bIsDefensive = false;

    RestBonus = 25.0f;
    WeatherProtection = 0.8f;

    // Bind overlap events
    InteractionVolume->OnComponentBeginOverlap.AddDynamic(this, &AArch_ShelterManager::OnInteractionVolumeBeginOverlap);
    InteractionVolume->OnComponentEndOverlap.AddDynamic(this, &AArch_ShelterManager::OnInteractionVolumeEndOverlap);
}

void AArch_ShelterManager::BeginPlay()
{
    Super::BeginPlay();
    
    UpdateShelterMesh();
    
    UE_LOG(LogTemp, Log, TEXT("Shelter Manager initialized: Type=%d, Protection=%.2f"), 
           (int32)ShelterProperties.ShelterType, ShelterProperties.ProtectionLevel);
}

void AArch_ShelterManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Apply continuous shelter effects to occupants
    for (AActor* Occupant : OccupiedBy)
    {
        if (IsValid(Occupant))
        {
            ApplyShelterEffects(Occupant);
        }
    }
}

bool AArch_ShelterManager::CanEnterShelter(AActor* Actor)
{
    if (!IsValid(Actor))
    {
        return false;
    }

    if (OccupiedBy.Num() >= ShelterProperties.MaxOccupants)
    {
        return false;
    }

    return true;
}

bool AArch_ShelterManager::EnterShelter(AActor* Actor)
{
    if (!CanEnterShelter(Actor))
    {
        return false;
    }

    if (!OccupiedBy.Contains(Actor))
    {
        OccupiedBy.Add(Actor);
        ApplyShelterEffects(Actor);
        
        UE_LOG(LogTemp, Log, TEXT("Actor %s entered shelter"), *Actor->GetName());
        return true;
    }

    return false;
}

bool AArch_ShelterManager::ExitShelter(AActor* Actor)
{
    if (OccupiedBy.Contains(Actor))
    {
        OccupiedBy.Remove(Actor);
        RemoveShelterEffects(Actor);
        
        UE_LOG(LogTemp, Log, TEXT("Actor %s exited shelter"), *Actor->GetName());
        return true;
    }

    return false;
}

float AArch_ShelterManager::GetProtectionLevel() const
{
    return ShelterProperties.ProtectionLevel;
}

float AArch_ShelterManager::GetTemperatureBonus() const
{
    return ShelterProperties.TemperatureBonus;
}

bool AArch_ShelterManager::HasFirePit() const
{
    return ShelterProperties.bHasFirePit;
}

bool AArch_ShelterManager::HasStorage() const
{
    return ShelterProperties.bHasStorage;
}

void AArch_ShelterManager::SetShelterType(EArch_ShelterType NewType)
{
    ShelterProperties.ShelterType = NewType;
    UpdateShelterMesh();
    
    // Adjust properties based on shelter type
    switch (NewType)
    {
        case EArch_ShelterType::Cave:
            ShelterProperties.ProtectionLevel = 0.9f;
            ShelterProperties.TemperatureBonus = 20.0f;
            ShelterProperties.MaxOccupants = 3;
            ShelterProperties.bHasFirePit = true;
            ShelterProperties.bHasStorage = true;
            ShelterProperties.bIsDefensive = true;
            break;
            
        case EArch_ShelterType::TreePlatform:
            ShelterProperties.ProtectionLevel = 0.6f;
            ShelterProperties.TemperatureBonus = 10.0f;
            ShelterProperties.MaxOccupants = 2;
            ShelterProperties.bHasFirePit = true;
            ShelterProperties.bHasStorage = true;
            ShelterProperties.bIsDefensive = false;
            break;
            
        case EArch_ShelterType::UndergroundBunker:
            ShelterProperties.ProtectionLevel = 0.95f;
            ShelterProperties.TemperatureBonus = 25.0f;
            ShelterProperties.MaxOccupants = 5;
            ShelterProperties.bHasFirePit = true;
            ShelterProperties.bHasStorage = true;
            ShelterProperties.bIsDefensive = true;
            break;
            
        case EArch_ShelterType::FortifiedCamp:
            ShelterProperties.ProtectionLevel = 0.8f;
            ShelterProperties.TemperatureBonus = 15.0f;
            ShelterProperties.MaxOccupants = 8;
            ShelterProperties.bHasFirePit = true;
            ShelterProperties.bHasStorage = true;
            ShelterProperties.bIsDefensive = true;
            break;
            
        case EArch_ShelterType::RockOverhang:
            ShelterProperties.ProtectionLevel = 0.5f;
            ShelterProperties.TemperatureBonus = 8.0f;
            ShelterProperties.MaxOccupants = 2;
            ShelterProperties.bHasFirePit = false;
            ShelterProperties.bHasStorage = false;
            ShelterProperties.bIsDefensive = false;
            break;
            
        default:
            ShelterProperties.ProtectionLevel = 0.3f;
            ShelterProperties.TemperatureBonus = 5.0f;
            ShelterProperties.MaxOccupants = 1;
            ShelterProperties.bHasFirePit = false;
            ShelterProperties.bHasStorage = false;
            ShelterProperties.bIsDefensive = false;
            break;
    }
}

EArch_ShelterType AArch_ShelterManager::GetShelterType() const
{
    return ShelterProperties.ShelterType;
}

void AArch_ShelterManager::OnInteractionVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (IsValid(OtherActor) && OtherActor->IsA<APawn>())
    {
        EnterShelter(OtherActor);
    }
}

void AArch_ShelterManager::OnInteractionVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (IsValid(OtherActor))
    {
        ExitShelter(OtherActor);
    }
}

void AArch_ShelterManager::ApplyShelterEffects(AActor* Actor)
{
    if (!IsValid(Actor))
    {
        return;
    }

    // Apply temperature bonus, rest bonus, and protection effects
    // This would integrate with the character's survival system
    UE_LOG(LogTemp, Log, TEXT("Applying shelter effects to %s: Protection=%.2f, Temperature=%.2f"), 
           *Actor->GetName(), ShelterProperties.ProtectionLevel, ShelterProperties.TemperatureBonus);
}

void AArch_ShelterManager::RemoveShelterEffects(AActor* Actor)
{
    if (!IsValid(Actor))
    {
        return;
    }

    // Remove shelter bonuses
    UE_LOG(LogTemp, Log, TEXT("Removing shelter effects from %s"), *Actor->GetName());
}

void AArch_ShelterManager::UpdateShelterMesh()
{
    // Update the visual representation based on shelter type
    // In a full implementation, this would load different static meshes
    // For now, we'll use basic shapes and log the change
    
    if (ShelterMesh)
    {
        FVector NewScale = FVector(1.0f);
        
        switch (ShelterProperties.ShelterType)
        {
            case EArch_ShelterType::Cave:
                NewScale = FVector(2.0f, 2.0f, 1.5f);
                break;
            case EArch_ShelterType::TreePlatform:
                NewScale = FVector(1.5f, 1.5f, 0.5f);
                break;
            case EArch_ShelterType::UndergroundBunker:
                NewScale = FVector(3.0f, 3.0f, 1.0f);
                break;
            case EArch_ShelterType::FortifiedCamp:
                NewScale = FVector(4.0f, 4.0f, 2.0f);
                break;
            case EArch_ShelterType::RockOverhang:
                NewScale = FVector(1.2f, 2.0f, 1.0f);
                break;
            default:
                NewScale = FVector(1.0f, 1.0f, 1.0f);
                break;
        }
        
        ShelterMesh->SetWorldScale3D(NewScale);
        
        UE_LOG(LogTemp, Log, TEXT("Updated shelter mesh for type %d with scale %.2f,%.2f,%.2f"), 
               (int32)ShelterProperties.ShelterType, NewScale.X, NewScale.Y, NewScale.Z);
    }
}