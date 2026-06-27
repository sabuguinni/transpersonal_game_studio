// AudioZoneManager.cpp
// Agent #16 — Audio Agent | PROD_CYCLE_AUTO_20260627_012
// Full implementation of ambient audio zones, screen shake, and proximity dialogue.

#include "AudioZoneManager.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

AAudioZoneManager::AAudioZoneManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Root component
    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->InitSphereRadius(1000.0f);
    TriggerSphere->SetCollisionProfileName(TEXT("Trigger"));
    RootComponent = TriggerSphere;

    // Ambient audio component
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    AmbientAudioComponent->SetupAttachment(RootComponent);
    AmbientAudioComponent->bAutoActivate = false;
    AmbientAudioComponent->bIsUISound = false;
}

// ─────────────────────────────────────────────────────────────────────────────
// BeginPlay
// ─────────────────────────────────────────────────────────────────────────────

void AAudioZoneManager::BeginPlay()
{
    Super::BeginPlay();

    // Sync sphere radius from config
    if (TriggerSphere)
    {
        TriggerSphere->SetSphereRadius(ZoneConfig.TriggerRadius);
        TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AAudioZoneManager::OnPlayerEnterZone);
        TriggerSphere->OnComponentEndOverlap.AddDynamic(this, &AAudioZoneManager::OnPlayerExitZone);
    }

    // Load and assign ambient sound if available
    if (ZoneConfig.AmbientLoop.IsValid() && AmbientAudioComponent)
    {
        USoundBase* Sound = ZoneConfig.AmbientLoop.LoadSynchronous();
        if (Sound)
        {
            AmbientAudioComponent->SetSound(Sound);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// EndPlay
// ─────────────────────────────────────────────────────────────────────────────

void AAudioZoneManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorldTimerManager().ClearTimer(DialogueTimerHandle);
    Super::EndPlay(EndPlayReason);
}

// ─────────────────────────────────────────────────────────────────────────────
// Tick
// ─────────────────────────────────────────────────────────────────────────────

void AAudioZoneManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Screen shake: check proximity of any large dinosaur pawns
    if (bEnableScreenShake && bPlayerInZone)
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (PC && PC->GetPawn())
        {
            FVector PlayerLoc = PC->GetPawn()->GetActorLocation();
            FVector MyLoc = GetActorLocation();
            float Dist = FVector::Dist(PlayerLoc, MyLoc);

            if (Dist < ScreenShakeConfig.TriggerRadius)
            {
                // Intensity scales with proximity
                float ProximityFactor = 1.0f - (Dist / ScreenShakeConfig.TriggerRadius);
                float FinalIntensity = ScreenShakeConfig.ShakeIntensity * ProximityFactor;

                if (FinalIntensity > 0.1f)
                {
                    PC->ClientStartCameraShake(
                        UCameraShakeBase::StaticClass(),
                        FinalIntensity
                    );
                }
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Zone Overlap Callbacks
// ─────────────────────────────────────────────────────────────────────────────

void AAudioZoneManager::OnPlayerEnterZone(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!OtherActor) return;

    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (!PlayerChar) return;

    APlayerController* PC = Cast<APlayerController>(PlayerChar->GetController());
    if (!PC) return;

    bPlayerInZone = true;
    FadeInAmbient();

    // Auto-start dialogue if configured
    if (DialogueLines.Num() > 0 && !bDialogueActive)
    {
        StartDialogue();
    }
}

void AAudioZoneManager::OnPlayerExitZone(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex)
{
    if (!OtherActor) return;

    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (!PlayerChar) return;

    APlayerController* PC = Cast<APlayerController>(PlayerChar->GetController());
    if (!PC) return;

    bPlayerInZone = false;
    FadeOutAmbient();
    StopDialogue();
}

// ─────────────────────────────────────────────────────────────────────────────
// Intensity Control
// ─────────────────────────────────────────────────────────────────────────────

void AAudioZoneManager::SetIntensity(EAudio_IntensityLevel NewIntensity)
{
    if (CurrentIntensity == NewIntensity) return;
    CurrentIntensity = NewIntensity;

    if (!AmbientAudioComponent) return;

    switch (NewIntensity)
    {
        case EAudio_IntensityLevel::Calm:
            AmbientAudioComponent->SetVolumeMultiplier(ZoneConfig.Volume);
            AmbientAudioComponent->SetPitchMultiplier(1.0f);
            break;

        case EAudio_IntensityLevel::Tense:
            AmbientAudioComponent->SetVolumeMultiplier(ZoneConfig.Volume * 0.7f);
            AmbientAudioComponent->SetPitchMultiplier(0.95f);
            break;

        case EAudio_IntensityLevel::Combat:
            AmbientAudioComponent->SetVolumeMultiplier(ZoneConfig.Volume * 0.3f);
            AmbientAudioComponent->SetPitchMultiplier(0.85f);
            break;

        case EAudio_IntensityLevel::Flee:
            AmbientAudioComponent->SetVolumeMultiplier(0.0f);
            break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Ambient Fade
// ─────────────────────────────────────────────────────────────────────────────

void AAudioZoneManager::FadeInAmbient(float FadeTime)
{
    if (!AmbientAudioComponent) return;
    float Time = (FadeTime < 0.0f) ? ZoneConfig.FadeInTime : FadeTime;
    AmbientAudioComponent->FadeIn(Time, ZoneConfig.Volume);
}

void AAudioZoneManager::FadeOutAmbient(float FadeTime)
{
    if (!AmbientAudioComponent) return;
    float Time = (FadeTime < 0.0f) ? ZoneConfig.FadeOutTime : FadeTime;
    AmbientAudioComponent->FadeOut(Time, 0.0f);
}

// ─────────────────────────────────────────────────────────────────────────────
// Screen Shake
// ─────────────────────────────────────────────────────────────────────────────

void AAudioZoneManager::TriggerScreenShakeAtLocation(FVector DinosaurLocation, float DinosaurMass)
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC || !PC->GetPawn()) return;

    FVector PlayerLoc = PC->GetPawn()->GetActorLocation();
    float Dist = FVector::Dist(PlayerLoc, DinosaurLocation);

    if (Dist > ScreenShakeConfig.TriggerRadius) return;

    float ProximityFactor = 1.0f - (Dist / ScreenShakeConfig.TriggerRadius);
    float MassFactor = FMath::Clamp(DinosaurMass, 0.1f, 3.0f);
    float FinalIntensity = ScreenShakeConfig.ShakeIntensity * ProximityFactor * MassFactor;

    if (FinalIntensity > 0.05f)
    {
        PC->ClientStartCameraShake(UCameraShakeBase::StaticClass(), FinalIntensity);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Dialogue System
// ─────────────────────────────────────────────────────────────────────────────

void AAudioZoneManager::StartDialogue()
{
    if (DialogueLines.Num() == 0) return;

    bDialogueActive = true;
    CurrentDialogueIndex = 0;
    AdvanceDialogueInternal();
}

void AAudioZoneManager::AdvanceDialogue()
{
    AdvanceDialogueInternal();
}

void AAudioZoneManager::StopDialogue()
{
    bDialogueActive = false;
    GetWorldTimerManager().ClearTimer(DialogueTimerHandle);
}

void AAudioZoneManager::AdvanceDialogueInternal()
{
    if (!bDialogueActive) return;
    if (CurrentDialogueIndex >= DialogueLines.Num())
    {
        StopDialogue();
        return;
    }

    const FAudio_DialogueLine& Line = DialogueLines[CurrentDialogueIndex];

    // Play audio if asset is loaded
    if (Line.AudioAsset.IsValid())
    {
        USoundBase* Sound = Line.AudioAsset.LoadSynchronous();
        if (Sound)
        {
            UGameplayStatics::PlaySound2D(GetWorld(), Sound, Line.Duration > 0.0f ? 1.0f : 0.0f);
        }
    }

    CurrentDialogueIndex++;

    // Schedule next line
    float Delay = FMath::Max(Line.Duration, 1.0f);
    GetWorldTimerManager().SetTimer(
        DialogueTimerHandle,
        this,
        &AAudioZoneManager::AdvanceDialogueInternal,
        Delay,
        false
    );
}
