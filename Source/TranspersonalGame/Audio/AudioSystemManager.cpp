// AudioSystemManager.cpp
// Audio Agent #16 — Transpersonal Game Studio
// Adaptive audio system: danger-level music, ambient layers, NPC voice proximity triggers

#include "AudioSystemManager.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ============================================================
// UAudio_AmbientLayerComponent
// ============================================================

UAudio_AmbientLayerComponent::UAudio_AmbientLayerComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    CurrentDangerLevel = EAudio_DangerLevel::Safe;
    CurrentTimeOfDay   = EAudio_TimeOfDay::Day;
    DangerUpdateInterval = 1.0f;
    bAudioInitialized  = false;
}

void UAudio_AmbientLayerComponent::BeginPlay()
{
    Super::BeginPlay();

    // Populate default ambient layers for each time-of-day
    FAudio_AmbientLayer DawnLayer;
    DawnLayer.LayerName      = FName("Dawn_Ambient");
    DawnLayer.bActiveAtDawn  = true;
    DawnLayer.bActiveAtDay   = false;
    DawnLayer.bActiveAtDusk  = false;
    DawnLayer.bActiveAtNight = false;
    DawnLayer.BaseVolume     = 0.6f;
    DawnLayer.FadeInTime     = 3.0f;
    DawnLayer.FadeOutTime    = 2.0f;
    AmbientLayers.Add(DawnLayer);

    FAudio_AmbientLayer NightLayer;
    NightLayer.LayerName      = FName("Night_Ambient");
    NightLayer.bActiveAtDawn  = false;
    NightLayer.bActiveAtDay   = false;
    NightLayer.bActiveAtDusk  = false;
    NightLayer.bActiveAtNight = true;
    NightLayer.BaseVolume     = 0.8f;
    NightLayer.FadeInTime     = 4.0f;
    NightLayer.FadeOutTime    = 3.0f;
    AmbientLayers.Add(NightLayer);

    bAudioInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("AudioAmbientLayerComponent initialized with %d layers"), AmbientLayers.Num());
}

void UAudio_AmbientLayerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    // Tick-based danger update handled via timer in BeginPlay for performance
}

void UAudio_AmbientLayerComponent::SetDangerLevel(EAudio_DangerLevel NewLevel)
{
    if (CurrentDangerLevel == NewLevel) return;
    EAudio_DangerLevel OldLevel = CurrentDangerLevel;
    CurrentDangerLevel = NewLevel;
    OnDangerLevelChanged(OldLevel, NewLevel);
    UE_LOG(LogTemp, Log, TEXT("Audio danger level: %d → %d"), (int32)OldLevel, (int32)NewLevel);
}

void UAudio_AmbientLayerComponent::SetTimeOfDay(EAudio_TimeOfDay NewTime)
{
    if (CurrentTimeOfDay == NewTime) return;
    CurrentTimeOfDay = NewTime;
    UpdateAmbientLayers();
    UE_LOG(LogTemp, Log, TEXT("Audio time of day changed to: %d"), (int32)NewTime);
}

void UAudio_AmbientLayerComponent::OnDangerLevelChanged(EAudio_DangerLevel OldLevel, EAudio_DangerLevel NewLevel)
{
    // Music intensity scales with danger level
    // Safe=0.3, Aware=0.5, Threatened=0.75, Critical=1.0
    const float VolumeMap[] = { 0.3f, 0.5f, 0.75f, 1.0f };
    int32 Idx = FMath::Clamp((int32)NewLevel, 0, 3);
    float TargetVolume = VolumeMap[Idx];

    for (UAudioComponent* Comp : ActiveAudioComponents)
    {
        if (IsValid(Comp))
        {
            Comp->SetVolumeMultiplier(TargetVolume);
        }
    }
}

void UAudio_AmbientLayerComponent::UpdateAmbientLayers()
{
    for (const FAudio_AmbientLayer& Layer : AmbientLayers)
    {
        bool bShouldBeActive = false;
        switch (CurrentTimeOfDay)
        {
            case EAudio_TimeOfDay::Dawn:  bShouldBeActive = Layer.bActiveAtDawn;  break;
            case EAudio_TimeOfDay::Day:   bShouldBeActive = Layer.bActiveAtDay;   break;
            case EAudio_TimeOfDay::Dusk:  bShouldBeActive = Layer.bActiveAtDusk;  break;
            case EAudio_TimeOfDay::Night: bShouldBeActive = Layer.bActiveAtNight; break;
        }
        UE_LOG(LogTemp, Verbose, TEXT("Layer %s active=%d"), *Layer.LayerName.ToString(), bShouldBeActive);
    }
}

// ============================================================
// AAudio_ProximityVoiceTrigger
// ============================================================

AAudio_ProximityVoiceTrigger::AAudio_ProximityVoiceTrigger()
{
    PrimaryActorTick.bCanEverTick = true;

    ProximitySphere = CreateDefaultSubobject<USphereComponent>(TEXT("ProximitySphere"));
    RootComponent   = ProximitySphere;
    ProximitySphere->InitSphereRadius(400.0f);
    ProximitySphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

    VoiceAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("VoiceAudioComponent"));
    VoiceAudioComponent->SetupAttachment(RootComponent);
    VoiceAudioComponent->bAutoActivate = false;

    TriggerRadius       = 400.0f;
    bHasTriggered       = false;
    bRepeatTrigger      = false;
    RepeatCooldown      = 30.0f;
    LastTriggerTime     = -9999.0f;
    SpeakerName         = FName("Unknown");
}

void AAudio_ProximityVoiceTrigger::BeginPlay()
{
    Super::BeginPlay();
    ProximitySphere->SetSphereRadius(TriggerRadius);
    ProximitySphere->OnComponentBeginOverlap.AddDynamic(this, &AAudio_ProximityVoiceTrigger::OnPlayerEnterRadius);
    UE_LOG(LogTemp, Log, TEXT("ProximityVoiceTrigger '%s' ready — radius=%.0f, line=%s"),
        *SpeakerName.ToString(), TriggerRadius, *VoiceLineID.ToString());
}

void AAudio_ProximityVoiceTrigger::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AAudio_ProximityVoiceTrigger::OnPlayerEnterRadius(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!IsValid(OtherActor)) return;
    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (!PlayerChar) return;

    float Now = GetWorld()->GetTimeSeconds();
    bool bCooldownPassed = (Now - LastTriggerTime) > RepeatCooldown;

    if (!bHasTriggered || (bRepeatTrigger && bCooldownPassed))
    {
        PlayVoiceLine();
        bHasTriggered   = true;
        LastTriggerTime = Now;
        UE_LOG(LogTemp, Log, TEXT("ProximityVoiceTrigger fired: speaker=%s, line=%s"),
            *SpeakerName.ToString(), *VoiceLineID.ToString());
    }
}

void AAudio_ProximityVoiceTrigger::PlayVoiceLine()
{
    if (IsValid(VoiceAudioComponent) && VoiceAudioComponent->Sound)
    {
        VoiceAudioComponent->Play();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("ProximityVoiceTrigger: no sound asset assigned for line %s — URL=%s"),
            *VoiceLineID.ToString(), *VoiceLineURL);
    }
}

// ============================================================
// AAudio_DangerZoneActor
// ============================================================

AAudio_DangerZoneActor::AAudio_DangerZoneActor()
{
    PrimaryActorTick.bCanEverTick = true;

    DangerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DangerSphere"));
    RootComponent = DangerSphere;
    DangerSphere->InitSphereRadius(800.0f);
    DangerSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

    DangerLevel       = EAudio_DangerLevel::Aware;
    DangerRadius      = 800.0f;
    bDynamicRadius    = false;
    bPlayerInside     = false;
}

void AAudio_DangerZoneActor::BeginPlay()
{
    Super::BeginPlay();
    DangerSphere->SetSphereRadius(DangerRadius);
    DangerSphere->OnComponentBeginOverlap.AddDynamic(this, &AAudio_DangerZoneActor::OnPlayerEnter);
    DangerSphere->OnComponentEndOverlap.AddDynamic(this, &AAudio_DangerZoneActor::OnPlayerExit);
    UE_LOG(LogTemp, Log, TEXT("DangerZoneActor '%s' ready — level=%d, radius=%.0f"),
        *GetName(), (int32)DangerLevel, DangerRadius);
}

void AAudio_DangerZoneActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (bDynamicRadius)
    {
        // Dynamic radius can be driven externally (e.g., by DinosaurAI)
        DangerSphere->SetSphereRadius(DangerRadius);
    }
}

void AAudio_DangerZoneActor::OnPlayerEnter(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!IsValid(OtherActor)) return;
    if (!Cast<ACharacter>(OtherActor)) return;

    bPlayerInside = true;
    NotifyAudioSystem(DangerLevel);
    UE_LOG(LogTemp, Log, TEXT("Player entered danger zone '%s' — level=%d"), *GetName(), (int32)DangerLevel);
}

void AAudio_DangerZoneActor::OnPlayerExit(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex)
{
    if (!IsValid(OtherActor)) return;
    if (!Cast<ACharacter>(OtherActor)) return;

    bPlayerInside = false;
    NotifyAudioSystem(EAudio_DangerLevel::Safe);
    UE_LOG(LogTemp, Log, TEXT("Player exited danger zone '%s' — reverting to Safe"), *GetName());
}

void AAudio_DangerZoneActor::NotifyAudioSystem(EAudio_DangerLevel NewLevel)
{
    // Find the ambient layer component on the player character and update it
    ACharacter* Player = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (!IsValid(Player)) return;

    UAudio_AmbientLayerComponent* AudioComp = Player->FindComponentByClass<UAudio_AmbientLayerComponent>();
    if (IsValid(AudioComp))
    {
        AudioComp->SetDangerLevel(NewLevel);
    }
}

// ============================================================
// AAudio_SystemManager
// ============================================================

AAudio_SystemManager::AAudio_SystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    CurrentDangerLevel = EAudio_DangerLevel::Safe;
    CurrentTimeOfDay   = EAudio_TimeOfDay::Day;
    DayNightCycleDuration = 600.0f; // 10 minutes per full cycle
    ElapsedDayTime     = 0.0f;
    bDayNightCycleActive = true;
}

void AAudio_SystemManager::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager online — day cycle=%.0fs, danger=%d"),
        DayNightCycleDuration, (int32)CurrentDangerLevel);
}

void AAudio_SystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bDayNightCycleActive) return;

    ElapsedDayTime += DeltaTime;
    if (ElapsedDayTime >= DayNightCycleDuration)
    {
        ElapsedDayTime = 0.0f;
    }

    // Map elapsed time to time-of-day enum
    float Phase = ElapsedDayTime / DayNightCycleDuration;
    EAudio_TimeOfDay NewTime;
    if      (Phase < 0.1f)  NewTime = EAudio_TimeOfDay::Dawn;
    else if (Phase < 0.5f)  NewTime = EAudio_TimeOfDay::Day;
    else if (Phase < 0.6f)  NewTime = EAudio_TimeOfDay::Dusk;
    else                    NewTime = EAudio_TimeOfDay::Night;

    if (NewTime != CurrentTimeOfDay)
    {
        CurrentTimeOfDay = NewTime;
        BroadcastTimeOfDayChange(NewTime);
    }
}

void AAudio_SystemManager::SetGlobalDangerLevel(EAudio_DangerLevel NewLevel)
{
    if (CurrentDangerLevel == NewLevel) return;
    CurrentDangerLevel = NewLevel;
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: global danger → %d"), (int32)NewLevel);
}

void AAudio_SystemManager::RegisterVoiceLineURL(FName LineID, const FString& URL)
{
    VoiceLineURLRegistry.Add(LineID, URL);
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: registered voice line '%s'"), *LineID.ToString());
}

FString AAudio_SystemManager::GetVoiceLineURL(FName LineID) const
{
    const FString* Found = VoiceLineURLRegistry.Find(LineID);
    return Found ? *Found : FString();
}

void AAudio_SystemManager::BroadcastTimeOfDayChange(EAudio_TimeOfDay NewTime)
{
    // Notify all ambient layer components in the world
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    for (AActor* Actor : AllActors)
    {
        if (!IsValid(Actor)) continue;
        UAudio_AmbientLayerComponent* Comp = Actor->FindComponentByClass<UAudio_AmbientLayerComponent>();
        if (IsValid(Comp))
        {
            Comp->SetTimeOfDay(NewTime);
        }
    }
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: time-of-day broadcast → %d"), (int32)NewTime);
}
