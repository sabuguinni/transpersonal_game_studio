#include "AudioZoneManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

AAudio_ZoneManager::AAudio_ZoneManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.5f;
}

void AAudio_ZoneManager::BeginPlay()
{
    Super::BeginPlay();
    InitialiseZoneAmbience();
    InitialiseDinoProfiles();
    InitialiseDangerCues();
    UE_LOG(LogTemp, Log, TEXT("AAudio_ZoneManager: Initialised — zones=%d dinos=%d danger_cues=%d"),
        ZoneAmbienceProfiles.Num(), DinoSoundProfiles.Num(), DangerCues.Num());
}

void AAudio_ZoneManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TimeSinceLastProximityCheck += DeltaTime;
    if (TimeSinceLastProximityCheck >= ProximityCheckInterval)
    {
        TimeSinceLastProximityCheck = 0.0f;
        UWorld* World = GetWorld();
        if (!World) return;

        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
        if (PlayerPawn)
        {
            CheckProximityDangerCues(PlayerPawn->GetActorLocation());
        }
    }
}

void AAudio_ZoneManager::InitialiseZoneAmbience()
{
    ZoneAmbienceProfiles.Empty();

    // Forest edge — dense insect/wind ambience
    FAudio_ZoneAmbience Forest;
    Forest.ZoneType = EAudio_ZoneType::Forest;
    Forest.AmbienceSoundCuePath = TEXT("/Game/Audio/Ambience/SC_ForestEdge");
    Forest.VoiceBriefURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781949111136_ElderNPC_ForestEdgeGather.mp3");
    Forest.AmbienceVolume = 0.65f;
    ZoneAmbienceProfiles.Add(Forest);

    // Riverbank — water flow, frogs
    FAudio_ZoneAmbience River;
    River.ZoneType = EAudio_ZoneType::Riverbank;
    River.AmbienceSoundCuePath = TEXT("/Game/Audio/Ambience/SC_Riverbank");
    River.VoiceBriefURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781949103097_ElderNPC_RiverFish.mp3");
    River.AmbienceVolume = 0.7f;
    ZoneAmbienceProfiles.Add(River);

    // Hilltop — wind, sparse birds
    FAudio_ZoneAmbience Hilltop;
    Hilltop.ZoneType = EAudio_ZoneType::Hilltop;
    Hilltop.AmbienceSoundCuePath = TEXT("/Game/Audio/Ambience/SC_Hilltop");
    Hilltop.VoiceBriefURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781949100818_ElderNPC_HilltopSurvive.mp3");
    Hilltop.AmbienceVolume = 0.55f;
    ZoneAmbienceProfiles.Add(Hilltop);

    // Cave — drips, deep resonance, silence
    FAudio_ZoneAmbience Cave;
    Cave.ZoneType = EAudio_ZoneType::Cave;
    Cave.AmbienceSoundCuePath = TEXT("/Game/Audio/Ambience/SC_Cave");
    Cave.AmbienceVolume = 0.4f;
    ZoneAmbienceProfiles.Add(Cave);

    // Clearing — open wind, distant birds
    FAudio_ZoneAmbience Clearing;
    Clearing.ZoneType = EAudio_ZoneType::Clearing;
    Clearing.AmbienceSoundCuePath = TEXT("/Game/Audio/Ambience/SC_Clearing");
    Clearing.VoiceBriefURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781949076484_ElderNPC_ClearingExplore.mp3");
    Clearing.AmbienceVolume = 0.5f;
    Clearing.bIsActive = true; // default start zone
    ZoneAmbienceProfiles.Add(Clearing);

    // Hunting Ground — tense, sparse, low wind
    FAudio_ZoneAmbience Hunting;
    Hunting.ZoneType = EAudio_ZoneType::HuntingGround;
    Hunting.AmbienceSoundCuePath = TEXT("/Game/Audio/Ambience/SC_HuntingGround");
    Hunting.VoiceBriefURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781949113762_ElderNPC_HuntingGroundHunt.mp3");
    Hunting.AmbienceVolume = 0.45f;
    ZoneAmbienceProfiles.Add(Hunting);

    // Rock Shelter — enclosed, muffled wind
    FAudio_ZoneAmbience RockShelter;
    RockShelter.ZoneType = EAudio_ZoneType::RockShelter;
    RockShelter.AmbienceSoundCuePath = TEXT("/Game/Audio/Ambience/SC_RockShelter");
    RockShelter.VoiceBriefURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781949078496_ElderNPC_RockShelterDefend.mp3");
    RockShelter.AmbienceVolume = 0.35f;
    ZoneAmbienceProfiles.Add(RockShelter);

    // Water Hole — insects, splashing, birds
    FAudio_ZoneAmbience WaterHole;
    WaterHole.ZoneType = EAudio_ZoneType::WaterHole;
    WaterHole.AmbienceSoundCuePath = TEXT("/Game/Audio/Ambience/SC_WaterHole");
    WaterHole.AmbienceVolume = 0.6f;
    ZoneAmbienceProfiles.Add(WaterHole);

    UE_LOG(LogTemp, Log, TEXT("AAudio_ZoneManager: InitialiseZoneAmbience — %d zones registered"), ZoneAmbienceProfiles.Num());
}

void AAudio_ZoneManager::InitialiseDinoProfiles()
{
    DinoSoundProfiles.Empty();

    FAudio_DinoSoundProfile TRex;
    TRex.Species = EAudio_DinoSpecies::TRex;
    TRex.RoarSoundCuePath = TEXT("/Game/Audio/Dinos/SC_TRex_Roar");
    TRex.FootstepSoundCuePath = TEXT("/Game/Audio/Dinos/SC_TRex_Footstep");
    TRex.IdleBreathPath = TEXT("/Game/Audio/Dinos/SC_TRex_Breath");
    TRex.RoarVolumeMultiplier = 1.5f;
    TRex.FootstepVolumeMultiplier = 1.3f;
    TRex.RoarCooldownSeconds = 12.0f;
    DinoSoundProfiles.Add(TRex);

    FAudio_DinoSoundProfile Raptor;
    Raptor.Species = EAudio_DinoSpecies::Raptor;
    Raptor.RoarSoundCuePath = TEXT("/Game/Audio/Dinos/SC_Raptor_Bark");
    Raptor.FootstepSoundCuePath = TEXT("/Game/Audio/Dinos/SC_Raptor_Footstep");
    Raptor.IdleBreathPath = TEXT("/Game/Audio/Dinos/SC_Raptor_Chatter");
    Raptor.RoarVolumeMultiplier = 0.9f;
    Raptor.FootstepVolumeMultiplier = 0.7f;
    Raptor.RoarCooldownSeconds = 5.0f;
    DinoSoundProfiles.Add(Raptor);

    FAudio_DinoSoundProfile Brach;
    Brach.Species = EAudio_DinoSpecies::Brachiosaurus;
    Brach.RoarSoundCuePath = TEXT("/Game/Audio/Dinos/SC_Brach_Rumble");
    Brach.FootstepSoundCuePath = TEXT("/Game/Audio/Dinos/SC_Brach_Footstep");
    Brach.IdleBreathPath = TEXT("/Game/Audio/Dinos/SC_Brach_Breath");
    Brach.RoarVolumeMultiplier = 1.2f;
    Brach.FootstepVolumeMultiplier = 1.8f;
    Brach.RoarCooldownSeconds = 20.0f;
    DinoSoundProfiles.Add(Brach);

    UE_LOG(LogTemp, Log, TEXT("AAudio_ZoneManager: InitialiseDinoProfiles — %d species registered"), DinoSoundProfiles.Num());
}

void AAudio_ZoneManager::InitialiseDangerCues()
{
    DangerCues.Empty();

    FAudio_DangerCue TRexCue;
    TRexCue.TriggerSpecies = EAudio_DinoSpecies::TRex;
    TRexCue.VoiceWarningURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781949225810_ElderNPC_TRexWarning.mp3");
    TRexCue.TriggerRadiusMeters = 120.0f;
    DangerCues.Add(TRexCue);

    FAudio_DangerCue RaptorCue;
    RaptorCue.TriggerSpecies = EAudio_DinoSpecies::Raptor;
    RaptorCue.VoiceWarningURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781949252191_ElderNPC_RaptorWarning.mp3");
    RaptorCue.TriggerRadiusMeters = 80.0f;
    DangerCues.Add(RaptorCue);

    UE_LOG(LogTemp, Log, TEXT("AAudio_ZoneManager: InitialiseDangerCues — %d danger cues registered"), DangerCues.Num());
}

void AAudio_ZoneManager::ActivateZone(EAudio_ZoneType ZoneType)
{
    for (FAudio_ZoneAmbience& Zone : ZoneAmbienceProfiles)
    {
        Zone.bIsActive = (Zone.ZoneType == ZoneType);
    }
    CurrentActiveZone = ZoneType;
    UE_LOG(LogTemp, Log, TEXT("AAudio_ZoneManager: ActivateZone — zone=%d"), (int32)ZoneType);
}

void AAudio_ZoneManager::DeactivateZone(EAudio_ZoneType ZoneType)
{
    for (FAudio_ZoneAmbience& Zone : ZoneAmbienceProfiles)
    {
        if (Zone.ZoneType == ZoneType)
        {
            Zone.bIsActive = false;
        }
    }
}

FAudio_DinoSoundProfile AAudio_ZoneManager::GetProfileForSpecies(EAudio_DinoSpecies Species) const
{
    for (const FAudio_DinoSoundProfile& Profile : DinoSoundProfiles)
    {
        if (Profile.Species == Species)
        {
            return Profile;
        }
    }
    return FAudio_DinoSoundProfile();
}

void AAudio_ZoneManager::TriggerDinoRoar(EAudio_DinoSpecies Species, FVector Location)
{
    FAudio_DinoSoundProfile Profile = GetProfileForSpecies(Species);
    UE_LOG(LogTemp, Log, TEXT("AAudio_ZoneManager: TriggerDinoRoar — species=%d loc=(%0.f,%0.f,%0.f) cue=%s"),
        (int32)Species, Location.X, Location.Y, Location.Z, *Profile.RoarSoundCuePath);
    // Sound cue playback handled by MetaSounds/AudioComponent in Blueprint
}

void AAudio_ZoneManager::CheckProximityDangerCues(FVector PlayerLocation)
{
    UWorld* World = GetWorld();
    if (!World) return;

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);

    for (FAudio_DangerCue& Cue : DangerCues)
    {
        if (Cue.bHasTriggered) continue;

        for (AActor* Actor : AllActors)
        {
            FString Label = Actor->GetActorLabel().ToLower();
            bool bMatchesTRex = (Cue.TriggerSpecies == EAudio_DinoSpecies::TRex && Label.Contains(TEXT("trex")));
            bool bMatchesRaptor = (Cue.TriggerSpecies == EAudio_DinoSpecies::Raptor && Label.Contains(TEXT("raptor")));

            if (bMatchesTRex || bMatchesRaptor)
            {
                float Dist = FVector::Dist(PlayerLocation, Actor->GetActorLocation()) / 100.0f; // cm to m
                if (Dist <= Cue.TriggerRadiusMeters)
                {
                    Cue.bHasTriggered = true;
                    UE_LOG(LogTemp, Warning, TEXT("AAudio_ZoneManager: DANGER_CUE_TRIGGERED species=%d dist=%.1fm url=%s"),
                        (int32)Cue.TriggerSpecies, Dist, *Cue.VoiceWarningURL);
                    break;
                }
            }
        }
    }
}

void AAudio_ZoneManager::ResetAllDangerCues()
{
    for (FAudio_DangerCue& Cue : DangerCues)
    {
        Cue.bHasTriggered = false;
    }
    UE_LOG(LogTemp, Log, TEXT("AAudio_ZoneManager: All danger cues reset"));
}
