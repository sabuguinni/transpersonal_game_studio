#include "AudioSystemArchitecture.h"
#include "AdaptiveMusicSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "AudioModulationStatics.h"

/**
 * Adaptive Music System Implementation
 * 
 * Creates a musical experience that the player never consciously notices
 * but that fundamentally changes how they feel about the world.
 * 
 * Music States:
 * - Exploration: Wonder with underlying tension
 * - Stalking: Minimal, focused, heart-rate synchronized
 * - Discovery: Awe mixed with caution  
 * - Threat: Building tension without telegraphing danger
 * - Panic: Chaotic but not overwhelming
 * - Stealth: Near-silence with micro-tensions
 */

UAdaptiveMusicSystem::UAdaptiveMusicSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    bWantsInitializeComponent = true;
    
    // Initialize music state parameters
    CurrentMusicState = EMusicState::Exploration;
    TransitionSpeed = 2.0f;
    TensionLevel = 0.0f;
    HeartRateSync = false;
    
    // Default music layers
    MusicLayers.Add("Ambient", nullptr);
    MusicLayers.Add("Tension", nullptr);
    MusicLayers.Add("Percussion", nullptr);
    MusicLayers.Add("Melody", nullptr);
    MusicLayers.Add("Drones", nullptr);
}

void UAdaptiveMusicSystem::InitializeComponent()
{
    Super::InitializeComponent();
    
    // Load MetaSound assets for adaptive music
    LoadMusicMetaSounds();
    
    // Start with exploration state
    TransitionToState(EMusicState::Exploration, 0.0f);
    
    // Begin music update timer
    GetWorld()->GetTimerManager().SetTimer(
        MusicUpdateTimer,
        this,
        &UAdaptiveMusicSystem::UpdateMusicSystem,
        0.1f, // Update 10 times per second
        true
    );
}

void UAdaptiveMusicSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Smooth parameter transitions
    UpdateParameterTransitions(DeltaTime);
}

void UAdaptiveMusicSystem::TransitionToState(EMusicState NewState, float TransitionTime)
{
    if (NewState == CurrentMusicState) return;
    
    EMusicState PreviousState = CurrentMusicState;
    CurrentMusicState = NewState;
    
    // Log state change for debugging
    UE_LOG(LogTemp, Log, TEXT("Music transitioning from %s to %s"), 
           *GetMusicStateName(PreviousState), 
           *GetMusicStateName(NewState));
    
    // Configure MetaSound parameters for new state
    ConfigureMusicParameters(NewState, TransitionTime);
    
    // Handle special transition logic
    HandleStateTransition(PreviousState, NewState, TransitionTime);
}

void UAdaptiveMusicSystem::UpdateThreatLevel(float NewThreatLevel)
{
    ThreatLevel = FMath::Clamp(NewThreatLevel, 0.0f, 1.0f);
    
    // Update MetaSound threat parameter
    if (MusicMetaSound && MusicAudioComponent)
    {
        MusicAudioComponent->SetFloatParameter(FName("ThreatLevel"), ThreatLevel);
    }
    
    // Determine appropriate music state based on threat level
    EMusicState TargetState = DetermineMusicStateFromThreat(ThreatLevel);
    
    if (TargetState != CurrentMusicState)
    {
        float TransitionTime = CalculateTransitionTime(CurrentMusicState, TargetState);
        TransitionToState(TargetState, TransitionTime);
    }
}

void UAdaptiveMusicSystem::SetHeartRateSync(bool bEnabled, float BPM)
{
    HeartRateSync = bEnabled;
    HeartRateBPM = FMath::Clamp(BPM, 60.0f, 180.0f);
    
    if (MusicMetaSound && MusicAudioComponent)
    {
        MusicAudioComponent->SetBoolParameter(FName("HeartRateSync"), HeartRateSync);
        MusicAudioComponent->SetFloatParameter(FName("HeartRateBPM"), HeartRateBPM);
    }
}

void UAdaptiveMusicSystem::SetEnvironmentType(EEnvironmentType Environment)
{
    CurrentEnvironment = Environment;
    
    // Update environmental music parameters
    if (MusicMetaSound && MusicAudioComponent)
    {
        MusicAudioComponent->SetIntParameter(FName("EnvironmentType"), (int32)Environment);
    }
    
    // Adjust reverb and spatial characteristics
    UpdateEnvironmentalAudio(Environment);
}

void UAdaptiveMusicSystem::ConfigureMusicParameters(EMusicState State, float TransitionTime)
{
    if (!MusicMetaSound || !MusicAudioComponent) return;
    
    // Set transition time
    MusicAudioComponent->SetFloatParameter(FName("TransitionTime"), TransitionTime);
    
    switch (State)
    {
        case EMusicState::Exploration:
            MusicAudioComponent->SetFloatParameter(FName("LayerMix_Ambient"), 0.8f);
            MusicAudioComponent->SetFloatParameter(FName("LayerMix_Tension"), 0.2f);
            MusicAudioComponent->SetFloatParameter(FName("LayerMix_Percussion"), 0.1f);
            MusicAudioComponent->SetFloatParameter(FName("LayerMix_Melody"), 0.6f);
            MusicAudioComponent->SetFloatParameter(FName("LayerMix_Drones"), 0.3f);
            MusicAudioComponent->SetFloatParameter(FName("Tempo"), 80.0f);
            break;
            
        case EMusicState::Stalking:
            MusicAudioComponent->SetFloatParameter(FName("LayerMix_Ambient"), 0.3f);
            MusicAudioComponent->SetFloatParameter(FName("LayerMix_Tension"), 0.7f);
            MusicAudioComponent->SetFloatParameter(FName("LayerMix_Percussion"), 0.0f);
            MusicAudioComponent->SetFloatParameter(FName("LayerMix_Melody"), 0.1f);
            MusicAudioComponent->SetFloatParameter(FName("LayerMix_Drones"), 0.8f);
            MusicAudioComponent->SetFloatParameter(FName("Tempo"), 60.0f);
            break;
            
        case EMusicState::Discovery:
            MusicAudioComponent->SetFloatParameter(FName("LayerMix_Ambient"), 0.6f);
            MusicAudioComponent->SetFloatParameter(FName("LayerMix_Tension"), 0.4f);
            MusicAudioComponent->SetFloatParameter(FName("LayerMix_Percussion"), 0.3f);
            MusicAudioComponent->SetFloatParameter(FName("LayerMix_Melody"), 0.9f);
            MusicAudioComponent->SetFloatParameter(FName("LayerMix_Drones"), 0.2f);
            MusicAudioComponent->SetFloatParameter(FName("Tempo"), 90.0f);
            break;
            
        case EMusicState::Threat:
            MusicAudioComponent->SetFloatParameter(FName("LayerMix_Ambient"), 0.2f);
            MusicAudioComponent->SetFloatParameter(FName("LayerMix_Tension"), 0.9f);
            MusicAudioComponent->SetFloatParameter(FName("LayerMix_Percussion"), 0.6f);
            MusicAudioComponent->SetFloatParameter(FName("LayerMix_Melody"), 0.3f);
            MusicAudioComponent->SetFloatParameter(FName("LayerMix_Drones"), 0.7f);
            MusicAudioComponent->SetFloatParameter(FName("Tempo"), 110.0f);
            break;
            
        case EMusicState::Panic:
            MusicAudioComponent->SetFloatParameter(FName("LayerMix_Ambient"), 0.0f);
            MusicAudioComponent->SetFloatParameter(FName("LayerMix_Tension"), 1.0f);
            MusicAudioComponent->SetFloatParameter(FName("LayerMix_Percussion"), 0.8f);
            MusicAudioComponent->SetFloatParameter(FName("LayerMix_Melody"), 0.2f);
            MusicAudioComponent->SetFloatParameter(FName("LayerMix_Drones"), 0.9f);
            MusicAudioComponent->SetFloatParameter(FName("Tempo"), 140.0f);
            break;
            
        case EMusicState::Stealth:
            MusicAudioComponent->SetFloatParameter(FName("LayerMix_Ambient"), 0.1f);
            MusicAudioComponent->SetFloatParameter(FName("LayerMix_Tension"), 0.3f);
            MusicAudioComponent->SetFloatParameter(FName("LayerMix_Percussion"), 0.0f);
            MusicAudioComponent->SetFloatParameter(FName("LayerMix_Melody"), 0.0f);
            MusicAudioComponent->SetFloatParameter(FName("LayerMix_Drones"), 0.5f);
            MusicAudioComponent->SetFloatParameter(FName("Tempo"), 50.0f);
            break;
    }
}

EMusicState UAdaptiveMusicSystem::DetermineMusicStateFromThreat(float Threat)
{
    if (Threat >= 0.9f) return EMusicState::Panic;
    if (Threat >= 0.7f) return EMusicState::Threat;
    if (Threat >= 0.4f) return EMusicState::Stalking;
    if (Threat >= 0.2f) return EMusicState::Discovery;
    return EMusicState::Exploration;
}

float UAdaptiveMusicSystem::CalculateTransitionTime(EMusicState From, EMusicState To)
{
    // Faster transitions for urgent states
    if (To == EMusicState::Panic) return 0.5f;
    if (To == EMusicState::Threat) return 1.0f;
    if (From == EMusicState::Panic) return 3.0f; // Slower recovery from panic
    
    return 2.0f; // Default transition time
}

void UAdaptiveMusicSystem::UpdateMusicSystem()
{
    // Calculate current threat level based on game state
    float CalculatedThreat = CalculateCurrentThreatLevel();
    
    // Update threat level (this may trigger state transitions)
    UpdateThreatLevel(CalculatedThreat);
    
    // Update time-of-day musical elements
    UpdateTimeOfDayMusic();
    
    // Update environmental musical elements
    UpdateEnvironmentalMusic();
}

float UAdaptiveMusicSystem::CalculateCurrentThreatLevel()
{
    float ThreatValue = 0.0f;
    
    // Get audio system manager
    UAudioSystemManager* AudioManager = GetAudioSystemManager();
    if (!AudioManager) return ThreatValue;
    
    const FAudioState& AudioState = AudioManager->GetCurrentAudioState();
    
    // Base threat from explicit threat level
    switch (AudioState.CurrentThreatLevel)
    {
        case EThreatLevel::Safe: ThreatValue = 0.0f; break;
        case EThreatLevel::Cautious: ThreatValue = 0.3f; break;
        case EThreatLevel::Danger: ThreatValue = 0.7f; break;
        case EThreatLevel::Panic: ThreatValue = 1.0f; break;
        case EThreatLevel::Stealth: ThreatValue = 0.5f; break;
    }
    
    // Modify based on nearby dinosaurs
    float DinosaurThreat = FMath::Min(AudioState.NearbyDinosaurs * 0.1f, 0.4f);
    ThreatValue += DinosaurThreat;
    
    // Modify based on time of day (night is more threatening)
    if (AudioState.TimeOfDay < 6.0f || AudioState.TimeOfDay > 20.0f)
    {
        ThreatValue += 0.2f;
    }
    
    // Modify based on weather
    ThreatValue += AudioState.WeatherIntensity * 0.1f;
    
    // Reduce threat if player is hidden
    if (AudioState.bPlayerHidden)
    {
        ThreatValue *= 0.6f;
    }
    
    return FMath::Clamp(ThreatValue, 0.0f, 1.0f);
}

void UAdaptiveMusicSystem::LoadMusicMetaSounds()
{
    // Load the main adaptive music MetaSound
    FString MetaSoundPath = TEXT("/Game/Audio/MetaSounds/MS_AdaptiveMusic");
    MusicMetaSound = LoadObject<UMetaSoundSource>(nullptr, *MetaSoundPath);
    
    if (!MusicMetaSound)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load adaptive music MetaSound at %s"), *MetaSoundPath);
    }
}

FString UAdaptiveMusicSystem::GetMusicStateName(EMusicState State)
{
    switch (State)
    {
        case EMusicState::Exploration: return TEXT("Exploration");
        case EMusicState::Stalking: return TEXT("Stalking");
        case EMusicState::Discovery: return TEXT("Discovery");
        case EMusicState::Threat: return TEXT("Threat");
        case EMusicState::Panic: return TEXT("Panic");
        case EMusicState::Stealth: return TEXT("Stealth");
        default: return TEXT("Unknown");
    }
}