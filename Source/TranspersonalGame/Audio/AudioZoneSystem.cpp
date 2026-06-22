// AudioZoneSystem.cpp
// Agent #16 — Audio Agent | PROD_CYCLE_AUTO_20260622_003
// Implementation of adaptive audio zones for prehistoric survival world.
// Campfire ambience at elder camp, raptor distant calls on patrol path, wind on open plains.

#include "AudioZoneSystem.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

// ─── AAudio_ZoneActor ─────────────────────────────────────────────────────────

AAudio_ZoneActor::AAudio_ZoneActor()
{
    PrimaryActorTick.bCanEverTick = true;

    // Trigger sphere — detects player proximity
    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->SetSphereRadius(500.0f);
    TriggerSphere->SetCollisionProfileName(TEXT("Trigger"));
    RootComponent = TriggerSphere;

    // Ambient audio component — looping background sound
    AmbientAudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudioComp"));
    AmbientAudioComp->SetupAttachment(RootComponent);
    AmbientAudioComp->bAutoActivate = false;
    AmbientAudioComp->SetVolumeMultiplier(0.0f);  // Start silent, fade in on enter

    // Threat audio component — one-shot distant calls
    ThreatAudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("ThreatAudioComp"));
    ThreatAudioComp->SetupAttachment(RootComponent);
    ThreatAudioComp->bAutoActivate = false;

    // Default zone config
    ZoneConfig.ZoneType = EAudio_ZoneType::OpenPlains;
    ZoneConfig.BlendRadius = 500.0f;
    ZoneConfig.AmbientVolume = 0.8f;
    ZoneConfig.ThreatCallInterval = 12.0f;
    ZoneConfig.bLoopAmbient = true;
    ZoneConfig.bHasThreatCalls = false;
}

void AAudio_ZoneActor::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AAudio_ZoneActor::OnSphereBeginOverlap);
    TriggerSphere->OnComponentEndOverlap.AddDynamic(this, &AAudio_ZoneActor::OnSphereEndOverlap);

    // Register with the world audio subsystem
    if (UWorld* World = GetWorld())
    {
        if (UAudio_WorldSubsystem* AudioMgr = World->GetSubsystem<UAudio_WorldSubsystem>())
        {
            AudioMgr->RegisterZone(this);
        }
    }

    // Configure zone-specific defaults
    switch (ZoneConfig.ZoneType)
    {
        case EAudio_ZoneType::TribalCamp:
            ZoneConfig.bHasThreatCalls = false;
            ZoneConfig.AmbientVolume = 0.9f;
            ZoneConfig.ThreatCallInterval = 20.0f;
            break;

        case EAudio_ZoneType::RaptorPatrol:
            ZoneConfig.bHasThreatCalls = true;
            ZoneConfig.AmbientVolume = 0.6f;
            ZoneConfig.ThreatCallInterval = 8.0f;
            CurrentState.ThreatLevel = EAudio_ThreatLevel::Cautious;
            break;

        case EAudio_ZoneType::OpenPlains:
            ZoneConfig.bHasThreatCalls = false;
            ZoneConfig.AmbientVolume = 0.7f;
            ZoneConfig.ThreatCallInterval = 30.0f;
            break;

        case EAudio_ZoneType::DenseForest:
            ZoneConfig.bHasThreatCalls = true;
            ZoneConfig.AmbientVolume = 0.85f;
            ZoneConfig.ThreatCallInterval = 10.0f;
            break;

        case EAudio_ZoneType::RiverBank:
            ZoneConfig.bHasThreatCalls = false;
            ZoneConfig.AmbientVolume = 0.75f;
            break;

        case EAudio_ZoneType::CaveEntrance:
            ZoneConfig.bHasThreatCalls = true;
            ZoneConfig.AmbientVolume = 0.5f;
            ZoneConfig.ThreatCallInterval = 15.0f;
            CurrentState.ThreatLevel = EAudio_ThreatLevel::Danger;
            break;

        default:
            break;
    }

    ThreatCallAccumulator = 0.0f;
}

void AAudio_ZoneActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Unregister from subsystem
    if (UWorld* World = GetWorld())
    {
        if (UAudio_WorldSubsystem* AudioMgr = World->GetSubsystem<UAudio_WorldSubsystem>())
        {
            AudioMgr->UnregisterZone(this);
        }
    }

    Super::EndPlay(EndPlayReason);
}

void AAudio_ZoneActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bPlayerInZone && ZoneConfig.bHasThreatCalls)
    {
        TickThreatCallTimer(DeltaTime);
    }

    // Update time-since-last-threat-call in state
    CurrentState.TimeSinceLastThreatCall += DeltaTime;
}

void AAudio_ZoneActor::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                              bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor->IsA<ACharacter>())
    {
        bPlayerInZone = true;
        CurrentState.ActiveZone = ZoneConfig.ZoneType;
        CurrentState.bPlayerNearCampfire = (ZoneConfig.ZoneType == EAudio_ZoneType::TribalCamp);

        // Activate ambient audio
        if (AmbientAudioComp && !AmbientAudioComp->IsPlaying())
        {
            AmbientAudioComp->Play();
            AmbientAudioComp->SetVolumeMultiplier(ZoneConfig.AmbientVolume);
        }

        OnPlayerEnterZone(OtherActor);
    }
}

void AAudio_ZoneActor::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                           UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor && OtherActor->IsA<ACharacter>())
    {
        bPlayerInZone = false;
        CurrentState.bPlayerNearCampfire = false;

        // Fade out ambient audio
        if (AmbientAudioComp && AmbientAudioComp->IsPlaying())
        {
            AmbientAudioComp->SetVolumeMultiplier(0.0f);
            AmbientAudioComp->Stop();
        }

        OnPlayerExitZone(OtherActor);
    }
}

void AAudio_ZoneActor::OnPlayerEnterZone_Implementation(AActor* PlayerActor)
{
    // Blueprint can override — default: log entry
    UE_LOG(LogTemp, Log, TEXT("AudioZone: Player entered zone type %d"),
           static_cast<int32>(ZoneConfig.ZoneType));
}

void AAudio_ZoneActor::OnPlayerExitZone_Implementation(AActor* PlayerActor)
{
    UE_LOG(LogTemp, Log, TEXT("AudioZone: Player exited zone type %d"),
           static_cast<int32>(ZoneConfig.ZoneType));
}

void AAudio_ZoneActor::SetThreatLevel(EAudio_ThreatLevel NewThreatLevel)
{
    CurrentState.ThreatLevel = NewThreatLevel;

    // Adjust ambient volume based on threat
    float VolumeScale = 1.0f;
    switch (NewThreatLevel)
    {
        case EAudio_ThreatLevel::Safe:     VolumeScale = 1.0f;  break;
        case EAudio_ThreatLevel::Cautious: VolumeScale = 0.8f;  break;
        case EAudio_ThreatLevel::Danger:   VolumeScale = 0.5f;  break;
        case EAudio_ThreatLevel::Critical: VolumeScale = 0.2f;  break;
    }

    if (AmbientAudioComp && AmbientAudioComp->IsPlaying())
    {
        AmbientAudioComp->SetVolumeMultiplier(ZoneConfig.AmbientVolume * VolumeScale);
    }

    // Increase threat call frequency on danger/critical
    if (NewThreatLevel == EAudio_ThreatLevel::Danger || NewThreatLevel == EAudio_ThreatLevel::Critical)
    {
        ZoneConfig.bHasThreatCalls = true;
        ZoneConfig.ThreatCallInterval = (NewThreatLevel == EAudio_ThreatLevel::Critical) ? 4.0f : 6.0f;
    }
}

void AAudio_ZoneActor::TickThreatCallTimer(float DeltaTime)
{
    ThreatCallAccumulator += DeltaTime;

    if (ThreatCallAccumulator >= ZoneConfig.ThreatCallInterval)
    {
        ThreatCallAccumulator = 0.0f;
        CurrentState.TimeSinceLastThreatCall = 0.0f;
        PlayThreatCall();
    }
}

void AAudio_ZoneActor::PlayThreatCall()
{
    // Play threat audio — sound asset assigned in Blueprint/editor
    if (ThreatAudioComp && !ThreatAudioComp->IsPlaying())
    {
        ThreatAudioComp->Play();
    }

    UE_LOG(LogTemp, Log, TEXT("AudioZone: Threat call played — zone %d, threat %d"),
           static_cast<int32>(ZoneConfig.ZoneType),
           static_cast<int32>(CurrentState.ThreatLevel));
}

// ─── UAudio_WorldSubsystem ────────────────────────────────────────────────────

void UAudio_WorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    RegisteredZones.Empty();
    UE_LOG(LogTemp, Log, TEXT("AudioWorldSubsystem: Initialized"));
}

void UAudio_WorldSubsystem::Deinitialize()
{
    RegisteredZones.Empty();
    Super::Deinitialize();
}

void UAudio_WorldSubsystem::RegisterZone(AAudio_ZoneActor* Zone)
{
    if (Zone && !RegisteredZones.Contains(Zone))
    {
        RegisteredZones.Add(Zone);
        UE_LOG(LogTemp, Log, TEXT("AudioWorldSubsystem: Registered zone %s (total: %d)"),
               *Zone->GetActorLabel(), RegisteredZones.Num());
    }
}

void UAudio_WorldSubsystem::UnregisterZone(AAudio_ZoneActor* Zone)
{
    if (Zone)
    {
        RegisteredZones.Remove(Zone);
    }
}

AAudio_ZoneActor* UAudio_WorldSubsystem::GetActiveZoneForLocation(FVector WorldLocation) const
{
    AAudio_ZoneActor* NearestZone = nullptr;
    float NearestDist = FLT_MAX;

    for (AAudio_ZoneActor* Zone : RegisteredZones)
    {
        if (!Zone) continue;

        float Dist = FVector::Dist(Zone->GetActorLocation(), WorldLocation);
        if (Dist < Zone->ZoneConfig.BlendRadius && Dist < NearestDist)
        {
            NearestDist = Dist;
            NearestZone = Zone;
        }
    }

    return NearestZone;
}

void UAudio_WorldSubsystem::BroadcastThreatLevel(FVector ThreatOrigin, float Radius, EAudio_ThreatLevel Level)
{
    for (AAudio_ZoneActor* Zone : RegisteredZones)
    {
        if (!Zone) continue;

        float Dist = FVector::Dist(Zone->GetActorLocation(), ThreatOrigin);
        if (Dist <= Radius)
        {
            Zone->SetThreatLevel(Level);
        }
    }

    GlobalState.ThreatLevel = Level;

    UE_LOG(LogTemp, Log, TEXT("AudioWorldSubsystem: Broadcast threat level %d from (%.0f,%.0f,%.0f) r=%.0f"),
           static_cast<int32>(Level), ThreatOrigin.X, ThreatOrigin.Y, ThreatOrigin.Z, Radius);
}
