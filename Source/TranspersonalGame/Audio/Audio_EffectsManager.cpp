#include "Audio_EffectsManager.h"
#include "Components/SceneComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"

AAudio_EffectsManager::AAudio_EffectsManager()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    MasterVolume = 1.0f;
    EffectsVolume = 0.8f;
    MaxSimultaneousEffects = 32;
}

void AAudio_EffectsManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeEffectLibrary();
    
    UE_LOG(LogTemp, Warning, TEXT("Audio Effects Manager initialized with %d effect types"), EffectLibrary.Num());
}

void AAudio_EffectsManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    CleanupFinishedComponents();
}

void AAudio_EffectsManager::InitializeEffectLibrary()
{
    // Initialize default effect data for each type
    FAudio_EffectData FootstepData;
    FootstepData.EffectType = EAudio_EffectType::Footstep;
    FootstepData.Volume = 0.6f;
    FootstepData.MaxDistance = 1000.0f;
    EffectLibrary.Add(EAudio_EffectType::Footstep, FootstepData);

    FAudio_EffectData RoarData;
    RoarData.EffectType = EAudio_EffectType::Roar;
    RoarData.Volume = 1.0f;
    RoarData.MaxDistance = 8000.0f;
    EffectLibrary.Add(EAudio_EffectType::Roar, RoarData);

    FAudio_EffectData AmbientData;
    AmbientData.EffectType = EAudio_EffectType::Ambient;
    AmbientData.Volume = 0.4f;
    AmbientData.MaxDistance = 15000.0f;
    EffectLibrary.Add(EAudio_EffectType::Ambient, AmbientData);

    FAudio_EffectData ImpactData;
    ImpactData.EffectType = EAudio_EffectType::Impact;
    ImpactData.Volume = 0.8f;
    ImpactData.MaxDistance = 3000.0f;
    EffectLibrary.Add(EAudio_EffectType::Impact, ImpactData);

    FAudio_EffectData EnvironmentalData;
    EnvironmentalData.EffectType = EAudio_EffectType::Environmental;
    EnvironmentalData.Volume = 0.5f;
    EnvironmentalData.MaxDistance = 12000.0f;
    EffectLibrary.Add(EAudio_EffectType::Environmental, EnvironmentalData);

    FAudio_EffectData UIData;
    UIData.EffectType = EAudio_EffectType::UI;
    UIData.Volume = 0.7f;
    UIData.bIs3D = false;
    UIData.MaxDistance = 0.0f;
    EffectLibrary.Add(EAudio_EffectType::UI, UIData);
}

void AAudio_EffectsManager::PlayEffect(EAudio_EffectType EffectType, FVector Location, float VolumeMultiplier)
{
    if (ActiveAudioComponents.Num() >= MaxSimultaneousEffects)
    {
        UE_LOG(LogTemp, Warning, TEXT("Max simultaneous audio effects reached (%d)"), (int32)MaxSimultaneousEffects);
        return;
    }

    if (!EffectLibrary.Contains(EffectType))
    {
        UE_LOG(LogTemp, Warning, TEXT("Effect type not found in library: %d"), (int32)EffectType);
        return;
    }

    const FAudio_EffectData& EffectData = EffectLibrary[EffectType];
    UAudioComponent* NewAudioComponent = CreateAudioComponent(EffectData, Location);
    
    if (NewAudioComponent)
    {
        float FinalVolume = EffectData.Volume * VolumeMultiplier * EffectsVolume * MasterVolume;
        NewAudioComponent->SetVolumeMultiplier(FinalVolume);
        NewAudioComponent->Play();
        
        ActiveAudioComponents.Add(NewAudioComponent);
        
        UE_LOG(LogTemp, Log, TEXT("Playing audio effect type %d at location %s"), 
               (int32)EffectType, *Location.ToString());
    }
}

void AAudio_EffectsManager::PlayEffectAtActor(EAudio_EffectType EffectType, AActor* TargetActor, float VolumeMultiplier)
{
    if (!TargetActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot play effect at null actor"));
        return;
    }

    PlayEffect(EffectType, TargetActor->GetActorLocation(), VolumeMultiplier);
}

void AAudio_EffectsManager::StopAllEffects()
{
    for (UAudioComponent* AudioComp : ActiveAudioComponents)
    {
        if (AudioComp && IsValid(AudioComp))
        {
            AudioComp->Stop();
        }
    }
    
    ActiveAudioComponents.Empty();
    UE_LOG(LogTemp, Log, TEXT("Stopped all audio effects"));
}

void AAudio_EffectsManager::StopEffectType(EAudio_EffectType EffectType)
{
    for (int32 i = ActiveAudioComponents.Num() - 1; i >= 0; i--)
    {
        UAudioComponent* AudioComp = ActiveAudioComponents[i];
        if (AudioComp && IsValid(AudioComp))
        {
            // Note: In a full implementation, we'd store effect type with each component
            // For now, we'll stop all as a simplified approach
            AudioComp->Stop();
            ActiveAudioComponents.RemoveAt(i);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Stopped all effects of type %d"), (int32)EffectType);
}

void AAudio_EffectsManager::SetMasterVolume(float NewVolume)
{
    MasterVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
    
    // Update all active components
    for (UAudioComponent* AudioComp : ActiveAudioComponents)
    {
        if (AudioComp && IsValid(AudioComp))
        {
            float CurrentVolume = AudioComp->GetVolumeMultiplier();
            AudioComp->SetVolumeMultiplier(CurrentVolume * MasterVolume);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Master volume set to %f"), MasterVolume);
}

void AAudio_EffectsManager::SetEffectsVolume(float NewVolume)
{
    EffectsVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
    UE_LOG(LogTemp, Log, TEXT("Effects volume set to %f"), EffectsVolume);
}

bool AAudio_EffectsManager::IsEffectPlaying(EAudio_EffectType EffectType) const
{
    for (UAudioComponent* AudioComp : ActiveAudioComponents)
    {
        if (AudioComp && IsValid(AudioComp) && AudioComp->IsPlaying())
        {
            return true;
        }
    }
    return false;
}

int32 AAudio_EffectsManager::GetActiveEffectCount() const
{
    int32 ActiveCount = 0;
    for (UAudioComponent* AudioComp : ActiveAudioComponents)
    {
        if (AudioComp && IsValid(AudioComp) && AudioComp->IsPlaying())
        {
            ActiveCount++;
        }
    }
    return ActiveCount;
}

void AAudio_EffectsManager::CleanupFinishedComponents()
{
    for (int32 i = ActiveAudioComponents.Num() - 1; i >= 0; i--)
    {
        UAudioComponent* AudioComp = ActiveAudioComponents[i];
        if (!AudioComp || !IsValid(AudioComp) || !AudioComp->IsPlaying())
        {
            if (AudioComp && IsValid(AudioComp))
            {
                AudioComp->DestroyComponent();
            }
            ActiveAudioComponents.RemoveAt(i);
        }
    }
}

UAudioComponent* AAudio_EffectsManager::CreateAudioComponent(const FAudio_EffectData& EffectData, FVector Location)
{
    UAudioComponent* NewAudioComponent = NewObject<UAudioComponent>(this);
    if (!NewAudioComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create audio component"));
        return nullptr;
    }

    NewAudioComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
    NewAudioComponent->SetWorldLocation(Location);
    
    // Configure audio component properties
    NewAudioComponent->SetVolumeMultiplier(EffectData.Volume);
    NewAudioComponent->SetPitchMultiplier(EffectData.Pitch);
    
    if (EffectData.bIs3D)
    {
        NewAudioComponent->bAllowSpatialization = true;
        NewAudioComponent->bOverrideAttenuation = true;
        // Set up basic attenuation settings
        NewAudioComponent->AttenuationOverrides.bAttenuate = true;
        NewAudioComponent->AttenuationOverrides.AttenuationShape = EAttenuationShape::Sphere;
        NewAudioComponent->AttenuationOverrides.FalloffDistance = EffectData.MaxDistance;
    }
    else
    {
        NewAudioComponent->bAllowSpatialization = false;
    }

    // Note: In a full implementation, we'd load the actual sound cue here
    // For now, we create the component structure
    
    NewAudioComponent->RegisterComponent();
    
    return NewAudioComponent;
}