#include "AudioSystem.h"
#include "GameFramework/Actor.h"

// ============================================================
// Constructor
// ============================================================

UAudio_SystemComponent::UAudio_SystemComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // Check every 0.5s — not every frame
}

// ============================================================
// BeginPlay
// ============================================================

void UAudio_SystemComponent::BeginPlay()
{
    Super::BeginPlay();
    LoadDefaultAmbientLayers();
    LoadDefaultDinoSounds();
    LoadDefaultVoiceLines();
    LoadFreesoundRefs();
}

// ============================================================
// TickComponent
// ============================================================

void UAudio_SystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    // Future: update attenuation, crossfade ambient layers based on player position
}

// ============================================================
// SetAmbientZone
// ============================================================

void UAudio_SystemComponent::SetAmbientZone(EAudio_AmbientZone NewZone)
{
    if (CurrentZone == NewZone) return;
    CurrentZone = NewZone;

    // Trigger music state change based on zone
    switch (NewZone)
    {
        case EAudio_AmbientZone::Danger:
            SetMusicState(EAudio_MusicState::Tension);
            break;
        case EAudio_AmbientZone::Camp:
            SetMusicState(EAudio_MusicState::Camp);
            break;
        case EAudio_AmbientZone::Cave:
            SetMusicState(EAudio_MusicState::Night);
            break;
        default:
            SetMusicState(EAudio_MusicState::Exploration);
            break;
    }
}

// ============================================================
// SetMusicState
// ============================================================

void UAudio_SystemComponent::SetMusicState(EAudio_MusicState NewState)
{
    if (CurrentMusicState == NewState) return;
    CurrentMusicState = NewState;

    // Adjust volume targets per state
    switch (NewState)
    {
        case EAudio_MusicState::Combat:
            CurrentMusicVolume = 1.0f;
            break;
        case EAudio_MusicState::Tension:
            CurrentMusicVolume = 0.8f;
            break;
        case EAudio_MusicState::Camp:
            CurrentMusicVolume = 0.5f;
            break;
        case EAudio_MusicState::Night:
            CurrentMusicVolume = 0.4f;
            break;
        case EAudio_MusicState::Discovery:
            CurrentMusicVolume = 0.7f;
            break;
        default: // Exploration
            CurrentMusicVolume = 0.6f;
            break;
    }
}

// ============================================================
// GetDinoSound
// ============================================================

FAudio_DinoSoundEntry UAudio_SystemComponent::GetDinoSound(FName Species, EAudio_DinoSoundType SoundType) const
{
    for (const FAudio_DinoSoundEntry& Entry : DinoSoundRegistry)
    {
        if (Entry.DinoSpecies == Species && Entry.SoundType == SoundType)
        {
            return Entry;
        }
    }
    return FAudio_DinoSoundEntry(); // Empty entry if not found
}

// ============================================================
// GetNextUnplayedVoiceLine
// ============================================================

FAudio_VoiceLine UAudio_SystemComponent::GetNextUnplayedVoiceLine(FName CharacterID)
{
    for (FAudio_VoiceLine& Line : VoiceLineRegistry)
    {
        if (Line.CharacterID == CharacterID && !Line.bHasBeenPlayed)
        {
            return Line;
        }
    }
    return FAudio_VoiceLine(); // Empty if all played
}

// ============================================================
// MarkVoiceLinePlayed
// ============================================================

void UAudio_SystemComponent::MarkVoiceLinePlayed(FName CharacterID, int32 LineIndex)
{
    int32 Count = 0;
    for (FAudio_VoiceLine& Line : VoiceLineRegistry)
    {
        if (Line.CharacterID == CharacterID)
        {
            if (Count == LineIndex)
            {
                Line.bHasBeenPlayed = true;
                return;
            }
            Count++;
        }
    }
}

// ============================================================
// SetNightMode
// ============================================================

void UAudio_SystemComponent::SetNightMode(bool bIsNight)
{
    bNightModeActive = bIsNight;
    if (bIsNight)
    {
        SetMusicState(EAudio_MusicState::Night);
        SetAmbientZone(EAudio_AmbientZone::OpenPlain); // Night open plains
    }
    else
    {
        SetMusicState(EAudio_MusicState::Exploration);
    }
}

// ============================================================
// RegisterFreesoundRef
// ============================================================

void UAudio_SystemComponent::RegisterFreesoundRef(int32 SoundID, const FString& Name, const FString& PreviewURL, const FString& Tags, float Duration)
{
    FAudio_FreesoundRef Ref;
    Ref.FreesoundID = SoundID;
    Ref.Name = Name;
    Ref.PreviewURL = PreviewURL;
    Ref.Tags = Tags;
    Ref.Duration = Duration;
    FreesoundRefs.Add(Ref);
}

// ============================================================
// GetFreesoundRefsByTag
// ============================================================

TArray<FAudio_FreesoundRef> UAudio_SystemComponent::GetFreesoundRefsByTag(const FString& Tag) const
{
    TArray<FAudio_FreesoundRef> Result;
    for (const FAudio_FreesoundRef& Ref : FreesoundRefs)
    {
        if (Ref.Tags.Contains(Tag))
        {
            Result.Add(Ref);
        }
    }
    return Result;
}

// ============================================================
// LoadDefaultAmbientLayers — curated Freesound references
// ============================================================

void UAudio_SystemComponent::LoadDefaultAmbientLayers()
{
    // Campfire — Freesound ID 394952 (1805s loop, campfire near lake)
    FAudio_AmbientLayer CampLayer;
    CampLayer.Zone = EAudio_AmbientZone::Camp;
    CampLayer.SoundURL = TEXT("https://cdn.freesound.org/previews/394/394952_7037-hq.mp3");
    CampLayer.BaseVolume = 0.8f;
    CampLayer.FadeInTime = 3.0f;
    CampLayer.bIsLooping = true;
    AmbientLayers.Add(CampLayer);

    // River — Freesound ID 588176 (mountain river, large rapids)
    FAudio_AmbientLayer RiverLayer;
    RiverLayer.Zone = EAudio_AmbientZone::River;
    RiverLayer.SoundURL = TEXT("https://cdn.freesound.org/previews/588/588176_13207434-hq.mp3");
    RiverLayer.BaseVolume = 0.9f;
    RiverLayer.FadeInTime = 2.0f;
    RiverLayer.bIsLooping = true;
    AmbientLayers.Add(RiverLayer);

    // River small — Freesound ID 588179
    FAudio_AmbientLayer RiverSmallLayer;
    RiverSmallLayer.Zone = EAudio_AmbientZone::Forest;
    RiverSmallLayer.SoundURL = TEXT("https://cdn.freesound.org/previews/588/588179_13207434-hq.mp3");
    RiverSmallLayer.BaseVolume = 0.5f;
    RiverSmallLayer.FadeInTime = 2.5f;
    RiverSmallLayer.bIsLooping = true;
    AmbientLayers.Add(RiverSmallLayer);
}

// ============================================================
// LoadDefaultDinoSounds — placeholder URLs (swap for real assets)
// ============================================================

void UAudio_SystemComponent::LoadDefaultDinoSounds()
{
    // T-Rex footstep
    FAudio_DinoSoundEntry TRexFootstep;
    TRexFootstep.DinoSpecies = FName("TRex");
    TRexFootstep.SoundType = EAudio_DinoSoundType::Footstep;
    TRexFootstep.SoundURL = TEXT("PLACEHOLDER_TREX_FOOTSTEP");
    TRexFootstep.Volume = 1.0f;
    TRexFootstep.AttenuationRadius = 5000.0f;
    DinoSoundRegistry.Add(TRexFootstep);

    // T-Rex roar
    FAudio_DinoSoundEntry TRexRoar;
    TRexRoar.DinoSpecies = FName("TRex");
    TRexRoar.SoundType = EAudio_DinoSoundType::Roar;
    TRexRoar.SoundURL = TEXT("PLACEHOLDER_TREX_ROAR");
    TRexRoar.Volume = 1.0f;
    TRexRoar.AttenuationRadius = 8000.0f;
    DinoSoundRegistry.Add(TRexRoar);

    // Raptor idle
    FAudio_DinoSoundEntry RaptorIdle;
    RaptorIdle.DinoSpecies = FName("Raptor");
    RaptorIdle.SoundType = EAudio_DinoSoundType::Idle;
    RaptorIdle.SoundURL = TEXT("PLACEHOLDER_RAPTOR_IDLE");
    RaptorIdle.Volume = 0.7f;
    RaptorIdle.AttenuationRadius = 2000.0f;
    DinoSoundRegistry.Add(RaptorIdle);

    // Raptor attack
    FAudio_DinoSoundEntry RaptorAttack;
    RaptorAttack.DinoSpecies = FName("Raptor");
    RaptorAttack.SoundType = EAudio_DinoSoundType::Attack;
    RaptorAttack.SoundURL = TEXT("PLACEHOLDER_RAPTOR_ATTACK");
    RaptorAttack.Volume = 0.9f;
    RaptorAttack.AttenuationRadius = 3000.0f;
    DinoSoundRegistry.Add(RaptorAttack);

    // Brachiosaurus herd call
    FAudio_DinoSoundEntry BrachioHerd;
    BrachioHerd.DinoSpecies = FName("Brachiosaurus");
    BrachioHerd.SoundType = EAudio_DinoSoundType::Herd;
    BrachioHerd.SoundURL = TEXT("PLACEHOLDER_BRACHIO_HERD");
    BrachioHerd.Volume = 0.8f;
    BrachioHerd.AttenuationRadius = 10000.0f;
    DinoSoundRegistry.Add(BrachioHerd);
}

// ============================================================
// LoadDefaultVoiceLines — ElevenLabs TTS from Supabase
// ============================================================

void UAudio_SystemComponent::LoadDefaultVoiceLines()
{
    // ScoutWarner — danger warning (cycle 004)
    FAudio_VoiceLine ScoutLine;
    ScoutLine.CharacterID = FName("ScoutWarner");
    ScoutLine.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782625843379_ScoutWarner.mp3");
    ScoutLine.SubtitleText = TEXT("Danger. Something large is moving through the valley. Stay low, stay still.");
    ScoutLine.EstimatedDuration = 14.0f;
    VoiceLineRegistry.Add(ScoutLine);

    // FireKeeper — fire urgency (cycle 004)
    FAudio_VoiceLine FireLine;
    FireLine.CharacterID = FName("FireKeeper");
    FireLine.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782625863929_FireKeeper.mp3");
    FireLine.SubtitleText = TEXT("Fire. We need fire tonight. Without it, the night hunters will come.");
    FireLine.EstimatedDuration = 14.0f;
    VoiceLineRegistry.Add(FireLine);

    // TrailReader — predator tracks (cycle 003)
    FAudio_VoiceLine TrailLine;
    TrailLine.CharacterID = FName("TrailReader");
    TrailLine.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782625717061_TrailReader.mp3");
    TrailLine.SubtitleText = TEXT("Predator tracks spotted. Go east to high ground.");
    TrailLine.EstimatedDuration = 10.0f;
    VoiceLineRegistry.Add(TrailLine);

    // HuntCaller — hunt quest (cycle 003)
    FAudio_VoiceLine HuntLine;
    HuntLine.CharacterID = FName("HuntCaller");
    HuntLine.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782625720437_HuntCaller.mp3");
    HuntLine.SubtitleText = TEXT("The tribe needs meat. Hunt the herd's weakest.");
    HuntLine.EstimatedDuration = 10.0f;
    VoiceLineRegistry.Add(HuntLine);

    // CampKeeper — flood warning (cycle 003)
    FAudio_VoiceLine CampLine;
    CampLine.CharacterID = FName("CampKeeper");
    CampLine.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782625739826_CampKeeper.mp3");
    CampLine.SubtitleText = TEXT("Cave flood warning. Move camp before the rains.");
    CampLine.EstimatedDuration = 10.0f;
    VoiceLineRegistry.Add(CampLine);

    // RiverGuide — river crossing (cycle 003)
    FAudio_VoiceLine RiverLine;
    RiverLine.CharacterID = FName("RiverGuide");
    RiverLine.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782625742553_RiverGuide.mp3");
    RiverLine.SubtitleText = TEXT("Dangerous river crossing. Go 3 bends east.");
    RiverLine.EstimatedDuration = 10.0f;
    VoiceLineRegistry.Add(RiverLine);
}

// ============================================================
// LoadFreesoundRefs — catalogue of curated Freesound assets
// ============================================================

void UAudio_SystemComponent::LoadFreesoundRefs()
{
    // Campfire sounds
    RegisterFreesoundRef(157187,  TEXT("Campfire Crackle 3"),                 TEXT("https://cdn.freesound.org/previews/157/157187_2840005-hq.mp3"),  TEXT("burning campfire crackling fire wood"),         2.0f);
    RegisterFreesoundRef(394952,  TEXT("Crackling Campfire near Lake"),        TEXT("https://cdn.freesound.org/previews/394/394952_7037-hq.mp3"),     TEXT("burn burning campfire crackle fire flame"),     1805.6f);
    RegisterFreesoundRef(790790,  TEXT("Dry Pine Needles Burning"),            TEXT("https://cdn.freesound.org/previews/790/790790_16993478-hq.mp3"), TEXT("ASMR burning campfire crackling fire nature"),   62.0f);
    RegisterFreesoundRef(660297,  TEXT("Campfire Wet Wood Hissing"),           TEXT("https://cdn.freesound.org/previews/660/660297_13721094-hq.mp3"), TEXT("bonfire burn campfire crackle fire flame"),      37.8f);
    RegisterFreesoundRef(620007,  TEXT("Wood Firestove Burning"),              TEXT("https://cdn.freesound.org/previews/620/620007_13601611-hq.mp3"), TEXT("burn burning campfire crackle fire heat"),       307.2f);

    // River / water sounds
    RegisterFreesoundRef(588176,  TEXT("Mountain River Large 002"),            TEXT("https://cdn.freesound.org/previews/588/588176_13207434-hq.mp3"), TEXT("ambient brook creek flow flowing gurgle liquid"), 43.6f);
    RegisterFreesoundRef(588179,  TEXT("Mountain River 001"),                  TEXT("https://cdn.freesound.org/previews/588/588179_13207434-hq.mp3"), TEXT("ambient babbling brook creek flow flowing"),      46.7f);
    RegisterFreesoundRef(588178,  TEXT("Mountain River 002"),                  TEXT("https://cdn.freesound.org/previews/588/588178_13207434-hq.mp3"), TEXT("ambient brook creek current flow flowing"),       42.9f);
    RegisterFreesoundRef(588177,  TEXT("Mountain River 003"),                  TEXT("https://cdn.freesound.org/previews/588/588177_13207434-hq.mp3"), TEXT("ambient babbling brook creek field-recording"),   58.0f);
    RegisterFreesoundRef(588180,  TEXT("Mountain River Large 001"),            TEXT("https://cdn.freesound.org/previews/588/588180_13207434-hq.mp3"), TEXT("ambient brook creek current flow flowing"),       50.0f);
}
