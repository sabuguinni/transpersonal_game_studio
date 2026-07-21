#include "Audio_PrehistoricSoundLibrary.h"

UAudio_PrehistoricSoundLibrary::UAudio_PrehistoricSoundLibrary()
{
}

TArray<FAudio_PrehistoricSound> UAudio_PrehistoricSoundLibrary::GetTribalDrumSounds()
{
    TArray<FAudio_PrehistoricSound> TribalSounds;
    InitializeTribalDrumLibrary(TribalSounds);
    return TribalSounds;
}

TArray<FAudio_PrehistoricSound> UAudio_PrehistoricSoundLibrary::GetDinosaurFootstepSounds()
{
    TArray<FAudio_PrehistoricSound> FootstepSounds;
    InitializeDinosaurFootstepLibrary(FootstepSounds);
    return FootstepSounds;
}

TArray<FAudio_PrehistoricSound> UAudio_PrehistoricSoundLibrary::GetAmbientNatureSounds()
{
    TArray<FAudio_PrehistoricSound> AmbientSounds;
    InitializeAmbientNatureLibrary(AmbientSounds);
    return AmbientSounds;
}

TArray<FAudio_PrehistoricSound> UAudio_PrehistoricSoundLibrary::GetPredatorGrowlSounds()
{
    TArray<FAudio_PrehistoricSound> GrowlSounds;
    InitializePredatorGrowlLibrary(GrowlSounds);
    return GrowlSounds;
}

FAudio_PrehistoricSound UAudio_PrehistoricSoundLibrary::GetSoundByType(EAudio_PrehistoricSoundType SoundType)
{
    switch (SoundType)
    {
        case EAudio_PrehistoricSoundType::TribalDrums:
        {
            TArray<FAudio_PrehistoricSound> Sounds = GetTribalDrumSounds();
            return Sounds.Num() > 0 ? Sounds[0] : FAudio_PrehistoricSound();
        }
        case EAudio_PrehistoricSoundType::DinosaurFootsteps:
        {
            TArray<FAudio_PrehistoricSound> Sounds = GetDinosaurFootstepSounds();
            return Sounds.Num() > 0 ? Sounds[0] : FAudio_PrehistoricSound();
        }
        case EAudio_PrehistoricSoundType::AmbientNature:
        {
            TArray<FAudio_PrehistoricSound> Sounds = GetAmbientNatureSounds();
            return Sounds.Num() > 0 ? Sounds[0] : FAudio_PrehistoricSound();
        }
        case EAudio_PrehistoricSoundType::PredatorGrowl:
        {
            TArray<FAudio_PrehistoricSound> Sounds = GetPredatorGrowlSounds();
            return Sounds.Num() > 0 ? Sounds[0] : FAudio_PrehistoricSound();
        }
        default:
            return FAudio_PrehistoricSound();
    }
}

TArray<FAudio_PrehistoricSound> UAudio_PrehistoricSoundLibrary::GetAllPrehistoricSounds()
{
    TArray<FAudio_PrehistoricSound> AllSounds;
    
    AllSounds.Append(GetTribalDrumSounds());
    AllSounds.Append(GetDinosaurFootstepSounds());
    AllSounds.Append(GetAmbientNatureSounds());
    AllSounds.Append(GetPredatorGrowlSounds());
    
    return AllSounds;
}

void UAudio_PrehistoricSoundLibrary::InitializeTribalDrumLibrary(TArray<FAudio_PrehistoricSound>& OutSounds)
{
    // Based on Freesound research results
    FAudio_PrehistoricSound TribalGameTheme;
    TribalGameTheme.SoundType = EAudio_PrehistoricSoundType::TribalDrums;
    TribalGameTheme.SoundName = TEXT("Tribal Game Theme Loop");
    TribalGameTheme.FreesoundID = TEXT("370294");
    TribalGameTheme.PreviewURL = TEXT("https://cdn.freesound.org/previews/370/370294_2402876-hq.mp3");
    TribalGameTheme.Duration = 18.2885f;
    TribalGameTheme.Tags = {TEXT("YouTube"), TEXT("ambiance"), TEXT("beat"), TEXT("dance"), TEXT("drumming"), TEXT("drums"), TEXT("game"), TEXT("groovy"), TEXT("loop"), TEXT("music")};
    TribalGameTheme.bIsLooping = true;
    TribalGameTheme.DefaultVolume = 0.7f;
    
    OutSounds.Add(TribalGameTheme);
}

void UAudio_PrehistoricSoundLibrary::InitializeDinosaurFootstepLibrary(TArray<FAudio_PrehistoricSound>& OutSounds)
{
    // Based on Freesound research results
    FAudio_PrehistoricSound MonsterStomp;
    MonsterStomp.SoundType = EAudio_PrehistoricSoundType::DinosaurFootsteps;
    MonsterStomp.SoundName = TEXT("Monster Stomp Footsteps Sequence");
    MonsterStomp.FreesoundID = TEXT("753178");
    MonsterStomp.PreviewURL = TEXT("https://cdn.freesound.org/previews/753/753178_9658839-hq.mp3");
    MonsterStomp.Duration = 8.34719f;
    MonsterStomp.Tags = {TEXT("animal"), TEXT("debris"), TEXT("deep"), TEXT("dinosaur"), TEXT("dirt"), TEXT("earth"), TEXT("earthquake"), TEXT("effect"), TEXT("foot"), TEXT("footstep")};
    MonsterStomp.bIsLooping = false;
    MonsterStomp.DefaultVolume = 1.0f;
    
    OutSounds.Add(MonsterStomp);
}

void UAudio_PrehistoricSoundLibrary::InitializeAmbientNatureLibrary(TArray<FAudio_PrehistoricSound>& OutSounds)
{
    // Placeholder for ambient nature sounds
    FAudio_PrehistoricSound CanyonWind;
    CanyonWind.SoundType = EAudio_PrehistoricSoundType::CanyonWind;
    CanyonWind.SoundName = TEXT("Canyon Wind Ambience");
    CanyonWind.FreesoundID = TEXT("TBD");
    CanyonWind.PreviewURL = TEXT("");
    CanyonWind.Duration = 30.0f;
    CanyonWind.Tags = {TEXT("wind"), TEXT("canyon"), TEXT("ambient"), TEXT("nature"), TEXT("prehistoric")};
    CanyonWind.bIsLooping = true;
    CanyonWind.DefaultVolume = 0.6f;
    
    OutSounds.Add(CanyonWind);
    
    FAudio_PrehistoricSound BirdCalls;
    BirdCalls.SoundType = EAudio_PrehistoricSoundType::BirdCalls;
    BirdCalls.SoundName = TEXT("Prehistoric Bird Calls");
    BirdCalls.FreesoundID = TEXT("TBD");
    BirdCalls.PreviewURL = TEXT("");
    BirdCalls.Duration = 15.0f;
    BirdCalls.Tags = {TEXT("birds"), TEXT("nature"), TEXT("prehistoric"), TEXT("calls"), TEXT("wildlife")};
    BirdCalls.bIsLooping = false;
    BirdCalls.DefaultVolume = 0.8f;
    
    OutSounds.Add(BirdCalls);
}

void UAudio_PrehistoricSoundLibrary::InitializePredatorGrowlLibrary(TArray<FAudio_PrehistoricSound>& OutSounds)
{
    // Placeholder for predator growl sounds
    FAudio_PrehistoricSound TRexRoar;
    TRexRoar.SoundType = EAudio_PrehistoricSoundType::PredatorGrowl;
    TRexRoar.SoundName = TEXT("T-Rex Deep Roar");
    TRexRoar.FreesoundID = TEXT("TBD");
    TRexRoar.PreviewURL = TEXT("");
    TRexRoar.Duration = 3.5f;
    TRexRoar.Tags = {TEXT("trex"), TEXT("roar"), TEXT("predator"), TEXT("dinosaur"), TEXT("threatening")};
    TRexRoar.bIsLooping = false;
    TRexRoar.DefaultVolume = 1.0f;
    
    OutSounds.Add(TRexRoar);
    
    FAudio_PrehistoricSound RaptorHiss;
    RaptorHiss.SoundType = EAudio_PrehistoricSoundType::PredatorGrowl;
    RaptorHiss.SoundName = TEXT("Raptor Pack Hiss");
    RaptorHiss.FreesoundID = TEXT("TBD");
    RaptorHiss.PreviewURL = TEXT("");
    RaptorHiss.Duration = 2.0f;
    RaptorHiss.Tags = {TEXT("raptor"), TEXT("hiss"), TEXT("pack"), TEXT("hunt"), TEXT("predator")};
    RaptorHiss.bIsLooping = false;
    RaptorHiss.DefaultVolume = 0.9f;
    
    OutSounds.Add(RaptorHiss);
}