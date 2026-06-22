#include "AudioZoneSystem.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ============================================================
// UAudio_ZoneComponent — Implementation
// ============================================================

UAudio_ZoneComponent::UAudio_ZoneComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz — sufficient for audio proximity

    // Default zone config
    ZoneConfig.ZoneType = EAudio_ZoneType::None;
    ZoneConfig.ProximityRadius = 400.0f;
    ZoneConfig.AmbientVolume = 0.8f;
    ZoneConfig.CrossfadeDuration = 2.5f;
    ZoneConfig.bLooping = true;

    // Default ambient layers
    FAudio_AmbientLayer WindLayer;
    WindLayer.LayerName = TEXT("Wind_Base");
    WindLayer.AssociatedZone = EAudio_ZoneType::None;
    WindLayer.BaseVolume = 0.3f;
    WindLayer.bActiveAtDay = true;
    WindLayer.bActiveAtNight = true;
    AmbientLayers.Add(WindLayer);
}

void UAudio_ZoneComponent::BeginPlay()
{
    Super::BeginPlay();

    // Initialize volume to 0 — will crossfade in when player enters
    CurrentVolume = 0.0f;
    bPlayerInZone = false;
    DistanceToPlayer = 99999.0f;
}

void UAudio_ZoneComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    UpdateProximity(DeltaTime);
}

void UAudio_ZoneComponent::UpdateProximity(float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return;

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn) return;

    AActor* Owner = GetOwner();
    if (!Owner) return;

    DistanceToPlayer = FVector::Dist(Owner->GetActorLocation(), PlayerPawn->GetActorLocation());
    bool bWasInZone = bPlayerInZone;
    bPlayerInZone = (DistanceToPlayer <= ZoneConfig.ProximityRadius);

    // Crossfade toward target volume
    float TargetVolume = ComputeTargetVolume();
    CrossfadeVolume(TargetVolume, DeltaTime);
}

float UAudio_ZoneComponent::ComputeTargetVolume() const
{
    if (!bPlayerInZone) return 0.0f;

    // Volume scales with proximity: full volume at center, fade at edge
    float NormalizedDist = FMath::Clamp(DistanceToPlayer / ZoneConfig.ProximityRadius, 0.0f, 1.0f);
    float ProximityFactor = 1.0f - (NormalizedDist * 0.4f); // 60-100% volume range

    // Threat level modifies volume
    float ThreatMultiplier = 1.0f;
    switch (CurrentThreatLevel)
    {
        case EAudio_ThreatLevel::Cautious:  ThreatMultiplier = 1.2f; break;
        case EAudio_ThreatLevel::Danger:    ThreatMultiplier = 1.5f; break;
        case EAudio_ThreatLevel::Critical:  ThreatMultiplier = 2.0f; break;
        default: break;
    }

    return FMath::Clamp(ZoneConfig.AmbientVolume * ProximityFactor * ThreatMultiplier, 0.0f, 1.0f);
}

void UAudio_ZoneComponent::CrossfadeVolume(float TargetVolume, float DeltaTime)
{
    if (ZoneConfig.CrossfadeDuration <= 0.0f)
    {
        CurrentVolume = TargetVolume;
        return;
    }

    float CrossfadeRate = 1.0f / ZoneConfig.CrossfadeDuration;
    CurrentVolume = FMath::FInterpTo(CurrentVolume, TargetVolume, DeltaTime, CrossfadeRate);
}

void UAudio_ZoneComponent::SetThreatLevel(EAudio_ThreatLevel NewThreatLevel)
{
    CurrentThreatLevel = NewThreatLevel;
}

void UAudio_ZoneComponent::SetZoneType(EAudio_ZoneType NewZoneType)
{
    ZoneConfig.ZoneType = NewZoneType;
}

bool UAudio_ZoneComponent::IsPlayerInZone() const
{
    return bPlayerInZone;
}

float UAudio_ZoneComponent::GetNormalizedProximity() const
{
    if (ZoneConfig.ProximityRadius <= 0.0f) return 0.0f;
    return FMath::Clamp(1.0f - (DistanceToPlayer / ZoneConfig.ProximityRadius), 0.0f, 1.0f);
}

void UAudio_ZoneComponent::ForceEnterZone()
{
    bPlayerInZone = true;
    CurrentVolume = ZoneConfig.AmbientVolume;
}

void UAudio_ZoneComponent::ForceExitZone()
{
    bPlayerInZone = false;
    CurrentVolume = 0.0f;
}

EAudio_ThreatLevel UAudio_ZoneComponent::GetCurrentThreatLevel() const
{
    return CurrentThreatLevel;
}

FString UAudio_ZoneComponent::GetZoneDebugInfo() const
{
    FString ZoneTypeName;
    switch (ZoneConfig.ZoneType)
    {
        case EAudio_ZoneType::Camp:         ZoneTypeName = TEXT("Camp"); break;
        case EAudio_ZoneType::PredatorNear: ZoneTypeName = TEXT("PredatorNear"); break;
        case EAudio_ZoneType::Water:        ZoneTypeName = TEXT("Water"); break;
        case EAudio_ZoneType::Cave:         ZoneTypeName = TEXT("Cave"); break;
        case EAudio_ZoneType::OpenPlains:   ZoneTypeName = TEXT("OpenPlains"); break;
        case EAudio_ZoneType::Forest:       ZoneTypeName = TEXT("Forest"); break;
        default:                            ZoneTypeName = TEXT("None"); break;
    }
    return FString::Printf(TEXT("Zone:%s InZone:%s Dist:%.0f Vol:%.2f Threat:%d"),
        *ZoneTypeName,
        bPlayerInZone ? TEXT("YES") : TEXT("NO"),
        DistanceToPlayer,
        CurrentVolume,
        (int32)CurrentThreatLevel);
}

// ============================================================
// UAudio_DinosaurSoundManager — Implementation
// ============================================================

UAudio_DinosaurSoundManager::UAudio_DinosaurSoundManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.05f; // 20Hz for footstep timing

    InitDefaultProfiles();
}

void UAudio_DinosaurSoundManager::InitDefaultProfiles()
{
    // Default to TRex-scale profile
    SoundProfile.DinosaurSpecies = FName("TRex");
    SoundProfile.RoarRadius = 2000.0f;
    SoundProfile.FootstepRadius = 800.0f;
    SoundProfile.FootstepInterval = 1.2f;
    SoundProfile.BreathingRadius = 300.0f;
    SoundProfile.RoarCooldown = 15.0f;
    SoundProfile.bCausesScreenShake = true;
    SoundProfile.ScreenShakeIntensity = 1.0f;
}

void UAudio_DinosaurSoundManager::BeginPlay()
{
    Super::BeginPlay();
    FootstepTimer = 0.0f;
    RoarCooldownTimer = 0.0f;
    bRoarOnCooldown = false;
}

void UAudio_DinosaurSoundManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateDistanceToPlayer();

    // Footstep timer
    if (bPlayerInFootstepRadius)
    {
        FootstepTimer += DeltaTime;
        if (FootstepTimer >= SoundProfile.FootstepInterval)
        {
            TriggerFootstep();
            FootstepTimer = 0.0f;
        }
    }
    else
    {
        FootstepTimer = 0.0f;
    }

    // Roar cooldown
    if (bRoarOnCooldown)
    {
        RoarCooldownTimer -= DeltaTime;
        if (RoarCooldownTimer <= 0.0f)
        {
            bRoarOnCooldown = false;
            RoarCooldownTimer = 0.0f;
        }
    }
}

void UAudio_DinosaurSoundManager::UpdateDistanceToPlayer()
{
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return;

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn) return;

    AActor* Owner = GetOwner();
    if (!Owner) return;

    CurrentDistanceToPlayer = FVector::Dist(Owner->GetActorLocation(), PlayerPawn->GetActorLocation());
    bPlayerInRoarRadius = (CurrentDistanceToPlayer <= SoundProfile.RoarRadius);
    bPlayerInFootstepRadius = (CurrentDistanceToPlayer <= SoundProfile.FootstepRadius);
}

void UAudio_DinosaurSoundManager::TriggerRoar()
{
    if (bRoarOnCooldown) return;

    // Mark cooldown
    bRoarOnCooldown = true;
    RoarCooldownTimer = SoundProfile.RoarCooldown;

    // Blueprint/MetaSound will handle actual audio playback via event
    // Screen shake is driven by GetFootstepScreenShakeIntensity()
    UE_LOG(LogTemp, Log, TEXT("AudioDino: %s ROAR triggered at dist=%.0f"),
        *SoundProfile.DinosaurSpecies.ToString(), CurrentDistanceToPlayer);
}

void UAudio_DinosaurSoundManager::TriggerFootstep()
{
    // Blueprint/MetaSound handles audio
    // Screen shake intensity computed from distance
    float ShakeIntensity = GetFootstepScreenShakeIntensity();
    if (ShakeIntensity > 0.01f)
    {
        UE_LOG(LogTemp, Verbose, TEXT("AudioDino: %s FOOTSTEP shake=%.2f dist=%.0f"),
            *SoundProfile.DinosaurSpecies.ToString(), ShakeIntensity, CurrentDistanceToPlayer);
    }
}

float UAudio_DinosaurSoundManager::GetFootstepScreenShakeIntensity() const
{
    if (!SoundProfile.bCausesScreenShake) return 0.0f;
    if (CurrentDistanceToPlayer > SoundProfile.FootstepRadius) return 0.0f;

    // Linear falloff: full intensity at 0 distance, zero at FootstepRadius
    float NormalizedDist = FMath::Clamp(CurrentDistanceToPlayer / SoundProfile.FootstepRadius, 0.0f, 1.0f);
    return SoundProfile.ScreenShakeIntensity * (1.0f - NormalizedDist);
}

bool UAudio_DinosaurSoundManager::ShouldTriggerScreenShake() const
{
    return GetFootstepScreenShakeIntensity() > 0.05f;
}

void UAudio_DinosaurSoundManager::SetDinosaurSpecies(FName Species)
{
    SoundProfile.DinosaurSpecies = Species;

    // Adjust profile by species
    if (Species == FName("TRex"))
    {
        SoundProfile.RoarRadius = 2000.0f;
        SoundProfile.FootstepRadius = 800.0f;
        SoundProfile.FootstepInterval = 1.2f;
        SoundProfile.ScreenShakeIntensity = 1.0f;
    }
    else if (Species == FName("Raptor"))
    {
        SoundProfile.RoarRadius = 800.0f;
        SoundProfile.FootstepRadius = 300.0f;
        SoundProfile.FootstepInterval = 0.4f;
        SoundProfile.ScreenShakeIntensity = 0.2f;
    }
    else if (Species == FName("Brachiosaurus"))
    {
        SoundProfile.RoarRadius = 1500.0f;
        SoundProfile.FootstepRadius = 1200.0f;
        SoundProfile.FootstepInterval = 2.0f;
        SoundProfile.ScreenShakeIntensity = 1.5f;
    }
    else if (Species == FName("Stegosaurus"))
    {
        SoundProfile.RoarRadius = 600.0f;
        SoundProfile.FootstepRadius = 400.0f;
        SoundProfile.FootstepInterval = 1.0f;
        SoundProfile.ScreenShakeIntensity = 0.5f;
    }
    else if (Species == FName("Triceratops"))
    {
        SoundProfile.RoarRadius = 1000.0f;
        SoundProfile.FootstepRadius = 600.0f;
        SoundProfile.FootstepInterval = 0.9f;
        SoundProfile.ScreenShakeIntensity = 0.7f;
    }
}
