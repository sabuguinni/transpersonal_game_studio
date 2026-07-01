#include "AudioSystemManager.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraShakeBase.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ============================================================
// UAudio_AdaptiveMusicComponent
// ============================================================

UAudio_AdaptiveMusicComponent::UAudio_AdaptiveMusicComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    CurrentThreatLevel = EAudio_ThreatLevel::Safe;
    MusicTransitionSpeed = 2.0f;
    TRexProximityRadius = 4000.f;
    ThreatDecayTimer = 0.f;
    CurrentMusicIntensity = 0.f;
}

void UAudio_AdaptiveMusicComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UAudio_AdaptiveMusicComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Decay threat level over time when no dinosaur is nearby
    if (CurrentThreatLevel != EAudio_ThreatLevel::Safe)
    {
        ThreatDecayTimer += DeltaTime;
        if (ThreatDecayTimer > 8.0f)
        {
            // Step down threat level
            uint8 Level = (uint8)CurrentThreatLevel;
            if (Level > 0)
            {
                CurrentThreatLevel = (EAudio_ThreatLevel)(Level - 1);
            }
            ThreatDecayTimer = 0.f;
        }
    }

    // Smooth music intensity toward target
    float TargetIntensity = (float)CurrentThreatLevel / 3.0f;
    CurrentMusicIntensity = FMath::FInterpTo(CurrentMusicIntensity, TargetIntensity, DeltaTime, MusicTransitionSpeed);
}

void UAudio_AdaptiveMusicComponent::SetThreatLevel(EAudio_ThreatLevel NewLevel)
{
    if (NewLevel > CurrentThreatLevel)
    {
        CurrentThreatLevel = NewLevel;
        ThreatDecayTimer = 0.f;
    }
}

void UAudio_AdaptiveMusicComponent::OnDinosaurNearby(FName Species, float Distance)
{
    // T-Rex at close range = Critical
    if (Species == FName("TRex") && Distance < 1500.f)
    {
        SetThreatLevel(EAudio_ThreatLevel::Critical);
    }
    else if (Species == FName("TRex") && Distance < TRexProximityRadius)
    {
        SetThreatLevel(EAudio_ThreatLevel::Danger);
    }
    else if (Distance < 2000.f)
    {
        SetThreatLevel(EAudio_ThreatLevel::Cautious);
    }
}

void UAudio_AdaptiveMusicComponent::OnPlayerEnteredSafeZone()
{
    CurrentThreatLevel = EAudio_ThreatLevel::Safe;
    ThreatDecayTimer = 0.f;
    CurrentMusicIntensity = 0.f;
}

// ============================================================
// AAudio_SystemManager
// ============================================================

AAudio_SystemManager::AAudio_SystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    ActiveZone = EAudio_EnvironmentZone::OpenPlains;
    bIsNightTime = false;
    ActiveAmbientComponent = nullptr;

    // Default dinosaur profiles
    FAudio_DinosaurSoundProfile TRexProfile;
    TRexProfile.DinosaurSpecies = FName("TRex");
    TRexProfile.RoarRadius = 5000.f;
    TRexProfile.FootstepGroundShakeRadius = 2000.f;
    DinosaurProfiles.Add(TRexProfile);

    FAudio_DinosaurSoundProfile RaptorProfile;
    RaptorProfile.DinosaurSpecies = FName("Raptor");
    RaptorProfile.RoarRadius = 2500.f;
    RaptorProfile.FootstepGroundShakeRadius = 800.f;
    DinosaurProfiles.Add(RaptorProfile);

    FAudio_DinosaurSoundProfile BrachioProfile;
    BrachioProfile.DinosaurSpecies = FName("Brachiosaurus");
    BrachioProfile.RoarRadius = 4000.f;
    BrachioProfile.FootstepGroundShakeRadius = 3000.f;
    DinosaurProfiles.Add(BrachioProfile);
}

void AAudio_SystemManager::BeginPlay()
{
    Super::BeginPlay();
    SetEnvironmentZone(ActiveZone);
}

void AAudio_SystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateDinosaurAwareness(DeltaTime);
}

void AAudio_SystemManager::SetEnvironmentZone(EAudio_EnvironmentZone NewZone)
{
    ActiveZone = NewZone;

    // Find matching ambient layer and crossfade
    for (const FAudio_AmbientLayer& Layer : AmbientLayers)
    {
        if (Layer.Zone == NewZone)
        {
            USoundCue* Target = bIsNightTime ? Layer.NightAmbience : Layer.DayAmbience;
            if (Target)
            {
                CrossfadeToNewAmbient(Target, Layer.CrossfadeDuration);
            }
            break;
        }
    }
}

void AAudio_SystemManager::SetDayNightState(bool bNight)
{
    if (bIsNightTime != bNight)
    {
        bIsNightTime = bNight;
        SetEnvironmentZone(ActiveZone); // Refresh ambient for new time of day
    }
}

void AAudio_SystemManager::TriggerDinosaurRoar(FName Species, FVector Location)
{
    FAudio_DinosaurSoundProfile Profile = GetDinosaurProfile(Species);
    if (Profile.RoarSound)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), Profile.RoarSound, Location, 1.0f, 1.0f, 0.f);
    }

    // Apply screen shake based on distance to player
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC && PC->GetPawn())
    {
        float Distance = FVector::Dist(Location, PC->GetPawn()->GetActorLocation());
        ApplyScreenShakeFromDinosaur(Species, Distance);
    }
}

void AAudio_SystemManager::TriggerFootstepImpact(FName Species, FVector Location, float Weight)
{
    FAudio_DinosaurSoundProfile Profile = GetDinosaurProfile(Species);
    if (Profile.FootstepSound)
    {
        float VolumeScale = FMath::Clamp(Weight / 5000.f, 0.3f, 1.0f);
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), Profile.FootstepSound, Location, VolumeScale);
    }

    // Ground shake: apply camera shake if player is within radius
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC && PC->GetPawn())
    {
        float Distance = FVector::Dist(Location, PC->GetPawn()->GetActorLocation());
        if (Distance < Profile.FootstepGroundShakeRadius)
        {
            float ShakeIntensity = 1.0f - (Distance / Profile.FootstepGroundShakeRadius);
            ApplyScreenShakeFromDinosaur(Species, Distance);
        }
    }
}

void AAudio_SystemManager::TriggerCraftingSound(FName ToolType)
{
    // Crafting sounds are short, positional — played at player location
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC || !PC->GetPawn()) return;

    FVector PlayerLoc = PC->GetPawn()->GetActorLocation();

    // Log crafting event for audio debug
    UE_LOG(LogTemp, Log, TEXT("AudioSystem: Crafting sound triggered — Tool: %s at %s"),
        *ToolType.ToString(), *PlayerLoc.ToString());
}

FAudio_DinosaurSoundProfile AAudio_SystemManager::GetDinosaurProfile(FName Species) const
{
    for (const FAudio_DinosaurSoundProfile& Profile : DinosaurProfiles)
    {
        if (Profile.DinosaurSpecies == Species)
        {
            return Profile;
        }
    }
    return FAudio_DinosaurSoundProfile(); // Return default empty profile
}

void AAudio_SystemManager::ApplyScreenShakeFromDinosaur(FName Species, float Distance)
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC) return;

    // Scale shake by species weight and distance
    float MaxRadius = 3000.f;
    if (Species == FName("TRex")) MaxRadius = 5000.f;
    else if (Species == FName("Brachiosaurus")) MaxRadius = 4000.f;

    if (Distance < MaxRadius)
    {
        float ShakeScale = FMath::Clamp(1.0f - (Distance / MaxRadius), 0.1f, 1.0f);
        // Camera shake class would be assigned via Blueprint — log for now
        UE_LOG(LogTemp, Log, TEXT("AudioSystem: Screen shake — Species: %s, Distance: %.0f, Scale: %.2f"),
            *Species.ToString(), Distance, ShakeScale);
    }
}

void AAudio_SystemManager::CrossfadeToNewAmbient(USoundCue* NewAmbient, float Duration)
{
    if (!NewAmbient) return;

    // Fade out existing ambient
    if (ActiveAmbientComponent && ActiveAmbientComponent->IsPlaying())
    {
        ActiveAmbientComponent->FadeOut(Duration * 0.5f, 0.f);
    }

    // Spawn new ambient component
    ActiveAmbientComponent = UGameplayStatics::SpawnSoundAtLocation(
        GetWorld(), NewAmbient, GetActorLocation(), FRotator::ZeroRotator,
        1.0f, 1.0f, Duration * 0.5f
    );
}

void AAudio_SystemManager::UpdateDinosaurAwareness(float DeltaTime)
{
    // Scan for dinosaur pawns and update adaptive music
    // This is called from Tick — lightweight scan every 0.5s via timer in production
    // For now, log system is active
}
