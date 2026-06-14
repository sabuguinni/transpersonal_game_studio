#include "Narr_StorytellingSystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"

ANarr_StorytellingSystem::ANarr_StorytellingSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create campfire mesh component
    CampfireMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CampfireMesh"));
    RootComponent = CampfireMesh;

    // Create fire particle effect
    FireEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("FireEffect"));
    FireEffect->SetupAttachment(RootComponent);

    // Create ambient audio component
    AmbientAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    AmbientAudio->SetupAttachment(RootComponent);

    // Create interaction sphere
    InteractionRadius = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionRadius"));
    InteractionRadius->SetupAttachment(RootComponent);
    InteractionRadius->SetSphereRadius(500.0f);

    // Initialize defaults
    StorytellingRange = 500.0f;
    MaxParticipants = 8;
    CurrentStoryStartTime = 0.0f;
    bIsPlayingStory = false;
}

void ANarr_StorytellingSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeStoryDatabase();
    LoadTribalStories();
}

void ANarr_StorytellingSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (CurrentSession.bIsActive)
    {
        UpdateSessionState();
    }
}

void ANarr_StorytellingSystem::StartStorytellingSession()
{
    if (CurrentSession.bIsActive)
    {
        return;
    }

    CurrentSession.bIsActive = true;
    CurrentSession.SessionStartTime = GetWorld()->GetTimeSeconds();
    CurrentSession.CurrentStoryIndex = 0;

    // Start fire effects
    if (FireEffect)
    {
        FireEffect->Activate();
    }

    // Start ambient audio
    if (AmbientAudio)
    {
        AmbientAudio->Play();
    }

    UE_LOG(LogTemp, Warning, TEXT("Storytelling session started at campfire"));
}

void ANarr_StorytellingSystem::EndStorytellingSession()
{
    CurrentSession.bIsActive = false;
    bIsPlayingStory = false;

    // Stop effects
    if (FireEffect)
    {
        FireEffect->Deactivate();
    }

    if (AmbientAudio)
    {
        AmbientAudio->Stop();
    }

    // Clear participants
    SessionParticipants.Empty();

    UE_LOG(LogTemp, Warning, TEXT("Storytelling session ended"));
}

void ANarr_StorytellingSystem::PlayNextStory()
{
    if (!CurrentSession.bIsActive || CurrentSession.AvailableStories.Num() == 0)
    {
        return;
    }

    if (CurrentSession.CurrentStoryIndex >= CurrentSession.AvailableStories.Num())
    {
        CurrentSession.CurrentStoryIndex = 0;
    }

    FNarr_StoryData CurrentStory = CurrentSession.AvailableStories[CurrentSession.CurrentStoryIndex];
    
    // Play story audio if available
    if (!CurrentStory.AudioURL.IsEmpty())
    {
        PlayStoryAudio(CurrentStory.AudioURL);
    }

    // Log story for participants
    for (APawn* Participant : SessionParticipants)
    {
        if (Participant)
        {
            UE_LOG(LogTemp, Warning, TEXT("Playing story '%s' for participant"), *CurrentStory.StoryTitle);
        }
    }

    CurrentStoryStartTime = GetWorld()->GetTimeSeconds();
    bIsPlayingStory = true;
    CurrentSession.CurrentStoryIndex++;
}

bool ANarr_StorytellingSystem::CanPlayerJoinSession(APawn* Player)
{
    if (!Player || !CurrentSession.bIsActive)
    {
        return false;
    }

    if (SessionParticipants.Num() >= MaxParticipants)
    {
        return false;
    }

    return IsPlayerInRange(Player);
}

void ANarr_StorytellingSystem::AddPlayerToSession(APawn* Player)
{
    if (!CanPlayerJoinSession(Player))
    {
        return;
    }

    if (!SessionParticipants.Contains(Player))
    {
        SessionParticipants.Add(Player);
        UE_LOG(LogTemp, Warning, TEXT("Player joined storytelling session"));
    }
}

void ANarr_StorytellingSystem::RemovePlayerFromSession(APawn* Player)
{
    if (SessionParticipants.Contains(Player))
    {
        SessionParticipants.Remove(Player);
        UE_LOG(LogTemp, Warning, TEXT("Player left storytelling session"));
    }
}

void ANarr_StorytellingSystem::LoadTribalStories()
{
    TribalStories.Empty();

    // Load hardcoded tribal stories for prehistoric survival
    FNarr_StoryData Story1;
    Story1.StoryTitle = TEXT("The Great Hunt");
    Story1.StoryText = TEXT("The ancient hunters gather around the fire. Their eyes tell stories of survival, of battles won against great beasts. Tonight, we share knowledge that will keep our tribe alive another day.");
    Story1.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781464331637_TribalElder.mp3");
    Story1.Duration = 13.0f;
    Story1.TribeReputationRequired = 0;
    TribalStories.Add(Story1);

    FNarr_StoryData Story2;
    Story2.StoryTitle = TEXT("Warning of Danger");
    Story2.StoryText = TEXT("Danger approaches from the north! The great predators have caught our scent. Grab your spears and form a circle - we must protect the young ones!");
    Story2.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781464338681_TribalWarrior.mp3");
    Story2.Duration = 10.0f;
    Story2.TribeReputationRequired = 10;
    TribalStories.Add(Story2);

    FNarr_StoryData Story3;
    Story3.StoryTitle = TEXT("The Clay River");
    Story3.StoryText = TEXT("The river runs red with clay today. Good for making pots, but the animals will not drink. We must find another water source before the sun sets.");
    Story3.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781464343373_TribalCrafter.mp3");
    Story3.Duration = 10.0f;
    Story3.TribeReputationRequired = 5;
    TribalStories.Add(Story3);

    FNarr_StoryData Story4;
    Story4.StoryTitle = TEXT("Reading the Tracks");
    Story4.StoryText = TEXT("Listen carefully, young hunter. The tracks tell us everything - size of the beast, how long ago it passed, whether it was wounded. This knowledge means the difference between feast and famine.");
    Story4.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781464348715_TribalTracker.mp3");
    Story4.Duration = 13.0f;
    Story4.TribeReputationRequired = 15;
    TribalStories.Add(Story4);

    UE_LOG(LogTemp, Warning, TEXT("Loaded %d tribal stories"), TribalStories.Num());
}

FNarr_StoryData ANarr_StorytellingSystem::GetRandomStory()
{
    if (TribalStories.Num() == 0)
    {
        return FNarr_StoryData();
    }

    int32 RandomIndex = FMath::RandRange(0, TribalStories.Num() - 1);
    return TribalStories[RandomIndex];
}

TArray<FNarr_StoryData> ANarr_StorytellingSystem::GetStoriesForReputation(int32 ReputationLevel)
{
    TArray<FNarr_StoryData> AvailableStories;

    for (const FNarr_StoryData& Story : TribalStories)
    {
        if (ReputationLevel >= Story.TribeReputationRequired)
        {
            AvailableStories.Add(Story);
        }
    }

    return AvailableStories;
}

void ANarr_StorytellingSystem::PlayStoryAudio(const FString& AudioURL)
{
    // In a full implementation, this would load and play audio from URL
    // For now, we log the audio playback
    UE_LOG(LogTemp, Warning, TEXT("Playing story audio: %s"), *AudioURL);
    
    if (AmbientAudio)
    {
        // Audio component would be configured to play the story audio
        AmbientAudio->Play();
    }
}

void ANarr_StorytellingSystem::StopCurrentAudio()
{
    if (AmbientAudio && AmbientAudio->IsPlaying())
    {
        AmbientAudio->Stop();
    }
    
    bIsPlayingStory = false;
}

void ANarr_StorytellingSystem::InitializeStoryDatabase()
{
    // Initialize session data
    CurrentSession.AvailableStories.Empty();
    CurrentSession.CurrentStoryIndex = 0;
    CurrentSession.bIsActive = false;
    CurrentSession.SessionStartTime = 0.0f;
}

void ANarr_StorytellingSystem::UpdateSessionState()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Check if current story has finished
    if (bIsPlayingStory && CurrentSession.AvailableStories.IsValidIndex(CurrentSession.CurrentStoryIndex - 1))
    {
        FNarr_StoryData CurrentStory = CurrentSession.AvailableStories[CurrentSession.CurrentStoryIndex - 1];
        if (CurrentTime - CurrentStoryStartTime >= CurrentStory.Duration)
        {
            bIsPlayingStory = false;
            StopCurrentAudio();
        }
    }

    // Remove players who are out of range
    for (int32 i = SessionParticipants.Num() - 1; i >= 0; i--)
    {
        if (!IsPlayerInRange(SessionParticipants[i]))
        {
            RemovePlayerFromSession(SessionParticipants[i]);
        }
    }

    // End session if no participants
    if (SessionParticipants.Num() == 0 && CurrentTime - CurrentSession.SessionStartTime > 30.0f)
    {
        EndStorytellingSession();
    }
}

bool ANarr_StorytellingSystem::IsPlayerInRange(APawn* Player)
{
    if (!Player)
    {
        return false;
    }

    float Distance = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
    return Distance <= StorytellingRange;
}