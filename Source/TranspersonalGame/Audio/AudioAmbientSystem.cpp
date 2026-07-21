#include "AudioAmbientSystem.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ─── AAudio_AmbientZoneActor ──────────────────────────────────────────────────

AAudio_AmbientZoneActor::AAudio_AmbientZoneActor()
{
    PrimaryActorTick.bCanEverTick = true;

    // Trigger sphere — default 800 unit radius
    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->InitSphereRadius(800.0f);
    TriggerSphere->SetCollisionProfileName(TEXT("Trigger"));
    RootComponent = TriggerSphere;

    // Primary ambient audio
    PrimaryAmbientAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("PrimaryAmbientAudio"));
    PrimaryAmbientAudio->SetupAttachment(RootComponent);
    PrimaryAmbientAudio->bAutoActivate = false;
    PrimaryAmbientAudio->VolumeMultiplier = 0.0f;

    // Secondary ambient audio (layered blend)
    SecondaryAmbientAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("SecondaryAmbientAudio"));
    SecondaryAmbientAudio->SetupAttachment(RootComponent);
    SecondaryAmbientAudio->bAutoActivate = false;
    SecondaryAmbientAudio->VolumeMultiplier = 0.0f;
}

void AAudio_AmbientZoneActor::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap delegates
    TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AAudio_AmbientZoneActor::OnPlayerEnterZone);
    TriggerSphere->OnComponentEndOverlap.AddDynamic(this, &AAudio_AmbientZoneActor::OnPlayerExitZone);

    // Register with Audio Manager
    if (UGameInstance* GI = GetGameInstance())
    {
        if (UAudio_AmbientManager* Manager = GI->GetSubsystem<UAudio_AmbientManager>())
        {
            Manager->RegisterAmbientZone(this);
        }
    }

    // Set trigger sphere radius from zone config
    TriggerSphere->SetSphereRadius(ZoneConfig.BlendRadius);

    // Assign first ambient layer sound to primary component
    if (ZoneConfig.AmbientLayers.Num() > 0 && ZoneConfig.AmbientLayers[0].AmbientSound)
    {
        PrimaryAmbientAudio->SetSound(ZoneConfig.AmbientLayers[0].AmbientSound);
    }

    // Assign second ambient layer sound to secondary component
    if (ZoneConfig.AmbientLayers.Num() > 1 && ZoneConfig.AmbientLayers[1].AmbientSound)
    {
        SecondaryAmbientAudio->SetSound(ZoneConfig.AmbientLayers[1].AmbientSound);
    }
}

void AAudio_AmbientZoneActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Unregister from Audio Manager
    if (UGameInstance* GI = GetGameInstance())
    {
        if (UAudio_AmbientManager* Manager = GI->GetSubsystem<UAudio_AmbientManager>())
        {
            Manager->UnregisterAmbientZone(this);
        }
    }

    Super::EndPlay(EndPlayReason);
}

void AAudio_AmbientZoneActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Handle fade in
    if (bFadingIn)
    {
        FadeElapsed += DeltaTime;
        float Alpha = FMath::Clamp(FadeElapsed / FadeTargetDuration, 0.0f, 1.0f);
        CurrentBlendWeight = Alpha;

        float TargetVol = ZoneConfig.AmbientLayers.Num() > 0
            ? ZoneConfig.AmbientLayers[0].BaseVolume * Alpha
            : Alpha;

        PrimaryAmbientAudio->SetVolumeMultiplier(TargetVol);

        if (ZoneConfig.AmbientLayers.Num() > 1)
        {
            float SecVol = ZoneConfig.AmbientLayers[1].BaseVolume * Alpha * 0.6f;
            SecondaryAmbientAudio->SetVolumeMultiplier(SecVol);
        }

        if (Alpha >= 1.0f)
        {
            bFadingIn = false;
            FadeElapsed = 0.0f;
        }
    }

    // Handle fade out
    if (bFadingOut)
    {
        FadeElapsed += DeltaTime;
        float Alpha = FMath::Clamp(1.0f - (FadeElapsed / FadeTargetDuration), 0.0f, 1.0f);
        CurrentBlendWeight = Alpha;

        PrimaryAmbientAudio->SetVolumeMultiplier(Alpha);
        SecondaryAmbientAudio->SetVolumeMultiplier(Alpha * 0.6f);

        if (Alpha <= 0.0f)
        {
            bFadingOut = false;
            FadeElapsed = 0.0f;
            PrimaryAmbientAudio->Stop();
            SecondaryAmbientAudio->Stop();
        }
    }
}

void AAudio_AmbientZoneActor::SetDangerLevel(EAudio_DangerLevel NewDanger)
{
    ZoneConfig.CurrentDanger = NewDanger;

    // Adjust volume based on danger — danger increases ambient tension layer
    float DangerMultiplier = 1.0f;
    switch (NewDanger)
    {
    case EAudio_DangerLevel::Safe:      DangerMultiplier = 1.0f;  break;
    case EAudio_DangerLevel::Cautious:  DangerMultiplier = 1.2f;  break;
    case EAudio_DangerLevel::Danger:    DangerMultiplier = 1.5f;  break;
    case EAudio_DangerLevel::Critical:  DangerMultiplier = 0.4f;  break; // Near-silence at critical — tension
    }

    if (bPlayerInZone && ZoneConfig.AmbientLayers.Num() > 0)
    {
        float BaseVol = ZoneConfig.AmbientLayers[0].BaseVolume * DangerMultiplier * CurrentBlendWeight;
        PrimaryAmbientAudio->SetVolumeMultiplier(FMath::Clamp(BaseVol, 0.0f, 2.0f));
    }
}

void AAudio_AmbientZoneActor::FadeIn(float Duration)
{
    FadeTargetDuration = Duration;
    FadeElapsed = 0.0f;
    bFadingIn = true;
    bFadingOut = false;

    if (!PrimaryAmbientAudio->IsPlaying())
    {
        PrimaryAmbientAudio->Play();
    }
    if (!SecondaryAmbientAudio->IsPlaying() && ZoneConfig.AmbientLayers.Num() > 1)
    {
        SecondaryAmbientAudio->Play();
    }
}

void AAudio_AmbientZoneActor::FadeOut(float Duration)
{
    FadeTargetDuration = Duration;
    FadeElapsed = 0.0f;
    bFadingOut = true;
    bFadingIn = false;
}

void AAudio_AmbientZoneActor::OnPlayerEnterZone(UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor, UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor) return;

    // Only trigger for player-controlled characters
    ACharacter* Char = Cast<ACharacter>(OtherActor);
    if (!Char) return;

    APlayerController* PC = Cast<APlayerController>(Char->GetController());
    if (!PC) return;

    bPlayerInZone = true;

    float FadeInTime = ZoneConfig.AmbientLayers.Num() > 0
        ? ZoneConfig.AmbientLayers[0].FadeInDuration
        : 2.0f;

    FadeIn(FadeInTime);
}

void AAudio_AmbientZoneActor::OnPlayerExitZone(UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor) return;

    ACharacter* Char = Cast<ACharacter>(OtherActor);
    if (!Char) return;

    APlayerController* PC = Cast<APlayerController>(Char->GetController());
    if (!PC) return;

    bPlayerInZone = false;

    float FadeOutTime = ZoneConfig.AmbientLayers.Num() > 0
        ? ZoneConfig.AmbientLayers[0].FadeOutDuration
        : 3.0f;

    FadeOut(FadeOutTime);
}

// ─── UAudio_AmbientManager ────────────────────────────────────────────────────

void UAudio_AmbientManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Pre-register known TTS audio URLs from Agent #15 and #16 production
    RegisterTTSAudio(TEXT("TribalElder"),
        TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782890261656_TribalElder.mp3"));
    RegisterTTSAudio(TEXT("HunterWarning"),
        TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782890278138_HunterWarning.mp3"));
    RegisterTTSAudio(TEXT("TribalElder_Intro"),
        TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782890313905_TribalElder_Intro.mp3"));
    RegisterTTSAudio(TEXT("Narrator_Ambient"),
        TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782890390342_Narrator_Ambient.mp3"));
    RegisterTTSAudio(TEXT("TribalElder_FireCamp"),
        TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782890409194_TribalElder_FireCamp.mp3"));
}

void UAudio_AmbientManager::Deinitialize()
{
    RegisteredZones.Empty();
    TTSAudioRegistry.Empty();
    Super::Deinitialize();
}

void UAudio_AmbientManager::RegisterAmbientZone(AAudio_AmbientZoneActor* Zone)
{
    if (Zone && !RegisteredZones.Contains(Zone))
    {
        RegisteredZones.Add(Zone);
    }
}

void UAudio_AmbientManager::UnregisterAmbientZone(AAudio_AmbientZoneActor* Zone)
{
    RegisteredZones.Remove(Zone);
}

void UAudio_AmbientManager::BroadcastDangerLevel(EAudio_DangerLevel NewDanger)
{
    for (AAudio_AmbientZoneActor* Zone : RegisteredZones)
    {
        if (Zone && Zone->bPlayerInZone)
        {
            Zone->SetDangerLevel(NewDanger);
        }
    }
}

TArray<AAudio_AmbientZoneActor*> UAudio_AmbientManager::GetActiveZones() const
{
    TArray<AAudio_AmbientZoneActor*> ActiveZones;
    for (AAudio_AmbientZoneActor* Zone : RegisteredZones)
    {
        if (Zone && Zone->bPlayerInZone)
        {
            ActiveZones.Add(Zone);
        }
    }
    return ActiveZones;
}

void UAudio_AmbientManager::RegisterTTSAudio(const FString& CharacterName, const FString& AudioURL)
{
    TTSAudioRegistry.Add(CharacterName, AudioURL);
}

FString UAudio_AmbientManager::GetTTSAudioURL(const FString& CharacterName) const
{
    const FString* URL = TTSAudioRegistry.Find(CharacterName);
    return URL ? *URL : FString();
}
