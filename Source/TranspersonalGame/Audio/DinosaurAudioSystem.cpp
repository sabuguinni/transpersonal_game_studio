#include "DinosaurAudioSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "DrawDebugHelpers.h"

ADinosaurAudioSystem::ADinosaurAudioSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10Hz tick — audio doesn't need per-frame update

    GlobalVolumeScale = 1.0f;
    MusicFadeSpeed = 2.0f;
    bEnableGroundShake = true;
    GroundShakeDecayRate = 0.95f;
    CurrentDangerIntensity = 0.0f;
    CurrentMusicIntensity = 0.0f;
    bIsNightTime = false;

    InitializeDinosaurProfiles();
}

void ADinosaurAudioSystem::BeginPlay()
{
    Super::BeginPlay();

    // Initialize state map for all species
    DinosaurStateMap.Add(EAudio_DinosaurSpecies::TRex, EAudio_DinosaurState::Idle);
    DinosaurStateMap.Add(EAudio_DinosaurSpecies::Raptor, EAudio_DinosaurState::Idle);
    DinosaurStateMap.Add(EAudio_DinosaurSpecies::Brachiosaurus, EAudio_DinosaurState::Idle);
    DinosaurStateMap.Add(EAudio_DinosaurSpecies::Triceratops, EAudio_DinosaurState::Idle);
    DinosaurStateMap.Add(EAudio_DinosaurSpecies::Pterodactyl, EAudio_DinosaurState::Idle);

    UE_LOG(LogTemp, Log, TEXT("[DinosaurAudioSystem] Initialized with %d dinosaur profiles"), DinosaurProfiles.Num());
}

void ADinosaurAudioSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateMusicIntensity(DeltaTime);
}

void ADinosaurAudioSystem::InitializeDinosaurProfiles()
{
    DinosaurProfiles.Empty();

    // T-Rex — massive, ground-shaking apex predator
    FAudio_DinosaurSoundProfile TRexProfile;
    TRexProfile.Species = EAudio_DinosaurSpecies::TRex;
    TRexProfile.FootstepVolumeMultiplier = 3.0f;
    TRexProfile.RoarRadius = 8000.0f;
    TRexProfile.FootstepRadius = 3000.0f;
    TRexProfile.GroundShakeThreshold = 2000.0f;
    TRexProfile.bCausesGroundShake = true;
    DinosaurProfiles.Add(TRexProfile);

    // Raptor — fast, precise, pack hunter
    FAudio_DinosaurSoundProfile RaptorProfile;
    RaptorProfile.Species = EAudio_DinosaurSpecies::Raptor;
    RaptorProfile.FootstepVolumeMultiplier = 0.8f;
    RaptorProfile.RoarRadius = 3000.0f;
    RaptorProfile.FootstepRadius = 800.0f;
    RaptorProfile.GroundShakeThreshold = 0.0f;
    RaptorProfile.bCausesGroundShake = false;
    DinosaurProfiles.Add(RaptorProfile);

    // Brachiosaurus — gentle giant, low rumbling calls
    FAudio_DinosaurSoundProfile BrachioProfile;
    BrachioProfile.Species = EAudio_DinosaurSpecies::Brachiosaurus;
    BrachioProfile.FootstepVolumeMultiplier = 2.5f;
    BrachioProfile.RoarRadius = 6000.0f;
    BrachioProfile.FootstepRadius = 2500.0f;
    BrachioProfile.GroundShakeThreshold = 1800.0f;
    BrachioProfile.bCausesGroundShake = true;
    DinosaurProfiles.Add(BrachioProfile);

    // Triceratops — territorial, low bellowing
    FAudio_DinosaurSoundProfile TriceProfile;
    TriceProfile.Species = EAudio_DinosaurSpecies::Triceratops;
    TriceProfile.FootstepVolumeMultiplier = 1.8f;
    TriceProfile.RoarRadius = 4000.0f;
    TriceProfile.FootstepRadius = 1500.0f;
    TriceProfile.GroundShakeThreshold = 1200.0f;
    TriceProfile.bCausesGroundShake = false;
    DinosaurProfiles.Add(TriceProfile);

    // Pterodactyl — aerial, high-pitched screeches
    FAudio_DinosaurSoundProfile PteroProfile;
    PteroProfile.Species = EAudio_DinosaurSpecies::Pterodactyl;
    PteroProfile.FootstepVolumeMultiplier = 0.3f;
    PteroProfile.RoarRadius = 5000.0f;
    PteroProfile.FootstepRadius = 200.0f;
    PteroProfile.GroundShakeThreshold = 0.0f;
    PteroProfile.bCausesGroundShake = false;
    DinosaurProfiles.Add(PteroProfile);
}

void ADinosaurAudioSystem::PlayDinosaurRoar(EAudio_DinosaurSpecies Species, FVector Location, float VolumeScale)
{
    FAudio_DinosaurSoundProfile* Profile = FindProfile(Species);
    if (!Profile) return;

    float FinalVolume = Profile->FootstepVolumeMultiplier * VolumeScale * GlobalVolumeScale;
    UE_LOG(LogTemp, Log, TEXT("[DinosaurAudioSystem] Roar: Species=%d Location=%s Volume=%.2f Radius=%.0f"),
        (int32)Species, *Location.ToString(), FinalVolume, Profile->RoarRadius);

    // Increase danger intensity when predators roar
    if (Species == EAudio_DinosaurSpecies::TRex || Species == EAudio_DinosaurSpecies::Raptor)
    {
        CurrentDangerIntensity = FMath::Min(1.0f, CurrentDangerIntensity + 0.4f);
    }
}

void ADinosaurAudioSystem::PlayDinosaurFootstep(EAudio_DinosaurSpecies Species, FVector Location, float FootMass)
{
    FAudio_DinosaurSoundProfile* Profile = FindProfile(Species);
    if (!Profile) return;

    float FinalVolume = Profile->FootstepVolumeMultiplier * FootMass * GlobalVolumeScale;

    // Ground shake if close enough and species causes shake
    if (Profile->bCausesGroundShake && bEnableGroundShake)
    {
        APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
        if (PC && PC->GetPawn())
        {
            float DistToPlayer = FVector::Dist(Location, PC->GetPawn()->GetActorLocation());
            if (DistToPlayer < Profile->GroundShakeThreshold)
            {
                float ShakeIntensity = 1.0f - (DistToPlayer / Profile->GroundShakeThreshold);
                TriggerGroundShake(Location, ShakeIntensity * FinalVolume, Profile->FootstepRadius);
            }
        }
    }

    UE_LOG(LogTemp, Verbose, TEXT("[DinosaurAudioSystem] Footstep: Species=%d Location=%s Volume=%.2f"),
        (int32)Species, *Location.ToString(), FinalVolume);
}

void ADinosaurAudioSystem::TriggerGroundShake(FVector EpicenterLocation, float Intensity, float Radius)
{
    if (!GetWorld()) return;

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return;

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn) return;

    float DistToPlayer = FVector::Dist(EpicenterLocation, PlayerPawn->GetActorLocation());
    if (DistToPlayer > Radius) return;

    float NormalizedDist = 1.0f - (DistToPlayer / Radius);
    float ShakeScale = Intensity * NormalizedDist;

    // Apply camera shake via player controller
    PC->ClientStartCameraShake(nullptr, ShakeScale);

    UE_LOG(LogTemp, Log, TEXT("[DinosaurAudioSystem] GroundShake: Epicenter=%s Intensity=%.2f Scale=%.2f"),
        *EpicenterLocation.ToString(), Intensity, ShakeScale);
}

void ADinosaurAudioSystem::SetDinosaurState(EAudio_DinosaurSpecies Species, EAudio_DinosaurState NewState)
{
    DinosaurStateMap.Add(Species, NewState);

    // Update danger intensity based on predator states
    float NewDanger = 0.0f;
    for (auto& Pair : DinosaurStateMap)
    {
        if (Pair.Key == EAudio_DinosaurSpecies::TRex || Pair.Key == EAudio_DinosaurSpecies::Raptor)
        {
            NewDanger = FMath::Max(NewDanger, CalculateDangerFromState(Pair.Value));
        }
    }
    CurrentDangerIntensity = FMath::Max(CurrentDangerIntensity, NewDanger);

    UE_LOG(LogTemp, Log, TEXT("[DinosaurAudioSystem] State change: Species=%d -> State=%d Danger=%.2f"),
        (int32)Species, (int32)NewState, CurrentDangerIntensity);
}

EAudio_DinosaurState ADinosaurAudioSystem::GetDinosaurState(EAudio_DinosaurSpecies Species) const
{
    const EAudio_DinosaurState* State = DinosaurStateMap.Find(Species);
    return State ? *State : EAudio_DinosaurState::Idle;
}

void ADinosaurAudioSystem::SetEnvironmentZone(const FAudio_EnvironmentZone& Zone)
{
    CurrentMusicIntensity = Zone.MusicIntensity;
    bIsNightTime = Zone.bIsNightZone;
    CurrentDangerIntensity = FMath::Max(CurrentDangerIntensity, Zone.DangerLevel);
    UE_LOG(LogTemp, Log, TEXT("[DinosaurAudioSystem] Zone set: %s Night=%d Danger=%.2f"),
        *Zone.ZoneName, bIsNightTime, Zone.DangerLevel);
}

void ADinosaurAudioSystem::FadeAmbientToNight(float FadeDuration)
{
    bIsNightTime = true;
    UE_LOG(LogTemp, Log, TEXT("[DinosaurAudioSystem] Fading to night ambient (%.1fs)"), FadeDuration);
}

void ADinosaurAudioSystem::FadeAmbientToDay(float FadeDuration)
{
    bIsNightTime = false;
    UE_LOG(LogTemp, Log, TEXT("[DinosaurAudioSystem] Fading to day ambient (%.1fs)"), FadeDuration);
}

void ADinosaurAudioSystem::SetDangerMusicIntensity(float Intensity)
{
    CurrentDangerIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
}

void ADinosaurAudioSystem::PlayPlayerDamageSound(float DamageAmount)
{
    float NormalizedDamage = FMath::Clamp(DamageAmount / 100.0f, 0.0f, 1.0f);
    UE_LOG(LogTemp, Log, TEXT("[DinosaurAudioSystem] Player damage sound: %.1f (normalized: %.2f)"),
        DamageAmount, NormalizedDamage);
    // Spike danger music on damage
    CurrentDangerIntensity = FMath::Min(1.0f, CurrentDangerIntensity + NormalizedDamage * 0.5f);
}

void ADinosaurAudioSystem::PlayPlayerFootstep(FString SurfaceType)
{
    UE_LOG(LogTemp, Verbose, TEXT("[DinosaurAudioSystem] Player footstep on: %s"), *SurfaceType);
}

void ADinosaurAudioSystem::PlayCraftingSound(FString ItemCrafted)
{
    UE_LOG(LogTemp, Log, TEXT("[DinosaurAudioSystem] Crafting sound: %s"), *ItemCrafted);
}

void ADinosaurAudioSystem::PlayFireCrackle(FVector FireLocation, float Intensity)
{
    UE_LOG(LogTemp, Log, TEXT("[DinosaurAudioSystem] Fire crackle at %s intensity=%.2f"),
        *FireLocation.ToString(), Intensity);
}

void ADinosaurAudioSystem::LogAudioSystemStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== DinosaurAudioSystem Status ==="));
    UE_LOG(LogTemp, Warning, TEXT("  DangerIntensity: %.2f"), CurrentDangerIntensity);
    UE_LOG(LogTemp, Warning, TEXT("  MusicIntensity:  %.2f"), CurrentMusicIntensity);
    UE_LOG(LogTemp, Warning, TEXT("  IsNightTime:     %s"), bIsNightTime ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("  GlobalVolume:    %.2f"), GlobalVolumeScale);
    UE_LOG(LogTemp, Warning, TEXT("  Profiles loaded: %d"), DinosaurProfiles.Num());
    for (auto& Pair : DinosaurStateMap)
    {
        UE_LOG(LogTemp, Warning, TEXT("  Species %d -> State %d"), (int32)Pair.Key, (int32)Pair.Value);
    }
}

float ADinosaurAudioSystem::GetDistanceToNearestPredator(FVector PlayerLocation) const
{
    float MinDist = TNumericLimits<float>::Max();
    for (auto& Pair : DinosaurPositionCache)
    {
        if (Pair.Key == EAudio_DinosaurSpecies::TRex || Pair.Key == EAudio_DinosaurSpecies::Raptor)
        {
            float Dist = FVector::Dist(PlayerLocation, Pair.Value);
            MinDist = FMath::Min(MinDist, Dist);
        }
    }
    return MinDist == TNumericLimits<float>::Max() ? -1.0f : MinDist;
}

void ADinosaurAudioSystem::UpdateMusicIntensity(float DeltaTime)
{
    // Decay danger intensity over time
    if (CurrentDangerIntensity > 0.0f)
    {
        CurrentDangerIntensity = FMath::Max(0.0f,
            CurrentDangerIntensity - (DeltaTime * 0.05f * GroundShakeDecayRate));
    }

    // Smoothly blend music intensity toward danger
    float TargetMusic = bIsNightTime
        ? FMath::Max(CurrentDangerIntensity, 0.3f)
        : CurrentDangerIntensity;

    CurrentMusicIntensity = FMath::FInterpTo(CurrentMusicIntensity, TargetMusic,
        DeltaTime, MusicFadeSpeed);
}

float ADinosaurAudioSystem::CalculateDangerFromState(EAudio_DinosaurState State) const
{
    switch (State)
    {
        case EAudio_DinosaurState::Idle:     return 0.0f;
        case EAudio_DinosaurState::Alert:    return 0.3f;
        case EAudio_DinosaurState::Hunting:  return 0.7f;
        case EAudio_DinosaurState::Attacking:return 1.0f;
        case EAudio_DinosaurState::Fleeing:  return 0.2f;
        case EAudio_DinosaurState::Feeding:  return 0.1f;
        default:                             return 0.0f;
    }
}

FAudio_DinosaurSoundProfile* ADinosaurAudioSystem::FindProfile(EAudio_DinosaurSpecies Species)
{
    for (FAudio_DinosaurSoundProfile& Profile : DinosaurProfiles)
    {
        if (Profile.Species == Species)
        {
            return &Profile;
        }
    }
    return nullptr;
}
