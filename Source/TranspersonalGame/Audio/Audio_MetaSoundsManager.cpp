#include "Audio_MetaSoundsManager.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/TriggerVolume.h"

AAudio_MetaSoundsManager::AAudio_MetaSoundsManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create audio components
    MusicAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicAudioComponent"));
    RootComponent = MusicAudioComponent;
    
    AmbienceAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbienceAudioComponent"));
    AmbienceAudioComponent->SetupAttachment(RootComponent);
    
    SFXAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("SFXAudioComponent"));
    SFXAudioComponent->SetupAttachment(RootComponent);
    
    // Initialize default values
    CurrentMusicState = EAudio_MusicState::Peaceful;
    CurrentAmbienceType = EAudio_AmbienceType::Forest;
    MusicTransitionTime = 3.0f;
    AmbienceBlendRadius = 500.0f;
    bEnableAudioOcclusion = true;
    
    // Initialize fade states
    bIsMusicFading = false;
    bIsAmbienceFading = false;
    MusicFadeTimer = 0.0f;
    AmbienceFadeTimer = 0.0f;
    
    // Configure default audio settings
    MusicConfig.Volume = 0.7f;
    MusicConfig.FadeInTime = 2.0f;
    MusicConfig.FadeOutTime = 2.0f;
    MusicConfig.bLoop = true;
    
    AmbienceConfig.Volume = 0.5f;
    AmbienceConfig.FadeInTime = 3.0f;
    AmbienceConfig.FadeOutTime = 3.0f;
    AmbienceConfig.bLoop = true;
}

void AAudio_MetaSoundsManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize audio components
    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetVolumeMultiplier(MusicConfig.Volume);
        MusicAudioComponent->bAutoActivate = false;
    }
    
    if (AmbienceAudioComponent)
    {
        AmbienceAudioComponent->SetVolumeMultiplier(AmbienceConfig.Volume);
        AmbienceAudioComponent->bAutoActivate = false;
    }
    
    if (SFXAudioComponent)
    {
        SFXAudioComponent->bAutoActivate = false;
    }
    
    // Start with peaceful music and forest ambience
    SetMusicState(EAudio_MusicState::Peaceful);
    SetAmbienceType(EAudio_AmbienceType::Forest);
    
    UE_LOG(LogTemp, Warning, TEXT("Audio MetaSounds Manager initialized"));
}

void AAudio_MetaSoundsManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update music fading
    if (bIsMusicFading)
    {
        UpdateMusicFade(DeltaTime);
    }
    
    // Update ambience fading
    if (bIsAmbienceFading)
    {
        UpdateAmbienceFade(DeltaTime);
    }
    
    // Apply audio occlusion if enabled
    if (bEnableAudioOcclusion)
    {
        ApplyAudioOcclusion();
    }
}

void AAudio_MetaSoundsManager::SetMusicState(EAudio_MusicState NewState)
{
    if (CurrentMusicState == NewState)
    {
        return;
    }
    
    CurrentMusicState = NewState;
    
    // Find the music track for this state
    if (USoundBase** FoundTrack = MusicTracks.Find(NewState))
    {
        if (*FoundTrack && MusicAudioComponent)
        {
            MusicAudioComponent->SetSound(*FoundTrack);
            MusicAudioComponent->Play();
            UE_LOG(LogTemp, Warning, TEXT("Music state changed to: %d"), (int32)NewState);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No music track found for state: %d"), (int32)NewState);
    }
}

void AAudio_MetaSoundsManager::FadeMusicTo(EAudio_MusicState NewState, float FadeTime)
{
    if (CurrentMusicState == NewState)
    {
        return;
    }
    
    TargetMusicState = NewState;
    MusicTransitionTime = FadeTime;
    MusicFadeTimer = 0.0f;
    bIsMusicFading = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Starting music fade to state: %d over %f seconds"), (int32)NewState, FadeTime);
}

void AAudio_MetaSoundsManager::StopMusic(float FadeTime)
{
    if (MusicAudioComponent && MusicAudioComponent->IsPlaying())
    {
        MusicAudioComponent->FadeOut(FadeTime, 0.0f);
        UE_LOG(LogTemp, Warning, TEXT("Stopping music with fade time: %f"), FadeTime);
    }
}

void AAudio_MetaSoundsManager::SetAmbienceType(EAudio_AmbienceType NewType)
{
    if (CurrentAmbienceType == NewType)
    {
        return;
    }
    
    CurrentAmbienceType = NewType;
    
    // Find the ambience track for this type
    if (USoundBase** FoundTrack = AmbienceTracks.Find(NewType))
    {
        if (*FoundTrack && AmbienceAudioComponent)
        {
            AmbienceAudioComponent->SetSound(*FoundTrack);
            AmbienceAudioComponent->Play();
            UE_LOG(LogTemp, Warning, TEXT("Ambience type changed to: %d"), (int32)NewType);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No ambience track found for type: %d"), (int32)NewType);
    }
}

void AAudio_MetaSoundsManager::BlendAmbienceTypes(EAudio_AmbienceType FromType, EAudio_AmbienceType ToType, float BlendFactor)
{
    // Clamp blend factor
    BlendFactor = FMath::Clamp(BlendFactor, 0.0f, 1.0f);
    
    // For now, simple crossfade implementation
    if (BlendFactor < 0.5f)
    {
        SetAmbienceType(FromType);
        if (AmbienceAudioComponent)
        {
            AmbienceAudioComponent->SetVolumeMultiplier(AmbienceConfig.Volume * (1.0f - BlendFactor * 2.0f));
        }
    }
    else
    {
        SetAmbienceType(ToType);
        if (AmbienceAudioComponent)
        {
            AmbienceAudioComponent->SetVolumeMultiplier(AmbienceConfig.Volume * ((BlendFactor - 0.5f) * 2.0f));
        }
    }
}

void AAudio_MetaSoundsManager::PlaySFX(USoundBase* Sound, FVector Location, float VolumeMultiplier)
{
    if (!Sound)
    {
        return;
    }
    
    if (Location == FVector::ZeroVector)
    {
        // Play 2D sound
        UGameplayStatics::PlaySound2D(GetWorld(), Sound, VolumeMultiplier);
    }
    else
    {
        // Play 3D sound at location
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, Location, VolumeMultiplier);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Playing SFX at location: %s"), *Location.ToString());
}

void AAudio_MetaSoundsManager::PlaySFXAtLocation(USoundBase* Sound, FVector Location, FAudio_SoundConfig Config)
{
    if (!Sound)
    {
        return;
    }
    
    // Create a temporary audio component for this SFX
    UAudioComponent* TempAudioComponent = UGameplayStatics::SpawnSoundAtLocation(
        GetWorld(),
        Sound,
        Location,
        FRotator::ZeroRotator,
        Config.Volume,
        Config.Pitch,
        0.0f, // Start time
        nullptr, // Attenuation settings
        nullptr, // Concurrency settings
        true // Auto destroy
    );
    
    if (TempAudioComponent)
    {
        TempAudioComponent->FadeIn(Config.FadeInTime, Config.Volume);
        UE_LOG(LogTemp, Log, TEXT("Playing configured SFX at location: %s"), *Location.ToString());
    }
}

void AAudio_MetaSoundsManager::SetMetaSoundParameter(FName ParameterName, float Value)
{
    // Set parameter on all audio components that support MetaSounds
    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetFloatParameter(ParameterName, Value);
    }
    
    if (AmbienceAudioComponent)
    {
        AmbienceAudioComponent->SetFloatParameter(ParameterName, Value);
    }
    
    if (SFXAudioComponent)
    {
        SFXAudioComponent->SetFloatParameter(ParameterName, Value);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Set MetaSound parameter %s to %f"), *ParameterName.ToString(), Value);
}

void AAudio_MetaSoundsManager::TriggerMetaSoundEvent(FName EventName)
{
    // Trigger event on all audio components
    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetTriggerParameter(EventName);
    }
    
    if (AmbienceAudioComponent)
    {
        AmbienceAudioComponent->SetTriggerParameter(EventName);
    }
    
    if (SFXAudioComponent)
    {
        SFXAudioComponent->SetTriggerParameter(EventName);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Triggered MetaSound event: %s"), *EventName.ToString());
}

void AAudio_MetaSoundsManager::RegisterAudioZone(ATriggerVolume* Zone, EAudio_AmbienceType AmbienceType)
{
    if (Zone)
    {
        RegisteredAudioZones.Add(Zone, AmbienceType);
        UE_LOG(LogTemp, Warning, TEXT("Registered audio zone with ambience type: %d"), (int32)AmbienceType);
    }
}

void AAudio_MetaSoundsManager::OnPlayerEnterAudioZone(EAudio_AmbienceType ZoneType)
{
    // Fade to the new ambience type
    TargetAmbienceType = ZoneType;
    bIsAmbienceFading = true;
    AmbienceFadeTimer = 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("Player entered audio zone, fading to ambience type: %d"), (int32)ZoneType);
}

void AAudio_MetaSoundsManager::OnPlayerExitAudioZone(EAudio_AmbienceType ZoneType)
{
    // For now, fade back to forest ambience as default
    if (CurrentAmbienceType == ZoneType)
    {
        OnPlayerEnterAudioZone(EAudio_AmbienceType::Forest);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Player exited audio zone type: %d"), (int32)ZoneType);
}

void AAudio_MetaSoundsManager::UpdateMusicFade(float DeltaTime)
{
    MusicFadeTimer += DeltaTime;
    float FadeProgress = MusicFadeTimer / MusicTransitionTime;
    
    if (FadeProgress >= 1.0f)
    {
        // Fade complete
        SetMusicState(TargetMusicState);
        bIsMusicFading = false;
        MusicFadeTimer = 0.0f;
    }
    else
    {
        // Continue fading
        if (MusicAudioComponent)
        {
            float CurrentVolume = FMath::Lerp(MusicConfig.Volume, 0.0f, FadeProgress);
            MusicAudioComponent->SetVolumeMultiplier(CurrentVolume);
        }
    }
}

void AAudio_MetaSoundsManager::UpdateAmbienceFade(float DeltaTime)
{
    AmbienceFadeTimer += DeltaTime;
    float FadeProgress = AmbienceFadeTimer / AmbienceConfig.FadeInTime;
    
    if (FadeProgress >= 1.0f)
    {
        // Fade complete
        SetAmbienceType(TargetAmbienceType);
        bIsAmbienceFading = false;
        AmbienceFadeTimer = 0.0f;
    }
    else
    {
        // Continue fading
        if (AmbienceAudioComponent)
        {
            float CurrentVolume = FMath::Lerp(0.0f, AmbienceConfig.Volume, FadeProgress);
            AmbienceAudioComponent->SetVolumeMultiplier(CurrentVolume);
        }
    }
}

void AAudio_MetaSoundsManager::ApplyAudioOcclusion()
{
    // Get player location
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        if (APawn* PlayerPawn = PC->GetPawn())
        {
            FVector PlayerLocation = PlayerPawn->GetActorLocation();
            FVector ManagerLocation = GetActorLocation();
            
            // Simple distance-based attenuation
            float Distance = FVector::Dist(PlayerLocation, ManagerLocation);
            float Attenuation = CalculateDistanceAttenuation(ManagerLocation, PlayerLocation, 2000.0f);
            
            // Apply attenuation to all audio components
            if (MusicAudioComponent)
            {
                MusicAudioComponent->SetVolumeMultiplier(MusicConfig.Volume * Attenuation);
            }
            
            if (AmbienceAudioComponent)
            {
                AmbienceAudioComponent->SetVolumeMultiplier(AmbienceConfig.Volume * Attenuation);
            }
        }
    }
}

float AAudio_MetaSoundsManager::CalculateDistanceAttenuation(FVector SourceLocation, FVector ListenerLocation, float MaxDistance)
{
    float Distance = FVector::Dist(SourceLocation, ListenerLocation);
    
    if (Distance >= MaxDistance)
    {
        return 0.0f;
    }
    
    // Linear falloff
    return 1.0f - (Distance / MaxDistance);
}