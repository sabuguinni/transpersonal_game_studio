#include "PrehistoricAudioManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "TimerManager.h"

APrehistoricAudioManager::APrehistoricAudioManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.5f; // Update every 0.5s — audio doesn't need per-frame ticks

    // Populate known Freesound campfire asset IDs (discovered via search_sounds)
    FreesoundCampfireIDs.Add(681367); // Campfire Position 2 — 22s, quiet night
    FreesoundCampfireIDs.Add(681366); // Campfire Position 1 — 83s, quiet night
    FreesoundCampfireIDs.Add(688992); // Campfire St. Marys River — 9 min, birds+fire
    FreesoundCampfireIDs.Add(802195); // Fire-Nature Sounds — 4 min, campfire at night

    FreesoundCampfireURLs.Add(TEXT("https://cdn.freesound.org/previews/681/681367_5752443-hq.mp3"));
    FreesoundCampfireURLs.Add(TEXT("https://cdn.freesound.org/previews/681/681366_5752443-hq.mp3"));
    FreesoundCampfireURLs.Add(TEXT("https://cdn.freesound.org/previews/688/688992_13721094-hq.mp3"));
    FreesoundCampfireURLs.Add(TEXT("https://cdn.freesound.org/previews/802/802195_17223245-hq.mp3"));
}

void APrehistoricAudioManager::BeginPlay()
{
    Super::BeginPlay();

    InitializeDefaultProfiles();

    // Start ambient update loop every 10 seconds
    GetWorldTimerManager().SetTimer(
        AmbientUpdateTimer,
        this,
        &APrehistoricAudioManager::UpdateAmbientLayers,
        10.0f,
        true
    );

    // Threat decay — reduce threat level over time if no new threats
    GetWorldTimerManager().SetTimer(
        ThreatDecayTimer,
        this,
        &APrehistoricAudioManager::DecayThreatLevel,
        30.0f,
        true
    );

    UE_LOG(LogTemp, Log, TEXT("[AudioManager] PrehistoricAudioManager initialized. Biome: %d, Time: %d"),
        (int32)CurrentBiome, (int32)CurrentTimeOfDay);
}

void APrehistoricAudioManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // Lightweight tick — heavy work done via timers
}

void APrehistoricAudioManager::InitializeDefaultProfiles()
{
    // T-Rex profile
    FAudio_DinosaurSoundProfile TRex;
    TRex.SpeciesName = TEXT("TyrannosaurusRex");
    TRex.FootstepRadius = 3000.0f;
    TRex.RoarRadius = 10000.0f;
    DinosaurProfiles.Add(TRex);

    // Raptor profile
    FAudio_DinosaurSoundProfile Raptor;
    Raptor.SpeciesName = TEXT("Velociraptor");
    Raptor.FootstepRadius = 800.0f;
    Raptor.RoarRadius = 3000.0f;
    DinosaurProfiles.Add(Raptor);

    // Triceratops profile
    FAudio_DinosaurSoundProfile Trike;
    Trike.SpeciesName = TEXT("Triceratops");
    Trike.FootstepRadius = 2000.0f;
    Trike.RoarRadius = 5000.0f;
    DinosaurProfiles.Add(Trike);

    // Brachiosaurus profile
    FAudio_DinosaurSoundProfile Brachio;
    Brachio.SpeciesName = TEXT("Brachiosaurus");
    Brachio.FootstepRadius = 4000.0f;
    Brachio.RoarRadius = 8000.0f;
    DinosaurProfiles.Add(Brachio);

    // Register voice lines from Agent #15 Narrative output
    FAudio_VoiceLine LineKora;
    LineKora.CharacterName = TEXT("Kora");
    LineKora.LineText = TEXT("The eastern ridge is raptor territory now. We lost two scouts last moon cycle.");
    LineKora.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782790823084_TribalLeader_Kora.mp3");
    LineKora.Duration = 8.0f;
    RegisteredVoiceLines.Add(LineKora);

    FAudio_VoiceLine LineDavan;
    LineDavan.CharacterName = TEXT("Davan");
    LineDavan.LineText = TEXT("We have tracked the Triceratops herd for three days. They know where the water is.");
    LineDavan.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782790841088_Scout_Davan.mp3");
    LineDavan.Duration = 9.0f;
    RegisteredVoiceLines.Add(LineDavan);

    FAudio_VoiceLine LineMaren;
    LineMaren.CharacterName = TEXT("Maren");
    LineMaren.LineText = TEXT("The T-Rex does not hunt by sight alone. It reads the ground — your footprints, your warmth.");
    LineMaren.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782790848707_Elder_Maren.mp3");
    LineMaren.Duration = 10.0f;
    RegisteredVoiceLines.Add(LineMaren);

    FAudio_VoiceLine LineBrek;
    LineBrek.CharacterName = TEXT("Brek");
    LineBrek.LineText = TEXT("Fire. That is the only thing they fear. Keep it burning through the night.");
    LineBrek.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782790850746_Hunter_Brek.mp3");
    LineBrek.Duration = 7.0f;
    RegisteredVoiceLines.Add(LineBrek);

    FAudio_VoiceLine LineBrekDanger;
    LineBrekDanger.CharacterName = TEXT("Brek");
    LineBrekDanger.LineText = TEXT("Something massive just moved through the tree line. Stay low. Do not run.");
    LineBrekDanger.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782790967276_Hunter_Brek_Danger.mp3");
    LineBrekDanger.Duration = 11.0f;
    RegisteredVoiceLines.Add(LineBrekDanger);

    FAudio_VoiceLine LineMarenSeasons;
    LineMarenSeasons.CharacterName = TEXT("Maren");
    LineMarenSeasons.LineText = TEXT("The dry season is ending. When the rains come, the herds move north through the valley pass.");
    LineMarenSeasons.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782790990071_Elder_Maren_Seasons.mp3");
    LineMarenSeasons.Duration = 13.0f;
    RegisteredVoiceLines.Add(LineMarenSeasons);

    UE_LOG(LogTemp, Log, TEXT("[AudioManager] Initialized %d dinosaur profiles, %d voice lines"),
        DinosaurProfiles.Num(), RegisteredVoiceLines.Num());
}

void APrehistoricAudioManager::SetTimeOfDay(EAudio_TimeOfDay NewTime)
{
    if (CurrentTimeOfDay == NewTime) return;

    EAudio_TimeOfDay OldTime = CurrentTimeOfDay;
    CurrentTimeOfDay = NewTime;

    UE_LOG(LogTemp, Log, TEXT("[AudioManager] Time of day changed: %d -> %d"), (int32)OldTime, (int32)NewTime);

    // Trigger ambient layer update on time change
    UpdateAmbientLayers();

    // Night transition — increase threat awareness
    if (NewTime == EAudio_TimeOfDay::Night && CurrentThreatLevel == EAudio_ThreatLevel::Safe)
    {
        SetThreatLevel(EAudio_ThreatLevel::Aware);
        UE_LOG(LogTemp, Log, TEXT("[AudioManager] Nightfall — threat level raised to Aware"));
    }
}

void APrehistoricAudioManager::SetThreatLevel(EAudio_ThreatLevel NewThreat)
{
    if (CurrentThreatLevel == NewThreat) return;

    CurrentThreatLevel = NewThreat;

    float MusicIntensity = GetThreatMusicIntensity();
    UE_LOG(LogTemp, Log, TEXT("[AudioManager] Threat level: %d | Music intensity: %.2f"), (int32)NewThreat, MusicIntensity);
}

void APrehistoricAudioManager::SetActiveBiome(EAudio_BiomeType NewBiome)
{
    if (CurrentBiome == NewBiome) return;
    CurrentBiome = NewBiome;
    UpdateAmbientLayers();
    UE_LOG(LogTemp, Log, TEXT("[AudioManager] Biome changed to: %d"), (int32)NewBiome);
}

void APrehistoricAudioManager::SetWeatherState(EAudio_WeatherState NewWeather)
{
    if (CurrentWeather == NewWeather) return;
    CurrentWeather = NewWeather;
    UpdateAmbientLayers();
    UE_LOG(LogTemp, Log, TEXT("[AudioManager] Weather changed to: %d"), (int32)NewWeather);
}

void APrehistoricAudioManager::PlayVoiceLine(const FAudio_VoiceLine& Line)
{
    if (Line.AudioAsset && GetWorld())
    {
        UGameplayStatics::PlaySound2D(GetWorld(), Line.AudioAsset, 1.0f, 1.0f, 0.0f);
        UE_LOG(LogTemp, Log, TEXT("[AudioManager] Playing voice line: %s — '%s'"), *Line.CharacterName, *Line.LineText);
    }
    else
    {
        // Asset not yet imported — log URL for Blueprint wiring
        UE_LOG(LogTemp, Warning, TEXT("[AudioManager] Voice line asset not set for %s. URL: %s"), *Line.CharacterName, *Line.AudioURL);
    }
}

void APrehistoricAudioManager::RegisterVoiceLine(const FAudio_VoiceLine& Line)
{
    RegisteredVoiceLines.Add(Line);
    UE_LOG(LogTemp, Log, TEXT("[AudioManager] Registered voice line for %s"), *Line.CharacterName);
}

void APrehistoricAudioManager::TriggerDinosaurRoar(const FString& SpeciesName, FVector Location)
{
    for (const FAudio_DinosaurSoundProfile& Profile : DinosaurProfiles)
    {
        if (Profile.SpeciesName == SpeciesName)
        {
            if (Profile.AlertSound && GetWorld())
            {
                UGameplayStatics::PlaySoundAtLocation(GetWorld(), Profile.AlertSound, Location, 1.0f, 1.0f, 0.0f);
            }

            // Escalate threat level based on proximity to player
            APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
            if (PC && PC->GetPawn())
            {
                float Distance = FVector::Dist(Location, PC->GetPawn()->GetActorLocation());
                if (Distance < 2000.0f)
                {
                    SetThreatLevel(EAudio_ThreatLevel::Critical);
                }
                else if (Distance < 5000.0f)
                {
                    SetThreatLevel(EAudio_ThreatLevel::Danger);
                }
                else if (Distance < Profile.RoarRadius)
                {
                    SetThreatLevel(EAudio_ThreatLevel::Aware);
                }
            }

            UE_LOG(LogTemp, Log, TEXT("[AudioManager] Dinosaur roar: %s at (%.0f, %.0f, %.0f)"),
                *SpeciesName, Location.X, Location.Y, Location.Z);
            return;
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("[AudioManager] No profile found for species: %s"), *SpeciesName);
}

void APrehistoricAudioManager::TriggerDinosaurFootstep(const FString& SpeciesName, FVector Location, float Mass)
{
    for (const FAudio_DinosaurSoundProfile& Profile : DinosaurProfiles)
    {
        if (Profile.SpeciesName == SpeciesName)
        {
            if (Profile.FootstepSound && GetWorld())
            {
                float VolumeScale = FMath::Clamp(Mass / 5000.0f, 0.1f, 2.0f);
                UGameplayStatics::PlaySoundAtLocation(GetWorld(), Profile.FootstepSound, Location, VolumeScale);
            }

            // Screen shake for heavy footsteps
            float ShakeIntensity = FMath::Clamp(Mass / 8000.0f, 0.0f, 1.0f);
            TriggerFootstepShake(ShakeIntensity, Location);
            return;
        }
    }
}

void APrehistoricAudioManager::RegisterDinosaurProfile(const FAudio_DinosaurSoundProfile& Profile)
{
    // Remove existing profile for this species if present
    DinosaurProfiles.RemoveAll([&Profile](const FAudio_DinosaurSoundProfile& Existing)
    {
        return Existing.SpeciesName == Profile.SpeciesName;
    });
    DinosaurProfiles.Add(Profile);
    UE_LOG(LogTemp, Log, TEXT("[AudioManager] Registered dinosaur profile: %s"), *Profile.SpeciesName);
}

void APrehistoricAudioManager::TriggerFootstepShake(float Intensity, FVector SourceLocation)
{
    if (!GetWorld() || Intensity <= 0.01f) return;

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return;

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn) return;

    float Distance = FVector::Dist(SourceLocation, PlayerPawn->GetActorLocation());
    float FalloffIntensity = Intensity * FMath::Clamp(1.0f - (Distance / 5000.0f), 0.0f, 1.0f);

    if (FalloffIntensity > 0.05f)
    {
        PC->ClientStartCameraShake(nullptr, FalloffIntensity);
        UE_LOG(LogTemp, Verbose, TEXT("[AudioManager] Screen shake: intensity=%.2f dist=%.0f"), FalloffIntensity, Distance);
    }
}

void APrehistoricAudioManager::SpawnCampfireAudio(FVector Location)
{
    if (!GetWorld()) return;

    if (CampfireLoopSound)
    {
        UAudioComponent* CampfireComp = UGameplayStatics::SpawnSoundAtLocation(
            GetWorld(),
            CampfireLoopSound,
            Location,
            FRotator::ZeroRotator,
            1.0f,
            1.0f,
            0.0f,
            nullptr,
            nullptr,
            true
        );

        if (CampfireComp)
        {
            CampfireComp->bAutoDestroy = false;
            ActiveAmbientComponents.Add(CampfireComp);
            UE_LOG(LogTemp, Log, TEXT("[AudioManager] Campfire audio spawned at (%.0f, %.0f, %.0f)"),
                Location.X, Location.Y, Location.Z);
        }
    }
    else
    {
        // Log Freesound reference for Blueprint wiring
        UE_LOG(LogTemp, Warning, TEXT("[AudioManager] Campfire sound not set. Use Freesound ID 681366 (83s campfire loop). URL: %s"),
            FreesoundCampfireURLs.Num() > 1 ? *FreesoundCampfireURLs[1] : TEXT("N/A"));
    }
}

void APrehistoricAudioManager::UpdateAmbientLayers()
{
    // Filter active layers by current biome and time of day
    int32 ActiveCount = 0;
    for (int32 i = 0; i < AmbientLayers.Num(); i++)
    {
        const FAudio_AmbientLayer& Layer = AmbientLayers[i];
        bool bShouldBeActive = (Layer.ActiveBiome == CurrentBiome && Layer.ActiveTimeOfDay == CurrentTimeOfDay);

        if (i < ActiveAmbientComponents.Num() && ActiveAmbientComponents[i])
        {
            float TargetVolume = bShouldBeActive ? Layer.BaseVolume : 0.0f;
            FadeAmbientLayer(i, TargetVolume, 3.0f);
            if (bShouldBeActive) ActiveCount++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[AudioManager] Ambient update: %d active layers (Biome=%d, Time=%d, Threat=%d, Weather=%d)"),
        ActiveCount, (int32)CurrentBiome, (int32)CurrentTimeOfDay, (int32)CurrentThreatLevel, (int32)CurrentWeather);
}

void APrehistoricAudioManager::FadeAmbientLayer(int32 LayerIndex, float TargetVolume, float FadeTime)
{
    if (LayerIndex < 0 || LayerIndex >= ActiveAmbientComponents.Num()) return;
    UAudioComponent* Comp = ActiveAmbientComponents[LayerIndex];
    if (!Comp) return;

    if (TargetVolume <= 0.0f)
    {
        Comp->FadeOut(FadeTime, 0.0f);
    }
    else
    {
        Comp->FadeIn(FadeTime, TargetVolume);
    }
}

void APrehistoricAudioManager::DecayThreatLevel()
{
    // Gradually reduce threat over time if no new threats
    switch (CurrentThreatLevel)
    {
        case EAudio_ThreatLevel::Critical:
            SetThreatLevel(EAudio_ThreatLevel::Danger);
            break;
        case EAudio_ThreatLevel::Danger:
            SetThreatLevel(EAudio_ThreatLevel::Aware);
            break;
        case EAudio_ThreatLevel::Aware:
            // Only decay to Safe during daytime
            if (CurrentTimeOfDay == EAudio_TimeOfDay::Day)
            {
                SetThreatLevel(EAudio_ThreatLevel::Safe);
            }
            break;
        case EAudio_ThreatLevel::Safe:
        default:
            break;
    }
}

float APrehistoricAudioManager::GetThreatMusicIntensity() const
{
    switch (CurrentThreatLevel)
    {
        case EAudio_ThreatLevel::Safe:     return 0.0f;
        case EAudio_ThreatLevel::Aware:    return 0.3f;
        case EAudio_ThreatLevel::Danger:   return 0.7f;
        case EAudio_ThreatLevel::Critical: return 1.0f;
        default:                           return 0.0f;
    }
}
