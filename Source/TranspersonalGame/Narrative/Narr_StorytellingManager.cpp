#include "Narr_StorytellingManager.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ANarr_StorytellingManager::ANarr_StorytellingManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create trigger sphere
    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->SetupAttachment(RootComponent);
    TriggerSphere->SetSphereRadius(500.0f);
    TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerSphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    TriggerSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    TriggerSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    // Create audio component
    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    AudioComponent->SetupAttachment(RootComponent);
    AudioComponent->bAutoActivate = false;

    // Initialize default values
    LastTriggerTime = -1000.0f;
    bIsPlaying = false;
    bRandomizeStories = true;
    MinTimeBetweenStories = 30.0f;

    // Set default story data
    StoryData.StoryType = ENarr_StoryType::CampGathering;
    StoryData.NarrativeText = TEXT("The flames dance high tonight, casting shadows that tell ancient tales of survival.");
    StoryData.TriggerRadius = 500.0f;
    StoryData.CooldownTime = 30.0f;
    StoryData.bRequiresLineOfSight = false;
}

void ANarr_StorytellingManager::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    if (TriggerSphere)
    {
        TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &ANarr_StorytellingManager::OnTriggerBeginOverlap);
        TriggerSphere->OnComponentEndOverlap.AddDynamic(this, &ANarr_StorytellingManager::OnTriggerEndOverlap);
        TriggerSphere->SetSphereRadius(StoryData.TriggerRadius);
    }

    // Bind audio finished event
    if (AudioComponent)
    {
        AudioComponent->OnAudioFinished.AddDynamic(this, &ANarr_StorytellingManager::OnAudioFinished);
    }
}

void ANarr_StorytellingManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update audio component position for 3D audio
    if (AudioComponent && bIsPlaying)
    {
        AudioComponent->SetWorldLocation(GetActorLocation());
    }
}

void ANarr_StorytellingManager::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    APawn* TriggeringPawn = Cast<APawn>(OtherActor);
    if (TriggeringPawn && TriggeringPawn->IsPlayerControlled())
    {
        TriggerStory(TriggeringPawn);
    }
}

void ANarr_StorytellingManager::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    // Optional: Stop story when player leaves area
    // StopCurrentStory();
}

void ANarr_StorytellingManager::TriggerStory(APawn* TriggeringPawn)
{
    if (!CanTriggerStory() || !TriggeringPawn)
    {
        return;
    }

    // Check line of sight if required
    FNarr_StoryData CurrentStory = bRandomizeStories ? GetRandomStory() : StoryData;
    if (CurrentStory.bRequiresLineOfSight && !HasLineOfSight(TriggeringPawn))
    {
        return;
    }

    // Update trigger time
    LastTriggerTime = GetWorld()->GetTimeSeconds();
    bIsPlaying = true;

    // Play audio if available
    if (AudioComponent && CurrentStory.AudioCue.LoadSynchronous())
    {
        AudioComponent->SetSound(CurrentStory.AudioCue.Get());
        AudioComponent->Play();
    }

    // Trigger Blueprint event
    OnStoryTriggered(CurrentStory, TriggeringPawn);

    // Log for debugging
    UE_LOG(LogTemp, Log, TEXT("Story triggered: %s"), *CurrentStory.NarrativeText);
}

void ANarr_StorytellingManager::StopCurrentStory()
{
    if (AudioComponent && AudioComponent->IsPlaying())
    {
        AudioComponent->Stop();
    }
    
    bIsPlaying = false;
    OnAudioFinished();
}

bool ANarr_StorytellingManager::CanTriggerStory() const
{
    if (bIsPlaying)
    {
        return false;
    }

    float CurrentTime = GetWorld()->GetTimeSeconds();
    float TimeSinceLastTrigger = CurrentTime - LastTriggerTime;
    
    return TimeSinceLastTrigger >= MinTimeBetweenStories;
}

FNarr_StoryData ANarr_StorytellingManager::GetRandomStory() const
{
    if (AdditionalStories.Num() > 0)
    {
        TArray<FNarr_StoryData> AllStories = AdditionalStories;
        AllStories.Add(StoryData);
        
        int32 RandomIndex = FMath::RandRange(0, AllStories.Num() - 1);
        return AllStories[RandomIndex];
    }
    
    return StoryData;
}

void ANarr_StorytellingManager::SetStoryData(const FNarr_StoryData& NewStoryData)
{
    StoryData = NewStoryData;
    
    // Update trigger sphere radius
    if (TriggerSphere)
    {
        TriggerSphere->SetSphereRadius(StoryData.TriggerRadius);
    }
}

void ANarr_StorytellingManager::OnAudioFinished()
{
    bIsPlaying = false;
    OnStoryCompleted(StoryData);
}

bool ANarr_StorytellingManager::HasLineOfSight(APawn* TargetPawn) const
{
    if (!TargetPawn || !GetWorld())
    {
        return false;
    }

    FVector StartLocation = GetActorLocation();
    FVector EndLocation = TargetPawn->GetActorLocation();
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    QueryParams.AddIgnoredActor(TargetPawn);
    QueryParams.bTraceComplex = false;
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );
    
    return !bHit; // No obstruction means clear line of sight
}