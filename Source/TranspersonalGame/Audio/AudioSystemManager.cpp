#include "AudioSystemManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

AAudio_SystemManager::AAudio_SystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    CurrentEnvironmentState = EAudio_EnvironmentState::Calm;
    TargetEnvironmentState = EAudio_EnvironmentState::Calm;

    // Pre-populate voice line URL catalog from ElevenLabs TTS (Cycle 004-005)
    // Elder_Survivor — survival wisdom NPC
    VoiceLineURLCatalog.Add(
        TEXT("Elder_Survivor_TRexStealth"),
        TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782717886789_Elder_Survivor.mp3")
    );
    VoiceLineURLCatalog.Add(
        TEXT("Elder_Survivor_FireWisdom"),
        TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782717907901_Elder_Survivor.mp3")
    );
    // Scout_NPC — environmental hazard warnings (Cycle 004)
    VoiceLineURLCatalog.Add(
        TEXT("Scout_NPC_RiverWarning"),
        TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/Scout_NPC_river_warning.mp3")
    );
    // Narrator — ambient tension lines (Cycle 003)
    VoiceLineURLCatalog.Add(
        TEXT("Narrator_SilenceWarning"),
        TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/Narrator_silence_warning.mp3")
    );
}

void AAudio_SystemManager::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: BeginPlay — environment state: Calm"));
    UpdateAmbientMix();
}

void AAudio_SystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateDinosaurAudioState(DeltaTime);
    UpdateDayNightAudio();
    UpdateHeartbeat(DeltaTime);

    // Handle state transitions
    if (bIsTransitioning)
    {
        StateTransitionTimer += DeltaTime;
        if (StateTransitionTimer >= StateTransitionDuration)
        {
            CurrentEnvironmentState = TargetEnvironmentState;
            bIsTransitioning = false;
            StateTransitionTimer = 0.0f;
            UpdateAmbientMix();
            UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Transition complete → state %d"),
                (int32)CurrentEnvironmentState);
        }
    }
}

void AAudio_SystemManager::SetEnvironmentState(EAudio_EnvironmentState NewState)
{
    if (NewState == CurrentEnvironmentState) return;
    CurrentEnvironmentState = NewState;
    UpdateAmbientMix();
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: State set to %d"), (int32)NewState);
}

void AAudio_SystemManager::TransitionToState(EAudio_EnvironmentState NewState, float TransitionTime)
{
    if (NewState == CurrentEnvironmentState) return;
    TargetEnvironmentState = NewState;
    StateTransitionDuration = FMath::Max(0.1f, TransitionTime);
    StateTransitionTimer = 0.0f;
    bIsTransitioning = true;
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Transitioning to state %d over %.1fs"),
        (int32)NewState, TransitionTime);
}

void AAudio_SystemManager::RegisterAmbientLayer(FAudio_AmbientLayer Layer)
{
    RegisteredAmbientLayers.Add(Layer);
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Registered ambient layer for state %d"),
        (int32)Layer.ActiveInState);
}

void AAudio_SystemManager::UpdateAmbientMix()
{
    // In full implementation: fade in/out audio components based on CurrentEnvironmentState
    // For now: log which layers are active
    int32 ActiveCount = 0;
    for (const FAudio_AmbientLayer& Layer : RegisteredAmbientLayers)
    {
        if (Layer.ActiveInState == CurrentEnvironmentState)
        {
            ActiveCount++;
        }
    }
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: UpdateAmbientMix — %d layers active for state %d"),
        ActiveCount, (int32)CurrentEnvironmentState);
}

void AAudio_SystemManager::RegisterVoiceLine(FAudio_VoiceLine VoiceLine)
{
    RegisteredVoiceLines.Add(VoiceLine);
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Registered voice line for %s"),
        *VoiceLine.CharacterName);
}

void AAudio_SystemManager::TriggerVoiceLine(const FString& CharacterName, const FString& Condition)
{
    FString Key = CharacterName + TEXT("_") + Condition;
    FString* URL = VoiceLineURLCatalog.Find(Key);
    if (URL)
    {
        UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Triggering voice line [%s] → %s"),
            *Key, **URL);
        // In full implementation: stream audio from URL or play pre-loaded SoundWave
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Voice line not found for key [%s]"), *Key);
    }
}

void AAudio_SystemManager::NotifyDinosaurProximity(float DistanceMeters, FName DinosaurSpecies)
{
    NearestDinosaurDistance = DistanceMeters;
    NearestDinosaurSpecies = DinosaurSpecies;

    // Adaptive state based on proximity
    if (DistanceMeters < 20.0f)
    {
        TransitionToState(EAudio_EnvironmentState::Combat, 0.5f);
        HeartbeatIntensity = 1.0f;
    }
    else if (DistanceMeters < 60.0f)
    {
        TransitionToState(EAudio_EnvironmentState::Alert, 1.5f);
        HeartbeatIntensity = FMath::GetMappedRangeValueClamped(
            FVector2D(20.0f, 60.0f), FVector2D(1.0f, 0.3f), DistanceMeters);
    }
    else
    {
        HeartbeatIntensity = FMath::GetMappedRangeValueClamped(
            FVector2D(60.0f, 150.0f), FVector2D(0.3f, 0.0f), DistanceMeters);
    }

    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Dinosaur [%s] at %.1fm — heartbeat %.2f"),
        *DinosaurSpecies.ToString(), DistanceMeters, HeartbeatIntensity);
}

void AAudio_SystemManager::NotifyDinosaurLost()
{
    NearestDinosaurDistance = 9999.0f;
    NearestDinosaurSpecies = NAME_None;
    HeartbeatIntensity = 0.0f;
    TransitionToState(EAudio_EnvironmentState::Calm, 4.0f);
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Dinosaur lost — transitioning to Calm"));
}

void AAudio_SystemManager::PlayHeartbeatTension(float IntensityNormalized)
{
    HeartbeatIntensity = FMath::Clamp(IntensityNormalized, 0.0f, 1.0f);
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Heartbeat intensity → %.2f"), HeartbeatIntensity);
}

void AAudio_SystemManager::PlayDamageImpact(float DamageAmount)
{
    float NormalizedDamage = FMath::Clamp(DamageAmount / 100.0f, 0.0f, 1.0f);
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Damage impact — %.1f pts (normalized %.2f)"),
        DamageAmount, NormalizedDamage);
    // In full implementation: play impact SoundCue with volume scaled to damage
}

void AAudio_SystemManager::PlayCraftingSound(FName ToolType)
{
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Crafting sound — tool [%s]"),
        *ToolType.ToString());
    // Map tool types to sound cues: Stone→flint_knap, Wood→wood_chop, Bone→bone_crack
}

void AAudio_SystemManager::PlayFootstep(FName SurfaceType)
{
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Footstep on [%s]"),
        *SurfaceType.ToString());
    // Map surface types: Dirt, Grass, Rock, Water, Mud, Leaves
}

void AAudio_SystemManager::SetTimeOfDay(float NormalizedTime)
{
    CurrentTimeOfDay = FMath::Clamp(NormalizedTime, 0.0f, 1.0f);
    UpdateDayNightAudio();
}

void AAudio_SystemManager::UpdateDinosaurAudioState(float DeltaTime)
{
    // Gradually reduce heartbeat if no dinosaur notification received
    if (NearestDinosaurDistance > 150.0f && HeartbeatIntensity > 0.0f)
    {
        HeartbeatIntensity = FMath::Max(0.0f, HeartbeatIntensity - DeltaTime * 0.1f);
    }
}

void AAudio_SystemManager::UpdateDayNightAudio()
{
    // 0.0 = midnight, 0.25 = dawn, 0.5 = noon, 0.75 = dusk, 1.0 = midnight
    bool bIsNight = (CurrentTimeOfDay < 0.2f || CurrentTimeOfDay > 0.8f);
    if (bIsNight && CurrentEnvironmentState == EAudio_EnvironmentState::Calm)
    {
        // Night ambient: crickets, distant howls — Freesound ID 516157
        // "Tropical Ambience Night Ecuador.wav" — crickets, frogs, insects
        SetEnvironmentState(EAudio_EnvironmentState::Night);
    }
    else if (!bIsNight && CurrentEnvironmentState == EAudio_EnvironmentState::Night)
    {
        SetEnvironmentState(EAudio_EnvironmentState::Calm);
    }
}

void AAudio_SystemManager::UpdateHeartbeat(float DeltaTime)
{
    // Heartbeat audio logic — drives MetaSound parameter "HeartbeatRate"
    // Rate increases with intensity: 60 BPM (calm) → 180 BPM (full terror)
    float HeartbeatBPM = FMath::Lerp(60.0f, 180.0f, HeartbeatIntensity);
    // In full implementation: set MetaSound parameter via UAudioComponent
    (void)HeartbeatBPM; // Suppress unused warning until MetaSound integration
}
