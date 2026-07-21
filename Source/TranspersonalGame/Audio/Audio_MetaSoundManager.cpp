#include "Audio_MetaSoundManager.h"
#include "Components/SceneComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Engine/Engine.h"

AAudio_MetaSoundManager::AAudio_MetaSoundManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default ambience zones
    FAudio_AmbienceZone ForestZone;
    ForestZone.AmbienceType = EAudio_AmbienceType::Forest;
    ForestZone.Location = FVector(2000, -1000, 200);
    ForestZone.Radius = 2000.0f;
    ForestZone.Volume = 0.6f;
    AmbienceZones.Add(ForestZone);

    FAudio_AmbienceZone DangerZone;
    DangerZone.AmbienceType = EAudio_AmbienceType::Danger;
    DangerZone.Location = FVector(-2000, 3000, 150);
    DangerZone.Radius = 1500.0f;
    DangerZone.Volume = 0.8f;
    AmbienceZones.Add(DangerZone);

    FAudio_AmbienceZone CampfireZone;
    CampfireZone.AmbienceType = EAudio_AmbienceType::Campfire;
    CampfireZone.Location = FVector(500, 500, 100);
    CampfireZone.Radius = 800.0f;
    CampfireZone.Volume = 0.7f;
    AmbienceZones.Add(CampfireZone);

    FAudio_AmbienceZone DrumsZone;
    DrumsZone.AmbienceType = EAudio_AmbienceType::TribalDrums;
    DrumsZone.Location = FVector(-500, -500, 120);
    DrumsZone.Radius = 1200.0f;
    DrumsZone.Volume = 0.5f;
    AmbienceZones.Add(DrumsZone);

    // Initialize narrative triggers
    FAudio_NarrativeTrigger NarrativeTrigger1;
    NarrativeTrigger1.TriggerName = TEXT("PrehistoricNarrator_001");
    NarrativeTrigger1.TriggerLocation = FVector(1000, 1000, 150);
    NarrativeTrigger1.TriggerRadius = 600.0f;
    NarrativeTrigger1.bCanRepeat = true;
    NarrativeTriggers.Add(NarrativeTrigger1);

    FAudio_NarrativeTrigger NarrativeTrigger2;
    NarrativeTrigger2.TriggerName = TEXT("TribalScout_001");
    NarrativeTrigger2.TriggerLocation = FVector(-1000, 1000, 150);
    NarrativeTrigger2.TriggerRadius = 600.0f;
    NarrativeTrigger2.bCanRepeat = true;
    NarrativeTriggers.Add(NarrativeTrigger2);

    MasterVolume = 1.0f;
    AmbienceVolume = 0.7f;
    NarrativeVolume = 1.0f;
    FootstepVolume = 0.8f;
}

void AAudio_MetaSoundManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAudioZones();
}

void AAudio_MetaSoundManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Get player location for proximity calculations
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        FVector PlayerLocation = PlayerPawn->GetActorLocation();
        UpdatePlayerProximity(PlayerLocation);
        CheckNarrativeTriggers(PlayerLocation);
    }
    
    UpdateAmbienceAudio(DeltaTime);
}

void AAudio_MetaSoundManager::InitializeAudioZones()
{
    // Create audio components for each ambience zone
    for (int32 i = 0; i < AmbienceZones.Num(); i++)
    {
        UAudioComponent* NewAudioComponent = CreateAudioComponent();
        if (NewAudioComponent)
        {
            AudioComponents.Add(NewAudioComponent);
            
            // Set initial position
            FVector RelativeLocation = AmbienceZones[i].Location - GetActorLocation();
            NewAudioComponent->SetRelativeLocation(RelativeLocation);
            NewAudioComponent->SetVolumeMultiplier(AmbienceZones[i].Volume * AmbienceVolume * MasterVolume);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Audio MetaSound Manager initialized with %d ambience zones"), AmbienceZones.Num());
}

void AAudio_MetaSoundManager::UpdatePlayerProximity(const FVector& PlayerLocation)
{
    for (int32 i = 0; i < AmbienceZones.Num() && i < AudioComponents.Num(); i++)
    {
        if (!AmbienceZones[i].bIsActive || !AudioComponents[i])
        {
            continue;
        }
        
        float Distance = FVector::Dist(PlayerLocation, AmbienceZones[i].Location);
        float Attenuation = CalculateDistanceAttenuation(AmbienceZones[i].Location, PlayerLocation, AmbienceZones[i].Radius);
        
        float FinalVolume = AmbienceZones[i].Volume * AmbienceVolume * MasterVolume * Attenuation;
        AudioComponents[i]->SetVolumeMultiplier(FinalVolume);
        
        // Auto-play/stop based on distance
        if (Attenuation > 0.1f && !AudioComponents[i]->IsPlaying())
        {
            AudioComponents[i]->Play();
        }
        else if (Attenuation <= 0.1f && AudioComponents[i]->IsPlaying())
        {
            AudioComponents[i]->Stop();
        }
    }
}

void AAudio_MetaSoundManager::TriggerNarrativeAudio(const FString& TriggerName)
{
    for (FAudio_NarrativeTrigger& Trigger : NarrativeTriggers)
    {
        if (Trigger.TriggerName == TriggerName)
        {
            if (!Trigger.bHasTriggered || Trigger.bCanRepeat)
            {
                // Create temporary audio component for narrative playback
                UAudioComponent* NarrativeAudioComp = CreateAudioComponent();
                if (NarrativeAudioComp && Trigger.NarrativeAudio.LoadSynchronous())
                {
                    NarrativeAudioComp->SetSound(Trigger.NarrativeAudio.LoadSynchronous());
                    NarrativeAudioComp->SetVolumeMultiplier(NarrativeVolume * MasterVolume);
                    NarrativeAudioComp->Play();
                    
                    Trigger.bHasTriggered = true;
                    
                    UE_LOG(LogTemp, Warning, TEXT("Triggered narrative audio: %s"), *TriggerName);
                }
            }
            break;
        }
    }
}

void AAudio_MetaSoundManager::SetAmbienceVolume(float NewVolume)
{
    AmbienceVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
    
    // Update all ambience audio components
    for (int32 i = 0; i < AudioComponents.Num() && i < AmbienceZones.Num(); i++)
    {
        if (AudioComponents[i])
        {
            float FinalVolume = AmbienceZones[i].Volume * AmbienceVolume * MasterVolume;
            AudioComponents[i]->SetVolumeMultiplier(FinalVolume);
        }
    }
}

void AAudio_MetaSoundManager::SetNarrativeVolume(float NewVolume)
{
    NarrativeVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
}

void AAudio_MetaSoundManager::PlayFootstepAudio(const FVector& Location, float Intensity)
{
    // Create temporary audio component for footstep sound
    UAudioComponent* FootstepAudioComp = CreateAudioComponent();
    if (FootstepAudioComp)
    {
        FVector RelativeLocation = Location - GetActorLocation();
        FootstepAudioComp->SetRelativeLocation(RelativeLocation);
        FootstepAudioComp->SetVolumeMultiplier(FootstepVolume * Intensity * MasterVolume);
        
        // Auto-destroy after playback
        FootstepAudioComp->bAutoDestroy = true;
        FootstepAudioComp->Play();
    }
}

void AAudio_MetaSoundManager::CreateAmbienceZone(EAudio_AmbienceType Type, const FVector& Location, float Radius)
{
    FAudio_AmbienceZone NewZone;
    NewZone.AmbienceType = Type;
    NewZone.Location = Location;
    NewZone.Radius = Radius;
    NewZone.Volume = 0.7f;
    NewZone.bIsActive = true;
    
    AmbienceZones.Add(NewZone);
    
    // Create corresponding audio component
    UAudioComponent* NewAudioComponent = CreateAudioComponent();
    if (NewAudioComponent)
    {
        AudioComponents.Add(NewAudioComponent);
        FVector RelativeLocation = Location - GetActorLocation();
        NewAudioComponent->SetRelativeLocation(RelativeLocation);
    }
}

void AAudio_MetaSoundManager::RemoveAmbienceZone(int32 ZoneIndex)
{
    if (ZoneIndex >= 0 && ZoneIndex < AmbienceZones.Num())
    {
        AmbienceZones.RemoveAt(ZoneIndex);
        
        if (ZoneIndex < AudioComponents.Num() && AudioComponents[ZoneIndex])
        {
            AudioComponents[ZoneIndex]->DestroyComponent();
            AudioComponents.RemoveAt(ZoneIndex);
        }
    }
}

void AAudio_MetaSoundManager::UpdateAmbienceAudio(float DeltaTime)
{
    // Implement dynamic audio mixing and transitions
    for (int32 i = 0; i < AudioComponents.Num(); i++)
    {
        if (AudioComponents[i] && AmbienceZones.IsValidIndex(i))
        {
            // Smooth volume transitions
            float CurrentVolume = AudioComponents[i]->GetVolumeMultiplier();
            float TargetVolume = AmbienceZones[i].Volume * AmbienceVolume * MasterVolume;
            
            if (FMath::Abs(CurrentVolume - TargetVolume) > 0.01f)
            {
                float NewVolume = FMath::FInterpTo(CurrentVolume, TargetVolume, DeltaTime, 2.0f);
                AudioComponents[i]->SetVolumeMultiplier(NewVolume);
            }
        }
    }
}

void AAudio_MetaSoundManager::CheckNarrativeTriggers(const FVector& PlayerLocation)
{
    for (FAudio_NarrativeTrigger& Trigger : NarrativeTriggers)
    {
        if (!Trigger.bHasTriggered || Trigger.bCanRepeat)
        {
            float Distance = FVector::Dist(PlayerLocation, Trigger.TriggerLocation);
            if (Distance <= Trigger.TriggerRadius)
            {
                TriggerNarrativeAudio(Trigger.TriggerName);
            }
        }
    }
}

UAudioComponent* AAudio_MetaSoundManager::CreateAudioComponent()
{
    UAudioComponent* NewAudioComponent = CreateDefaultSubobject<UAudioComponent>(FName(*FString::Printf(TEXT("AudioComponent_%d"), AudioComponents.Num())));
    if (NewAudioComponent)
    {
        NewAudioComponent->SetupAttachment(RootComponent);
        NewAudioComponent->bAutoActivate = false;
        NewAudioComponent->SetVolumeMultiplier(1.0f);
    }
    return NewAudioComponent;
}

float AAudio_MetaSoundManager::CalculateDistanceAttenuation(const FVector& SourceLocation, const FVector& ListenerLocation, float MaxDistance)
{
    float Distance = FVector::Dist(SourceLocation, ListenerLocation);
    if (Distance >= MaxDistance)
    {
        return 0.0f;
    }
    
    // Linear falloff with smooth curve
    float NormalizedDistance = Distance / MaxDistance;
    return FMath::Clamp(1.0f - (NormalizedDistance * NormalizedDistance), 0.0f, 1.0f);
}