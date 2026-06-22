// AudioZoneSystem.cpp
// Agent #16 — Audio Agent | PROD_CYCLE_AUTO_20260622_011
// Full implementation of ambient audio zone system for prehistoric survival game.

#include "AudioZoneSystem.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

// ─────────────────────────────────────────────────────────────────────────────
// AAudio_AmbientZone
// ─────────────────────────────────────────────────────────────────────────────

AAudio_AmbientZone::AAudio_AmbientZone()
{
    PrimaryActorTick.bCanEverTick = true;

    ZoneSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ZoneSphere"));
    ZoneSphere->SetSphereRadius(500.0f);
    ZoneSphere->SetCollisionProfileName(TEXT("Trigger"));
    RootComponent = ZoneSphere;

    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    AmbientAudioComponent->SetupAttachment(RootComponent);
    AmbientAudioComponent->bAutoActivate = false;
    AmbientAudioComponent->SetVolumeMultiplier(0.0f);
}

void AAudio_AmbientZone::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    ZoneSphere->OnComponentBeginOverlap.AddDynamic(this, &AAudio_AmbientZone::OnSphereBeginOverlap);
    ZoneSphere->OnComponentEndOverlap.AddDynamic(this, &AAudio_AmbientZone::OnSphereEndOverlap);

    // Apply zone radius from config
    ZoneSphere->SetSphereRadius(ZoneConfig.BlendRadius);

    // Assign sound if set
    if (AmbientSound)
    {
        AmbientAudioComponent->SetSound(AmbientSound);
    }

    // Start playing at zero volume (will fade in when player enters)
    if (ZoneConfig.bLooping && AmbientAudioComponent->Sound)
    {
        AmbientAudioComponent->Play();
        AmbientAudioComponent->SetVolumeMultiplier(0.0f);
    }
}

void AAudio_AmbientZone::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (AmbientAudioComponent && AmbientAudioComponent->IsPlaying())
    {
        AmbientAudioComponent->Stop();
    }
    Super::EndPlay(EndPlayReason);
}

void AAudio_AmbientZone::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Smooth fade in/out
    if (bFadingIn)
    {
        FadeTimer += DeltaTime;
        float Alpha = FMath::Clamp(FadeTimer / FMath::Max(ZoneConfig.FadeInTime, 0.01f), 0.0f, 1.0f);
        CurrentVolume = FMath::Lerp(0.0f, ZoneConfig.BaseVolume, Alpha);
        AmbientAudioComponent->SetVolumeMultiplier(CurrentVolume);

        if (Alpha >= 1.0f)
        {
            bFadingIn = false;
            FadeTimer = 0.0f;
        }
    }
    else if (bFadingOut)
    {
        FadeTimer += DeltaTime;
        float Alpha = FMath::Clamp(FadeTimer / FMath::Max(ZoneConfig.FadeOutTime, 0.01f), 0.0f, 1.0f);
        CurrentVolume = FMath::Lerp(ZoneConfig.BaseVolume, 0.0f, Alpha);
        AmbientAudioComponent->SetVolumeMultiplier(CurrentVolume);

        if (Alpha >= 1.0f)
        {
            bFadingOut = false;
            FadeTimer = 0.0f;
            if (AmbientAudioComponent->IsPlaying() && !ZoneConfig.bLooping)
            {
                AmbientAudioComponent->Stop();
            }
        }
    }
}

void AAudio_AmbientZone::OnPlayerEnterZone(AActor* Player)
{
    if (!Player) return;
    bPlayerInZone = true;
    bFadingIn = true;
    bFadingOut = false;
    FadeTimer = 0.0f;

    if (!AmbientAudioComponent->IsPlaying() && AmbientAudioComponent->Sound)
    {
        AmbientAudioComponent->Play();
    }

    UE_LOG(LogTemp, Log, TEXT("AudioZone [%s]: Player entered — fading in (%.1fs)"),
           *GetActorLabel(), ZoneConfig.FadeInTime);
}

void AAudio_AmbientZone::OnPlayerExitZone(AActor* Player)
{
    if (!Player) return;
    bPlayerInZone = false;
    bFadingOut = true;
    bFadingIn = false;
    FadeTimer = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("AudioZone [%s]: Player exited — fading out (%.1fs)"),
           *GetActorLabel(), ZoneConfig.FadeOutTime);
}

void AAudio_AmbientZone::SetDangerLevel(EAudio_DangerLevel NewDanger)
{
    CurrentDanger = NewDanger;

    // Adjust volume multiplier based on danger — danger suppresses ambient
    float DangerMult = 1.0f;
    switch (NewDanger)
    {
        case EAudio_DangerLevel::Cautious:  DangerMult = 0.7f; break;
        case EAudio_DangerLevel::Danger:    DangerMult = 0.4f; break;
        case EAudio_DangerLevel::Critical:  DangerMult = 0.1f; break;
        default:                            DangerMult = 1.0f; break;
    }

    if (bPlayerInZone)
    {
        AmbientAudioComponent->SetVolumeMultiplier(ZoneConfig.BaseVolume * DangerMult);
    }
}

float AAudio_AmbientZone::GetDistanceBlend(AActor* Player) const
{
    if (!Player) return 0.0f;
    float Dist = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
    float Radius = ZoneConfig.BlendRadius;
    return FMath::Clamp(1.0f - (Dist / Radius), 0.0f, 1.0f);
}

void AAudio_AmbientZone::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp,
                                               AActor* OtherActor,
                                               UPrimitiveComponent* OtherComp,
                                               int32 OtherBodyIndex,
                                               bool bFromSweep,
                                               const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor->IsA<ACharacter>())
    {
        OnPlayerEnterZone(OtherActor);
    }
}

void AAudio_AmbientZone::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComp,
                                             AActor* OtherActor,
                                             UPrimitiveComponent* OtherComp,
                                             int32 OtherBodyIndex)
{
    if (OtherActor && OtherActor->IsA<ACharacter>())
    {
        OnPlayerExitZone(OtherActor);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// AAudio_DinoCallEmitter
// ─────────────────────────────────────────────────────────────────────────────

AAudio_DinoCallEmitter::AAudio_DinoCallEmitter()
{
    PrimaryActorTick.bCanEverTick = true;

    CallAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("DinoCallAudio"));
    RootComponent = CallAudioComponent;
    CallAudioComponent->bAutoActivate = false;
}

void AAudio_DinoCallEmitter::BeginPlay()
{
    Super::BeginPlay();
    ScheduleNextCall();
}

void AAudio_DinoCallEmitter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TimeUntilNextCall -= DeltaTime;
    if (TimeUntilNextCall <= 0.0f)
    {
        TriggerCall();
        ScheduleNextCall();
    }
}

void AAudio_DinoCallEmitter::TriggerCall()
{
    if (CallSounds.Num() == 0) return;

    // Pick random call from array
    int32 Idx = FMath::RandRange(0, CallSounds.Num() - 1);
    USoundBase* ChosenSound = CallSounds[Idx];
    if (!ChosenSound) return;

    // Play at world location with attenuation
    UGameplayStatics::PlaySoundAtLocation(this, ChosenSound, GetActorLocation(),
                                          1.0f, 1.0f, 0.0f, nullptr, nullptr);

    UE_LOG(LogTemp, Log, TEXT("DinoCallEmitter [%s]: %s vocalization triggered"),
           *GetActorLabel(), *DinoCallConfig.DinoSpecies);
}

void AAudio_DinoCallEmitter::ScheduleNextCall()
{
    TimeUntilNextCall = FMath::FRandRange(DinoCallConfig.MinCallInterval,
                                          DinoCallConfig.MaxCallInterval);
}

// ─────────────────────────────────────────────────────────────────────────────
// UAudio_SoundscapeManager
// ─────────────────────────────────────────────────────────────────────────────

UAudio_SoundscapeManager::UAudio_SoundscapeManager()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UAudio_SoundscapeManager::BeginPlay()
{
    Super::BeginPlay();

    // Auto-discover all ambient zones and emitters in the world
    UWorld* World = GetWorld();
    if (!World) return;

    TArray<AActor*> FoundZones;
    UGameplayStatics::GetAllActorsOfClass(World, AAudio_AmbientZone::StaticClass(), FoundZones);
    for (AActor* A : FoundZones)
    {
        if (AAudio_AmbientZone* Zone = Cast<AAudio_AmbientZone>(A))
        {
            RegisteredZones.Add(Zone);
        }
    }

    TArray<AActor*> FoundEmitters;
    UGameplayStatics::GetAllActorsOfClass(World, AAudio_DinoCallEmitter::StaticClass(), FoundEmitters);
    for (AActor* A : FoundEmitters)
    {
        if (AAudio_DinoCallEmitter* Emitter = Cast<AAudio_DinoCallEmitter>(A))
        {
            RegisteredEmitters.Add(Emitter);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("SoundscapeManager: %d zones, %d emitters registered"),
           RegisteredZones.Num(), RegisteredEmitters.Num());
}

void UAudio_SoundscapeManager::TickComponent(float DeltaTime, ELevelTick TickType,
                                              FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    // Manager tick — future: dynamic mix updates based on time of day / weather
}

void UAudio_SoundscapeManager::RegisterZone(AAudio_AmbientZone* Zone)
{
    if (Zone && !RegisteredZones.Contains(Zone))
    {
        RegisteredZones.Add(Zone);
    }
}

void UAudio_SoundscapeManager::RegisterEmitter(AAudio_DinoCallEmitter* Emitter)
{
    if (Emitter && !RegisteredEmitters.Contains(Emitter))
    {
        RegisteredEmitters.Add(Emitter);
    }
}

void UAudio_SoundscapeManager::SetGlobalDanger(EAudio_DangerLevel NewLevel)
{
    GlobalDangerLevel = NewLevel;
    for (AAudio_AmbientZone* Zone : RegisteredZones)
    {
        if (Zone)
        {
            Zone->SetDangerLevel(NewLevel);
        }
    }
}

void UAudio_SoundscapeManager::NotifyTRexProximity(float Distance)
{
    // T-Rex proximity drives danger level and ambient suppression
    // Distance thresholds (cm): 3000 = danger, 1500 = critical
    if (Distance < 1500.0f)
    {
        SetGlobalDanger(EAudio_DangerLevel::Critical);
    }
    else if (Distance < 3000.0f)
    {
        SetGlobalDanger(EAudio_DangerLevel::Danger);
    }
    else if (Distance < 6000.0f)
    {
        SetGlobalDanger(EAudio_DangerLevel::Cautious);
    }
    else
    {
        SetGlobalDanger(EAudio_DangerLevel::Safe);
    }

    UE_LOG(LogTemp, Log, TEXT("SoundscapeManager: TRex dist=%.0f -> DangerLevel=%d"),
           Distance, (int32)GlobalDangerLevel);
}
