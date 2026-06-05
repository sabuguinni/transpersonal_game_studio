#include "Audio_MetaSoundManager.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UAudio_MetaSoundManager::UAudio_MetaSoundManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    MasterVolume = 1.0f;
    AmbientVolume = 0.7f;
    DinosaurVolume = 1.0f;
    WeatherVolume = 0.8f;
}

void UAudio_MetaSoundManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize audio component arrays
    ActiveAudioComponents.Add(EAudio_SoundCategory::Ambient, TArray<UAudioComponent*>());
    ActiveAudioComponents.Add(EAudio_SoundCategory::Dinosaur, TArray<UAudioComponent*>());
    ActiveAudioComponents.Add(EAudio_SoundCategory::Player, TArray<UAudioComponent*>());
    ActiveAudioComponents.Add(EAudio_SoundCategory::Weather, TArray<UAudioComponent*>());
    ActiveAudioComponents.Add(EAudio_SoundCategory::Combat, TArray<UAudioComponent*>());
    ActiveAudioComponents.Add(EAudio_SoundCategory::UI, TArray<UAudioComponent*>());
    
    // Start ambient layers
    UpdateAmbientLayers();
}

void UAudio_MetaSoundManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    CleanupFinishedComponents();
}

void UAudio_MetaSoundManager::PlayMetaSoundLayer(const FAudio_SoundLayer& SoundLayer, FVector Location)
{
    if (!SoundLayer.MetaSoundAsset.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("MetaSound asset is not valid"));
        return;
    }
    
    UMetaSoundSource* MetaSound = SoundLayer.MetaSoundAsset.LoadSynchronous();
    if (!MetaSound)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load MetaSound asset"));
        return;
    }
    
    UAudioComponent* AudioComp = CreateAudioComponentForMetaSound(MetaSound, Location);
    if (AudioComp)
    {
        // Apply sound layer settings
        AudioComp->SetVolumeMultiplier(SoundLayer.Volume * MasterVolume);
        AudioComp->SetPitchMultiplier(SoundLayer.Pitch);
        
        // Set category-specific volume
        float CategoryVolume = 1.0f;
        switch (SoundLayer.Category)
        {
            case EAudio_SoundCategory::Ambient:
                CategoryVolume = AmbientVolume;
                break;
            case EAudio_SoundCategory::Dinosaur:
                CategoryVolume = DinosaurVolume;
                break;
            case EAudio_SoundCategory::Weather:
                CategoryVolume = WeatherVolume;
                break;
            default:
                CategoryVolume = 1.0f;
                break;
        }
        
        AudioComp->SetVolumeMultiplier(SoundLayer.Volume * MasterVolume * CategoryVolume);
        
        // Add to tracking array
        if (ActiveAudioComponents.Contains(SoundLayer.Category))
        {
            ActiveAudioComponents[SoundLayer.Category].Add(AudioComp);
        }
        
        AudioComp->Play();
    }
}

void UAudio_MetaSoundManager::StopSoundCategory(EAudio_SoundCategory Category)
{
    if (!ActiveAudioComponents.Contains(Category))
        return;
    
    TArray<UAudioComponent*>& Components = ActiveAudioComponents[Category];
    for (UAudioComponent* Comp : Components)
    {
        if (IsValid(Comp))
        {
            Comp->Stop();
        }
    }
    Components.Empty();
}

void UAudio_MetaSoundManager::SetCategoryVolume(EAudio_SoundCategory Category, float NewVolume)
{
    switch (Category)
    {
        case EAudio_SoundCategory::Ambient:
            AmbientVolume = NewVolume;
            break;
        case EAudio_SoundCategory::Dinosaur:
            DinosaurVolume = NewVolume;
            break;
        case EAudio_SoundCategory::Weather:
            WeatherVolume = NewVolume;
            break;
    }
    
    // Update existing components
    if (ActiveAudioComponents.Contains(Category))
    {
        for (UAudioComponent* Comp : ActiveAudioComponents[Category])
        {
            if (IsValid(Comp))
            {
                Comp->SetVolumeMultiplier(Comp->VolumeMultiplier * NewVolume);
            }
        }
    }
}

void UAudio_MetaSoundManager::UpdateAmbientLayers()
{
    for (const FAudio_SoundLayer& Layer : AmbientLayers)
    {
        if (Layer.Category == EAudio_SoundCategory::Ambient)
        {
            PlayMetaSoundLayer(Layer, GetOwner()->GetActorLocation());
        }
    }
}

void UAudio_MetaSoundManager::TriggerDinosaurProximityAudio(float Distance, float DinosaurSize)
{
    // Find appropriate dinosaur audio layer based on size and distance
    for (const FAudio_SoundLayer& Layer : DinosaurLayers)
    {
        if (Layer.Category == EAudio_SoundCategory::Dinosaur)
        {
            FAudio_SoundLayer ModifiedLayer = Layer;
            
            // Adjust volume based on distance (closer = louder)
            float DistanceVolume = FMath::Clamp(1.0f - (Distance / 2000.0f), 0.1f, 1.0f);
            ModifiedLayer.Volume *= DistanceVolume;
            
            // Adjust pitch based on dinosaur size (bigger = lower pitch)
            ModifiedLayer.Pitch = FMath::Clamp(1.0f - (DinosaurSize * 0.3f), 0.5f, 1.5f);
            
            PlayMetaSoundLayer(ModifiedLayer, GetOwner()->GetActorLocation());
            break;
        }
    }
}

void UAudio_MetaSoundManager::CleanupFinishedComponents()
{
    for (auto& CategoryPair : ActiveAudioComponents)
    {
        TArray<UAudioComponent*>& Components = CategoryPair.Value;
        
        for (int32 i = Components.Num() - 1; i >= 0; --i)
        {
            UAudioComponent* Comp = Components[i];
            if (!IsValid(Comp) || !Comp->IsPlaying())
            {
                Components.RemoveAt(i);
            }
        }
    }
}

UAudioComponent* UAudio_MetaSoundManager::CreateAudioComponentForMetaSound(UMetaSoundSource* MetaSound, FVector Location)
{
    if (!MetaSound || !GetWorld())
        return nullptr;
    
    UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(
        GetWorld(),
        MetaSound,
        Location,
        FRotator::ZeroRotator,
        1.0f,
        1.0f,
        0.0f,
        nullptr,
        nullptr,
        true
    );
    
    return AudioComp;
}