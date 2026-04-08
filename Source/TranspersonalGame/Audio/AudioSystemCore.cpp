#include "AudioSystemCore.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "AudioMixerBlueprintLibrary.h"
#include "MetasoundParameterPack.h"

UAudioSystemCore::UAudioSystemCore()
{
    // Initialize default audio state
    CurrentAudioState.EmotionalState = EEmotionalState::Calm;
    CurrentAudioState.TimeOfDay = ETimeOfDay::Morning;
    CurrentAudioState.CurrentBiome = EBiomeType::DenseForest;
    CurrentAudioState.ThreatLevel = 0.0f;
    CurrentAudioState.ExplorationProgress = 0.0f;
    CurrentAudioState.bNearDinosaurs = false;
    CurrentAudioState.bInShelter = false;
    CurrentAudioState.DistanceToNearestThreat = 1000.0f;

    DefaultTransitionTime = 2.0f;
}

void UAudioSystemCore::UpdateAudioState(const FAudioStateData& NewState)
{
    PreviousAudioState = CurrentAudioState;
    CurrentAudioState = NewState;

    // Trigger necessary audio updates based on state changes
    if (PreviousAudioState.EmotionalState != CurrentAudioState.EmotionalState)
    {
        TriggerMusicTransition(CurrentAudioState.EmotionalState);
    }

    if (PreviousAudioState.TimeOfDay != CurrentAudioState.TimeOfDay ||
        PreviousAudioState.CurrentBiome != CurrentAudioState.CurrentBiome)
    {
        UpdateAmbientLayers();
    }

    // Update MetaSound parameters
    UpdateMetaSoundParameters();
}

void UAudioSystemCore::SetEmotionalState(EEmotionalState NewState, float TransitionTime)
{
    if (CurrentAudioState.EmotionalState == NewState)
        return;

    EEmotionalState PrevState = CurrentAudioState.EmotionalState;
    CurrentAudioState.EmotionalState = NewState;

    ProcessEmotionalTransition(PrevState, NewState, TransitionTime);
}

void UAudioSystemCore::SetTimeOfDay(ETimeOfDay NewTime, float TransitionTime)
{
    if (CurrentAudioState.TimeOfDay == NewTime)
        return;

    CurrentAudioState.TimeOfDay = NewTime;
    UpdateAmbientLayers();
}

void UAudioSystemCore::SetBiome(EBiomeType NewBiome, float TransitionTime)
{
    if (CurrentAudioState.CurrentBiome == NewBiome)
        return;

    CurrentAudioState.CurrentBiome = NewBiome;
    UpdateAmbientLayers();
}

void UAudioSystemCore::TriggerMusicTransition(EEmotionalState TargetState)
{
    if (!MusicAudioComponent || !AdaptiveMusicMetaSound)
        return;

    // Calculate transition parameters based on emotional states
    float IntensityTarget = CalculateEmotionalIntensity();
    
    // Set MetaSound parameters for the transition
    if (UMetasoundParameterPack* ParameterPack = UMetasoundParameterPack::CreateMetasoundParameterPack())
    {
        // Emotional state as integer for MetaSound switching
        ParameterPack->SetInt(FName("EmotionalState"), static_cast<int32>(TargetState));
        ParameterPack->SetFloat(FName("Intensity"), IntensityTarget);
        ParameterPack->SetFloat(FName("ThreatLevel"), CurrentAudioState.ThreatLevel);
        ParameterPack->SetBool(FName("NearDinosaurs"), CurrentAudioState.bNearDinosaurs);
        ParameterPack->SetFloat(FName("TransitionTime"), DefaultTransitionTime);

        MusicAudioComponent->SetParameterPack(ParameterPack);
    }
}

void UAudioSystemCore::SetMusicIntensity(float Intensity)
{
    Intensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    
    if (MusicAudioComponent && AdaptiveMusicMetaSound)
    {
        if (UMetasoundParameterPack* ParameterPack = UMetasoundParameterPack::CreateMetasoundParameterPack())
        {
            ParameterPack->SetFloat(FName("Intensity"), Intensity);
            MusicAudioComponent->SetParameterPack(ParameterPack);
        }
    }
}

void UAudioSystemCore::UpdateAmbientLayers()
{
    if (!AmbientAudioComponent || !AmbientLayersMetaSound)
        return;

    if (UMetasoundParameterPack* ParameterPack = UMetasoundParameterPack::CreateMetasoundParameterPack())
    {
        // Biome parameters
        ParameterPack->SetInt(FName("BiomeType"), static_cast<int32>(CurrentAudioState.CurrentBiome));
        ParameterPack->SetInt(FName("TimeOfDay"), static_cast<int32>(CurrentAudioState.TimeOfDay));
        
        // Environmental parameters
        ParameterPack->SetFloat(FName("WindIntensity"), FMath::RandRange(0.3f, 0.8f));
        ParameterPack->SetFloat(FName("InsectActivity"), GetInsectActivityForTime());
        ParameterPack->SetFloat(FName("BirdActivity"), GetBirdActivityForTime());
        ParameterPack->SetBool(FName("InShelter"), CurrentAudioState.bInShelter);

        AmbientAudioComponent->SetParameterPack(ParameterPack);
    }
}

void UAudioSystemCore::PlayDinosaurSound(FVector Location, FString DinosaurType, FString SoundType)
{
    if (!DinosaurAudioMetaSound)
        return;

    // Create a new audio component for this dinosaur sound
    if (UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull))
    {
        UAudioComponent* DinoAudioComp = UGameplayStatics::SpawnSoundAtLocation(
            World, 
            DinosaurAudioMetaSound, 
            Location,
            FRotator::ZeroRotator,
            1.0f, // Volume
            1.0f, // Pitch
            0.0f, // Start time
            nullptr, // Attenuation settings (will use MetaSound's built-in)
            nullptr, // Concurrency settings
            true    // Auto destroy
        );

        if (DinoAudioComp)
        {
            if (UMetasoundParameterPack* ParameterPack = UMetasoundParameterPack::CreateMetasoundParameterPack())
            {
                // Convert dinosaur type and sound type to parameters
                int32 DinoTypeIndex = GetDinosaurTypeIndex(DinosaurType);
                int32 SoundTypeIndex = GetSoundTypeIndex(SoundType);
                
                ParameterPack->SetInt(FName("DinosaurType"), DinoTypeIndex);
                ParameterPack->SetInt(FName("SoundType"), SoundTypeIndex);
                ParameterPack->SetFloat(FName("Distance"), FVector::Dist(Location, GetListenerLocation()));
                ParameterPack->SetFloat(FName("EmotionalIntensity"), CalculateEmotionalIntensity());

                DinoAudioComp->SetParameterPack(ParameterPack);
            }

            DynamicAudioComponents.Add(DinoAudioComp);
        }
    }
}

void UAudioSystemCore::TriggerFootstepVariation(FVector Location, FString SurfaceType)
{
    // Implementation for procedural footstep audio
    // This would use a separate MetaSound for footstep generation
    // Based on surface type, player weight, movement speed, etc.
}

void UAudioSystemCore::PlayEnvironmentalEvent(FVector Location, FString EventType)
{
    // Implementation for environmental audio events
    // Tree falling, water splash, rock slide, etc.
}

void UAudioSystemCore::Update3DAudioPosition(FVector ListenerLocation, FRotator ListenerRotation)
{
    // Update all active audio components with new listener position
    for (UAudioComponent* AudioComp : DynamicAudioComponents)
    {
        if (IsValid(AudioComp))
        {
            // Update 3D positioning parameters if needed
            float Distance = FVector::Dist(AudioComp->GetComponentLocation(), ListenerLocation);
            
            if (UMetasoundParameterPack* ParameterPack = UMetasoundParameterPack::CreateMetasoundParameterPack())
            {
                ParameterPack->SetFloat(FName("ListenerDistance"), Distance);
                AudioComp->SetParameterPack(ParameterPack);
            }
        }
    }

    // Clean up finished components
    CleanupFinishedAudioComponents();
}

void UAudioSystemCore::SetOcclusionLevel(float OcclusionAmount)
{
    OcclusionAmount = FMath::Clamp(OcclusionAmount, 0.0f, 1.0f);
    
    // Apply occlusion to all active audio components
    for (UAudioComponent* AudioComp : DynamicAudioComponents)
    {
        if (IsValid(AudioComp))
        {
            if (UMetasoundParameterPack* ParameterPack = UMetasoundParameterPack::CreateMetasoundParameterPack())
            {
                ParameterPack->SetFloat(FName("OcclusionLevel"), OcclusionAmount);
                AudioComp->SetParameterPack(ParameterPack);
            }
        }
    }
}

void UAudioSystemCore::ProcessEmotionalTransition(EEmotionalState FromState, EEmotionalState ToState, float TransitionTime)
{
    // Log the emotional transition for debugging
    UE_LOG(LogTemp, Log, TEXT("Audio: Emotional transition from %d to %d over %.2f seconds"), 
           static_cast<int32>(FromState), static_cast<int32>(ToState), TransitionTime);

    // Trigger the music transition
    TriggerMusicTransition(ToState);
}

void UAudioSystemCore::UpdateMetaSoundParameters()
{
    // Update all MetaSound parameters based on current state
    float EmotionalIntensity = CalculateEmotionalIntensity();
    
    // Update music parameters
    if (MusicAudioComponent && AdaptiveMusicMetaSound)
    {
        if (UMetasoundParameterPack* ParameterPack = UMetasoundParameterPack::CreateMetasoundParameterPack())
        {
            ParameterPack->SetInt(FName("EmotionalState"), static_cast<int32>(CurrentAudioState.EmotionalState));
            ParameterPack->SetFloat(FName("Intensity"), EmotionalIntensity);
            ParameterPack->SetFloat(FName("ThreatLevel"), CurrentAudioState.ThreatLevel);
            ParameterPack->SetBool(FName("InShelter"), CurrentAudioState.bInShelter);
            
            MusicAudioComponent->SetParameterPack(ParameterPack);
        }
    }
}

float UAudioSystemCore::CalculateEmotionalIntensity() const
{
    float BaseIntensity = 0.0f;

    switch (CurrentAudioState.EmotionalState)
    {
        case EEmotionalState::Calm:
            BaseIntensity = 0.1f;
            break;
        case EEmotionalState::Tension:
            BaseIntensity = 0.4f;
            break;
        case EEmotionalState::Fear:
            BaseIntensity = 0.8f;
            break;
        case EEmotionalState::Wonder:
            BaseIntensity = 0.3f;
            break;
        case EEmotionalState::Danger:
            BaseIntensity = 0.9f;
            break;
        case EEmotionalState::Discovery:
            BaseIntensity = 0.5f;
            break;
        case EEmotionalState::Stalking:
            BaseIntensity = 0.6f;
            break;
        case EEmotionalState::Chase:
            BaseIntensity = 1.0f;
            break;
        case EEmotionalState::Safety:
            BaseIntensity = 0.2f;
            break;
        case EEmotionalState::Loneliness:
            BaseIntensity = 0.3f;
            break;
    }

    // Modify based on threat level and other factors
    float ThreatModifier = CurrentAudioState.ThreatLevel * 0.3f;
    float ProximityModifier = CurrentAudioState.bNearDinosaurs ? 0.2f : 0.0f;
    
    return FMath::Clamp(BaseIntensity + ThreatModifier + ProximityModifier, 0.0f, 1.0f);
}

void UAudioSystemCore::CleanupFinishedAudioComponents()
{
    DynamicAudioComponents.RemoveAll([](UAudioComponent* AudioComp)
    {
        return !IsValid(AudioComp) || !AudioComp->IsPlaying();
    });
}

// Helper functions
float UAudioSystemCore::GetInsectActivityForTime() const
{
    switch (CurrentAudioState.TimeOfDay)
    {
        case ETimeOfDay::Dawn:
            return 0.7f;
        case ETimeOfDay::Morning:
            return 0.5f;
        case ETimeOfDay::Midday:
            return 0.3f;
        case ETimeOfDay::Afternoon:
            return 0.4f;
        case ETimeOfDay::Dusk:
            return 0.8f;
        case ETimeOfDay::Night:
            return 0.9f;
        case ETimeOfDay::DeepNight:
            return 0.6f;
        default:
            return 0.5f;
    }
}

float UAudioSystemCore::GetBirdActivityForTime() const
{
    switch (CurrentAudioState.TimeOfDay)
    {
        case ETimeOfDay::Dawn:
            return 0.9f;
        case ETimeOfDay::Morning:
            return 0.8f;
        case ETimeOfDay::Midday:
            return 0.4f;
        case ETimeOfDay::Afternoon:
            return 0.5f;
        case ETimeOfDay::Dusk:
            return 0.7f;
        case ETimeOfDay::Night:
            return 0.2f;
        case ETimeOfDay::DeepNight:
            return 0.1f;
        default:
            return 0.5f;
    }
}

int32 UAudioSystemCore::GetDinosaurTypeIndex(const FString& DinosaurType) const
{
    // Map dinosaur type strings to indices for MetaSound
    if (DinosaurType.Contains("Raptor")) return 0;
    if (DinosaurType.Contains("TRex")) return 1;
    if (DinosaurType.Contains("Triceratops")) return 2;
    if (DinosaurType.Contains("Brontosaurus")) return 3;
    if (DinosaurType.Contains("Pteranodon")) return 4;
    return 0; // Default
}

int32 UAudioSystemCore::GetSoundTypeIndex(const FString& SoundType) const
{
    // Map sound type strings to indices for MetaSound
    if (SoundType.Contains("Roar")) return 0;
    if (SoundType.Contains("Growl")) return 1;
    if (SoundType.Contains("Footstep")) return 2;
    if (SoundType.Contains("Breathing")) return 3;
    if (SoundType.Contains("Call")) return 4;
    return 0; // Default
}

FVector UAudioSystemCore::GetListenerLocation() const
{
    if (UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull))
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            if (APawn* PlayerPawn = PC->GetPawn())
            {
                return PlayerPawn->GetActorLocation();
            }
        }
    }
    return FVector::ZeroVector;
}