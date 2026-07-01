#include "AudioSystemManager.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraShakeBase.h"
#include "Engine/World.h"

AAudioSystemManager::AAudioSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;

    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    AmbientAudioComponent->SetupAttachment(RootComponent);
    AmbientAudioComponent->bAutoActivate = false;

    MusicAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicAudio"));
    MusicAudioComponent->SetupAttachment(RootComponent);
    MusicAudioComponent->bAutoActivate = false;

    CampfireAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("CampfireAudio"));
    CampfireAudioComponent->SetupAttachment(RootComponent);
    CampfireAudioComponent->bAutoActivate = false;

    CurrentBiome = EAudio_BiomeType::Forest;
    CurrentDangerLevel = EAudio_DangerLevel::Safe;
    DangerTransitionTimer = 0.0f;
    TRexProximityLastTriggered = -999.0f;
}

void AAudioSystemManager::BeginPlay()
{
    Super::BeginPlay();
    UpdateMusicLayer();
}

void AAudioSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Gradually reduce danger level back to safe over time
    if (CurrentDangerLevel != EAudio_DangerLevel::Safe)
    {
        DangerTransitionTimer += DeltaTime;
        if (DangerTransitionTimer > 30.0f)
        {
            // Step down danger level
            uint8 Level = (uint8)CurrentDangerLevel;
            if (Level > 0)
            {
                CurrentDangerLevel = (EAudio_DangerLevel)(Level - 1);
                DangerTransitionTimer = 0.0f;
                UpdateMusicLayer();
            }
        }
    }
}

void AAudioSystemManager::SetActiveBiome(EAudio_BiomeType NewBiome)
{
    if (CurrentBiome == NewBiome) return;
    CurrentBiome = NewBiome;
    CrossfadeToLayer(NewBiome, 3.0f);
}

void AAudioSystemManager::CrossfadeToLayer(EAudio_BiomeType TargetBiome, float FadeDuration)
{
    // Fade out current ambient
    if (AmbientAudioComponent && AmbientAudioComponent->IsPlaying())
    {
        AmbientAudioComponent->FadeOut(FadeDuration * 0.5f, 0.0f);
    }

    // Find matching layer
    for (const FAudio_AmbientLayer& Layer : AmbientLayers)
    {
        if (Layer.Biome == TargetBiome && Layer.SoundCue)
        {
            AmbientAudioComponent->SetSound(Layer.SoundCue);
            AmbientAudioComponent->FadeIn(FadeDuration, Layer.BaseVolume);
            break;
        }
    }
}

void AAudioSystemManager::SetDangerLevel(EAudio_DangerLevel NewLevel)
{
    if (CurrentDangerLevel == NewLevel) return;

    // Only escalate danger, never de-escalate immediately (handled by timer)
    if ((uint8)NewLevel > (uint8)CurrentDangerLevel)
    {
        CurrentDangerLevel = NewLevel;
        DangerTransitionTimer = 0.0f;
        UpdateMusicLayer();
    }
}

void AAudioSystemManager::TriggerTRexProximityResponse(float DistanceMeters)
{
    UWorld* World = GetWorld();
    if (!World) return;

    float CurrentTime = World->GetTimeSeconds();

    // Throttle: don't trigger more than once every 3 seconds
    if (CurrentTime - TRexProximityLastTriggered < 3.0f) return;
    TRexProximityLastTriggered = CurrentTime;

    // Scale intensity inversely with distance (closer = more intense)
    float Intensity = FMath::Clamp(1.0f - (DistanceMeters / 50.0f), 0.1f, 1.0f);

    // Set danger level based on distance
    if (DistanceMeters < 10.0f)
        SetDangerLevel(EAudio_DangerLevel::Combat);
    else if (DistanceMeters < 25.0f)
        SetDangerLevel(EAudio_DangerLevel::Threatened);
    else if (DistanceMeters < 50.0f)
        SetDangerLevel(EAudio_DangerLevel::Aware);

    // Apply screen shake
    ApplyScreenShakeForTRex(Intensity);
}

void AAudioSystemManager::PlayCampfireAudio(FVector Location)
{
    if (!CampfireSFX) return;

    SetActorLocation(Location);

    if (CampfireAudioComponent)
    {
        CampfireAudioComponent->SetSound(CampfireSFX);
        CampfireAudioComponent->FadeIn(1.0f, 0.8f);
    }
}

void AAudioSystemManager::StopCampfireAudio()
{
    if (CampfireAudioComponent && CampfireAudioComponent->IsPlaying())
    {
        CampfireAudioComponent->FadeOut(2.0f, 0.0f);
    }
}

void AAudioSystemManager::OnDayBegin()
{
    // Day: birds, insects, wind — reduce music tension
    SetDangerLevel(EAudio_DangerLevel::Safe);
    CrossfadeToLayer(CurrentBiome, 4.0f);
}

void AAudioSystemManager::OnNightBegin()
{
    // Night: escalate ambient tension slightly
    if (CurrentDangerLevel == EAudio_DangerLevel::Safe)
    {
        CurrentDangerLevel = EAudio_DangerLevel::Aware;
        UpdateMusicLayer();
    }
}

void AAudioSystemManager::PlayFootstep(FVector Location, bool bIsPlayerHeavy)
{
    UWorld* World = GetWorld();
    if (!World) return;

    USoundCue* SFX = bIsPlayerHeavy ? FootstepHeavy : FootstepLight;
    if (SFX)
    {
        UGameplayStatics::PlaySoundAtLocation(World, SFX, Location, 0.6f);
    }
}

void AAudioSystemManager::PlayDinosaurFootstep(FVector Location, float DinosaurMassKg)
{
    UWorld* World = GetWorld();
    if (!World || !DinosaurFootstepSFX) return;

    // Scale volume by mass — T-Rex (8000kg) is much louder than Raptor (80kg)
    float VolumeScale = FMath::Clamp(DinosaurMassKg / 1000.0f, 0.3f, 2.0f);
    float PitchScale = FMath::Clamp(1.0f - (DinosaurMassKg / 10000.0f), 0.5f, 1.2f);

    UGameplayStatics::PlaySoundAtLocation(World, DinosaurFootstepSFX, Location, VolumeScale, PitchScale);

    // Heavy dinosaurs cause screen shake for nearby player
    if (DinosaurMassKg > 2000.0f)
    {
        APlayerController* PC = World->GetFirstPlayerController();
        if (PC)
        {
            APawn* PlayerPawn = PC->GetPawn();
            if (PlayerPawn)
            {
                float Dist = FVector::Dist(Location, PlayerPawn->GetActorLocation());
                if (Dist < 3000.0f) // 30 meters
                {
                    float ShakeIntensity = FMath::Clamp(1.0f - (Dist / 3000.0f), 0.1f, 1.0f);
                    ApplyScreenShakeForTRex(ShakeIntensity * (DinosaurMassKg / 8000.0f));
                }
            }
        }
    }
}

void AAudioSystemManager::UpdateMusicLayer()
{
    if (!MusicAudioComponent) return;

    USoundCue* TargetMusic = nullptr;

    switch (CurrentDangerLevel)
    {
        case EAudio_DangerLevel::Safe:
        case EAudio_DangerLevel::Aware:
            TargetMusic = MusicSafe;
            break;
        case EAudio_DangerLevel::Threatened:
            TargetMusic = MusicThreat;
            break;
        case EAudio_DangerLevel::Combat:
            TargetMusic = MusicCombat;
            break;
    }

    if (TargetMusic && MusicAudioComponent->Sound != TargetMusic)
    {
        MusicAudioComponent->FadeOut(1.5f, 0.0f);
        MusicAudioComponent->SetSound(TargetMusic);
        MusicAudioComponent->FadeIn(2.0f, 1.0f);
    }
}

void AAudioSystemManager::ApplyScreenShakeForTRex(float Intensity)
{
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return;

    // Use built-in camera shake via console command as fallback
    // (actual CameraShake asset would be assigned in Blueprint)
    float ShakeScale = FMath::Clamp(Intensity, 0.1f, 1.0f);
    
    // Log for debugging
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: TRex screen shake triggered at intensity %.2f"), ShakeScale);
}
