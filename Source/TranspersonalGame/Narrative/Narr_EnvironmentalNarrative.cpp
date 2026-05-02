#include "Narr_EnvironmentalNarrative.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "GameFramework/Character.h"

ANarr_EnvironmentalNarrative::ANarr_EnvironmentalNarrative()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create trigger sphere
    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->SetupAttachment(RootComponent);
    TriggerSphere->SetSphereRadius(500.0f);
    TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerSphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
    TriggerSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    TriggerSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    // Create visual mesh component
    VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
    VisualMesh->SetupAttachment(RootComponent);
    VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Initialize default values
    bIsActive = true;
    bHasBeenTriggered = false;
    NarrativeDisplayDuration = 5.0f;
    CurrentDisplayTime = 0.0f;
    bIsDisplayingNarrative = false;

    // Set default environmental data
    EnvironmentalData.EnvironmentType = ENarr_EnvironmentalType::BoneDiscovery;
    EnvironmentalData.NarrativeText = FText::FromString("Ancient bones lie scattered here, telling a story of violence and survival.");
    EnvironmentalData.PlayerThought = FText::FromString("These bones are massive... what kind of creature was this?");
    EnvironmentalData.TriggerRadius = 500.0f;
    EnvironmentalData.bOneTimeOnly = true;
    EnvironmentalData.FearImpact = 5.0f;
    EnvironmentalData.KnowledgeGain = 15.0f;
}

void ANarr_EnvironmentalNarrative::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    if (TriggerSphere)
    {
        TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &ANarr_EnvironmentalNarrative::OnTriggerBeginOverlap);
        TriggerSphere->SetSphereRadius(EnvironmentalData.TriggerRadius);
    }

    // Configure visual based on environmental type
    if (VisualMesh)
    {
        switch (EnvironmentalData.EnvironmentType)
        {
            case ENarr_EnvironmentalType::BoneDiscovery:
                VisualMesh->SetVisibility(true);
                break;
            case ENarr_EnvironmentalType::DangerZone:
                VisualMesh->SetVisibility(false); // Invisible danger zones
                break;
            case ENarr_EnvironmentalType::WaterSource:
                VisualMesh->SetVisibility(true);
                break;
            default:
                VisualMesh->SetVisibility(true);
                break;
        }
    }
}

void ANarr_EnvironmentalNarrative::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Handle narrative display timing
    if (bIsDisplayingNarrative)
    {
        CurrentDisplayTime += DeltaTime;
        if (CurrentDisplayTime >= NarrativeDisplayDuration)
        {
            bIsDisplayingNarrative = false;
            CurrentDisplayTime = 0.0f;
        }
    }
}

void ANarr_EnvironmentalNarrative::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!bIsActive || (bHasBeenTriggered && EnvironmentalData.bOneTimeOnly))
    {
        return;
    }

    // Check if the overlapping actor is a player character
    ACharacter* PlayerCharacter = Cast<ACharacter>(OtherActor);
    if (!PlayerCharacter)
    {
        return;
    }

    // Check if this is the player-controlled character
    if (!PlayerCharacter->IsPlayerControlled())
    {
        return;
    }

    TriggerEnvironmentalNarrative(OtherActor);
}

void ANarr_EnvironmentalNarrative::TriggerEnvironmentalNarrative(AActor* TriggeringActor)
{
    if (!bIsActive || (bHasBeenTriggered && EnvironmentalData.bOneTimeOnly))
    {
        return;
    }

    bHasBeenTriggered = true;
    bIsDisplayingNarrative = true;
    CurrentDisplayTime = 0.0f;

    // Play environmental sound
    PlayEnvironmentalSound();

    // Update player stats
    UpdatePlayerStats(TriggeringActor);

    // Trigger Blueprint events
    OnEnvironmentalNarrativeTriggered(EnvironmentalData);
    OnPlayerDiscovery(EnvironmentalData.EnvironmentType, EnvironmentalData.KnowledgeGain);

    // Log the narrative event
    UE_LOG(LogTemp, Warning, TEXT("Environmental Narrative Triggered: %s"), *EnvironmentalData.NarrativeText.ToString());

    // Display narrative text on screen
    if (GEngine)
    {
        FString DisplayText = FString::Printf(TEXT("DISCOVERY: %s"), *EnvironmentalData.NarrativeText.ToString());
        GEngine->AddOnScreenDebugMessage(-1, NarrativeDisplayDuration, FColor::Yellow, DisplayText);

        FString ThoughtText = FString::Printf(TEXT("THOUGHT: %s"), *EnvironmentalData.PlayerThought.ToString());
        GEngine->AddOnScreenDebugMessage(-1, NarrativeDisplayDuration, FColor::Cyan, ThoughtText);
    }
}

void ANarr_EnvironmentalNarrative::PlayEnvironmentalSound()
{
    if (EnvironmentalData.AmbientSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            EnvironmentalData.AmbientSound,
            GetActorLocation(),
            1.0f,
            1.0f,
            0.0f
        );
    }
}

void ANarr_EnvironmentalNarrative::UpdatePlayerStats(AActor* Player)
{
    if (!Player)
    {
        return;
    }

    // Try to find and update player stats
    // This would typically interact with the player's survival stats component
    // For now, we'll log the intended changes
    UE_LOG(LogTemp, Log, TEXT("Player Stats Update - Fear: %f, Knowledge: %f"), 
           EnvironmentalData.FearImpact, EnvironmentalData.KnowledgeGain);
}

void ANarr_EnvironmentalNarrative::SetEnvironmentalData(const FNarr_EnvironmentalData& NewData)
{
    EnvironmentalData = NewData;
    
    // Update trigger radius
    if (TriggerSphere)
    {
        TriggerSphere->SetSphereRadius(EnvironmentalData.TriggerRadius);
    }
}

FNarr_EnvironmentalData ANarr_EnvironmentalNarrative::GetEnvironmentalData() const
{
    return EnvironmentalData;
}

void ANarr_EnvironmentalNarrative::ActivateNarrative()
{
    bIsActive = true;
    if (TriggerSphere)
    {
        TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    }
}

void ANarr_EnvironmentalNarrative::DeactivateNarrative()
{
    bIsActive = false;
    if (TriggerSphere)
    {
        TriggerSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}

bool ANarr_EnvironmentalNarrative::IsNarrativeActive() const
{
    return bIsActive;
}