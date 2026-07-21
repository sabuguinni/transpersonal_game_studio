#include "Audio_NarrativeAudioIntegration.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"

UAudio_NarrativeAudioIntegration::UAudio_NarrativeAudioIntegration()
{
    PrimaryComponentTick.bCanEverTick = true;
    MaxAudibleDistance = 1500.0f;
    VolumeMultiplier = 1.0f;
    CurrentWindIntensity = 0.5f;
    bCampfireActive = false;
}

void UAudio_NarrativeAudioIntegration::BeginPlay()
{
    Super::BeginPlay();
    InitializeAudioComponents();
    
    // Setup default narrative audio events
    FAudio_NarrativeAudioEvent CampfireEvent;
    CampfireEvent.AudioType = EAudio_NarrativeAudioType::CampfireStory;
    CampfireEvent.TriggerLocation = FVector(500, 500, 100);
    CampfireEvent.TriggerRadius = 300.0f;
    CampfireEvent.Volume = 0.8f;
    CampfireEvent.bIs3DPositional = true;
    NarrativeAudioEvents.Add(CampfireEvent);

    FAudio_NarrativeAudioEvent WindEvent;
    WindEvent.AudioType = EAudio_NarrativeAudioType::AmbientWind;
    WindEvent.TriggerLocation = FVector(-200, -200, 150);
    WindEvent.TriggerRadius = 800.0f;
    WindEvent.Volume = 0.6f;
    WindEvent.bIs3DPositional = true;
    NarrativeAudioEvents.Add(WindEvent);

    FAudio_NarrativeAudioEvent ProximityEvent;
    ProximityEvent.AudioType = EAudio_NarrativeAudioType::DinosaurProximity;
    ProximityEvent.TriggerLocation = FVector(0, 0, 0); // Dynamic
    ProximityEvent.TriggerRadius = 1000.0f;
    ProximityEvent.Volume = 1.0f;
    ProximityEvent.bIs3DPositional = true;
    NarrativeAudioEvents.Add(ProximityEvent);
}

void UAudio_NarrativeAudioIntegration::InitializeAudioComponents()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    // Create campfire audio component
    CampfireAudioComponent = NewObject<UAudioComponent>(Owner);
    if (CampfireAudioComponent)
    {
        CampfireAudioComponent->SetupAttachment(Owner->GetRootComponent());
        CampfireAudioComponent->SetWorldLocation(FVector(500, 500, 100));
        CampfireAudioComponent->SetVolumeMultiplier(0.8f);
        CampfireAudioComponent->bAutoActivate = false;
        AudioComponentMap.Add(EAudio_NarrativeAudioType::CampfireStory, CampfireAudioComponent);
    }

    // Create ambient wind component
    AmbientWindComponent = NewObject<UAudioComponent>(Owner);
    if (AmbientWindComponent)
    {
        AmbientWindComponent->SetupAttachment(Owner->GetRootComponent());
        AmbientWindComponent->SetWorldLocation(FVector(-200, -200, 150));
        AmbientWindComponent->SetVolumeMultiplier(0.6f);
        AmbientWindComponent->bAutoActivate = true;
        AudioComponentMap.Add(EAudio_NarrativeAudioType::AmbientWind, AmbientWindComponent);
    }

    // Create proximity warning component
    UAudioComponent* ProximityComponent = NewObject<UAudioComponent>(Owner);
    if (ProximityComponent)
    {
        ProximityComponent->SetupAttachment(Owner->GetRootComponent());
        ProximityComponent->SetVolumeMultiplier(1.0f);
        ProximityComponent->bAutoActivate = false;
        AudioComponentMap.Add(EAudio_NarrativeAudioType::DinosaurProximity, ProximityComponent);
    }
}

void UAudio_NarrativeAudioIntegration::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    UpdateAudioPositions();
}

void UAudio_NarrativeAudioIntegration::UpdateAudioPositions()
{
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC || !PC->GetPawn()) return;

    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();

    // Update audio component volumes based on distance
    for (const auto& AudioPair : AudioComponentMap)
    {
        UAudioComponent* AudioComp = AudioPair.Value;
        if (!AudioComp) continue;

        FVector AudioLocation = AudioComp->GetWorldLocation();
        float Distance = FVector::Dist(PlayerLocation, AudioLocation);
        
        if (Distance <= MaxAudibleDistance)
        {
            float VolumeScale = 1.0f - (Distance / MaxAudibleDistance);
            VolumeScale = FMath::Clamp(VolumeScale, 0.0f, 1.0f);
            AudioComp->SetVolumeMultiplier(VolumeScale * VolumeMultiplier);
        }
        else
        {
            AudioComp->SetVolumeMultiplier(0.0f);
        }
    }
}

void UAudio_NarrativeAudioIntegration::TriggerNarrativeAudio(EAudio_NarrativeAudioType AudioType, FVector PlayerLocation)
{
    UAudioComponent** AudioCompPtr = AudioComponentMap.Find(AudioType);
    if (!AudioCompPtr || !*AudioCompPtr) return;

    UAudioComponent* AudioComp = *AudioCompPtr;
    
    // Find the corresponding audio event
    for (const FAudio_NarrativeAudioEvent& Event : NarrativeAudioEvents)
    {
        if (Event.AudioType == AudioType)
        {
            if (IsPlayerInRange(Event.TriggerLocation, Event.TriggerRadius))
            {
                AudioComp->SetVolumeMultiplier(Event.Volume * VolumeMultiplier);
                
                if (Event.bIs3DPositional)
                {
                    AudioComp->SetWorldLocation(Event.TriggerLocation);
                }
                
                if (!AudioComp->IsPlaying())
                {
                    AudioComp->Play();
                }
            }
            break;
        }
    }
}

void UAudio_NarrativeAudioIntegration::SetCampfireAudioActive(bool bActive)
{
    bCampfireActive = bActive;
    
    if (CampfireAudioComponent)
    {
        if (bActive && !CampfireAudioComponent->IsPlaying())
        {
            CampfireAudioComponent->Play();
        }
        else if (!bActive && CampfireAudioComponent->IsPlaying())
        {
            CampfireAudioComponent->Stop();
        }
    }
}

void UAudio_NarrativeAudioIntegration::UpdateAmbientWindIntensity(float WindStrength)
{
    CurrentWindIntensity = FMath::Clamp(WindStrength, 0.0f, 1.0f);
    
    if (AmbientWindComponent)
    {
        float WindVolume = CurrentWindIntensity * 0.6f * VolumeMultiplier;
        AmbientWindComponent->SetVolumeMultiplier(WindVolume);
        
        float WindPitch = 0.8f + (CurrentWindIntensity * 0.4f);
        AmbientWindComponent->SetPitchMultiplier(WindPitch);
    }
}

void UAudio_NarrativeAudioIntegration::PlaySurvivalVoiceLine(const FString& VoiceLineID)
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Map voice line IDs to audio types
    EAudio_NarrativeAudioType AudioType = EAudio_NarrativeAudioType::SurvivalWarning;
    
    if (VoiceLineID.Contains("Scout_Vex"))
    {
        AudioType = EAudio_NarrativeAudioType::SurvivalWarning;
    }
    else if (VoiceLineID.Contains("Hunter_Thane"))
    {
        AudioType = EAudio_NarrativeAudioType::HuntAftermath;
    }
    else if (VoiceLineID.Contains("Campfire"))
    {
        AudioType = EAudio_NarrativeAudioType::CampfireStory;
    }

    APlayerController* PC = World->GetFirstPlayerController();
    if (PC && PC->GetPawn())
    {
        FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
        TriggerNarrativeAudio(AudioType, PlayerLocation);
    }
}

void UAudio_NarrativeAudioIntegration::SetNarrativeAudioVolume(float NewVolume)
{
    VolumeMultiplier = FMath::Clamp(NewVolume, 0.0f, 2.0f);
    
    // Update all active audio components
    for (const auto& AudioPair : AudioComponentMap)
    {
        UAudioComponent* AudioComp = AudioPair.Value;
        if (AudioComp && AudioComp->IsPlaying())
        {
            float CurrentBase = AudioComp->VolumeMultiplier / VolumeMultiplier;
            AudioComp->SetVolumeMultiplier(CurrentBase * VolumeMultiplier);
        }
    }
}

bool UAudio_NarrativeAudioIntegration::IsPlayerInRange(FVector AudioLocation, float Range)
{
    UWorld* World = GetWorld();
    if (!World) return false;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC || !PC->GetPawn()) return false;

    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
    float Distance = FVector::Dist(PlayerLocation, AudioLocation);
    
    return Distance <= Range;
}