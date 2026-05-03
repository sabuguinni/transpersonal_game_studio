#include "AudioSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UAudioSystemManager::UAudioSystemManager()
{
    MasterVolume = 1.0f;
}

void UAudioSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeVolumeSettings();
    
    // Configurar timer para limpeza de componentes
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(CleanupTimerHandle, this, &UAudioSystemManager::CleanupFinishedComponents, 5.0f, true);
    }
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager initialized"));
}

void UAudioSystemManager::Deinitialize()
{
    StopAllSounds();
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(CleanupTimerHandle);
    }
    
    ActiveAudioComponents.Empty();
    SoundDatabase.Empty();
    
    Super::Deinitialize();
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager deinitialized"));
}

void UAudioSystemManager::InitializeVolumeSettings()
{
    VolumeSettings.Add(EAudio_SoundType::Dialogue, 0.8f);
    VolumeSettings.Add(EAudio_SoundType::Ambient, 0.6f);
    VolumeSettings.Add(EAudio_SoundType::Footsteps, 0.7f);
    VolumeSettings.Add(EAudio_SoundType::DinosaurRoar, 0.9f);
    VolumeSettings.Add(EAudio_SoundType::Warning, 1.0f);
    VolumeSettings.Add(EAudio_SoundType::Music, 0.5f);
}

void UAudioSystemManager::PlaySoundAtLocation(const FAudio_SoundData& SoundData, const FVector& Location, AActor* SourceActor)
{
    if (SoundData.SoundID.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Cannot play sound with empty ID"));
        return;
    }
    
    // Criar componente de áudio
    UAudioComponent* AudioComp = CreateAudioComponent(SourceActor, Location);
    if (!AudioComp)
    {
        UE_LOG(LogTemp, Error, TEXT("AudioSystemManager: Failed to create audio component"));
        return;
    }
    
    // Configurar volume
    float TypeVolume = GetVolumeByType(SoundData.SoundType);
    float FinalVolume = SoundData.Volume * TypeVolume * MasterVolume;
    AudioComp->SetVolumeMultiplier(FinalVolume);
    
    // Configurar áudio espacial
    if (SoundData.bIs3D)
    {
        AudioComp->bAllowSpatialization = true;
        AudioComp->AttenuationSettings = nullptr; // Usar configuração padrão por agora
    }
    else
    {
        AudioComp->bAllowSpatialization = false;
    }
    
    // Registar componente activo
    ActiveAudioComponents.Add(SoundData.SoundID, AudioComp);
    
    // Reproduzir áudio
    AudioComp->Play();
    
    // Disparar evento
    OnSoundStarted.Broadcast(SoundData.SoundID, SoundData.SoundType);
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Playing sound %s at location %s"), *SoundData.SoundID, *Location.ToString());
}

void UAudioSystemManager::PlayDialogue(const FString& DialogueID, AActor* Speaker, AActor* Listener)
{
    if (!HasSoundData(DialogueID))
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Dialogue %s not found in database"), *DialogueID);
        return;
    }
    
    FAudio_SoundData SoundData = GetSoundData(DialogueID);
    FVector SpeakerLocation = Speaker ? Speaker->GetActorLocation() : FVector::ZeroVector;
    
    PlaySoundAtLocation(SoundData, SpeakerLocation, Speaker);
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Playing dialogue %s"), *DialogueID);
}

void UAudioSystemManager::PlayAmbientSound(const FString& SoundID, const FVector& Location, float FadeInTime)
{
    if (!HasSoundData(SoundID))
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Ambient sound %s not found"), *SoundID);
        return;
    }
    
    FAudio_SoundData SoundData = GetSoundData(SoundID);
    PlaySoundAtLocation(SoundData, Location);
    
    // TODO: Implementar fade in
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Playing ambient sound %s"), *SoundID);
}

void UAudioSystemManager::StopSound(const FString& SoundID, float FadeOutTime)
{
    if (UAudioComponent** AudioCompPtr = ActiveAudioComponents.Find(SoundID))
    {
        UAudioComponent* AudioComp = *AudioCompPtr;
        if (AudioComp && AudioComp->IsValidLowLevel())
        {
            AudioComp->Stop();
            OnSoundFinished.Broadcast(SoundID, EAudio_SoundType::Ambient); // TODO: Guardar tipo original
            
            UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Stopped sound %s"), *SoundID);
        }
        
        ActiveAudioComponents.Remove(SoundID);
    }
}

void UAudioSystemManager::StopAllSounds(EAudio_SoundType SoundType)
{
    TArray<FString> SoundsToRemove;
    
    for (auto& AudioPair : ActiveAudioComponents)
    {
        UAudioComponent* AudioComp = AudioPair.Value;
        if (AudioComp && AudioComp->IsValidLowLevel())
        {
            // TODO: Filtrar por tipo quando tivermos essa informação guardada
            AudioComp->Stop();
            SoundsToRemove.Add(AudioPair.Key);
        }
    }
    
    for (const FString& SoundID : SoundsToRemove)
    {
        ActiveAudioComponents.Remove(SoundID);
        OnSoundFinished.Broadcast(SoundID, SoundType);
    }
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Stopped all sounds of type %d"), (int32)SoundType);
}

void UAudioSystemManager::SetSpatialConfig(const FAudio_SpatialConfig& Config)
{
    SpatialConfig = Config;
    
    // Aplicar configuração a componentes activos
    for (auto& AudioPair : ActiveAudioComponents)
    {
        UAudioComponent* AudioComp = AudioPair.Value;
        if (AudioComp && AudioComp->IsValidLowLevel())
        {
            // TODO: Aplicar configurações espaciais
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Updated spatial configuration"));
}

void UAudioSystemManager::UpdateListenerPosition(const FVector& Position, const FRotator& Rotation)
{
    // TODO: Actualizar posição do listener para cálculos espaciais
    UE_LOG(LogTemp, VeryVerbose, TEXT("AudioSystemManager: Updated listener position to %s"), *Position.ToString());
}

void UAudioSystemManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    // Actualizar volume de todos os componentes activos
    for (auto& AudioPair : ActiveAudioComponents)
    {
        UAudioComponent* AudioComp = AudioPair.Value;
        if (AudioComp && AudioComp->IsValidLowLevel())
        {
            // TODO: Recalcular volume com novo master volume
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Set master volume to %f"), MasterVolume);
}

void UAudioSystemManager::SetVolumeByType(EAudio_SoundType SoundType, float Volume)
{
    VolumeSettings.Add(SoundType, FMath::Clamp(Volume, 0.0f, 1.0f));
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Set volume for type %d to %f"), (int32)SoundType, Volume);
}

float UAudioSystemManager::GetVolumeByType(EAudio_SoundType SoundType) const
{
    if (const float* Volume = VolumeSettings.Find(SoundType))
    {
        return *Volume;
    }
    
    return 1.0f; // Volume padrão
}

void UAudioSystemManager::RegisterSoundData(const FAudio_SoundData& SoundData)
{
    if (SoundData.SoundID.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Cannot register sound with empty ID"));
        return;
    }
    
    SoundDatabase.Add(SoundData.SoundID, SoundData);
    
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Registered sound %s"), *SoundData.SoundID);
}

FAudio_SoundData UAudioSystemManager::GetSoundData(const FString& SoundID) const
{
    if (const FAudio_SoundData* SoundData = SoundDatabase.Find(SoundID))
    {
        return *SoundData;
    }
    
    return FAudio_SoundData(); // Retornar dados vazios se não encontrado
}

bool UAudioSystemManager::HasSoundData(const FString& SoundID) const
{
    return SoundDatabase.Contains(SoundID);
}

UAudioComponent* UAudioSystemManager::CreateAudioComponent(AActor* SourceActor, const FVector& Location)
{
    UAudioComponent* AudioComp = nullptr;
    
    if (SourceActor)
    {
        // Criar componente no actor
        AudioComp = NewObject<UAudioComponent>(SourceActor);
        AudioComp->AttachToComponent(SourceActor->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
    }
    else
    {
        // Criar componente standalone no mundo
        if (UWorld* World = GetWorld())
        {
            AudioComp = NewObject<UAudioComponent>(World);
            AudioComp->SetWorldLocation(Location);
        }
    }
    
    return AudioComp;
}

void UAudioSystemManager::CleanupFinishedComponents()
{
    TArray<FString> ComponentsToRemove;
    
    for (auto& AudioPair : ActiveAudioComponents)
    {
        UAudioComponent* AudioComp = AudioPair.Value;
        if (!AudioComp || !AudioComp->IsValidLowLevel() || !AudioComp->IsPlaying())
        {
            ComponentsToRemove.Add(AudioPair.Key);
            
            if (AudioComp && AudioComp->IsValidLowLevel())
            {
                OnSoundFinished.Broadcast(AudioPair.Key, EAudio_SoundType::Ambient); // TODO: Tipo correcto
            }
        }
    }
    
    for (const FString& SoundID : ComponentsToRemove)
    {
        ActiveAudioComponents.Remove(SoundID);
    }
    
    if (ComponentsToRemove.Num() > 0)
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("AudioSystemManager: Cleaned up %d finished audio components"), ComponentsToRemove.Num());
    }
}