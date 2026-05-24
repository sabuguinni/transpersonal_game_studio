#include "Audio_NarrativeIntegration.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UAudio_NarrativeIntegration::UAudio_NarrativeIntegration()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostUpdateWork;

    // Initialize default values
    bIsPlayingNarrative = false;
    CurrentContext = EAudio_NarrativeContext::Atmospheric;
    FadeInDuration = 1.0f;
    FadeOutDuration = 1.5f;
    SpatialRange = 2000.0f;

    // Fade state
    bIsFading = false;
    FadeTimer = 0.0f;
    FadeTargetDuration = 0.0f;
    FadingComponent = nullptr;
    bFadeIn = false;

    // Initialize audio components
    PrimaryAudioComponent = nullptr;
    SecondaryAudioComponent = nullptr;
}

void UAudio_NarrativeIntegration::BeginPlay()
{
    Super::BeginPlay();

    InitializeAudioComponents();
    LoadPredefinedNarrativeClips();

    UE_LOG(LogTemp, Log, TEXT("Audio_NarrativeIntegration: Component initialized with %d narrative clips"), NarrativeClips.Num());
}

void UAudio_NarrativeIntegration::InitializeAudioComponents()
{
    if (!GetOwner())
    {
        UE_LOG(LogTemp, Error, TEXT("Audio_NarrativeIntegration: No owner actor found"));
        return;
    }

    // Create primary audio component
    PrimaryAudioComponent = NewObject<UAudioComponent>(GetOwner());
    if (PrimaryAudioComponent)
    {
        PrimaryAudioComponent->SetupAttachment(GetOwner()->GetRootComponent());
        PrimaryAudioComponent->bAutoActivate = false;
        PrimaryAudioComponent->SetVolumeMultiplier(1.0f);
        PrimaryAudioComponent->SetPitchMultiplier(1.0f);
        PrimaryAudioComponent->bAllowSpatialization = true;
        PrimaryAudioComponent->AttenuationSettings = nullptr; // Use default 3D settings
        GetOwner()->AddInstanceComponent(PrimaryAudioComponent);
        PrimaryAudioComponent->RegisterComponent();
    }

    // Create secondary audio component for crossfading
    SecondaryAudioComponent = NewObject<UAudioComponent>(GetOwner());
    if (SecondaryAudioComponent)
    {
        SecondaryAudioComponent->SetupAttachment(GetOwner()->GetRootComponent());
        SecondaryAudioComponent->bAutoActivate = false;
        SecondaryAudioComponent->SetVolumeMultiplier(1.0f);
        SecondaryAudioComponent->SetPitchMultiplier(1.0f);
        SecondaryAudioComponent->bAllowSpatialization = true;
        SecondaryAudioComponent->AttenuationSettings = nullptr;
        GetOwner()->AddInstanceComponent(SecondaryAudioComponent);
        SecondaryAudioComponent->RegisterComponent();
    }

    UE_LOG(LogTemp, Log, TEXT("Audio_NarrativeIntegration: Audio components initialized"));
}

void UAudio_NarrativeIntegration::LoadPredefinedNarrativeClips()
{
    // Load the voice lines generated in this cycle
    FAudio_NarrativeClip AtmosphericClip;
    AtmosphericClip.ClipName = TEXT("AtmosphericNarrator");
    AtmosphericClip.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777667789154_AtmosphericNarrator.mp3");
    AtmosphericClip.Context = EAudio_NarrativeContext::Atmospheric;
    AtmosphericClip.Duration = 15.0f;
    AtmosphericClip.Volume = 0.8f;
    AtmosphericClip.bIsLooping = false;
    NarrativeClips.Add(AtmosphericClip);

    FAudio_NarrativeClip SurvivalClip;
    SurvivalClip.ClipName = TEXT("SurvivalExpert");
    SurvivalClip.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777667791541_SurvivalExpert.mp3");
    SurvivalClip.Context = EAudio_NarrativeContext::Survival;
    SurvivalClip.Duration = 12.0f;
    SurvivalClip.Volume = 0.9f;
    SurvivalClip.bIsLooping = false;
    NarrativeClips.Add(SurvivalClip);

    // Load previous cycle clips
    FAudio_NarrativeClip TribalElder;
    TribalElder.ClipName = TEXT("TribalElder");
    TribalElder.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777667781397_TribalElder.mp3");
    TribalElder.Context = EAudio_NarrativeContext::Discovery;
    TribalElder.Duration = 18.0f;
    TribalElder.Volume = 0.7f;
    TribalElder.bIsLooping = false;
    NarrativeClips.Add(TribalElder);

    FAudio_NarrativeClip PlayerCrafting;
    PlayerCrafting.ClipName = TEXT("PlayerCharacter_Crafting");
    PlayerCrafting.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777667782886_PlayerCharacter.mp3");
    PlayerCrafting.Context = EAudio_NarrativeContext::Crafting;
    PlayerCrafting.Duration = 15.0f;
    PlayerCrafting.Volume = 0.8f;
    PlayerCrafting.bIsLooping = false;
    NarrativeClips.Add(PlayerCrafting);

    UE_LOG(LogTemp, Log, TEXT("Audio_NarrativeIntegration: Loaded %d predefined narrative clips"), NarrativeClips.Num());
}

void UAudio_NarrativeIntegration::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Handle fade in/out
    if (bIsFading && FadingComponent)
    {
        FadeTimer += DeltaTime;
        float FadeProgress = FMath::Clamp(FadeTimer / FadeTargetDuration, 0.0f, 1.0f);

        if (bFadeIn)
        {
            float Volume = FMath::Lerp(0.0f, 1.0f, FadeProgress);
            FadingComponent->SetVolumeMultiplier(Volume);
        }
        else
        {
            float Volume = FMath::Lerp(1.0f, 0.0f, FadeProgress);
            FadingComponent->SetVolumeMultiplier(Volume);
        }

        if (FadeProgress >= 1.0f)
        {
            bIsFading = false;
            if (!bFadeIn)
            {
                FadingComponent->Stop();
                bIsPlayingNarrative = false;
            }
        }
    }

    // Check if audio finished playing
    if (bIsPlayingNarrative && PrimaryAudioComponent && !PrimaryAudioComponent->IsPlaying())
    {
        if (!bIsFading) // Only stop if not currently fading
        {
            HandleAudioFinished();
        }
    }
}

void UAudio_NarrativeIntegration::PlayNarrativeClip(const FString& ClipName, bool bInterruptCurrent)
{
    if (bIsPlayingNarrative && !bInterruptCurrent)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_NarrativeIntegration: Already playing narrative, skipping %s"), *ClipName);
        return;
    }

    FAudio_NarrativeClip* FoundClip = FindClipByName(ClipName);
    if (!FoundClip)
    {
        UE_LOG(LogTemp, Error, TEXT("Audio_NarrativeIntegration: Clip not found: %s"), *ClipName);
        return;
    }

    if (bInterruptCurrent && bIsPlayingNarrative)
    {
        StopNarrativeAudio(true);
    }

    // For now, log the clip info (actual audio loading would require additional implementation)
    UE_LOG(LogTemp, Log, TEXT("Audio_NarrativeIntegration: Playing narrative clip '%s' (Context: %d, Duration: %.1fs)"), 
           *FoundClip->ClipName, 
           (int32)FoundClip->Context, 
           FoundClip->Duration);

    bIsPlayingNarrative = true;
    CurrentContext = FoundClip->Context;

    // Start fade in
    if (PrimaryAudioComponent)
    {
        StartFadeIn(PrimaryAudioComponent, FadeInDuration);
    }
}

void UAudio_NarrativeIntegration::PlayContextualNarrative(EAudio_NarrativeContext Context, bool bRandomSelection)
{
    TArray<FAudio_NarrativeClip> ContextClips = GetClipsByContext(Context);
    
    if (ContextClips.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_NarrativeIntegration: No clips found for context %d"), (int32)Context);
        return;
    }

    FAudio_NarrativeClip* SelectedClip = nullptr;
    if (bRandomSelection)
    {
        int32 RandomIndex = FMath::RandRange(0, ContextClips.Num() - 1);
        SelectedClip = &ContextClips[RandomIndex];
    }
    else
    {
        SelectedClip = &ContextClips[0];
    }

    if (SelectedClip)
    {
        PlayNarrativeClip(SelectedClip->ClipName, false);
    }
}

void UAudio_NarrativeIntegration::StopNarrativeAudio(bool bFadeOut)
{
    if (!bIsPlayingNarrative)
    {
        return;
    }

    if (bFadeOut && PrimaryAudioComponent)
    {
        StartFadeOut(PrimaryAudioComponent, FadeOutDuration);
    }
    else
    {
        if (PrimaryAudioComponent)
        {
            PrimaryAudioComponent->Stop();
        }
        bIsPlayingNarrative = false;
    }

    UE_LOG(LogTemp, Log, TEXT("Audio_NarrativeIntegration: Stopping narrative audio (fade: %s)"), bFadeOut ? TEXT("true") : TEXT("false"));
}

void UAudio_NarrativeIntegration::RegisterNarrativeClip(const FAudio_NarrativeClip& NewClip)
{
    // Check if clip already exists
    for (const FAudio_NarrativeClip& ExistingClip : NarrativeClips)
    {
        if (ExistingClip.ClipName == NewClip.ClipName)
        {
            UE_LOG(LogTemp, Warning, TEXT("Audio_NarrativeIntegration: Clip '%s' already exists, skipping registration"), *NewClip.ClipName);
            return;
        }
    }

    NarrativeClips.Add(NewClip);
    UE_LOG(LogTemp, Log, TEXT("Audio_NarrativeIntegration: Registered new narrative clip '%s'"), *NewClip.ClipName);
}

void UAudio_NarrativeIntegration::OnNarrativeTriggerEntered(const FString& TriggerID, EAudio_NarrativeContext Context)
{
    UE_LOG(LogTemp, Log, TEXT("Audio_NarrativeIntegration: Narrative trigger entered - %s (Context: %d)"), *TriggerID, (int32)Context);

    // Map trigger IDs to specific clips or play contextual audio
    if (TriggerID.Contains(TEXT("DangerZone")))
    {
        PlayContextualNarrative(EAudio_NarrativeContext::Danger, true);
    }
    else if (TriggerID.Contains(TEXT("WaterSource")))
    {
        PlayContextualNarrative(EAudio_NarrativeContext::Survival, true);
    }
    else if (TriggerID.Contains(TEXT("SafeSpot")))
    {
        PlayContextualNarrative(EAudio_NarrativeContext::Atmospheric, true);
    }
    else
    {
        PlayContextualNarrative(Context, true);
    }
}

void UAudio_NarrativeIntegration::OnNarrativeTriggerExited(const FString& TriggerID)
{
    UE_LOG(LogTemp, Log, TEXT("Audio_NarrativeIntegration: Narrative trigger exited - %s"), *TriggerID);
    
    // Optionally fade out current narrative when leaving trigger
    if (bIsPlayingNarrative)
    {
        StopNarrativeAudio(true);
    }
}

TArray<FAudio_NarrativeClip> UAudio_NarrativeIntegration::GetClipsByContext(EAudio_NarrativeContext Context) const
{
    TArray<FAudio_NarrativeClip> ContextClips;
    
    for (const FAudio_NarrativeClip& Clip : NarrativeClips)
    {
        if (Clip.Context == Context)
        {
            ContextClips.Add(Clip);
        }
    }
    
    return ContextClips;
}

FAudio_NarrativeClip* UAudio_NarrativeIntegration::FindClipByName(const FString& ClipName)
{
    for (FAudio_NarrativeClip& Clip : NarrativeClips)
    {
        if (Clip.ClipName == ClipName)
        {
            return &Clip;
        }
    }
    return nullptr;
}

void UAudio_NarrativeIntegration::HandleAudioFinished()
{
    bIsPlayingNarrative = false;
    CurrentContext = EAudio_NarrativeContext::Atmospheric;
    
    UE_LOG(LogTemp, Log, TEXT("Audio_NarrativeIntegration: Narrative audio finished"));
}

void UAudio_NarrativeIntegration::StartFadeIn(UAudioComponent* AudioComp, float Duration)
{
    if (!AudioComp)
    {
        return;
    }

    bIsFading = true;
    bFadeIn = true;
    FadeTimer = 0.0f;
    FadeTargetDuration = Duration;
    FadingComponent = AudioComp;

    AudioComp->SetVolumeMultiplier(0.0f);
    // AudioComp->Play(); // Would be called when actual audio loading is implemented
}

void UAudio_NarrativeIntegration::StartFadeOut(UAudioComponent* AudioComp, float Duration)
{
    if (!AudioComp)
    {
        return;
    }

    bIsFading = true;
    bFadeIn = false;
    FadeTimer = 0.0f;
    FadeTargetDuration = Duration;
    FadingComponent = AudioComp;
}