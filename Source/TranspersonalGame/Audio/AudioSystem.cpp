#include "AudioSystem.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundAttenuation.h"
#include "MetasoundSource.h"

void UAudioSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Initialize adaptive music system
    if (UWorld* World = GetWorld())
    {
        // Create main music component
        MusicComponent = UGameplayStatics::CreateSound2D(World, nullptr);
        if (MusicComponent)
        {
            MusicComponent->bAutoDestroy = false;
            MusicComponent->bIsUISound = true;
        }

        // Create ambience component
        AmbienceComponent = UGameplayStatics::CreateSound2D(World, nullptr);
        if (AmbienceComponent)
        {
            AmbienceComponent->bAutoDestroy = false;
            AmbienceComponent->bIsUISound = true;
        }

        // Load default attenuation settings
        DinosaurAttenuation = LoadObject<USoundAttenuation>(nullptr, TEXT("/Game/Audio/Attenuation/SA_DinosaurSounds"));
        EnvironmentAttenuation = LoadObject<USoundAttenuation>(nullptr, TEXT("/Game/Audio/Attenuation/SA_Environment"));
    }

    UE_LOG(LogTemp, Log, TEXT("Audio System initialized"));
}

void UAudioSystem::Deinitialize()
{
    // Clean up audio components
    if (MusicComponent)
    {
        MusicComponent->Stop();
        MusicComponent = nullptr;
    }

    if (AmbienceComponent)
    {
        AmbienceComponent->Stop();
        AmbienceComponent = nullptr;
    }

    for (UAudioComponent* Component : DynamicAudioComponents)
    {
        if (Component)
        {
            Component->Stop();
        }
    }
    DynamicAudioComponents.Empty();

    Super::Deinitialize();
}

void UAudioSystem::UpdateAudioState(const FAudioStateData& NewState)
{
    FAudioStateData OldState = CurrentAudioState;
    CurrentAudioState = NewState;

    // Process transitions between states
    ProcessAudioTransition(OldState, NewState);

    // Update systems based on new state
    UpdateAdaptiveMusic();
    UpdateAmbience();
}

void UAudioSystem::PlayDinosaurSound(ADinosaur* Dinosaur, const FString& SoundType)
{
    if (!Dinosaur)
    {
        return;
    }

    // Construct sound path based on dinosaur type and sound type
    FString SoundPath = FString::Printf(TEXT("/Game/Audio/Dinosaurs/%s/%s"), 
        *Dinosaur->GetDinosaurSpecies(), *SoundType);

    USoundBase* Sound = LoadObject<USoundBase>(nullptr, *SoundPath);
    if (Sound)
    {
        UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAttached(
            Sound, 
            Dinosaur->GetRootComponent(),
            NAME_None,
            FVector::ZeroVector,
            EAttachLocation::KeepRelativeOffset,
            true, // Stop when attached actor is destroyed
            1.0f, // Volume
            1.0f, // Pitch
            0.0f, // Start time
            DinosaurAttenuation
        );

        if (AudioComp)
        {
            DynamicAudioComponents.Add(AudioComp);
            
            // Remove from array when finished
            AudioComp->OnAudioFinished.AddDynamic(this, &UAudioSystem::OnDynamicAudioFinished);
        }
    }
}

void UAudioSystem::PlayEnvironmentalSound(const FVector& Location, const FString& SoundType, float Volume)
{
    FString SoundPath = FString::Printf(TEXT("/Game/Audio/Environment/%s"), *SoundType);
    
    USoundBase* Sound = LoadObject<USoundBase>(nullptr, *SoundPath);
    if (Sound)
    {
        UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(
            GetWorld(),
            Sound,
            Location,
            FRotator::ZeroRotator,
            Volume,
            1.0f, // Pitch
            0.0f, // Start time
            EnvironmentAttenuation
        );

        if (AudioComp)
        {
            DynamicAudioComponents.Add(AudioComp);
            AudioComp->OnAudioFinished.AddDynamic(this, &UAudioSystem::OnDynamicAudioFinished);
        }
    }
}

void UAudioSystem::SetMusicIntensity(float Intensity)
{
    if (MusicComponent && AdaptiveMusicSource)
    {
        // Set MetaSound parameter for intensity
        MusicComponent->SetFloatParameter(FName("Intensity"), FMath::Clamp(Intensity, 0.0f, 1.0f));
    }
}

void UAudioSystem::UpdateAdaptiveMusic()
{
    if (!MusicComponent || !AdaptiveMusicSource)
    {
        return;
    }

    // Calculate music intensity based on current state
    float MusicIntensity = 0.0f;
    
    switch (CurrentAudioState.EmotionalState)
    {
        case EEmotionalState::Calm:
            MusicIntensity = 0.2f;
            break;
        case EEmotionalState::Tense:
            MusicIntensity = 0.5f;
            break;
        case EEmotionalState::Danger:
            MusicIntensity = 0.8f;
            break;
        case EEmotionalState::Terror:
            MusicIntensity = 1.0f;
            break;
        case EEmotionalState::Wonder:
            MusicIntensity = 0.3f;
            break;
        case EEmotionalState::Melancholy:
            MusicIntensity = 0.1f;
            break;
    }

    // Adjust for threat level and time of day
    MusicIntensity = FMath::Lerp(MusicIntensity, 1.0f, CurrentAudioState.ThreatLevel);
    
    // Night time adds tension
    if (CurrentAudioState.TimeOfDay < 0.2f || CurrentAudioState.TimeOfDay > 0.8f)
    {
        MusicIntensity += 0.2f;
    }

    MusicIntensity = FMath::Clamp(MusicIntensity, 0.0f, 1.0f);
    
    // Update MetaSound parameters
    MusicComponent->SetFloatParameter(FName("Intensity"), MusicIntensity);
    MusicComponent->SetFloatParameter(FName("ThreatLevel"), CurrentAudioState.ThreatLevel);
    MusicComponent->SetFloatParameter(FName("TimeOfDay"), CurrentAudioState.TimeOfDay);
    MusicComponent->SetIntParameter(FName("EnvironmentType"), (int32)CurrentAudioState.EnvironmentType);
}

void UAudioSystem::UpdateAmbience()
{
    if (!AmbienceComponent)
    {
        return;
    }

    // Load appropriate ambience based on environment
    FString AmbiencePath;
    switch (CurrentAudioState.EnvironmentType)
    {
        case EEnvironmentType::DenseForest:
            AmbiencePath = TEXT("/Game/Audio/Ambience/MS_ForestAmbience");
            break;
        case EEnvironmentType::OpenPlains:
            AmbiencePath = TEXT("/Game/Audio/Ambience/MS_PlainsAmbience");
            break;
        case EEnvironmentType::RiverSide:
            AmbiencePath = TEXT("/Game/Audio/Ambience/MS_RiverAmbience");
            break;
        case EEnvironmentType::CaveSystem:
            AmbiencePath = TEXT("/Game/Audio/Ambience/MS_CaveAmbience");
            break;
        case EEnvironmentType::DinosaurNest:
            AmbiencePath = TEXT("/Game/Audio/Ambience/MS_NestAmbience");
            break;
        case EEnvironmentType::SafeZone:
            AmbiencePath = TEXT("/Game/Audio/Ambience/MS_SafeAmbience");
            break;
    }

    UMetaSoundSource* AmbienceSource = LoadObject<UMetaSoundSource>(nullptr, *AmbiencePath);
    if (AmbienceSource && AmbienceComponent->GetSound() != AmbienceSource)
    {
        AmbienceComponent->SetSound(AmbienceSource);
        AmbienceComponent->Play();
        
        // Set ambience parameters
        AmbienceComponent->SetFloatParameter(FName("WeatherIntensity"), CurrentAudioState.WeatherIntensity);
        AmbienceComponent->SetFloatParameter(FName("TimeOfDay"), CurrentAudioState.TimeOfDay);
        AmbienceComponent->SetIntParameter(FName("DinosaurCount"), CurrentAudioState.NearbyDinosaurCount);
    }
}

void UAudioSystem::ProcessAudioTransition(const FAudioStateData& OldState, const FAudioStateData& NewState)
{
    // Handle dramatic state changes with transition sounds
    if (OldState.EmotionalState != NewState.EmotionalState)
    {
        if (NewState.EmotionalState == EEmotionalState::Danger || 
            NewState.EmotionalState == EEmotionalState::Terror)
        {
            // Play tension sting
            PlayEnvironmentalSound(FVector::ZeroVector, TEXT("TensionSting"), 0.7f);
        }
        else if (OldState.EmotionalState == EEmotionalState::Terror && 
                 NewState.EmotionalState == EEmotionalState::Calm)
        {
            // Play relief sound
            PlayEnvironmentalSound(FVector::ZeroVector, TEXT("ReliefSting"), 0.5f);
        }
    }

    // Environment transition sounds
    if (OldState.EnvironmentType != NewState.EnvironmentType)
    {
        FString TransitionSound = FString::Printf(TEXT("Transition_%s_to_%s"), 
            *UEnum::GetValueAsString(OldState.EnvironmentType),
            *UEnum::GetValueAsString(NewState.EnvironmentType));
        
        PlayEnvironmentalSound(FVector::ZeroVector, TransitionSound, 0.6f);
    }
}

UFUNCTION()
void UAudioSystem::OnDynamicAudioFinished()
{
    // Clean up finished audio components
    for (int32 i = DynamicAudioComponents.Num() - 1; i >= 0; i--)
    {
        if (!DynamicAudioComponents[i] || !DynamicAudioComponents[i]->IsPlaying())
        {
            DynamicAudioComponents.RemoveAt(i);
        }
    }
}