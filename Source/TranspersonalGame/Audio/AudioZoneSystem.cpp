// AudioZoneSystem.cpp
// Audio Agent #16 — PROD_CYCLE_AUTO_20260622_002
// Manages ambient audio zone transitions and event cues for the prehistoric survival world.
// Designed to work with MetaSounds when available; degrades gracefully without them.

#include "Audio/AudioZoneSystem.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UAudio_ZoneComponent::UAudio_ZoneComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Check every 100ms — not every frame
}

void UAudio_ZoneComponent::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultLayers();
    ZoneActiveTime = 0.0f;
    CrossfadeProgress = 1.0f;
    CrossfadeDurationRemaining = 0.0f;
    PreviousZone = EAudio_ZoneType::None;
}

void UAudio_ZoneComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bZoneActive)
    {
        return;
    }

    ZoneActiveTime += DeltaTime;

    // Advance crossfade
    if (CrossfadeDurationRemaining > 0.0f)
    {
        CrossfadeDurationRemaining -= DeltaTime;
        if (CrossfadeDurationRemaining <= 0.0f)
        {
            CrossfadeProgress = 1.0f;
            CrossfadeDurationRemaining = 0.0f;
            PreviousZone = EAudio_ZoneType::None;
        }
        else
        {
            // Linear crossfade — could be replaced with curve-based in future
            float TotalDuration = CrossfadeProgress > 0.0f ? CrossfadeDurationRemaining / CrossfadeProgress : 1.0f;
            CrossfadeProgress = 1.0f - (CrossfadeDurationRemaining / FMath::Max(TotalDuration, 0.001f));
        }
    }
}

void UAudio_ZoneComponent::TransitionToZone(EAudio_ZoneType NewZone, float CrossfadeDuration)
{
    if (NewZone == ActiveZone)
    {
        return; // Already in this zone
    }

    PreviousZone = ActiveZone;
    ActiveZone = NewZone;
    CrossfadeProgress = 0.0f;
    CrossfadeDurationRemaining = FMath::Max(CrossfadeDuration, 0.1f);
    ZoneActiveTime = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("AudioZone: Transitioning from %d to %d over %.1fs"),
        (int32)PreviousZone, (int32)ActiveZone, CrossfadeDuration);
}

void UAudio_ZoneComponent::FireEventCue(FName CueID)
{
    for (const FAudio_EventCue& Cue : EventCues)
    {
        if (Cue.CueID == CueID)
        {
            UE_LOG(LogTemp, Log, TEXT("AudioZone: Firing event cue '%s' (vol=%.2f, delay=%.1fs, dur=%.1fs)"),
                *CueID.ToString(), Cue.Volume, Cue.TriggerDelay, Cue.Duration);
            // MetaSound parameter dispatch would go here when MetaSounds are wired
            return;
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("AudioZone: Event cue '%s' not found"), *CueID.ToString());
}

bool UAudio_ZoneComponent::IsPlayerInZone() const
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return false;
    }

    UWorld* World = Owner->GetWorld();
    if (!World)
    {
        return false;
    }

    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (!PC || !PC->GetPawn())
    {
        return false;
    }

    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
    FVector ZoneLocation = Owner->GetActorLocation();
    float DistanceSq = FVector::DistSquared(PlayerLocation, ZoneLocation);

    return DistanceSq <= (ZoneRadius * ZoneRadius);
}

void UAudio_ZoneComponent::RegisterAmbientLayer(FAudio_AmbientLayer NewLayer)
{
    // Prevent duplicate layer IDs
    for (const FAudio_AmbientLayer& Existing : AmbientLayers)
    {
        if (Existing.LayerID == NewLayer.LayerID)
        {
            UE_LOG(LogTemp, Warning, TEXT("AudioZone: Layer '%s' already registered — skipping"), *NewLayer.LayerID.ToString());
            return;
        }
    }
    AmbientLayers.Add(NewLayer);
    UE_LOG(LogTemp, Log, TEXT("AudioZone: Registered ambient layer '%s' for zone type %d"),
        *NewLayer.LayerID.ToString(), (int32)NewLayer.ZoneType);
}

void UAudio_ZoneComponent::RegisterEventCue(FAudio_EventCue NewCue)
{
    // Prevent duplicate cue IDs
    for (const FAudio_EventCue& Existing : EventCues)
    {
        if (Existing.CueID == NewCue.CueID)
        {
            UE_LOG(LogTemp, Warning, TEXT("AudioZone: Cue '%s' already registered — skipping"), *NewCue.CueID.ToString());
            return;
        }
    }
    EventCues.Add(NewCue);
    UE_LOG(LogTemp, Log, TEXT("AudioZone: Registered event cue '%s' (trigger: '%s')"),
        *NewCue.CueID.ToString(), *NewCue.TriggerEvent.ToString());
}

void UAudio_ZoneComponent::InitializeDefaultLayers()
{
    // Default layers based on active zone type at BeginPlay
    // These are documentation stubs — actual USoundBase assets assigned in Blueprint/Editor

    switch (ActiveZone)
    {
        case EAudio_ZoneType::Camp:
        {
            // Campfire crackling — Freesound #856943 (FIREBurn_Campfire Forest Birds)
            FAudio_AmbientLayer CampfireLayer;
            CampfireLayer.LayerID = FName("Campfire_Crackle");
            CampfireLayer.ZoneType = EAudio_ZoneType::Camp;
            CampfireLayer.Volume = 0.7f;
            CampfireLayer.FadeInDuration = 1.5f;
            CampfireLayer.FadeOutDuration = 2.0f;
            CampfireLayer.bLooping = true;
            CampfireLayer.FreesoundID = TEXT("856943");
            AmbientLayers.Add(CampfireLayer);

            // Soft wind at camp perimeter
            FAudio_AmbientLayer WindLayer;
            WindLayer.LayerID = FName("Camp_Wind_Soft");
            WindLayer.ZoneType = EAudio_ZoneType::Camp;
            WindLayer.Volume = 0.3f;
            WindLayer.FadeInDuration = 3.0f;
            WindLayer.FadeOutDuration = 4.0f;
            WindLayer.bLooping = true;
            AmbientLayers.Add(WindLayer);

            // TRex quest accepted — tension sting cue
            FAudio_EventCue TRexQuestCue;
            TRexQuestCue.CueID = FName("TRex_Quest_Accepted");
            TRexQuestCue.TriggerEvent = FName("Quest_HuntTRex_Accepted");
            TRexQuestCue.Volume = 0.9f;
            TRexQuestCue.TriggerDelay = 0.5f;
            TRexQuestCue.Duration = 5.0f; // Matches FNarr_DialogueLine.DisplayDuration range
            EventCues.Add(TRexQuestCue);

            // Elder dialogue start cue
            FAudio_EventCue ElderDialogueCue;
            ElderDialogueCue.CueID = FName("Elder_Dialogue_Start");
            ElderDialogueCue.TriggerEvent = FName("Dialogue_Elder_Start");
            ElderDialogueCue.Volume = 0.6f;
            ElderDialogueCue.TriggerDelay = 0.0f;
            ElderDialogueCue.Duration = 4.0f;
            EventCues.Add(ElderDialogueCue);
            break;
        }

        case EAudio_ZoneType::TRexProximity:
        {
            // Ground rumble — low frequency, felt more than heard
            FAudio_AmbientLayer RumbleLayer;
            RumbleLayer.LayerID = FName("TRex_Ground_Rumble");
            RumbleLayer.ZoneType = EAudio_ZoneType::TRexProximity;
            RumbleLayer.Volume = 0.8f;
            RumbleLayer.FadeInDuration = 4.0f; // Slow build — player feels dread
            RumbleLayer.FadeOutDuration = 6.0f;
            RumbleLayer.bLooping = true;
            AmbientLayers.Add(RumbleLayer);

            // Silence of prey — reduce all other ambient layers (handled by zone transition)
            FAudio_EventCue TRexApproachCue;
            TRexApproachCue.CueID = FName("TRex_Approach_Warning");
            TRexApproachCue.TriggerEvent = FName("TRex_Within_400_Units");
            TRexApproachCue.Volume = 1.0f;
            TRexApproachCue.TriggerDelay = 0.0f;
            TRexApproachCue.Duration = 6.0f;
            EventCues.Add(TRexApproachCue);
            break;
        }

        case EAudio_ZoneType::Forest:
        default:
        {
            // Forest default — insects, wind, distant birds
            FAudio_AmbientLayer ForestLayer;
            ForestLayer.LayerID = FName("Forest_Insects_Wind");
            ForestLayer.ZoneType = EAudio_ZoneType::Forest;
            ForestLayer.Volume = 0.5f;
            ForestLayer.FadeInDuration = 2.0f;
            ForestLayer.FadeOutDuration = 3.0f;
            ForestLayer.bLooping = true;
            AmbientLayers.Add(ForestLayer);
            break;
        }
    }
}
