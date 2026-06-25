#include "AudioSystemManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraShakeBase.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"

// ============================================================
// UAudio_ProximityTrigger
// ============================================================

UAudio_ProximityTrigger::UAudio_ProximityTrigger()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UAudio_ProximityTrigger::BeginPlay()
{
    Super::BeginPlay();
    bHasTriggered = false;
}

void UAudio_ProximityTrigger::CheckPlayerProximity(const FVector& PlayerLocation)
{
    if (bPlayOnce && bHasTriggered) return;

    AActor* Owner = GetOwner();
    if (!Owner) return;

    float Distance = FVector::Dist(Owner->GetActorLocation(), PlayerLocation);
    if (Distance <= TriggerRadius)
    {
        USoundBase* Sound = TriggerSound.LoadSynchronous();
        if (Sound)
        {
            UGameplayStatics::PlaySoundAtLocation(
                GetWorld(),
                Sound,
                Owner->GetActorLocation(),
                Volume
            );
            bHasTriggered = true;
        }
    }
}

void UAudio_ProximityTrigger::ResetTrigger()
{
    bHasTriggered = false;
}

// ============================================================
// AAudio_SystemManager
// ============================================================

AAudio_SystemManager::AAudio_SystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update every 100ms — not every frame
}

void AAudio_SystemManager::BeginPlay()
{
    Super::BeginPlay();
    DangerMusicBlend = 0.0f;
    BiomeFadeTimer = 0.0f;
    PreviousBiome = CurrentBiome;
}

void AAudio_SystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateDynamicMix(DeltaTime);
}

void AAudio_SystemManager::SetDangerLevel(EAudio_DangerLevel NewLevel)
{
    CurrentDangerLevel = NewLevel;
}

void AAudio_SystemManager::SetCurrentBiome(EAudio_BiomeType NewBiome)
{
    if (NewBiome != CurrentBiome)
    {
        PreviousBiome = CurrentBiome;
        CurrentBiome = NewBiome;
        BiomeFadeTimer = 0.0f;
        UpdateAmbientLayers();
    }
}

void AAudio_SystemManager::UpdateNearestDinosaurDistance(float Distance)
{
    NearestDinosaurDistance = Distance;

    // Auto-escalate danger level based on proximity
    if (Distance < 500.0f)
    {
        SetDangerLevel(EAudio_DangerLevel::Critical);
    }
    else if (Distance < 1500.0f)
    {
        SetDangerLevel(EAudio_DangerLevel::Danger);
    }
    else if (Distance < 3000.0f)
    {
        SetDangerLevel(EAudio_DangerLevel::Cautious);
    }
    else
    {
        SetDangerLevel(EAudio_DangerLevel::Safe);
    }
}

void AAudio_SystemManager::PlayDinosaurFootstep(FName Species, const FVector& FootLocation)
{
    // Find matching profile
    for (const FAudio_DinosaurSoundProfile& Profile : DinosaurProfiles)
    {
        if (Profile.DinosaurSpecies == Species)
        {
            USoundBase* FootSound = Profile.FootstepSound.LoadSynchronous();
            if (FootSound)
            {
                UGameplayStatics::PlaySoundAtLocation(
                    GetWorld(),
                    FootSound,
                    FootLocation,
                    SFXVolume * MasterVolume
                );
            }

            // Trigger ground shake if player is close enough
            APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
            if (PC && PC->GetPawn())
            {
                float DistToPlayer = FVector::Dist(FootLocation, PC->GetPawn()->GetActorLocation());
                if (DistToPlayer <= Profile.FootstepGroundShakeRadius)
                {
                    float ScaledIntensity = Profile.FootstepGroundShakeIntensity *
                        (1.0f - (DistToPlayer / Profile.FootstepGroundShakeRadius));
                    TriggerScreenShakeFromFootstep(ScaledIntensity, DistToPlayer);
                }
            }
            break;
        }
    }
}

void AAudio_SystemManager::TriggerScreenShakeFromFootstep(float Intensity, float Distance)
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC) return;

    // Scale shake by intensity and distance
    float ShakeScale = FMath::Clamp(Intensity * (1.0f - Distance / 2000.0f), 0.0f, 1.0f);
    if (ShakeScale > 0.05f)
    {
        // Use built-in camera shake via console command as fallback
        // (Blueprint should assign a proper UCameraShakeBase subclass)
        UGameplayStatics::PlayWorldCameraShake(
            GetWorld(),
            nullptr, // Camera shake class — assign in Blueprint
            PC->GetPawn() ? PC->GetPawn()->GetActorLocation() : FVector::ZeroVector,
            0.0f,
            1500.0f * ShakeScale,
            ShakeScale
        );
    }
}

float AAudio_SystemManager::GetDangerMusicIntensity() const
{
    switch (CurrentDangerLevel)
    {
        case EAudio_DangerLevel::Safe:     return 0.0f;
        case EAudio_DangerLevel::Cautious: return 0.3f;
        case EAudio_DangerLevel::Danger:   return 0.7f;
        case EAudio_DangerLevel::Critical: return 1.0f;
        default: return 0.0f;
    }
}

void AAudio_SystemManager::FadeAmbientToNewBiome(EAudio_BiomeType TargetBiome, float FadeTime)
{
    SetCurrentBiome(TargetBiome);
    // Fade is handled in UpdateDynamicMix via BiomeFadeTimer
    BiomeFadeTimer = FadeTime;
}

bool AAudio_SystemManager::IsPlayerInDanger() const
{
    return CurrentDangerLevel == EAudio_DangerLevel::Danger ||
           CurrentDangerLevel == EAudio_DangerLevel::Critical;
}

void AAudio_SystemManager::UpdateDynamicMix(float DeltaTime)
{
    // Blend danger music intensity toward target
    float TargetBlend = GetDangerMusicIntensity();
    float BlendSpeed = 0.5f * DeltaTime; // Smooth transition
    DangerMusicBlend = FMath::FInterpTo(DangerMusicBlend, TargetBlend, DeltaTime, 1.5f);

    // Fade biome timer
    if (BiomeFadeTimer > 0.0f)
    {
        BiomeFadeTimer -= DeltaTime;
    }
}

void AAudio_SystemManager::UpdateAmbientLayers()
{
    // Stop all current ambient components
    for (UAudioComponent* Comp : ActiveAmbientComponents)
    {
        if (Comp && Comp->IsValidLowLevel())
        {
            Comp->FadeOut(2.0f, 0.0f);
        }
    }
    ActiveAmbientComponents.Empty();

    // Start new ambient layer for current biome
    for (const FAudio_AmbientLayer& Layer : AmbientLayers)
    {
        if (Layer.BiomeType == CurrentBiome)
        {
            USoundBase* Sound = Layer.AmbientSound.LoadSynchronous();
            if (Sound)
            {
                UAudioComponent* NewComp = UGameplayStatics::SpawnSound2D(
                    GetWorld(),
                    Sound,
                    Layer.BaseVolume * AmbientVolume * MasterVolume,
                    1.0f,
                    0.0f,
                    nullptr,
                    true,
                    true
                );
                if (NewComp)
                {
                    ActiveAmbientComponents.Add(NewComp);
                }
            }
        }
    }
}
