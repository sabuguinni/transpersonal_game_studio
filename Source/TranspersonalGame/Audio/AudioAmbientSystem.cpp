// AudioAmbientSystem.cpp — Audio Agent #16 — PROD_CYCLE_AUTO_20260701_002
// Full implementation of adaptive ambient audio system
#include "AudioAmbientSystem.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ============================================================
// UAudio_AmbientZoneComponent
// ============================================================

UAudio_AmbientZoneComponent::UAudio_AmbientZoneComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz is enough for audio blending
}

void UAudio_AmbientZoneComponent::BeginPlay()
{
    Super::BeginPlay();
    SpawnAudioComponents();
}

void UAudio_AmbientZoneComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    CleanupAudioComponents();
    Super::EndPlay(EndPlayReason);
}

void UAudio_AmbientZoneComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    UpdateAudioBlend(DeltaTime);
}

void UAudio_AmbientZoneComponent::SetDangerState(EAudio_DangerState NewState)
{
    if (CurrentDangerState == NewState) return;
    CurrentDangerState = NewState;

    // When danger hits Critical, silence insect layers if configured
    if (ZoneConfig.bSilenceInsectsOnDanger && NewState == EAudio_DangerState::Critical)
    {
        for (UAudioComponent* AudioComp : ActiveAudioComponents)
        {
            if (AudioComp && AudioComp->IsPlaying())
            {
                // Fade out insect/ambient layers rapidly on critical danger
                AudioComp->AdjustVolume(0.5f, 0.0f);
            }
        }
    }
}

void UAudio_AmbientZoneComponent::FadeInZone(float FadeTime)
{
    bIsActive = true;
    for (UAudioComponent* AudioComp : ActiveAudioComponents)
    {
        if (AudioComp)
        {
            if (!AudioComp->IsPlaying())
            {
                AudioComp->Play();
            }
            AudioComp->AdjustVolume(FadeTime, 1.0f);
        }
    }
}

void UAudio_AmbientZoneComponent::FadeOutZone(float FadeTime)
{
    bIsActive = false;
    for (UAudioComponent* AudioComp : ActiveAudioComponents)
    {
        if (AudioComp && AudioComp->IsPlaying())
        {
            AudioComp->AdjustVolume(FadeTime, 0.0f);
        }
    }
}

void UAudio_AmbientZoneComponent::UpdateAudioBlend(float DeltaTime)
{
    if (!bIsActive) return;

    // Smooth blend alpha toward target based on danger state
    float TargetAlpha = 1.0f;
    switch (CurrentDangerState)
    {
        case EAudio_DangerState::Safe:     TargetAlpha = 1.0f; break;
        case EAudio_DangerState::Cautious: TargetAlpha = 0.7f; break;
        case EAudio_DangerState::Danger:   TargetAlpha = 0.3f; break;
        case EAudio_DangerState::Critical: TargetAlpha = 0.0f; break;
    }

    CurrentBlendAlpha = FMath::FInterpTo(CurrentBlendAlpha, TargetAlpha, DeltaTime, 2.0f);

    // Apply blend to audio components
    for (int32 i = 0; i < ActiveAudioComponents.Num() && i < ZoneConfig.AmbientLayers.Num(); ++i)
    {
        UAudioComponent* AudioComp = ActiveAudioComponents[i];
        if (AudioComp)
        {
            float TargetVolume = ZoneConfig.AmbientLayers[i].BaseVolume * CurrentBlendAlpha;
            AudioComp->SetVolumeMultiplier(TargetVolume);
        }
    }
}

void UAudio_AmbientZoneComponent::SpawnAudioComponents()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    for (const FAudio_AmbientLayer& Layer : ZoneConfig.AmbientLayers)
    {
        if (!Layer.Sound) continue;

        UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAttached(
            Layer.Sound,
            Owner->GetRootComponent(),
            NAME_None,
            FVector::ZeroVector,
            EAttachLocation::KeepRelativeOffset,
            false,   // bStopWhenAttachedToDestroyed
            Layer.BaseVolume,
            1.0f,    // PitchMultiplier
            0.0f,    // StartTime
            nullptr, // AttenuationSettings
            nullptr, // ConcurrencySettings
            false    // bAutoDestroy — keep alive for looping
        );

        if (AudioComp)
        {
            AudioComp->bIsUISound = false;
            AudioComp->SetVolumeMultiplier(0.0f); // Start silent, fade in
            ActiveAudioComponents.Add(AudioComp);
        }
    }
}

void UAudio_AmbientZoneComponent::CleanupAudioComponents()
{
    for (UAudioComponent* AudioComp : ActiveAudioComponents)
    {
        if (AudioComp)
        {
            AudioComp->Stop();
            AudioComp->DestroyComponent();
        }
    }
    ActiveAudioComponents.Empty();
}

// ============================================================
// AAudio_AmbientZoneActor
// ============================================================

AAudio_AmbientZoneActor::AAudio_AmbientZoneActor()
{
    PrimaryActorTick.bCanEverTick = false;

    ZoneBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("ZoneBounds"));
    ZoneBounds->SetBoxExtent(FVector(1000.0f, 1000.0f, 500.0f));
    ZoneBounds->SetCollisionProfileName(TEXT("OverlapAll"));
    ZoneBounds->SetGenerateOverlapEvents(true);
    RootComponent = ZoneBounds;

    AmbientZoneComponent = CreateDefaultSubobject<UAudio_AmbientZoneComponent>(TEXT("AmbientZoneComponent"));
}

void AAudio_AmbientZoneActor::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    ZoneBounds->OnComponentBeginOverlap.AddDynamic(this, &AAudio_AmbientZoneActor::HandleBeginOverlap);
    ZoneBounds->OnComponentEndOverlap.AddDynamic(this, &AAudio_AmbientZoneActor::HandleEndOverlap);

    // Register with the danger state manager
    if (UWorld* World = GetWorld())
    {
        UAudio_DangerStateManager* DangerMgr = World->GetSubsystem<UAudio_DangerStateManager>();
        if (DangerMgr)
        {
            DangerMgr->RegisterAmbientZone(this);
        }
    }
}

void AAudio_AmbientZoneActor::HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor->IsA<ACharacter>())
    {
        OnPlayerEnterZone(OtherActor);
    }
}

void AAudio_AmbientZoneActor::HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor && OtherActor->IsA<ACharacter>())
    {
        OnPlayerExitZone(OtherActor);
    }
}

void AAudio_AmbientZoneActor::OnPlayerEnterZone(AActor* OverlappingActor)
{
    if (AmbientZoneComponent)
    {
        AmbientZoneComponent->FadeInZone(2.0f);
    }
}

void AAudio_AmbientZoneActor::OnPlayerExitZone(AActor* OverlappingActor)
{
    if (AmbientZoneComponent)
    {
        AmbientZoneComponent->FadeOutZone(3.0f);
    }
}

void AAudio_AmbientZoneActor::BroadcastDangerState(EAudio_DangerState NewState)
{
    if (AmbientZoneComponent)
    {
        AmbientZoneComponent->SetDangerState(NewState);
    }
}

// ============================================================
// UAudio_DangerStateManager
// ============================================================

void UAudio_DangerStateManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    GlobalDangerState = EAudio_DangerState::Safe;
    DangerCooldownTimer = 0.0f;
    UE_LOG(LogTemp, Log, TEXT("UAudio_DangerStateManager: Initialized — danger state tracking active"));
}

void UAudio_DangerStateManager::Deinitialize()
{
    RegisteredZones.Empty();
    Super::Deinitialize();
}

void UAudio_DangerStateManager::SetGlobalDangerState(EAudio_DangerState NewState)
{
    if (GlobalDangerState == NewState) return;
    GlobalDangerState = NewState;

    UE_LOG(LogTemp, Log, TEXT("UAudio_DangerStateManager: Global danger state -> %d"), (int32)NewState);

    // Broadcast to all registered zones
    for (TWeakObjectPtr<AAudio_AmbientZoneActor>& ZonePtr : RegisteredZones)
    {
        if (ZonePtr.IsValid())
        {
            ZonePtr->BroadcastDangerState(NewState);
        }
    }

    // Reset cooldown when danger increases
    if (NewState > EAudio_DangerState::Safe)
    {
        DangerCooldownTimer = DangerCooldownDuration;
    }
}

void UAudio_DangerStateManager::RegisterAmbientZone(AAudio_AmbientZoneActor* Zone)
{
    if (Zone)
    {
        RegisteredZones.AddUnique(Zone);
        UE_LOG(LogTemp, Log, TEXT("UAudio_DangerStateManager: Registered zone %s"), *Zone->GetName());
    }
}

void UAudio_DangerStateManager::UnregisterAmbientZone(AAudio_AmbientZoneActor* Zone)
{
    RegisteredZones.RemoveAll([Zone](const TWeakObjectPtr<AAudio_AmbientZoneActor>& Ptr)
    {
        return !Ptr.IsValid() || Ptr.Get() == Zone;
    });
}

void UAudio_DangerStateManager::EvaluateDangerFromDinosaurProximity(float TRexDistance, float RaptorDistance)
{
    EAudio_DangerState NewState = EAudio_DangerState::Safe;

    if (TRexDistance < TRexDangerRadius * 0.4f || RaptorDistance < RaptorDangerRadius * 0.3f)
    {
        NewState = EAudio_DangerState::Critical;
    }
    else if (TRexDistance < TRexDangerRadius * 0.7f || RaptorDistance < RaptorDangerRadius * 0.6f)
    {
        NewState = EAudio_DangerState::Danger;
    }
    else if (TRexDistance < TRexDangerRadius || RaptorDistance < RaptorDangerRadius)
    {
        NewState = EAudio_DangerState::Cautious;
    }

    // Apply cooldown — don't drop danger state too quickly
    if (NewState < GlobalDangerState && DangerCooldownTimer > 0.0f)
    {
        return; // Still in cooldown, maintain current danger state
    }

    SetGlobalDangerState(NewState);
}
