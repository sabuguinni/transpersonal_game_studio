#include "Narr_EnvironmentalNarrator.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ANarr_EnvironmentalNarrator::ANarr_EnvironmentalNarrator()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create audio component
    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    AudioComponent->SetupAttachment(RootComponent);
    AudioComponent->bAutoActivate = false;

    // Initialize default values
    CheckInterval = 5.0f;
    PlayerDetectionRadius = 2000.0f;
    LastNarrationTime = -1000.0f;
    bIsNarrating = false;
    CurrentEnvironment = ENarr_EnvironmentType::Forest;

    // Setup default environmental cues
    FNarr_EnvironmentalCue ForestCue;
    ForestCue.EnvironmentType = ENarr_EnvironmentType::Forest;
    ForestCue.NarrativeText = TEXT("Ancient trees tower overhead, their branches whispering tales of countless seasons.");
    ForestCue.TriggerChance = 0.25f;
    ForestCue.MinTimeBetweenTriggers = 90.0f;
    EnvironmentalCues.Add(ForestCue);

    FNarr_EnvironmentalCue RiverCue;
    RiverCue.EnvironmentType = ENarr_EnvironmentType::River;
    RiverCue.NarrativeText = TEXT("The rushing waters carry the scent of distant lands and the promise of life.");
    RiverCue.TriggerChance = 0.4f;
    RiverCue.MinTimeBetweenTriggers = 60.0f;
    EnvironmentalCues.Add(RiverCue);

    FNarr_EnvironmentalCue CaveCue;
    CaveCue.EnvironmentType = ENarr_EnvironmentType::Cave;
    CaveCue.NarrativeText = TEXT("Darkness embraces you here, where echoes of the past linger in stone chambers.");
    CaveCue.TriggerChance = 0.5f;
    CaveCue.MinTimeBetweenTriggers = 120.0f;
    EnvironmentalCues.Add(CaveCue);

    FNarr_EnvironmentalCue PlainsCue;
    PlainsCue.EnvironmentType = ENarr_EnvironmentType::Plains;
    PlainsCue.NarrativeText = TEXT("Endless grasslands stretch to the horizon, where great herds once roamed freely.");
    PlainsCue.TriggerChance = 0.2f;
    PlainsCue.MinTimeBetweenTriggers = 80.0f;
    EnvironmentalCues.Add(PlainsCue);
}

void ANarr_EnvironmentalNarrator::BeginPlay()
{
    Super::BeginPlay();

    // Bind audio finished event
    if (AudioComponent)
    {
        AudioComponent->OnAudioFinished.AddDynamic(this, &ANarr_EnvironmentalNarrator::OnNarrationFinished);
    }

    // Start periodic environment checking
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            CheckTimer,
            this,
            &ANarr_EnvironmentalNarrator::CheckForNarrationTrigger,
            CheckInterval,
            true
        );
    }
}

void ANarr_EnvironmentalNarrator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update current environment detection
    CurrentEnvironment = DetectCurrentEnvironment();
}

void ANarr_EnvironmentalNarrator::CheckForNarrationTrigger()
{
    if (bIsNarrating || !IsPlayerNearby())
    {
        return;
    }

    // Get cue for current environment
    FNarr_EnvironmentalCue CurrentCue = GetCueForEnvironment(CurrentEnvironment);
    
    if (CanTriggerNarration(CurrentCue))
    {
        // Random chance to trigger
        float RandomValue = FMath::RandRange(0.0f, 1.0f);
        if (RandomValue <= CurrentCue.TriggerChance)
        {
            TriggerEnvironmentalNarration(CurrentEnvironment);
        }
    }
}

void ANarr_EnvironmentalNarrator::TriggerEnvironmentalNarration(ENarr_EnvironmentType EnvironmentType)
{
    FNarr_EnvironmentalCue Cue = GetCueForEnvironment(EnvironmentType);
    
    if (!CanTriggerNarration(Cue))
    {
        return;
    }

    // Update state
    LastNarrationTime = GetWorld()->GetTimeSeconds();
    bIsNarrating = true;

    // Play audio if available
    if (AudioComponent && Cue.AudioCue.LoadSynchronous())
    {
        AudioComponent->SetSound(Cue.AudioCue.Get());
        AudioComponent->Play();
    }

    // Trigger Blueprint event
    OnEnvironmentalNarrationTriggered(Cue);

    // Log for debugging
    UE_LOG(LogTemp, Log, TEXT("Environmental narration triggered: %s"), *Cue.NarrativeText);
}

void ANarr_EnvironmentalNarrator::StopCurrentNarration()
{
    if (AudioComponent && AudioComponent->IsPlaying())
    {
        AudioComponent->Stop();
    }
    
    bIsNarrating = false;
    OnNarrationFinished();
}

bool ANarr_EnvironmentalNarrator::CanTriggerNarration(const FNarr_EnvironmentalCue& Cue) const
{
    if (bIsNarrating)
    {
        return false;
    }

    float CurrentTime = GetWorld()->GetTimeSeconds();
    float TimeSinceLastNarration = CurrentTime - LastNarrationTime;
    
    return TimeSinceLastNarration >= Cue.MinTimeBetweenTriggers;
}

ENarr_EnvironmentType ANarr_EnvironmentalNarrator::DetectCurrentEnvironment() const
{
    // Simple environment detection based on actor location and world analysis
    FVector ActorLocation = GetActorLocation();
    
    // Check height for mountain/cave detection
    if (ActorLocation.Z > 1000.0f)
    {
        return ENarr_EnvironmentType::Mountain;
    }
    else if (ActorLocation.Z < -200.0f)
    {
        return ENarr_EnvironmentType::Cave;
    }
    
    // Use simple heuristics based on location
    // In a real implementation, this would query the biome system
    float X = ActorLocation.X;
    float Y = ActorLocation.Y;
    
    if (FMath::Abs(X) < 1000.0f && FMath::Abs(Y) < 1000.0f)
    {
        return ENarr_EnvironmentType::Plains; // Central area
    }
    else if (X > 2000.0f || Y > 2000.0f)
    {
        return ENarr_EnvironmentType::Forest; // Outer areas
    }
    else
    {
        return ENarr_EnvironmentType::River; // Mid areas
    }
}

bool ANarr_EnvironmentalNarrator::IsPlayerNearby() const
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return false;
    }

    float DistanceToPlayer = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
    return DistanceToPlayer <= PlayerDetectionRadius;
}

FNarr_EnvironmentalCue ANarr_EnvironmentalNarrator::GetCueForEnvironment(ENarr_EnvironmentType EnvironmentType) const
{
    for (const FNarr_EnvironmentalCue& Cue : EnvironmentalCues)
    {
        if (Cue.EnvironmentType == EnvironmentType)
        {
            return Cue;
        }
    }

    // Return default cue if not found
    if (EnvironmentalCues.Num() > 0)
    {
        return EnvironmentalCues[0];
    }

    return FNarr_EnvironmentalCue();
}

void ANarr_EnvironmentalNarrator::OnNarrationFinished()
{
    bIsNarrating = false;
    
    FNarr_EnvironmentalCue CompletedCue = GetCueForEnvironment(CurrentEnvironment);
    OnEnvironmentalNarrationCompleted(CompletedCue);
}