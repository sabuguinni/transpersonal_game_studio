#include "AudioSystemManager.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UAudioSystemManager::UAudioSystemManager()
{
    CurrentAudioState = EAudioState::Calm;
    CurrentEnvironment = EEnvironmentType::DenseForest;
    CurrentMusicIntensity = 0.3f;
}

void UAudioSystemManager::InitializeAudioSystem()
{
    InitializeMetaSounds();
    SetupAudioComponents();
    
    // Start with calm ambient audio
    UpdateAudioState(EAudioState::Calm, 0.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Audio System Initialized - Jurassic Survival Audio Engine"));
}

void UAudioSystemManager::UpdateAudioState(EAudioState NewState, float TransitionTime)
{
    if (CurrentAudioState == NewState) return;
    
    EAudioState PreviousState = CurrentAudioState;
    CurrentAudioState = NewState;
    
    // Trigger music transition based on state change
    TriggerMusicTransition(NewState, TransitionTime <= 0.1f);
    
    // Update MetaSound parameters
    UpdateMetaSoundParameters();
    
    // Log state change for debugging
    UE_LOG(LogTemp, Log, TEXT("Audio State Changed: %d -> %d (Transition: %.2fs)"), 
           (int32)PreviousState, (int32)NewState, TransitionTime);
}

void UAudioSystemManager::SetEnvironmentType(EEnvironmentType Environment)
{
    if (CurrentEnvironment == Environment) return;
    
    CurrentEnvironment = Environment;
    
    // Update ambient layers for new environment
    if (AmbientAudioComponent && AmbientLayersMetaSound)
    {
        AmbientAudioComponent->SetFloatParameter(FName("EnvironmentType"), (float)Environment);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Environment Changed to: %d"), (int32)Environment);
}

void UAudioSystemManager::TriggerMusicTransition(EAudioState TargetState, bool bForceImmediate)
{
    if (!MusicAudioComponent || !MasterMusicMetaSound) return;
    
    float TransitionTime = bForceImmediate ? 0.1f : 3.0f;
    
    // Set MetaSound parameters for music transition
    MusicAudioComponent->SetFloatParameter(FName("TargetState"), (float)TargetState);
    MusicAudioComponent->SetFloatParameter(FName("TransitionTime"), TransitionTime);
    MusicAudioComponent->SetBoolParameter(FName("TriggerTransition"), true);
    
    // Adjust intensity based on state
    switch (TargetState)
    {
        case EAudioState::Calm:
            SetMusicIntensity(0.2f);
            break;
        case EAudioState::Tense:
            SetMusicIntensity(0.5f);
            break;
        case EAudioState::Danger:
            SetMusicIntensity(0.8f);
            break;
        case EAudioState::Combat:
            SetMusicIntensity(1.0f);
            break;
        case EAudioState::Discovery:
            SetMusicIntensity(0.6f);
            break;
        case EAudioState::Domestication:
            SetMusicIntensity(0.4f);
            break;
        case EAudioState::Exploration:
            SetMusicIntensity(0.3f);
            break;
    }
}

void UAudioSystemManager::SetMusicIntensity(float Intensity)
{
    CurrentMusicIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    
    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetFloatParameter(FName("Intensity"), CurrentMusicIntensity);
    }
}

void UAudioSystemManager::UpdateAmbientLayers(EEnvironmentType Environment, float TimeOfDay, float WeatherIntensity)
{
    if (!AmbientAudioComponent || !AmbientLayersMetaSound) return;
    
    // Update all ambient parameters
    AmbientAudioComponent->SetFloatParameter(FName("EnvironmentType"), (float)Environment);
    AmbientAudioComponent->SetFloatParameter(FName("TimeOfDay"), TimeOfDay); // 0.0 = dawn, 0.5 = noon, 1.0 = night
    AmbientAudioComponent->SetFloatParameter(FName("WeatherIntensity"), WeatherIntensity);
    
    // Environmental-specific adjustments
    switch (Environment)
    {
        case EEnvironmentType::DenseForest:
            AmbientAudioComponent->SetFloatParameter(FName("ForestDensity"), 1.0f);
            AmbientAudioComponent->SetFloatParameter(FName("OpenSpace"), 0.2f);
            break;
        case EEnvironmentType::OpenPlains:
            AmbientAudioComponent->SetFloatParameter(FName("ForestDensity"), 0.1f);
            AmbientAudioComponent->SetFloatParameter(FName("OpenSpace"), 1.0f);
            break;
        case EEnvironmentType::RiverBanks:
            AmbientAudioComponent->SetFloatParameter(FName("WaterProximity"), 1.0f);
            break;
        case EEnvironmentType::Caves:
            AmbientAudioComponent->SetFloatParameter(FName("Reverb"), 0.8f);
            AmbientAudioComponent->SetFloatParameter(FName("Echo"), 0.6f);
            break;
    }
}

void UAudioSystemManager::PlayDinosaurCall(class ADinosaurCharacter* Dinosaur, FString CallType)
{
    if (!Dinosaur || !DinosaurAudioMetaSound) return;
    
    // Create temporary audio component for dinosaur call
    UAudioComponent* DinosaurCallComponent = UGameplayStatics::SpawnSoundAtLocation(
        Dinosaur->GetWorld(),
        DinosaurAudioMetaSound,
        Dinosaur->GetActorLocation(),
        FRotator::ZeroRotator,
        1.0f, // Volume
        1.0f, // Pitch
        0.0f, // Start time
        nullptr, // Attenuation settings (will use MetaSound's built-in)
        nullptr, // Concurrency settings
        true    // Auto destroy
    );
    
    if (DinosaurCallComponent)
    {
        // Set dinosaur-specific parameters
        DinosaurCallComponent->SetStringParameter(FName("CallType"), CallType);
        DinosaurCallComponent->SetFloatParameter(FName("DinosaurSize"), Dinosaur->GetDinosaurSize());
        DinosaurCallComponent->SetBoolParameter(FName("IsAggressive"), Dinosaur->IsAggressive());
    }
}

void UAudioSystemManager::UpdateDinosaurProximityAudio(TArray<class ADinosaurCharacter*> NearbyDinosaurs)
{
    if (!MusicAudioComponent) return;
    
    float TotalThreatLevel = 0.0f;
    int32 PredatorCount = 0;
    
    for (ADinosaurCharacter* Dinosaur : NearbyDinosaurs)
    {
        if (Dinosaur && Dinosaur->IsPredator())
        {
            float Distance = FVector::Dist(Dinosaur->GetActorLocation(), 
                                         UGameplayStatics::GetPlayerPawn(Dinosaur->GetWorld(), 0)->GetActorLocation());
            
            // Closer predators increase threat more
            float ThreatContribution = FMath::Clamp(1.0f - (Distance / 2000.0f), 0.0f, 1.0f);
            TotalThreatLevel += ThreatContribution;
            PredatorCount++;
        }
    }
    
    // Update music system with proximity threat
    MusicAudioComponent->SetFloatParameter(FName("ProximityThreat"), TotalThreatLevel);
    MusicAudioComponent->SetIntParameter(FName("NearbyPredators"), PredatorCount);
}

void UAudioSystemManager::InitializeMetaSounds()
{
    // MetaSounds will be assigned in Blueprint or through asset references
    // This is where we'd load the MetaSound assets if needed
}

void UAudioSystemManager::SetupAudioComponents()
{
    // Audio components will be created and configured
    // This would typically be done in BeginPlay of the owning actor
}

void UAudioSystemManager::UpdateMetaSoundParameters()
{
    // Update all MetaSound parameters based on current state
    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetFloatParameter(FName("CurrentState"), (float)CurrentAudioState);
        MusicAudioComponent->SetFloatParameter(FName("StateIntensity"), CurrentMusicIntensity);
    }
    
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->SetFloatParameter(FName("CurrentState"), (float)CurrentAudioState);
        AmbientAudioComponent->SetFloatParameter(FName("Environment"), (float)CurrentEnvironment);
    }
}