#include "Arch_ShelterEntranceActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"

AArch_ShelterEntranceActor::AArch_ShelterEntranceActor()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create entrance mesh component
    EntranceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EntranceMesh"));
    EntranceMesh->SetupAttachment(RootComponent);
    EntranceMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    EntranceMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);

    // Create interaction volume
    InteractionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionVolume"));
    InteractionVolume->SetupAttachment(RootComponent);
    InteractionVolume->SetBoxExtent(FVector(ShelterRadius, ShelterRadius, 300.0f));
    InteractionVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionVolume->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
    InteractionVolume->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    InteractionVolume->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    // Create structure system component
    StructureSystem = CreateDefaultSubobject<UArch_PrimitiveStructureSystem>(TEXT("StructureSystem"));

    // Set default values
    ShelterRadius = 500.0f;
    TemperatureBonus = 10.0f;
    bProvidesWindProtection = true;
    bProvidesRainProtection = true;
    bShowDebugRadius = false;

    // Bind overlap events
    InteractionVolume->OnComponentBeginOverlap.AddDynamic(this, &AArch_ShelterEntranceActor::OnInteractionVolumeBeginOverlap);
    InteractionVolume->OnComponentEndOverlap.AddDynamic(this, &AArch_ShelterEntranceActor::OnInteractionVolumeEndOverlap);
}

void AArch_ShelterEntranceActor::BeginPlay()
{
    Super::BeginPlay();

    // Initialize structure system with cave entrance type
    if (StructureSystem)
    {
        StructureSystem->InitializeStructure(EArch_StructureType::CaveEntrance, EArch_MaterialType::Stone);
    }

    // Update interaction volume size based on shelter radius
    if (InteractionVolume)
    {
        InteractionVolume->SetBoxExtent(FVector(ShelterRadius, ShelterRadius, 300.0f));
    }

    UE_LOG(LogTemp, Log, TEXT("Architecture: Shelter entrance initialized at %s"), *GetActorLocation().ToString());
}

void AArch_ShelterEntranceActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateShelterEffects();

    if (bShowDebugRadius)
    {
        DrawDebugShelterRadius();
    }
}

bool AArch_ShelterEntranceActor::IsPlayerInShelter(AActor* PlayerActor) const
{
    if (!PlayerActor)
    {
        return false;
    }

    float Distance = FVector::Dist(GetActorLocation(), PlayerActor->GetActorLocation());
    return Distance <= ShelterRadius && ActorsInShelter.Contains(PlayerActor);
}

float AArch_ShelterEntranceActor::GetShelterEffectiveness() const
{
    if (!StructureSystem)
    {
        return 0.0f;
    }

    // Base effectiveness on structural integrity
    float BaseEffectiveness = StructureSystem->StructureData.StructuralIntegrity / 100.0f;
    
    // Weather resistance bonus
    float WeatherBonus = StructureSystem->StructureData.WeatherResistance / 200.0f;
    
    return FMath::Clamp(BaseEffectiveness + WeatherBonus, 0.0f, 1.0f);
}

FVector AArch_ShelterEntranceActor::GetOptimalShelterPosition() const
{
    // Return position slightly inside the shelter entrance
    FVector EntranceLocation = GetActorLocation();
    FVector EntranceForward = GetActorForwardVector();
    
    return EntranceLocation + (EntranceForward * -200.0f) + FVector(0.0f, 0.0f, 50.0f);
}

void AArch_ShelterEntranceActor::OnInteractionVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || ActorsInShelter.Contains(OtherActor))
    {
        return;
    }

    // Check if it's a character (player or NPC)
    ACharacter* Character = Cast<ACharacter>(OtherActor);
    if (Character)
    {
        ActorsInShelter.Add(OtherActor);
        
        UE_LOG(LogTemp, Log, TEXT("Architecture: %s entered shelter at %s"), 
               *OtherActor->GetName(), *GetActorLocation().ToString());
        
        // Call Blueprint event
        OnPlayerEnterShelter(OtherActor);
    }
}

void AArch_ShelterEntranceActor::OnInteractionVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor)
    {
        return;
    }

    if (ActorsInShelter.Remove(OtherActor) > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Architecture: %s exited shelter at %s"), 
               *OtherActor->GetName(), *GetActorLocation().ToString());
        
        // Call Blueprint event
        OnPlayerExitShelter(OtherActor);
    }
}

void AArch_ShelterEntranceActor::UpdateShelterEffects()
{
    if (!StructureSystem || !StructureSystem->IsStructureStable())
    {
        return;
    }

    float Effectiveness = GetShelterEffectiveness();
    
    // Apply shelter effects to actors inside
    for (AActor* Actor : ActorsInShelter)
    {
        if (!IsValid(Actor))
        {
            continue;
        }

        // Here you would apply temperature bonus, weather protection, etc.
        // This would integrate with the character's survival system
        
        // Example: Apply temperature bonus (would need character survival component)
        // if (UCharacterSurvivalComponent* SurvivalComp = Actor->FindComponentByClass<UCharacterSurvivalComponent>())
        // {
        //     SurvivalComp->ApplyTemperatureModifier(TemperatureBonus * Effectiveness);
        // }
    }
}

void AArch_ShelterEntranceActor::DrawDebugShelterRadius()
{
    if (GetWorld())
    {
        FVector Center = GetActorLocation();
        FColor DebugColor = StructureSystem && StructureSystem->IsStructureStable() ? FColor::Green : FColor::Red;
        
        // Draw shelter radius
        DrawDebugSphere(GetWorld(), Center, ShelterRadius, 32, DebugColor, false, -1.0f, 0, 2.0f);
        
        // Draw optimal shelter position
        FVector OptimalPos = GetOptimalShelterPosition();
        DrawDebugSphere(GetWorld(), OptimalPos, 50.0f, 16, FColor::Blue, false, -1.0f, 0, 3.0f);
        
        // Draw connection line
        DrawDebugLine(GetWorld(), Center, OptimalPos, FColor::Yellow, false, -1.0f, 0, 1.0f);
    }
}