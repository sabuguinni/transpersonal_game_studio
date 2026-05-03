#include "AudioManager.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

AAudioManager::AAudioManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create audio components
    EnvironmentAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("EnvironmentAudio"));
    RootComponent = EnvironmentAudioComponent;

    MusicAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicAudio"));
    MusicAudioComponent->SetupAttachment(RootComponent);

    NarrativeAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("NarrativeAudio"));
    NarrativeAudioComponent->SetupAttachment(RootComponent);

    // Initialize default settings
    CurrentEnvironmentSettings = FAudio_EnvironmentSettings();
    CurrentIntensity = EAudio_IntensityLevel::Ambient;
    MasterVolume = 1.0f;
    MusicVolume = 0.7f;
    SFXVolume = 0.8f;
    NarrativeVolume = 0.9f;

    // Initialize dinosaur sound profiles
    DinosaurSoundProfiles.Empty();

    // T-Rex profile
    FAudio_DinosaurSoundProfile TRexProfile;
    TRexProfile.DinosaurType = EAudio_DinosaurType::TRex;
    TRexProfile.FootstepVolume = 1.0f;
    TRexProfile.VocalizationVolume = 1.0f;
    TRexProfile.FootstepRange = 3000.0f;
    TRexProfile.VocalizationRange = 8000.0f;
    TRexProfile.bCausesGroundShake = true;
    DinosaurSoundProfiles.Add(TRexProfile);

    // Raptor profile
    FAudio_DinosaurSoundProfile RaptorProfile;
    RaptorProfile.DinosaurType = EAudio_DinosaurType::Raptor;
    RaptorProfile.FootstepVolume = 0.4f;
    RaptorProfile.VocalizationVolume = 0.6f;
    RaptorProfile.FootstepRange = 800.0f;
    RaptorProfile.VocalizationRange = 2000.0f;
    RaptorProfile.bCausesGroundShake = false;
    DinosaurSoundProfiles.Add(RaptorProfile);

    // Brachiosaurus profile
    FAudio_DinosaurSoundProfile BrachiosaurusProfile;
    BrachiosaurusProfile.DinosaurType = EAudio_DinosaurType::Brachiosaurus;
    BrachiosaurusProfile.FootstepVolume = 0.9f;
    BrachiosaurusProfile.VocalizationVolume = 0.8f;
    BrachiosaurusProfile.FootstepRange = 2500.0f;
    BrachiosaurusProfile.VocalizationRange = 6000.0f;
    BrachiosaurusProfile.bCausesGroundShake = true;
    DinosaurSoundProfiles.Add(BrachiosaurusProfile);

    bIsInitialized = false;
}

void AAudioManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAudioSystem();

    // Register with subsystem
    if (UWorld* World = GetWorld())
    {
        if (UAudioWorldSubsystem* AudioSubsystem = World->GetSubsystem<UAudioWorldSubsystem>())
        {
            AudioSubsystem->RegisterAudioManager(this);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("AudioManager: BeginPlay completed"));
}

void AAudioManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsInitialized)
    {
        UpdateMusicFade(DeltaTime);
    }
}

void AAudioManager::InitializeAudioSystem()
{
    if (bIsInitialized)
    {
        return;
    }

    // Configure audio components
    if (EnvironmentAudioComponent)
    {
        EnvironmentAudioComponent->bAutoActivate = true;
        EnvironmentAudioComponent->SetVolumeMultiplier(MasterVolume * SFXVolume);
    }

    if (MusicAudioComponent)
    {
        MusicAudioComponent->bAutoActivate = false;
        MusicAudioComponent->SetVolumeMultiplier(MasterVolume * MusicVolume);
    }

    if (NarrativeAudioComponent)
    {
        NarrativeAudioComponent->bAutoActivate = false;
        NarrativeAudioComponent->SetVolumeMultiplier(MasterVolume * NarrativeVolume);
    }

    // Set initial environment
    SetEnvironmentType(EAudio_EnvironmentType::Forest);
    SetIntensityLevel(EAudio_IntensityLevel::Ambient);

    bIsInitialized = true;
    UE_LOG(LogTemp, Warning, TEXT("AudioManager: System initialized"));
}

void AAudioManager::ShutdownAudioSystem()
{
    if (!bIsInitialized)
    {
        return;
    }

    // Stop all audio
    if (EnvironmentAudioComponent && EnvironmentAudioComponent->IsPlaying())
    {
        EnvironmentAudioComponent->Stop();
    }

    if (MusicAudioComponent && MusicAudioComponent->IsPlaying())
    {
        MusicAudioComponent->Stop();
    }

    if (NarrativeAudioComponent && NarrativeAudioComponent->IsPlaying())
    {
        NarrativeAudioComponent->Stop();
    }

    bIsInitialized = false;
    UE_LOG(LogTemp, Warning, TEXT("AudioManager: System shutdown"));
}

void AAudioManager::SetEnvironmentType(EAudio_EnvironmentType NewEnvironment)
{
    CurrentEnvironmentSettings.EnvironmentType = NewEnvironment;
    UpdateEnvironmentAudio();

    UE_LOG(LogTemp, Warning, TEXT("AudioManager: Environment changed to %d"), (int32)NewEnvironment);
}

void AAudioManager::SetIntensityLevel(EAudio_IntensityLevel NewIntensity)
{
    CurrentIntensity = NewIntensity;
    
    // Adjust music based on intensity
    switch (NewIntensity)
    {
        case EAudio_IntensityLevel::Calm:
        case EAudio_IntensityLevel::Ambient:
            if (BackgroundMusicCalm)
            {
                TransitionToMusic(BackgroundMusicCalm, 3.0f);
            }
            break;
        case EAudio_IntensityLevel::Tense:
        case EAudio_IntensityLevel::Danger:
        case EAudio_IntensityLevel::Combat:
            if (BackgroundMusicTense)
            {
                TransitionToMusic(BackgroundMusicTense, 1.5f);
            }
            break;
    }

    UE_LOG(LogTemp, Warning, TEXT("AudioManager: Intensity changed to %d"), (int32)NewIntensity);
}

void AAudioManager::UpdateEnvironmentAudio()
{
    if (!EnvironmentAudioComponent)
    {
        return;
    }

    USoundCue* TargetSound = nullptr;

    // Select ambient sound based on environment
    switch (CurrentEnvironmentSettings.EnvironmentType)
    {
        case EAudio_EnvironmentType::Forest:
            TargetSound = ForestAmbientSound;
            break;
        case EAudio_EnvironmentType::Swamp:
            TargetSound = SwampAmbientSound;
            break;
        case EAudio_EnvironmentType::Grassland:
            TargetSound = GrasslandAmbientSound;
            break;
        default:
            TargetSound = ForestAmbientSound; // Default fallback
            break;
    }

    if (TargetSound && TargetSound != EnvironmentAudioComponent->GetSound())
    {
        EnvironmentAudioComponent->SetSound(TargetSound);
        EnvironmentAudioComponent->SetVolumeMultiplier(MasterVolume * SFXVolume * CurrentEnvironmentSettings.AmbientVolume);
        
        if (!EnvironmentAudioComponent->IsPlaying())
        {
            EnvironmentAudioComponent->Play();
        }
    }
}

void AAudioManager::PlayDinosaurFootstep(EAudio_DinosaurType DinosaurType, FVector Location)
{
    FAudio_DinosaurSoundProfile Profile = GetDinosaurProfile(DinosaurType);
    
    USoundCue* FootstepSound = nullptr;
    switch (DinosaurType)
    {
        case EAudio_DinosaurType::TRex:
            FootstepSound = TRexFootstepSound;
            break;
        // Add other dinosaur footstep sounds when available
        default:
            FootstepSound = TRexFootstepSound; // Fallback
            break;
    }

    if (FootstepSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            FootstepSound,
            Location,
            MasterVolume * SFXVolume * Profile.FootstepVolume
        );

        // Trigger ground shake for large dinosaurs
        if (Profile.bCausesGroundShake)
        {
            TriggerGroundShake(Location, Profile.FootstepVolume);
        }

        UE_LOG(LogTemp, Warning, TEXT("AudioManager: Played footstep for dinosaur %d at location %s"), 
               (int32)DinosaurType, *Location.ToString());
    }
}

void AAudioManager::PlayDinosaurVocalization(EAudio_DinosaurType DinosaurType, FVector Location)
{
    FAudio_DinosaurSoundProfile Profile = GetDinosaurProfile(DinosaurType);
    
    USoundCue* VocalizationSound = nullptr;
    switch (DinosaurType)
    {
        case EAudio_DinosaurType::TRex:
            VocalizationSound = TRexRoarSound;
            break;
        // Add other dinosaur vocalizations when available
        default:
            VocalizationSound = TRexRoarSound; // Fallback
            break;
    }

    if (VocalizationSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            VocalizationSound,
            Location,
            MasterVolume * SFXVolume * Profile.VocalizationVolume
        );

        // Increase intensity when dangerous dinosaurs vocalize
        if (DinosaurType == EAudio_DinosaurType::TRex && CurrentIntensity < EAudio_IntensityLevel::Danger)
        {
            SetIntensityLevel(EAudio_IntensityLevel::Danger);
        }

        UE_LOG(LogTemp, Warning, TEXT("AudioManager: Played vocalization for dinosaur %d at location %s"), 
               (int32)DinosaurType, *Location.ToString());
    }
}

void AAudioManager::TriggerGroundShake(FVector Location, float Intensity)
{
    // This would integrate with camera shake system
    // For now, just log the event
    UE_LOG(LogTemp, Warning, TEXT("AudioManager: Ground shake triggered at %s with intensity %f"), 
           *Location.ToString(), Intensity);
}

void AAudioManager::PlayNarrativeLine(const FString& AudioPath, FVector Location)
{
    if (!NarrativeAudioComponent)
    {
        return;
    }

    // Stop current narrative if playing
    if (NarrativeAudioComponent->IsPlaying())
    {
        NarrativeAudioComponent->Stop();
    }

    // Load and play the narrative audio
    // Note: In a real implementation, you'd load the audio asset from the path
    // For now, we'll just log the request
    UE_LOG(LogTemp, Warning, TEXT("AudioManager: Playing narrative line: %s at location %s"), 
           *AudioPath, *Location.ToString());

    // Set volume and play
    NarrativeAudioComponent->SetVolumeMultiplier(MasterVolume * NarrativeVolume);
    // NarrativeAudioComponent->Play(); // Uncomment when actual audio asset is loaded
}

void AAudioManager::StopNarrativeAudio()
{
    if (NarrativeAudioComponent && NarrativeAudioComponent->IsPlaying())
    {
        NarrativeAudioComponent->Stop();
        UE_LOG(LogTemp, Warning, TEXT("AudioManager: Narrative audio stopped"));
    }
}

bool AAudioManager::IsNarrativeAudioPlaying() const
{
    return NarrativeAudioComponent && NarrativeAudioComponent->IsPlaying();
}

void AAudioManager::TransitionToMusic(USoundCue* NewMusic, float FadeTime)
{
    if (!MusicAudioComponent || NewMusic == CurrentMusic)
    {
        return;
    }

    PendingMusic = NewMusic;
    MusicFadeDuration = FadeTime;
    MusicFadeTimer = 0.0f;

    UE_LOG(LogTemp, Warning, TEXT("AudioManager: Starting music transition (fade time: %f)"), FadeTime);
}

void AAudioManager::UpdateMusicFade(float DeltaTime)
{
    if (!PendingMusic || MusicFadeDuration <= 0.0f)
    {
        return;
    }

    MusicFadeTimer += DeltaTime;
    float FadeProgress = FMath::Clamp(MusicFadeTimer / MusicFadeDuration, 0.0f, 1.0f);

    if (MusicAudioComponent)
    {
        // Fade out current music
        if (CurrentMusic && FadeProgress < 0.5f)
        {
            float FadeOutVolume = (1.0f - (FadeProgress * 2.0f)) * MasterVolume * MusicVolume;
            MusicAudioComponent->SetVolumeMultiplier(FadeOutVolume);
        }
        // Switch to new music and fade in
        else if (FadeProgress >= 0.5f)
        {
            if (CurrentMusic != PendingMusic)
            {
                MusicAudioComponent->SetSound(PendingMusic);
                CurrentMusic = PendingMusic;
                if (!MusicAudioComponent->IsPlaying())
                {
                    MusicAudioComponent->Play();
                }
            }

            float FadeInVolume = ((FadeProgress - 0.5f) * 2.0f) * MasterVolume * MusicVolume;
            MusicAudioComponent->SetVolumeMultiplier(FadeInVolume);
        }

        // Complete transition
        if (FadeProgress >= 1.0f)
        {
            MusicAudioComponent->SetVolumeMultiplier(MasterVolume * MusicVolume);
            PendingMusic = nullptr;
            MusicFadeTimer = 0.0f;
            MusicFadeDuration = 0.0f;
        }
    }
}

void AAudioManager::SetMusicVolume(float Volume)
{
    MusicVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    ApplyVolumeSettings();
}

void AAudioManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    ApplyVolumeSettings();
}

void AAudioManager::SetSFXVolume(float Volume)
{
    SFXVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    ApplyVolumeSettings();
}

void AAudioManager::SetNarrativeVolume(float Volume)
{
    NarrativeVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    ApplyVolumeSettings();
}

void AAudioManager::ApplyVolumeSettings()
{
    if (EnvironmentAudioComponent)
    {
        EnvironmentAudioComponent->SetVolumeMultiplier(MasterVolume * SFXVolume * CurrentEnvironmentSettings.AmbientVolume);
    }

    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetVolumeMultiplier(MasterVolume * MusicVolume);
    }

    if (NarrativeAudioComponent)
    {
        NarrativeAudioComponent->SetVolumeMultiplier(MasterVolume * NarrativeVolume);
    }
}

void AAudioManager::PlaySpatialSound(USoundCue* Sound, FVector Location, float Volume)
{
    if (Sound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            Sound,
            Location,
            MasterVolume * SFXVolume * Volume
        );
    }
}

FAudio_DinosaurSoundProfile AAudioManager::GetDinosaurProfile(EAudio_DinosaurType DinosaurType)
{
    for (const FAudio_DinosaurSoundProfile& Profile : DinosaurSoundProfiles)
    {
        if (Profile.DinosaurType == DinosaurType)
        {
            return Profile;
        }
    }

    // Return default profile if not found
    return FAudio_DinosaurSoundProfile();
}

// Audio World Subsystem Implementation
void UAudioWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Warning, TEXT("AudioWorldSubsystem: Initialized"));
}

void UAudioWorldSubsystem::Deinitialize()
{
    AudioManagerInstance = nullptr;
    Super::Deinitialize();
    UE_LOG(LogTemp, Warning, TEXT("AudioWorldSubsystem: Deinitialized"));
}

AAudioManager* UAudioWorldSubsystem::GetAudioManager()
{
    return AudioManagerInstance;
}

void UAudioWorldSubsystem::RegisterAudioManager(AAudioManager* Manager)
{
    AudioManagerInstance = Manager;
    UE_LOG(LogTemp, Warning, TEXT("AudioWorldSubsystem: AudioManager registered"));
}