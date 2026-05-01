#include "Narr_SurvivalNarrator.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

ANarr_SurvivalNarrator::ANarr_SurvivalNarrator()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create trigger sphere
    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->SetupAttachment(RootComponent);
    TriggerSphere->SetSphereRadius(500.0f);
    TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Create audio component
    NarrativeAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("NarrativeAudioComponent"));
    NarrativeAudioComponent->SetupAttachment(RootComponent);
    NarrativeAudioComponent->bAutoActivate = false;

    // Initialize default values
    PlayerDetectionRadius = 500.0f;
    NarrativeCooldown = 10.0f;
    VolumeMultiplier = 1.0f;
    PitchMultiplier = 1.0f;
    bIsNarrativePlaying = false;
    LastNarrativeTime = 0.0f;
    CachedPlayerPawn = nullptr;

    // Setup default narrative events
    FNarr_NarrativeEvent EnvironmentalEvent;
    EnvironmentalEvent.EventID = TEXT("ENV_001");
    EnvironmentalEvent.NarrativeText = FText::FromString(TEXT("The ground trembles beneath your feet. Something massive moves through the forest..."));
    EnvironmentalEvent.NarrationType = ENarr_NarrationType::Environmental;
    EnvironmentalEvent.TriggerRadius = 400.0f;
    EnvironmentalEvent.bIsOneShot = true;
    EnvironmentalEvent.Priority = 1.0f;
    NarrativeEvents.Add(EnvironmentalEvent);

    FNarr_NarrativeEvent DangerEvent;
    DangerEvent.EventID = TEXT("DANGER_001");
    DangerEvent.NarrativeText = FText::FromString(TEXT("The silence is unnatural here. Predators hunt in such stillness."));
    DangerEvent.NarrationType = ENarr_NarrationType::Danger;
    DangerEvent.TriggerRadius = 300.0f;
    DangerEvent.bIsOneShot = false;
    DangerEvent.Priority = 2.0f;
    NarrativeEvents.Add(DangerEvent);

    FNarr_NarrativeEvent SurvivalEvent;
    SurvivalEvent.EventID = TEXT("SURVIVAL_001");
    SurvivalEvent.NarrativeText = FText::FromString(TEXT("Water means life, but it also draws the hungry. Stay alert."));
    SurvivalEvent.NarrationType = ENarr_NarrationType::Survival;
    SurvivalEvent.TriggerRadius = 600.0f;
    SurvivalEvent.bIsOneShot = true;
    SurvivalEvent.Priority = 1.5f;
    NarrativeEvents.Add(SurvivalEvent);
}

void ANarr_SurvivalNarrator::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    if (TriggerSphere)
    {
        TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &ANarr_SurvivalNarrator::OnTriggerOverlapBegin);
        TriggerSphere->OnComponentEndOverlap.AddDynamic(this, &ANarr_SurvivalNarrator::OnTriggerOverlapEnd);
    }

    // Cache player pawn
    CachedPlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
}

void ANarr_SurvivalNarrator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    CheckPlayerProximity();
    ProcessNarrativeQueue();
}

void ANarr_SurvivalNarrator::TriggerNarrative(const FString& EventID)
{
    if (!CanTriggerNarrative())
    {
        return;
    }

    FNarr_NarrativeEvent Event = GetNarrativeEventByID(EventID);
    if (!Event.EventID.IsEmpty())
    {
        PlayNarrativeEvent(Event);
    }
}

void ANarr_SurvivalNarrator::PlayNarrativeEvent(const FNarr_NarrativeEvent& Event)
{
    if (bIsNarrativePlaying)
    {
        return;
    }

    // Check if this is a one-shot event that has already been triggered
    if (Event.bIsOneShot && TriggeredEvents.Contains(Event.EventID))
    {
        return;
    }

    bIsNarrativePlaying = true;
    LastNarrativeTime = GetWorld()->GetTimeSeconds();

    // Play audio if available
    if (NarrativeAudioComponent && DefaultNarrativeCue)
    {
        NarrativeAudioComponent->SetSound(DefaultNarrativeCue);
        NarrativeAudioComponent->SetVolumeMultiplier(VolumeMultiplier);
        NarrativeAudioComponent->SetPitchMultiplier(PitchMultiplier);
        NarrativeAudioComponent->Play();
    }

    // Add to triggered events if one-shot
    if (Event.bIsOneShot)
    {
        TriggeredEvents.AddUnique(Event.EventID);
    }

    // Trigger Blueprint event
    OnNarrativeTriggered(Event);

    // Log the narrative
    UE_LOG(LogTemp, Warning, TEXT("Narrative Triggered: %s - %s"), 
           *Event.EventID, 
           *Event.NarrativeText.ToString());

    // Set timer to mark narrative as complete
    FTimerHandle NarrativeTimer;
    GetWorld()->GetTimerManager().SetTimer(NarrativeTimer, [this, Event]()
    {
        bIsNarrativePlaying = false;
        OnNarrativeCompleted(Event);
    }, 5.0f, false);
}

bool ANarr_SurvivalNarrator::CanTriggerNarrative() const
{
    if (bIsNarrativePlaying)
    {
        return false;
    }

    float CurrentTime = GetWorld()->GetTimeSeconds();
    return (CurrentTime - LastNarrativeTime) >= NarrativeCooldown;
}

void ANarr_SurvivalNarrator::StopCurrentNarrative()
{
    if (NarrativeAudioComponent && NarrativeAudioComponent->IsPlaying())
    {
        NarrativeAudioComponent->Stop();
    }
    
    bIsNarrativePlaying = false;
}

FNarr_NarrativeEvent ANarr_SurvivalNarrator::GetNarrativeEventByID(const FString& EventID) const
{
    for (const FNarr_NarrativeEvent& Event : NarrativeEvents)
    {
        if (Event.EventID == EventID)
        {
            return Event;
        }
    }
    
    return FNarr_NarrativeEvent();
}

void ANarr_SurvivalNarrator::OnTriggerOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || !OtherActor->IsA<APawn>())
    {
        return;
    }

    APawn* PlayerPawn = Cast<APawn>(OtherActor);
    if (!PlayerPawn || !PlayerPawn->IsPlayerControlled())
    {
        return;
    }

    // Find the highest priority narrative event that can be triggered
    FNarr_NarrativeEvent BestEvent;
    float HighestPriority = 0.0f;

    for (const FNarr_NarrativeEvent& Event : NarrativeEvents)
    {
        if (Event.bIsOneShot && TriggeredEvents.Contains(Event.EventID))
        {
            continue;
        }

        if (Event.Priority > HighestPriority)
        {
            BestEvent = Event;
            HighestPriority = Event.Priority;
        }
    }

    if (!BestEvent.EventID.IsEmpty())
    {
        PendingNarratives.AddUnique(BestEvent);
    }
}

void ANarr_SurvivalNarrator::OnTriggerOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    // Clear pending narratives when player leaves the area
    PendingNarratives.Empty();
}

void ANarr_SurvivalNarrator::CheckPlayerProximity()
{
    if (!CachedPlayerPawn)
    {
        CachedPlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
        return;
    }

    float DistanceToPlayer = FVector::Dist(GetActorLocation(), CachedPlayerPawn->GetActorLocation());
    
    // Update trigger sphere radius based on detection radius
    if (TriggerSphere && TriggerSphere->GetScaledSphereRadius() != PlayerDetectionRadius)
    {
        TriggerSphere->SetSphereRadius(PlayerDetectionRadius);
    }
}

void ANarr_SurvivalNarrator::ProcessNarrativeQueue()
{
    if (PendingNarratives.Num() == 0 || !CanTriggerNarrative())
    {
        return;
    }

    // Sort by priority and play the highest priority narrative
    PendingNarratives.Sort([](const FNarr_NarrativeEvent& A, const FNarr_NarrativeEvent& B)
    {
        return A.Priority > B.Priority;
    });

    FNarr_NarrativeEvent EventToPlay = PendingNarratives[0];
    PendingNarratives.RemoveAt(0);

    PlayNarrativeEvent(EventToPlay);
}