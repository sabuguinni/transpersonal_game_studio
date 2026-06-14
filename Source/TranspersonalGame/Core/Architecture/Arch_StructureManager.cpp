#include "Arch_StructureManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Engine.h"

AArch_StructureManager::AArch_StructureManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create structure mesh component
    StructureMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StructureMesh"));
    StructureMesh->SetupAttachment(RootComponent);

    // Create protection zone
    ProtectionZone = CreateDefaultSubobject<UBoxComponent>(TEXT("ProtectionZone"));
    ProtectionZone->SetupAttachment(RootComponent);
    ProtectionZone->SetBoxExtent(FVector(500.0f, 500.0f, 300.0f));
    ProtectionZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    ProtectionZone->SetCollisionResponseToAllChannels(ECR_Ignore);
    ProtectionZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Bind overlap events
    ProtectionZone->OnComponentBeginOverlap.AddDynamic(this, &AArch_StructureManager::OnProtectionZoneBeginOverlap);
    ProtectionZone->OnComponentEndOverlap.AddDynamic(this, &AArch_StructureManager::OnProtectionZoneEndOverlap);

    // Initialize default config
    StructureConfig.StructureType = EArch_StructureType::StoneArchway;
    StructureConfig.ProtectionRadius = 500.0f;
    StructureConfig.TemperatureModifier = 5.0f;
    StructureConfig.bProvidesWeatherProtection = true;
    StructureConfig.MaxOccupants = 4;

    CurrentOccupants.Empty();
}

void AArch_StructureManager::BeginPlay()
{
    Super::BeginPlay();
    
    UpdateStructureMesh();
    UpdateProtectionRadius();
    
    UE_LOG(LogTemp, Warning, TEXT("Arch_StructureManager: Structure initialized - Type: %d, Protection Radius: %.1f"), 
           (int32)StructureConfig.StructureType, StructureConfig.ProtectionRadius);
}

void AArch_StructureManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Clean up invalid occupants
    CurrentOccupants.RemoveAll([](AActor* Actor) {
        return !IsValid(Actor);
    });

    // Update protection effects for occupants
    for (AActor* Occupant : CurrentOccupants)
    {
        if (IsValid(Occupant))
        {
            // Apply temperature modifier and weather protection
            // This would integrate with survival system
            FVector OccupantLocation = Occupant->GetActorLocation();
            float ProtectionLevel = GetProtectionLevel(OccupantLocation);
            
            // Log protection status for debugging
            if (GEngine && GetWorld()->GetTimeSeconds() - GetWorld()->GetDeltaSeconds() < 1.0f)
            {
                FString DebugMsg = FString::Printf(TEXT("Occupant %s - Protection: %.1f%%"), 
                                                 *Occupant->GetName(), ProtectionLevel * 100.0f);
                GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, DebugMsg);
            }
        }
    }
}

bool AArch_StructureManager::CanEnterStructure(AActor* Actor)
{
    if (!IsValid(Actor))
    {
        return false;
    }

    // Check if already inside
    if (CurrentOccupants.Contains(Actor))
    {
        return false;
    }

    // Check occupancy limit
    if (CurrentOccupants.Num() >= StructureConfig.MaxOccupants)
    {
        return false;
    }

    // Check if actor is within protection zone
    FVector ActorLocation = Actor->GetActorLocation();
    return IsLocationProtected(ActorLocation);
}

bool AArch_StructureManager::EnterStructure(AActor* Actor)
{
    if (!CanEnterStructure(Actor))
    {
        return false;
    }

    CurrentOccupants.AddUnique(Actor);
    
    UE_LOG(LogTemp, Warning, TEXT("Arch_StructureManager: Actor %s entered structure. Occupants: %d/%d"), 
           *Actor->GetName(), CurrentOccupants.Num(), StructureConfig.MaxOccupants);
    
    return true;
}

bool AArch_StructureManager::ExitStructure(AActor* Actor)
{
    if (!IsValid(Actor))
    {
        return false;
    }

    bool bWasRemoved = CurrentOccupants.Remove(Actor) > 0;
    
    if (bWasRemoved)
    {
        UE_LOG(LogTemp, Warning, TEXT("Arch_StructureManager: Actor %s exited structure. Occupants: %d/%d"), 
               *Actor->GetName(), CurrentOccupants.Num(), StructureConfig.MaxOccupants);
    }
    
    return bWasRemoved;
}

float AArch_StructureManager::GetProtectionLevel(const FVector& Location)
{
    if (!StructureConfig.bProvidesWeatherProtection)
    {
        return 0.0f;
    }

    FVector StructureLocation = GetActorLocation();
    float Distance = FVector::Dist(Location, StructureLocation);
    
    if (Distance <= StructureConfig.ProtectionRadius)
    {
        // Linear falloff from center to edge
        float ProtectionLevel = 1.0f - (Distance / StructureConfig.ProtectionRadius);
        return FMath::Clamp(ProtectionLevel, 0.0f, 1.0f);
    }
    
    return 0.0f;
}

bool AArch_StructureManager::IsLocationProtected(const FVector& Location)
{
    return GetProtectionLevel(Location) > 0.0f;
}

void AArch_StructureManager::SetStructureType(EArch_StructureType NewType)
{
    if (StructureConfig.StructureType != NewType)
    {
        StructureConfig.StructureType = NewType;
        UpdateStructureMesh();
        UpdateProtectionRadius();
        
        UE_LOG(LogTemp, Warning, TEXT("Arch_StructureManager: Structure type changed to %d"), (int32)NewType);
    }
}

void AArch_StructureManager::OnProtectionZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (IsValid(OtherActor) && OtherActor != this)
    {
        // Auto-enter for player characters
        if (OtherActor->IsA<APawn>())
        {
            EnterStructure(OtherActor);
        }
    }
}

void AArch_StructureManager::OnProtectionZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (IsValid(OtherActor) && OtherActor != this)
    {
        ExitStructure(OtherActor);
    }
}

void AArch_StructureManager::UpdateStructureMesh()
{
    if (!StructureMesh)
    {
        return;
    }

    // Set appropriate mesh based on structure type
    UStaticMesh* MeshToUse = nullptr;
    
    switch (StructureConfig.StructureType)
    {
        case EArch_StructureType::StoneArchway:
            // Use default cube for now - will be replaced with proper archway mesh
            MeshToUse = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
            if (MeshToUse)
            {
                StructureMesh->SetWorldScale3D(FVector(4.0f, 1.0f, 3.0f));
            }
            break;
            
        case EArch_StructureType::CaveEntrance:
            MeshToUse = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
            if (MeshToUse)
            {
                StructureMesh->SetWorldScale3D(FVector(3.0f, 3.0f, 2.0f));
            }
            break;
            
        case EArch_StructureType::RockShelter:
            MeshToUse = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
            if (MeshToUse)
            {
                StructureMesh->SetWorldScale3D(FVector(5.0f, 5.0f, 2.0f));
            }
            break;
            
        case EArch_StructureType::StonePlatform:
            MeshToUse = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
            if (MeshToUse)
            {
                StructureMesh->SetWorldScale3D(FVector(4.0f, 4.0f, 0.5f));
            }
            break;
            
        case EArch_StructureType::AncientRuins:
            MeshToUse = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
            if (MeshToUse)
            {
                StructureMesh->SetWorldScale3D(FVector(6.0f, 6.0f, 4.0f));
            }
            break;
    }
    
    if (MeshToUse)
    {
        StructureMesh->SetStaticMesh(MeshToUse);
        UE_LOG(LogTemp, Warning, TEXT("Arch_StructureManager: Mesh updated for structure type %d"), (int32)StructureConfig.StructureType);
    }
}

void AArch_StructureManager::UpdateProtectionRadius()
{
    if (ProtectionZone)
    {
        FVector BoxExtent = FVector(StructureConfig.ProtectionRadius, StructureConfig.ProtectionRadius, 300.0f);
        ProtectionZone->SetBoxExtent(BoxExtent);
        
        UE_LOG(LogTemp, Warning, TEXT("Arch_StructureManager: Protection radius updated to %.1f"), StructureConfig.ProtectionRadius);
    }
}