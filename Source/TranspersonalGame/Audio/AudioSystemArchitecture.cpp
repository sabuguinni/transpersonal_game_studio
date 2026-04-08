#include "AudioSystemArchitecture.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "MetasoundParameterPack.h"

UAudioSystemManager::UAudioSystemManager()
{
    // Initialize default state
    CurrentAudioState = FAudioStateData();
}

void UAudioSystemManager::InitializeAudioSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("Audio System Manager: Initializing adaptive audio system"));

    // Create audio components for each layer
    if (UWorld* World = GetWorld())
    {
        // Music layer - handles emotional state and tension
        if (!MusicAudioComponent)
        {
            MusicAudioComponent = UGameplayStatics::CreateSound2D(World, nullptr);
            if (MusicAudioComponent)
            {
                MusicAudioComponent->bAutoDestroy = false;
                MusicAudioComponent->VolumeMultiplier = 0.7f; // Music should support, not dominate
            }
        }

        // Ambience layer - environmental sounds and creature presence
        if (!AmbienceAudioComponent)
        {
            AmbienceAudioComponent = UGameplayStatics::CreateSound2D(World, nullptr);
            if (AmbienceAudioComponent)
            {
                AmbienceAudioComponent->bAutoDestroy = false;
                AmbienceAudioComponent->VolumeMultiplier = 0.8f;
            }
        }

        // Creature layer - specific creature behaviors and vocalizations
        if (!CreatureAudioComponent)
        {
            CreatureAudioComponent = UGameplayStatics::CreateSound2D(World, nullptr);
            if (CreatureAudioComponent)
            {
                CreatureAudioComponent->bAutoDestroy = false;
                CreatureAudioComponent->VolumeMultiplier = 0.9f;
            }
        }
    }

    // Initialize with safe exploration state
    SetTensionLevel(EAudioTensionLevel::Safe);
}

void UAudioSystemManager::UpdateAudioState(const FAudioStateData& NewState)
{
    FAudioStateData PreviousState = CurrentAudioState;
    CurrentAudioState = NewState;

    // Check for significant state changes that require audio transitions
    if (PreviousState.TensionLevel != NewState.TensionLevel)
    {
        TriggerMusicTransition(NewState.TensionLevel);
    }

    if (PreviousState.Environment != NewState.Environment)
    {
        TransitionToEnvironment(NewState.Environment);
    }

    // Update MetaSound parameters
    UpdateMusicParameters();
    UpdateAmbienceParameters();

    // Check if we should trigger unnatural silence
    CalculateOptimalSilenceMoments();
}

void UAudioSystemManager::SetTensionLevel(EAudioTensionLevel NewTension)
{
    if (CurrentAudioState.TensionLevel != NewTension)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio System: Tension level changed from %d to %d"), 
               (int32)CurrentAudioState.TensionLevel, (int32)NewTension);
        
        CurrentAudioState.TensionLevel = NewTension;
        TriggerMusicTransition(NewTension);
    }
}

void UAudioSystemManager::TransitionToEnvironment(EEnvironmentalState NewEnvironment, float TransitionTime)
{
    UE_LOG(LogTemp, Warning, TEXT("Audio System: Transitioning to environment %d"), (int32)NewEnvironment);
    
    CurrentAudioState.Environment = NewEnvironment;
    
    // Update ambience MetaSound parameters based on new environment
    if (AmbienceAudioComponent && EnvironmentalAmbienceMetaSound)
    {
        // Create parameter pack for environmental transition
        UMetasoundParameterPack* ParamPack = NewObject<UMetasoundParameterPack>();
        
        // Set environment-specific parameters
        switch (NewEnvironment)
        {
            case EEnvironmentalState::Forest_Dense:
                ParamPack->SetFloat("TreeDensity", 0.9f);
                ParamPack->SetFloat("BirdActivity", 0.7f);
                ParamPack->SetFloat("WindIntensity", 0.3f);
                break;
                
            case EEnvironmentalState::Forest_Clearing:
                ParamPack->SetFloat("TreeDensity", 0.3f);
                ParamPack->SetFloat("BirdActivity", 0.9f);
                ParamPack->SetFloat("WindIntensity", 0.6f);
                break;
                
            case EEnvironmentalState::Riverside:
                ParamPack->SetFloat("WaterFlow", 0.8f);
                ParamPack->SetFloat("BirdActivity", 0.5f);
                ParamPack->SetFloat("InsectActivity", 0.7f);
                break;
                
            case EEnvironmentalState::Caves:
                ParamPack->SetFloat("Reverb", 0.9f);
                ParamPack->SetFloat("Dripping", 0.6f);
                ParamPack->SetFloat("EchoDelay", 0.8f);
                break;
        }
        
        ParamPack->SetFloat("TransitionTime", TransitionTime);
        AmbienceAudioComponent->SetParameterPack(ParamPack);
    }
}

void UAudioSystemManager::RegisterCreaturePresence(class ADinosaurBase* Creature, float Distance)
{
    if (Creature)
    {
        NearbyCreatures.AddUnique(Creature);
        
        // Update creature density parameter
        CurrentAudioState.NearbyCreatureDensity = FMath::Clamp(
            static_cast<float>(NearbyCreatures.Num()) / 10.0f, 0.0f, 1.0f
        );
        
        UE_LOG(LogTemp, Log, TEXT("Audio System: Creature registered at distance %.2f. Density: %.2f"), 
               Distance, CurrentAudioState.NearbyCreatureDensity);
    }
}

void UAudioSystemManager::UnregisterCreaturePresence(class ADinosaurBase* Creature)
{
    if (Creature)
    {
        NearbyCreatures.RemoveAll([Creature](const TWeakObjectPtr<ADinosaurBase>& WeakPtr) {
            return !WeakPtr.IsValid() || WeakPtr.Get() == Creature;
        });
        
        // Update creature density
        CurrentAudioState.NearbyCreatureDensity = FMath::Clamp(
            static_cast<float>(NearbyCreatures.Num()) / 10.0f, 0.0f, 1.0f
        );
    }
}

void UAudioSystemManager::TriggerMusicTransition(EAudioTensionLevel TargetTension, float TransitionTime)
{
    if (MusicAudioComponent && AdaptiveMusicMetaSound)
    {
        UMetasoundParameterPack* ParamPack = NewObject<UMetasoundParameterPack>();
        
        // Map tension levels to musical parameters
        switch (TargetTension)
        {
            case EAudioTensionLevel::Safe:
                ParamPack->SetFloat("TensionLevel", 0.0f);
                ParamPack->SetFloat("HeartRate", 60.0f);
                ParamPack->SetFloat("DissonanceAmount", 0.1f);
                ParamPack->SetBool("EnableMelody", true);
                break;
                
            case EAudioTensionLevel::Cautious:
                ParamPack->SetFloat("TensionLevel", 0.3f);
                ParamPack->SetFloat("HeartRate", 80.0f);
                ParamPack->SetFloat("DissonanceAmount", 0.3f);
                ParamPack->SetBool("EnableMelody", true);
                break;
                
            case EAudioTensionLevel::Alert:
                ParamPack->SetFloat("TensionLevel", 0.7f);
                ParamPack->SetFloat("HeartRate", 120.0f);
                ParamPack->SetFloat("DissonanceAmount", 0.6f);
                ParamPack->SetBool("EnableMelody", false);
                break;
                
            case EAudioTensionLevel::Panic:
                ParamPack->SetFloat("TensionLevel", 1.0f);
                ParamPack->SetFloat("HeartRate", 160.0f);
                ParamPack->SetFloat("DissonanceAmount", 0.9f);
                ParamPack->SetBool("EnableMelody", false);
                break;
                
            case EAudioTensionLevel::Silent:
                // Fade everything to silence
                ParamPack->SetFloat("MasterVolume", 0.0f);
                ParamPack->SetFloat("SilenceDuration", 5.0f);
                break;
        }
        
        ParamPack->SetFloat("TransitionTime", TransitionTime);
        MusicAudioComponent->SetParameterPack(ParamPack);
        
        UE_LOG(LogTemp, Warning, TEXT("Audio System: Music transition triggered to tension level %d"), 
               (int32)TargetTension);
    }
}

void UAudioSystemManager::TriggerUnnaturalSilence(float Duration)
{
    UE_LOG(LogTemp, Warning, TEXT("Audio System: Triggering unnatural silence for %.2f seconds"), Duration);
    
    bInSilentMode = true;
    SilenceStartTime = GetWorld()->GetTimeSeconds();
    SilenceDuration = Duration;
    
    // Fade all audio layers to silence
    if (MusicAudioComponent)
    {
        MusicAudioComponent->FadeOut(1.0f, 0.0f);
    }
    
    if (AmbienceAudioComponent)
    {
        AmbienceAudioComponent->FadeOut(1.5f, 0.0f);
    }
    
    if (CreatureAudioComponent)
    {
        CreatureAudioComponent->FadeOut(0.5f, 0.0f);
    }
    
    // Set timer to break silence
    if (UWorld* World = GetWorld())
    {
        FTimerHandle SilenceTimer;
        World->GetTimerManager().SetTimer(SilenceTimer, [this]()
        {
            BreakSilence(true); // Break with potential jump scare
        }, Duration, false);
    }
}

void UAudioSystemManager::BreakSilence(bool bWithJumpScare)
{
    if (!bInSilentMode) return;
    
    UE_LOG(LogTemp, Warning, TEXT("Audio System: Breaking silence %s"), 
           bWithJumpScare ? TEXT("with jump scare") : TEXT("naturally"));
    
    bInSilentMode = false;
    
    if (bWithJumpScare)
    {
        // Sudden audio spike before returning to normal
        SetTensionLevel(EAudioTensionLevel::Panic);
        
        // Return to appropriate tension level after jump scare
        if (UWorld* World = GetWorld())
        {
            FTimerHandle ReturnTimer;
            World->GetTimerManager().SetTimer(ReturnTimer, [this]()
            {
                SetTensionLevel(EAudioTensionLevel::Alert);
            }, 2.0f, false);
        }
    }
    else
    {
        // Gradual return to ambient audio
        if (MusicAudioComponent)
        {
            MusicAudioComponent->FadeIn(3.0f, 0.7f);
        }
        
        if (AmbienceAudioComponent)
        {
            AmbienceAudioComponent->FadeIn(4.0f, 0.8f);
        }
    }
}

void UAudioSystemManager::UpdateMusicParameters()
{
    if (MusicAudioComponent && AdaptiveMusicMetaSound)
    {
        UMetasoundParameterPack* ParamPack = NewObject<UMetasoundParameterPack>();
        
        // Update continuous parameters
        ParamPack->SetFloat("PlayerStealthLevel", CurrentAudioState.PlayerStealthLevel);
        ParamPack->SetFloat("CreatureDensity", CurrentAudioState.NearbyCreatureDensity);
        ParamPack->SetFloat("TimeOfDay", static_cast<float>(CurrentAudioState.TimeOfDay) / 6.0f);
        ParamPack->SetBool("PlayerHidden", CurrentAudioState.bIsPlayerHidden);
        ParamPack->SetBool("RecentCombat", CurrentAudioState.bRecentCombat);
        
        MusicAudioComponent->SetParameterPack(ParamPack);
    }
}

void UAudioSystemManager::UpdateAmbienceParameters()
{
    if (AmbienceAudioComponent && EnvironmentalAmbienceMetaSound)
    {
        UMetasoundParameterPack* ParamPack = NewObject<UMetasoundParameterPack>();
        
        // Environmental parameters
        ParamPack->SetFloat("EnvironmentType", static_cast<float>(CurrentAudioState.Environment));
        ParamPack->SetFloat("TimeOfDay", static_cast<float>(CurrentAudioState.TimeOfDay) / 6.0f);
        ParamPack->SetFloat("CreatureActivity", CurrentAudioState.NearbyCreatureDensity);
        
        AmbienceAudioComponent->SetParameterPack(ParamPack);
    }
}

void UAudioSystemManager::CalculateOptimalSilenceMoments()
{
    // Logic to determine when unnatural silence would be most effective
    // Based on current tension, recent activity, and player behavior
    
    if (bInSilentMode) return; // Already in silence
    
    bool bShouldTriggerSilence = false;
    
    // Trigger silence when transitioning from high activity to calm
    static EAudioTensionLevel LastTensionLevel = EAudioTensionLevel::Safe;
    if (LastTensionLevel >= EAudioTensionLevel::Alert && 
        CurrentAudioState.TensionLevel <= EAudioTensionLevel::Cautious)
    {
        // Player might think they're safe - perfect time for silence
        if (FMath::RandRange(0.0f, 1.0f) < 0.3f) // 30% chance
        {
            bShouldTriggerSilence = true;
        }
    }
    
    // Trigger silence in dense forest when player is moving slowly
    if (CurrentAudioState.Environment == EEnvironmentalState::Forest_Dense &&
        CurrentAudioState.PlayerStealthLevel > 0.7f &&
        CurrentAudioState.NearbyCreatureDensity < 0.2f)
    {
        if (FMath::RandRange(0.0f, 1.0f) < 0.15f) // 15% chance
        {
            bShouldTriggerSilence = true;
        }
    }
    
    if (bShouldTriggerSilence)
    {
        float SilenceDuration = FMath::RandRange(3.0f, 8.0f);
        TriggerUnnaturalSilence(SilenceDuration);
    }
    
    LastTensionLevel = CurrentAudioState.TensionLevel;
}