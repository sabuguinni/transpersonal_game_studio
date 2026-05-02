#include "Audio_SystemManager.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"

AAudio_SystemManager::AAudio_SystemManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create audio components
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    RootComponent = AmbientAudioComponent;

    NarrativeAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("NarrativeAudio"));
    NarrativeAudioComponent->SetupAttachment(RootComponent);

    EffectsAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("EffectsAudio"));
    EffectsAudioComponent->SetupAttachment(RootComponent);

    // Initialize default values
    MasterVolume = 1.0f;
    AmbientVolume = 0.7f;
    NarrativeVolume = 1.0f;
    EffectsVolume = 0.8f;
    FadeTime = 2.0f;

    CurrentAtmosphere = EAudio_AtmosphereType::Forest_Calm;
    CurrentTensionLevel = 0.0f;
    bIsNarrativePlaying = false;

    LastPlayerLocation = FVector::ZeroVector;
    TensionUpdateTimer = 0.0f;
    CurrentTriggerZoneIndex = -1;

    // Configure audio components
    AmbientAudioComponent->bAutoActivate = true;
    AmbientAudioComponent->SetVolumeMultiplier(AmbientVolume);

    NarrativeAudioComponent->bAutoActivate = false;
    NarrativeAudioComponent->SetVolumeMultiplier(NarrativeVolume);

    EffectsAudioComponent->bAutoActivate = false;
    EffectsAudioComponent->SetVolumeMultiplier(EffectsVolume);
}

void AAudio_SystemManager::BeginPlay()
{
    Super::BeginPlay();

    // Initialize default trigger zones and narrative clips
    InitializeDefaultTriggerZones();
    InitializeDefaultNarrativeClips();

    // Start with forest calm atmosphere
    SetAtmosphere(EAudio_AtmosphereType::Forest_Calm, false);
}

void AAudio_SystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update tension timer
    TensionUpdateTimer += DeltaTime;

    // Update audio state every 0.5 seconds
    if (TensionUpdateTimer >= 0.5f)
    {
        TensionUpdateTimer = 0.0f;

        // Get player location
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (PlayerPawn)
        {
            FVector PlayerLocation = PlayerPawn->GetActorLocation();
            UpdatePlayerLocation(PlayerLocation);
        }
    }

    // Update narrative playing status
    bIsNarrativePlaying = NarrativeAudioComponent->IsPlaying();
}

void AAudio_SystemManager::SetAtmosphere(EAudio_AtmosphereType NewAtmosphere, bool bFadeTransition)
{
    if (CurrentAtmosphere == NewAtmosphere)
    {
        return;
    }

    CurrentAtmosphere = NewAtmosphere;

    // Update ambient audio based on atmosphere
    UpdateAmbientAudio();

    UE_LOG(LogTemp, Log, TEXT("Audio System: Changed atmosphere to %d"), (int32)NewAtmosphere);
}

void AAudio_SystemManager::PlayNarrativeClip(const FString& ClipName, float Priority)
{
    FAudio_NarrativeClip* Clip = FindNarrativeClip(ClipName);
    if (!Clip)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio System: Narrative clip '%s' not found"), *ClipName);
        return;
    }

    // Check if already played and set to play once
    if (Clip->bPlayOnce && Clip->bHasPlayed)
    {
        UE_LOG(LogTemp, Log, TEXT("Audio System: Narrative clip '%s' already played"), *ClipName);
        return;
    }

    // Stop current narrative if lower priority
    if (bIsNarrativePlaying)
    {
        // For now, always interrupt with new clip
        StopNarrative();
    }

    // Load and play the sound
    if (Clip->AudioClip.IsValid())
    {
        USoundCue* SoundCue = Clip->AudioClip.LoadSynchronous();
        if (SoundCue)
        {
            NarrativeAudioComponent->SetSound(SoundCue);
            NarrativeAudioComponent->Play();
            Clip->bHasPlayed = true;
            bIsNarrativePlaying = true;

            UE_LOG(LogTemp, Log, TEXT("Audio System: Playing narrative clip '%s'"), *ClipName);
        }
    }
}

void AAudio_SystemManager::StopNarrative()
{
    if (NarrativeAudioComponent->IsPlaying())
    {
        NarrativeAudioComponent->Stop();
        bIsNarrativePlaying = false;
        UE_LOG(LogTemp, Log, TEXT("Audio System: Stopped narrative"));
    }
}

void AAudio_SystemManager::SetTensionLevel(float TensionLevel)
{
    CurrentTensionLevel = FMath::Clamp(TensionLevel, 0.0f, 1.0f);

    // Adjust ambient volume based on tension
    float TensionVolume = AmbientVolume * (1.0f + CurrentTensionLevel * 0.5f);
    AmbientAudioComponent->SetVolumeMultiplier(TensionVolume * MasterVolume);

    UE_LOG(LogTemp, VeryVerbose, TEXT("Audio System: Tension level set to %.2f"), CurrentTensionLevel);
}

void AAudio_SystemManager::UpdatePlayerLocation(FVector PlayerLocation)
{
    LastPlayerLocation = PlayerLocation;

    // Check for trigger zone changes
    int32 NewZoneIndex = GetActiveZoneForLocation(PlayerLocation);
    
    if (NewZoneIndex != CurrentTriggerZoneIndex && NewZoneIndex >= 0)
    {
        CurrentTriggerZoneIndex = NewZoneIndex;
        const FAudio_TriggerZone& Zone = TriggerZones[NewZoneIndex];
        
        // Change atmosphere based on zone
        SetAtmosphere(Zone.AtmosphereType, true);
        
        UE_LOG(LogTemp, Log, TEXT("Audio System: Entered zone %d, atmosphere: %d"), 
               NewZoneIndex, (int32)Zone.AtmosphereType);
    }

    // Update tension based on location
    UpdateTensionBasedOnLocation(PlayerLocation);
}

void AAudio_SystemManager::AddTriggerZone(FVector Location, float Radius, EAudio_AtmosphereType AtmosphereType)
{
    FAudio_TriggerZone NewZone;
    NewZone.Location = Location;
    NewZone.Radius = Radius;
    NewZone.AtmosphereType = AtmosphereType;
    NewZone.VolumeMultiplier = 1.0f;
    NewZone.bIsActive = true;

    TriggerZones.Add(NewZone);

    UE_LOG(LogTemp, Log, TEXT("Audio System: Added trigger zone at %s"), *Location.ToString());
}

void AAudio_SystemManager::RemoveTriggerZone(int32 ZoneIndex)
{
    if (TriggerZones.IsValidIndex(ZoneIndex))
    {
        TriggerZones.RemoveAt(ZoneIndex);
        UE_LOG(LogTemp, Log, TEXT("Audio System: Removed trigger zone %d"), ZoneIndex);
    }
}

int32 AAudio_SystemManager::GetActiveZoneForLocation(FVector Location)
{
    for (int32 i = 0; i < TriggerZones.Num(); i++)
    {
        const FAudio_TriggerZone& Zone = TriggerZones[i];
        if (Zone.bIsActive)
        {
            float Distance = FVector::Dist(Location, Zone.Location);
            if (Distance <= Zone.Radius)
            {
                return i;
            }
        }
    }
    return -1;
}

void AAudio_SystemManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    // Update all component volumes
    AmbientAudioComponent->SetVolumeMultiplier(AmbientVolume * MasterVolume);
    NarrativeAudioComponent->SetVolumeMultiplier(NarrativeVolume * MasterVolume);
    EffectsAudioComponent->SetVolumeMultiplier(EffectsVolume * MasterVolume);
}

void AAudio_SystemManager::SetAmbientVolume(float Volume)
{
    AmbientVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    AmbientAudioComponent->SetVolumeMultiplier(AmbientVolume * MasterVolume);
}

void AAudio_SystemManager::SetNarrativeVolume(float Volume)
{
    NarrativeVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    NarrativeAudioComponent->SetVolumeMultiplier(NarrativeVolume * MasterVolume);
}

void AAudio_SystemManager::SetEffectsVolume(float Volume)
{
    EffectsVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    EffectsAudioComponent->SetVolumeMultiplier(EffectsVolume * MasterVolume);
}

bool AAudio_SystemManager::IsNarrativePlaying() const
{
    return bIsNarrativePlaying;
}

float AAudio_SystemManager::GetCurrentTensionLevel() const
{
    return CurrentTensionLevel;
}

EAudio_AtmosphereType AAudio_SystemManager::GetCurrentAtmosphere() const
{
    return CurrentAtmosphere;
}

void AAudio_SystemManager::UpdateAmbientAudio()
{
    // This would load different ambient sounds based on atmosphere
    // For now, just log the change
    UE_LOG(LogTemp, Log, TEXT("Audio System: Updated ambient audio for atmosphere %d"), (int32)CurrentAtmosphere);
}

void AAudio_SystemManager::UpdateTensionBasedOnLocation(FVector PlayerLocation)
{
    // Calculate tension based on proximity to danger zones
    float MaxTension = 0.0f;

    for (const FAudio_TriggerZone& Zone : TriggerZones)
    {
        if (Zone.AtmosphereType == EAudio_AtmosphereType::Predator_Territory ||
            Zone.AtmosphereType == EAudio_AtmosphereType::Danger_Zone)
        {
            float Distance = FVector::Dist(PlayerLocation, Zone.Location);
            float TensionRadius = Zone.Radius * 1.5f; // Tension extends beyond zone
            
            if (Distance < TensionRadius)
            {
                float TensionValue = 1.0f - (Distance / TensionRadius);
                MaxTension = FMath::Max(MaxTension, TensionValue);
            }
        }
    }

    SetTensionLevel(MaxTension);
}

void AAudio_SystemManager::CrossfadeAmbientAudio(USoundCue* NewSound)
{
    if (NewSound && AmbientAudioComponent)
    {
        // Simple implementation - just change the sound
        // In a full implementation, this would do proper crossfading
        AmbientAudioComponent->SetSound(NewSound);
        if (!AmbientAudioComponent->IsPlaying())
        {
            AmbientAudioComponent->Play();
        }
    }
}

FAudio_NarrativeClip* AAudio_SystemManager::FindNarrativeClip(const FString& ClipName)
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

void AAudio_SystemManager::InitializeDefaultTriggerZones()
{
    // Add default trigger zones matching the map layout
    AddTriggerZone(FVector(500, 500, 100), 1000, EAudio_AtmosphereType::Forest_Calm);
    AddTriggerZone(FVector(-800, 200, 150), 800, EAudio_AtmosphereType::Predator_Territory);
    AddTriggerZone(FVector(1200, -600, 80), 600, EAudio_AtmosphereType::Water_Source);
    AddTriggerZone(FVector(0, 1000, 120), 700, EAudio_AtmosphereType::Danger_Zone);
    AddTriggerZone(FVector(-500, -800, 90), 400, EAudio_AtmosphereType::Safe_Zone);

    UE_LOG(LogTemp, Log, TEXT("Audio System: Initialized %d default trigger zones"), TriggerZones.Num());
}

void AAudio_SystemManager::InitializeDefaultNarrativeClips()
{
    // Initialize narrative clips with default settings
    FAudio_NarrativeClip AtmosphereClip;
    AtmosphereClip.ClipName = TEXT("AtmosphereNarrator");
    AtmosphereClip.Priority = 1.0f;
    AtmosphereClip.bPlayOnce = false;
    NarrativeClips.Add(AtmosphereClip);

    FAudio_NarrativeClip SurvivalClip;
    SurvivalClip.ClipName = TEXT("SurvivalGuide");
    SurvivalClip.Priority = 2.0f;
    SurvivalClip.bPlayOnce = true;
    NarrativeClips.Add(SurvivalClip);

    FAudio_NarrativeClip QuestClip;
    QuestClip.ClipName = TEXT("QuestNarrator");
    QuestClip.Priority = 1.5f;
    QuestClip.bPlayOnce = false;
    NarrativeClips.Add(QuestClip);

    FAudio_NarrativeClip WarningClip;
    WarningClip.ClipName = TEXT("FieldResearcher");
    WarningClip.Priority = 3.0f;
    WarningClip.bPlayOnce = false;
    NarrativeClips.Add(WarningClip);

    UE_LOG(LogTemp, Log, TEXT("Audio System: Initialized %d narrative clips"), NarrativeClips.Num());
}