#include "Audio_NarrativeAudioManager.h"
#include "Components/SceneComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

AAudio_NarrativeAudioManager::AAudio_NarrativeAudioManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create audio components
    NarrativeAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("NarrativeAudioComponent"));
    NarrativeAudioComponent->SetupAttachment(RootComponent);
    NarrativeAudioComponent->bAutoActivate = false;

    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudioComponent"));
    AmbientAudioComponent->SetupAttachment(RootComponent);
    AmbientAudioComponent->bAutoActivate = false;

    // Initialize default settings
    MasterVolume = 1.0f;
    NarrativeVolume = 0.8f;
    AmbientVolume = 0.6f;
    FadeInTime = 2.0f;
    FadeOutTime = 1.5f;

    // Internal state
    CurrentNarrativeClip = TEXT("");
    CurrentAmbientZone = TEXT("");
    LastTriggerCheckTime = 0.0f;
    TriggerCheckInterval = 0.5f; // Check triggers twice per second
    PlayerPawn = nullptr;
}

void AAudio_NarrativeAudioManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAudioComponents();
    UpdatePlayerReference();

    // Initialize default narrative clips for testing
    FAudio_NarrativeClip CaveClip;
    CaveClip.ClipName = TEXT("CaveWarning");
    CaveClip.TriggerLocation = TEXT("CaveEntrance");
    CaveClip.TriggerRadius = 800.0f;
    CaveClip.bPlayOnce = false;
    CaveClip.CooldownTime = 45.0f;
    NarrativeClips.Add(CaveClip);

    FAudio_NarrativeClip RiverClip;
    RiverClip.ClipName = TEXT("RiverCrossing");
    RiverClip.TriggerLocation = TEXT("RiverCrossing");
    RiverClip.TriggerRadius = 600.0f;
    RiverClip.bPlayOnce = true;
    RiverClip.CooldownTime = 0.0f;
    NarrativeClips.Add(RiverClip);

    // Initialize default ambient zones
    FAudio_AmbientZone ForestZone;
    ForestZone.ZoneName = TEXT("Forest");
    ForestZone.ZoneLocation = FVector(0, 0, 0);
    ForestZone.ZoneRadius = 2000.0f;
    ForestZone.VolumeMultiplier = 0.7f;
    ForestZone.bLooping = true;
    AmbientZones.Add(ForestZone);

    FAudio_AmbientZone CaveZone;
    CaveZone.ZoneName = TEXT("Cave");
    CaveZone.ZoneLocation = FVector(2000, 0, 0);
    CaveZone.ZoneRadius = 1000.0f;
    CaveZone.VolumeMultiplier = 0.5f;
    CaveZone.bLooping = true;
    AmbientZones.Add(CaveZone);

    UE_LOG(LogTemp, Warning, TEXT("Audio_NarrativeAudioManager initialized with %d clips and %d zones"), 
           NarrativeClips.Num(), AmbientZones.Num());
}

void AAudio_NarrativeAudioManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update player reference if needed
    if (!PlayerPawn)
    {
        UpdatePlayerReference();
    }

    // Check triggers at intervals
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastTriggerCheckTime >= TriggerCheckInterval)
    {
        CheckNarrativeTriggers();
        CheckAmbientZones();
        LastTriggerCheckTime = CurrentTime;
    }
}

void AAudio_NarrativeAudioManager::InitializeAudioComponents()
{
    if (NarrativeAudioComponent)
    {
        NarrativeAudioComponent->SetVolumeMultiplier(NarrativeVolume * MasterVolume);
        NarrativeAudioComponent->bOverrideAttenuation = true;
    }

    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->SetVolumeMultiplier(AmbientVolume * MasterVolume);
        AmbientAudioComponent->bOverrideAttenuation = true;
    }
}

void AAudio_NarrativeAudioManager::UpdatePlayerReference()
{
    if (UWorld* World = GetWorld())
    {
        PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    }
}

void AAudio_NarrativeAudioManager::PlayNarrativeClip(const FString& ClipName)
{
    for (FAudio_NarrativeClip& Clip : NarrativeClips)
    {
        if (Clip.ClipName == ClipName && CanPlayNarrativeClip(Clip))
        {
            if (NarrativeAudioComponent && Clip.AudioClip)
            {
                // Stop current narrative audio
                StopNarrativeAudio();

                // Play new clip
                NarrativeAudioComponent->SetSound(Clip.AudioClip);
                NarrativeAudioComponent->Play();

                // Update clip state
                Clip.bHasPlayed = true;
                Clip.LastPlayTime = GetWorld()->GetTimeSeconds();
                CurrentNarrativeClip = ClipName;

                UE_LOG(LogTemp, Warning, TEXT("Playing narrative clip: %s"), *ClipName);
                return;
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Could not play narrative clip: %s"), *ClipName);
}

void AAudio_NarrativeAudioManager::StopNarrativeAudio()
{
    if (NarrativeAudioComponent && NarrativeAudioComponent->IsPlaying())
    {
        NarrativeAudioComponent->Stop();
        CurrentNarrativeClip = TEXT("");
    }
}

void AAudio_NarrativeAudioManager::SetAmbientZone(const FString& ZoneName)
{
    if (CurrentAmbientZone == ZoneName)
    {
        return; // Already in this zone
    }

    for (FAudio_AmbientZone& Zone : AmbientZones)
    {
        if (Zone.ZoneName == ZoneName)
        {
            // Stop current ambient audio
            StopAmbientAudio();

            // Start new zone audio
            if (AmbientAudioComponent && Zone.AmbientSound)
            {
                AmbientAudioComponent->SetSound(Zone.AmbientSound);
                AmbientAudioComponent->SetVolumeMultiplier(Zone.VolumeMultiplier * AmbientVolume * MasterVolume);
                AmbientAudioComponent->Play();

                Zone.bIsActive = true;
                CurrentAmbientZone = ZoneName;

                UE_LOG(LogTemp, Warning, TEXT("Switched to ambient zone: %s"), *ZoneName);
            }
            return;
        }
    }
}

void AAudio_NarrativeAudioManager::StopAmbientAudio()
{
    if (AmbientAudioComponent && AmbientAudioComponent->IsPlaying())
    {
        AmbientAudioComponent->Stop();
        
        // Mark all zones as inactive
        for (FAudio_AmbientZone& Zone : AmbientZones)
        {
            Zone.bIsActive = false;
        }
        
        CurrentAmbientZone = TEXT("");
    }
}

void AAudio_NarrativeAudioManager::CheckNarrativeTriggers()
{
    if (!PlayerPawn)
    {
        return;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();

    for (const FAudio_NarrativeClip& Clip : NarrativeClips)
    {
        if (!CanPlayNarrativeClip(Clip))
        {
            continue;
        }

        // For now, use simple distance check based on trigger location name
        // In a full implementation, this would check against actual trigger volumes
        FVector TriggerLocation = FVector::ZeroVector;
        
        if (Clip.TriggerLocation == TEXT("CaveEntrance"))
        {
            TriggerLocation = FVector(2000, 0, 100);
        }
        else if (Clip.TriggerLocation == TEXT("RiverCrossing"))
        {
            TriggerLocation = FVector(-1500, 1000, 100);
        }
        else if (Clip.TriggerLocation == TEXT("HuntingGrounds"))
        {
            TriggerLocation = FVector(0, -2000, 100);
        }
        else if (Clip.TriggerLocation == TEXT("MigrationPath"))
        {
            TriggerLocation = FVector(3000, 2000, 100);
        }

        float Distance = FVector::Dist(PlayerLocation, TriggerLocation);
        if (Distance <= Clip.TriggerRadius)
        {
            PlayNarrativeClip(Clip.ClipName);
            break; // Only play one clip at a time
        }
    }
}

void AAudio_NarrativeAudioManager::CheckAmbientZones()
{
    if (!PlayerPawn)
    {
        return;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    FString ClosestZone = TEXT("");
    float ClosestDistance = FLT_MAX;

    for (const FAudio_AmbientZone& Zone : AmbientZones)
    {
        float Distance = FVector::Dist(PlayerLocation, Zone.ZoneLocation);
        if (Distance <= Zone.ZoneRadius && Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            ClosestZone = Zone.ZoneName;
        }
    }

    if (!ClosestZone.IsEmpty() && ClosestZone != CurrentAmbientZone)
    {
        SetAmbientZone(ClosestZone);
    }
    else if (ClosestZone.IsEmpty() && !CurrentAmbientZone.IsEmpty())
    {
        StopAmbientAudio();
    }
}

void AAudio_NarrativeAudioManager::RegisterNarrativeClip(const FAudio_NarrativeClip& NewClip)
{
    NarrativeClips.Add(NewClip);
    UE_LOG(LogTemp, Warning, TEXT("Registered narrative clip: %s"), *NewClip.ClipName);
}

void AAudio_NarrativeAudioManager::RegisterAmbientZone(const FAudio_AmbientZone& NewZone)
{
    AmbientZones.Add(NewZone);
    UE_LOG(LogTemp, Warning, TEXT("Registered ambient zone: %s"), *NewZone.ZoneName);
}

bool AAudio_NarrativeAudioManager::IsNarrativeClipAvailable(const FString& ClipName)
{
    for (const FAudio_NarrativeClip& Clip : NarrativeClips)
    {
        if (Clip.ClipName == ClipName)
        {
            return CanPlayNarrativeClip(Clip);
        }
    }
    return false;
}

float AAudio_NarrativeAudioManager::GetDistanceToPlayer(const FVector& Location)
{
    if (PlayerPawn)
    {
        return FVector::Dist(PlayerPawn->GetActorLocation(), Location);
    }
    return FLT_MAX;
}

bool AAudio_NarrativeAudioManager::CanPlayNarrativeClip(const FAudio_NarrativeClip& Clip)
{
    // Check if already played and set to play once
    if (Clip.bPlayOnce && Clip.bHasPlayed)
    {
        return false;
    }

    // Check cooldown
    if (Clip.CooldownTime > 0.0f)
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        float TimeSinceLastPlay = CurrentTime - Clip.LastPlayTime;
        if (TimeSinceLastPlay < Clip.CooldownTime)
        {
            return false;
        }
    }

    // Check if currently playing narrative audio
    if (NarrativeAudioComponent && NarrativeAudioComponent->IsPlaying())
    {
        return false;
    }

    return true;
}

void AAudio_NarrativeAudioManager::FadeAudioComponent(UAudioComponent* AudioComp, float TargetVolume, float Duration)
{
    if (AudioComp)
    {
        // Simple fade implementation - in full version would use timeline or tween
        AudioComp->SetVolumeMultiplier(TargetVolume);
    }
}