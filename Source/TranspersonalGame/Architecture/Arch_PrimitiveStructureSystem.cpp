#include "Arch_PrimitiveStructureSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"

AArch_PrimitiveStructureActor::AArch_PrimitiveStructureActor()
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

    // Bind overlap events
    InteractionVolume->OnComponentBeginOverlap.AddDynamic(this, &AArch_PrimitiveStructureActor::OnInteractionVolumeBeginOverlap);
    InteractionVolume->OnComponentEndOverlap.AddDynamic(this, &AArch_PrimitiveStructureActor::OnInteractionVolumeEndOverlap);

    // Initialize default properties
    StructureConfig.StructureType = EArch_PrimitiveStructureType::StoneArchway;
    StructureConfig.WeatheringLevel = 0.5f;
    StructureConfig.bHasMossGrowth = true;
    StructureConfig.bHasToolMarks = false;
    StructureConfig.StructuralIntegrity = 1.0f;
    StructureConfig.Dimensions = FVector(400.0f, 200.0f, 300.0f);

    bProvidesWeatherProtection = true;
    ProtectionRadius = 500.0f;
    bPlayerInside = false;
    LastWeatherCheckTime = 0.0f;
}

void AArch_PrimitiveStructureActor::BeginPlay()
{
    Super::BeginPlay();
    
    UpdateStructureMesh();
    ConfigureCollisionAndPhysics();

    // Start ambient sound if configured
    if (AmbientSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, AmbientSound, GetActorLocation(), 0.3f);
    }
}

void AArch_PrimitiveStructureActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Periodic weather protection checks
    LastWeatherCheckTime += DeltaTime;
    if (LastWeatherCheckTime >= 1.0f) // Check every second
    {
        LastWeatherCheckTime = 0.0f;
        
        if (bPlayerInside && bProvidesWeatherProtection)
        {
            // Could trigger weather protection effects here
        }
    }
}

void AArch_PrimitiveStructureActor::SetStructureType(EArch_PrimitiveStructureType NewType)
{
    StructureConfig.StructureType = NewType;
    UpdateStructureMesh();
}

bool AArch_PrimitiveStructureActor::IsPlayerInProtectedArea(const FVector& PlayerLocation) const
{
    if (!bProvidesWeatherProtection)
    {
        return false;
    }

    float Distance = FVector::Dist(GetActorLocation(), PlayerLocation);
    return Distance <= ProtectionRadius;
}

float AArch_PrimitiveStructureActor::GetWeatherProtectionStrength() const
{
    if (!bProvidesWeatherProtection)
    {
        return 0.0f;
    }

    float BaseProtection = 0.8f;
    float IntegrityModifier = StructureConfig.StructuralIntegrity;
    float WeatheringPenalty = StructureConfig.WeatheringLevel * 0.3f;

    return BaseProtection * IntegrityModifier * (1.0f - WeatheringPenalty);
}

void AArch_PrimitiveStructureActor::OnInteractionVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, 
    AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor->IsA<APawn>())
    {
        bPlayerInside = true;
        OnPlayerEnterStructure();
        
        if (GEngine)
        {
            FString StructureName = UEnum::GetValueAsString(StructureConfig.StructureType);
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                FString::Printf(TEXT("Entered %s - Weather Protection: %.1f%%"), 
                *StructureName, GetWeatherProtectionStrength() * 100.0f));
        }
    }
}

void AArch_PrimitiveStructureActor::OnInteractionVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, 
    AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor && OtherActor->IsA<APawn>())
    {
        bPlayerInside = false;
        OnPlayerExitStructure();
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Left structure protection"));
        }
    }
}

void AArch_PrimitiveStructureActor::UpdateStructureMesh()
{
    if (!StructureMesh)
    {
        return;
    }

    // Set mesh based on structure type
    UStaticMesh* MeshToUse = nullptr;
    
    switch (StructureConfig.StructureType)
    {
        case EArch_PrimitiveStructureType::StoneArchway:
            // Try to load archway mesh, fallback to cube
            MeshToUse = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
            break;
            
        case EArch_PrimitiveStructureType::CaveEntrance:
            MeshToUse = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere"));
            break;
            
        case EArch_PrimitiveStructureType::RockOverhang:
            MeshToUse = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Wedge"));
            break;
            
        case EArch_PrimitiveStructureType::StonePlatform:
            MeshToUse = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
            break;
            
        case EArch_PrimitiveStructureType::NaturalBridge:
            MeshToUse = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder"));
            break;
            
        case EArch_PrimitiveStructureType::WeatheredPillar:
            MeshToUse = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder"));
            break;
    }

    if (MeshToUse)
    {
        StructureMesh->SetStaticMesh(MeshToUse);
        
        // Scale based on dimensions
        FVector Scale = StructureConfig.Dimensions / 100.0f; // Convert to scale factor
        SetActorScale3D(Scale);
    }
}

void AArch_PrimitiveStructureActor::ConfigureCollisionAndPhysics()
{
    if (StructureMesh)
    {
        // Configure collision based on structure type
        switch (StructureConfig.StructureType)
        {
            case EArch_PrimitiveStructureType::StoneArchway:
            case EArch_PrimitiveStructureType::CaveEntrance:
                // These should allow passage
                StructureMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
                break;
                
            default:
                // Solid structures
                StructureMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
                break;
        }
    }
}

// Blueprint Function Library Implementation
TArray<AArch_PrimitiveStructureActor*> UArch_PrimitiveStructureLibrary::FindNearbyStructures(const FVector& Location, float Radius)
{
    TArray<AArch_PrimitiveStructureActor*> NearbyStructures;
    
    if (UWorld* World = GEngine->GetCurrentPlayWorld())
    {
        for (TActorIterator<AArch_PrimitiveStructureActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AArch_PrimitiveStructureActor* Structure = *ActorItr;
            if (Structure && FVector::Dist(Structure->GetActorLocation(), Location) <= Radius)
            {
                NearbyStructures.Add(Structure);
            }
        }
    }
    
    return NearbyStructures;
}

AArch_PrimitiveStructureActor* UArch_PrimitiveStructureLibrary::GetClosestStructure(const FVector& Location)
{
    AArch_PrimitiveStructureActor* ClosestStructure = nullptr;
    float ClosestDistance = FLT_MAX;
    
    if (UWorld* World = GEngine->GetCurrentPlayWorld())
    {
        for (TActorIterator<AArch_PrimitiveStructureActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AArch_PrimitiveStructureActor* Structure = *ActorItr;
            if (Structure)
            {
                float Distance = FVector::Dist(Structure->GetActorLocation(), Location);
                if (Distance < ClosestDistance)
                {
                    ClosestDistance = Distance;
                    ClosestStructure = Structure;
                }
            }
        }
    }
    
    return ClosestStructure;
}

bool UArch_PrimitiveStructureLibrary::IsLocationProtectedByStructure(const FVector& Location)
{
    if (UWorld* World = GEngine->GetCurrentPlayWorld())
    {
        for (TActorIterator<AArch_PrimitiveStructureActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AArch_PrimitiveStructureActor* Structure = *ActorItr;
            if (Structure && Structure->IsPlayerInProtectedArea(Location))
            {
                return true;
            }
        }
    }
    
    return false;
}

void UArch_PrimitiveStructureLibrary::SpawnStructureCluster(const FVector& CenterLocation, int32 Count, float SpreadRadius)
{
    if (UWorld* World = GEngine->GetCurrentPlayWorld())
    {
        for (int32 i = 0; i < Count; i++)
        {
            // Random position within spread radius
            FVector RandomOffset = FVector(
                FMath::RandRange(-SpreadRadius, SpreadRadius),
                FMath::RandRange(-SpreadRadius, SpreadRadius),
                0.0f
            );
            
            FVector SpawnLocation = CenterLocation + RandomOffset;
            
            // Random structure type
            int32 TypeIndex = FMath::RandRange(0, 5);
            EArch_PrimitiveStructureType StructureType = static_cast<EArch_PrimitiveStructureType>(TypeIndex);
            
            // Spawn the structure
            AArch_PrimitiveStructureActor* NewStructure = World->SpawnActor<AArch_PrimitiveStructureActor>(
                AArch_PrimitiveStructureActor::StaticClass(),
                SpawnLocation,
                FRotator::ZeroRotator
            );
            
            if (NewStructure)
            {
                NewStructure->SetStructureType(StructureType);
            }
        }
    }
}