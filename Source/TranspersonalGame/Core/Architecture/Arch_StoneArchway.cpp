#include "Arch_StoneArchway.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "Kismet/KismetMathLibrary.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"

AArch_StoneArchway::AArch_StoneArchway()
{
    PrimaryActorTick.bCanEverTick = true;

    // Root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Archway mesh
    ArchwayMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArchwayMesh"));
    ArchwayMesh->SetupAttachment(RootComponent);
    ArchwayMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    ArchwayMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);

    // Interior volume
    InteriorVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteriorVolume"));
    InteriorVolume->SetupAttachment(RootComponent);
    InteriorVolume->SetBoxExtent(FVector(200.0f, 200.0f, 300.0f));
    InteriorVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteriorVolume->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    InteriorVolume->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);

    // Entrance trigger
    EntranceTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("EntranceTrigger"));
    EntranceTrigger->SetupAttachment(RootComponent);
    EntranceTrigger->SetBoxExtent(FVector(250.0f, 250.0f, 350.0f));
    EntranceTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    EntranceTrigger->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    EntranceTrigger->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);

    // Bind overlap events
    EntranceTrigger->OnComponentBeginOverlap.AddDynamic(this, &AArch_StoneArchway::OnEntranceTriggerBeginOverlap);
    EntranceTrigger->OnComponentEndOverlap.AddDynamic(this, &AArch_StoneArchway::OnEntranceTriggerEndOverlap);

    // Initialize default config
    ArchwayConfig.ArchwayType = EArch_ArchwayType::CaveEntrance;
    ArchwayConfig.Width = 400.0f;
    ArchwayConfig.Height = 600.0f;
    ArchwayConfig.Depth = 200.0f;
    ArchwayConfig.bHasInteriorSpace = true;
    ArchwayConfig.bBlocksWeather = true;
    ArchwayConfig.TemperatureModifier = 5.0f;

    WeatherProtectionRadius = 500.0f;
    RainProtectionFactor = 0.8f;
    WindProtectionFactor = 0.6f;
}

void AArch_StoneArchway::BeginPlay()
{
    Super::BeginPlay();
    
    UpdateArchwayMesh();
    UpdateInteriorVolume();
    SetupInteriorProps();
    ApplyWeatherEffects();
}

void AArch_StoneArchway::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update weather protection for actors inside
    for (AActor* Actor : ActorsInside)
    {
        if (IsValid(Actor))
        {
            // Apply temperature modifier and weather protection
            // This would integrate with the survival system
            if (ArchwayConfig.bBlocksWeather)
            {
                // Reduce environmental stress for actors inside
                FVector ActorLocation = Actor->GetActorLocation();
                float ProtectionStrength = GetWeatherProtectionAt(ActorLocation);
                
                // Log protection for debugging
                if (GEngine && Actor->IsA<APawn>())
                {
                    GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Green, 
                        FString::Printf(TEXT("Weather Protection: %.2f"), ProtectionStrength));
                }
            }
        }
    }
}

void AArch_StoneArchway::SetArchwayType(EArch_ArchwayType NewType)
{
    ArchwayConfig.ArchwayType = NewType;
    UpdateArchwayMesh();
    UpdateInteriorVolume();
}

bool AArch_StoneArchway::IsPointInsideArchway(const FVector& Point) const
{
    if (!InteriorVolume)
    {
        return false;
    }

    FVector LocalPoint = InteriorVolume->GetComponentTransform().InverseTransformPosition(Point);
    FVector BoxExtent = InteriorVolume->GetScaledBoxExtent();
    
    return (FMath::Abs(LocalPoint.X) <= BoxExtent.X) &&
           (FMath::Abs(LocalPoint.Y) <= BoxExtent.Y) &&
           (FMath::Abs(LocalPoint.Z) <= BoxExtent.Z);
}

float AArch_StoneArchway::GetWeatherProtectionAt(const FVector& Point) const
{
    if (!ArchwayConfig.bBlocksWeather)
    {
        return 0.0f;
    }

    float Distance = FVector::Dist(GetActorLocation(), Point);
    if (Distance > WeatherProtectionRadius)
    {
        return 0.0f;
    }

    // Calculate protection based on distance and whether point is inside
    float ProtectionStrength = 1.0f - (Distance / WeatherProtectionRadius);
    
    if (IsPointInsideArchway(Point))
    {
        ProtectionStrength = FMath::Max(ProtectionStrength, 0.8f);
    }

    return FMath::Clamp(ProtectionStrength, 0.0f, 1.0f);
}

void AArch_StoneArchway::SetupInteriorProps()
{
    // Clear existing prop locations
    InteriorPropLocations.Empty();

    // Add default interior prop locations based on archway type
    FVector BaseLocation = GetActorLocation();
    
    switch (ArchwayConfig.ArchwayType)
    {
        case EArch_ArchwayType::CaveEntrance:
            if (bHasFirePit)
            {
                InteriorPropLocations.Add(BaseLocation + FVector(0.0f, 0.0f, -50.0f));
            }
            if (bHasSleepingArea)
            {
                InteriorPropLocations.Add(BaseLocation + FVector(-100.0f, 100.0f, -30.0f));
                InteriorPropLocations.Add(BaseLocation + FVector(-100.0f, -100.0f, -30.0f));
            }
            if (bHasStorageArea)
            {
                InteriorPropLocations.Add(BaseLocation + FVector(150.0f, 0.0f, 0.0f));
            }
            break;

        case EArch_ArchwayType::RuinedGateway:
            // Add scattered debris and broken stone props
            InteriorPropLocations.Add(BaseLocation + FVector(50.0f, 80.0f, -20.0f));
            InteriorPropLocations.Add(BaseLocation + FVector(-60.0f, -70.0f, -15.0f));
            break;

        case EArch_ArchwayType::NaturalArch:
            // Add natural elements like fallen rocks
            InteriorPropLocations.Add(BaseLocation + FVector(0.0f, 120.0f, -40.0f));
            break;

        case EArch_ArchwayType::CarvedPortal:
            // Add ceremonial or decorative elements
            if (bHasFirePit)
            {
                InteriorPropLocations.Add(BaseLocation + FVector(0.0f, 0.0f, -30.0f));
            }
            break;
    }
}

void AArch_StoneArchway::AddInteriorProp(const FVector& Location, const FString& PropType)
{
    InteriorPropLocations.Add(Location);
    
    // Log prop addition for debugging
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue,
            FString::Printf(TEXT("Added interior prop: %s at %s"), *PropType, *Location.ToString()));
    }
}

void AArch_StoneArchway::OnEntranceTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (IsValid(OtherActor) && !ActorsInside.Contains(OtherActor))
    {
        ActorsInside.Add(OtherActor);
        
        // Trigger Blueprint event for player
        if (OtherActor->IsA<APawn>())
        {
            OnPlayerEnterArchway(Cast<APawn>(OtherActor));
            
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green,
                    FString::Printf(TEXT("Player entered archway: %s"), *GetName()));
            }
        }
    }
}

void AArch_StoneArchway::OnEntranceTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (IsValid(OtherActor) && ActorsInside.Contains(OtherActor))
    {
        ActorsInside.Remove(OtherActor);
        
        // Trigger Blueprint event for player
        if (OtherActor->IsA<APawn>())
        {
            OnPlayerExitArchway(Cast<APawn>(OtherActor));
            
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow,
                    FString::Printf(TEXT("Player exited archway: %s"), *GetName()));
            }
        }
    }
}

void AArch_StoneArchway::UpdateArchwayMesh()
{
    if (!ArchwayMesh)
    {
        return;
    }

    // Update mesh scale based on configuration
    FVector NewScale = FVector(
        ArchwayConfig.Depth / 200.0f,
        ArchwayConfig.Width / 400.0f,
        ArchwayConfig.Height / 600.0f
    );
    
    ArchwayMesh->SetWorldScale3D(NewScale);
}

void AArch_StoneArchway::UpdateInteriorVolume()
{
    if (!InteriorVolume)
    {
        return;
    }

    // Update interior volume size based on archway configuration
    FVector NewExtent = FVector(
        ArchwayConfig.Depth * 0.8f,
        ArchwayConfig.Width * 0.8f,
        ArchwayConfig.Height * 0.9f
    );
    
    InteriorVolume->SetBoxExtent(NewExtent);
    
    // Update entrance trigger to be slightly larger
    if (EntranceTrigger)
    {
        EntranceTrigger->SetBoxExtent(NewExtent * 1.2f);
    }
}

void AArch_StoneArchway::ApplyWeatherEffects()
{
    // This would integrate with weather and particle systems
    // For now, we'll set up basic ambient effects
    
    if (ArchwayConfig.ArchwayType == EArch_ArchwayType::CaveEntrance)
    {
        // Cave entrances might have water dripping or cool air effects
        // This would be implemented with Niagara particles
    }
    else if (ArchwayConfig.ArchwayType == EArch_ArchwayType::RuinedGateway)
    {
        // Ruined gateways might have dust or debris particles
    }
}