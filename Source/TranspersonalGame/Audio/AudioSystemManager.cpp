#include "AudioSystemManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "MetasoundSource.h"
#include "Sound/SoundAttenuation.h"
#include "Components/AudioComponent.h"

void UAudioSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Initializing adaptive audio system"));
    
    // Configurar Sound Classes
    MasterSoundClass = LoadObject<USoundClass>(nullptr, TEXT("/Game/Audio/SoundClasses/SC_Master"));
    MusicSoundClass = LoadObject<USoundClass>(nullptr, TEXT("/Game/Audio/SoundClasses/SC_Music"));
    EnvironmentSoundClass = LoadObject<USoundClass>(nullptr, TEXT("/Game/Audio/SoundClasses/SC_Environment"));
    DinosaurSoundClass = LoadObject<USoundClass>(nullptr, TEXT("/Game/Audio/SoundClasses/SC_Dinosaur"));
    
    // Configurar Sound Mix dinâmico
    DynamicSoundMix = LoadObject<USoundMix>(nullptr, TEXT("/Game/Audio/SoundMixes/SM_Dynamic"));
    
    // Inicializar timers para updates contínuos
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(TensionUpdateTimer, this, 
            &UAudioSystemManager::ProcessTensionEvents, 0.1f, true);
        
        World->GetTimerManager().SetTimer(EnvironmentUpdateTimer, this, 
            &UAudioSystemManager::UpdateEnvironmentAudio, 1.0f, true);
    }
    
    // Estado inicial
    SetEmotionalState(EEmotionalState::Calm, 0.0f);
    SetEnvironmentType(EEnvironmentType::Forest, 0.0f);
}

void UAudioSystemManager::Deinitialize()
{
    // Limpar timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(TensionUpdateTimer);
        World->GetTimerManager().ClearTimer(EnvironmentUpdateTimer);
    }
    
    // Parar todos os áudios ativos
    for (auto& Layer : EmotionalLayers)
    {
        if (Layer.Value.AudioComponent && IsValid(Layer.Value.AudioComponent))
        {
            Layer.Value.AudioComponent->Stop();
        }
    }
    
    for (auto& Layer : EnvironmentLayers)
    {
        if (Layer.Value.AudioComponent && IsValid(Layer.Value.AudioComponent))
        {
            Layer.Value.AudioComponent->Stop();
        }
    }
    
    for (auto& DinosaurAudio : ActiveDinosaurAudio)
    {
        if (DinosaurAudio.Value && IsValid(DinosaurAudio.Value))
        {
            DinosaurAudio.Value->Stop();
        }
    }
    
    Super::Deinitialize();
}

void UAudioSystemManager::SetEmotionalState(EEmotionalState NewState, float TransitionTime)
{
    if (NewState == CurrentEmotionalState)
        return;
    
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Transitioning from %d to %d"), 
        (int32)CurrentEmotionalState, (int32)NewState);
    
    // Fade out do estado anterior
    if (EmotionalLayers.Contains(CurrentEmotionalState))
    {
        FAudioLayer& PreviousLayer = EmotionalLayers[CurrentEmotionalState];
        if (PreviousLayer.AudioComponent && IsValid(PreviousLayer.AudioComponent))
        {
            PreviousLayer.AudioComponent->FadeOut(TransitionTime, 0.0f);
            PreviousLayer.bIsActive = false;
        }
    }
    
    CurrentEmotionalState = NewState;
    
    // Fade in do novo estado
    if (EmotionalLayers.Contains(NewState))
    {
        FAudioLayer& NewLayer = EmotionalLayers[NewState];
        if (NewLayer.MetaSound)
        {
            if (!NewLayer.AudioComponent || !IsValid(NewLayer.AudioComponent))
            {
                NewLayer.AudioComponent = UGameplayStatics::SpawnSound2D(
                    GetWorld(), NewLayer.MetaSound, 0.0f, 1.0f, 0.0f, nullptr, false, false);
            }
            
            if (NewLayer.AudioComponent)
            {
                NewLayer.AudioComponent->SetSoundClass(MusicSoundClass);
                NewLayer.AudioComponent->FadeIn(TransitionTime, NewLayer.Volume);
                NewLayer.bIsActive = true;
            }
        }
    }
    
    UpdateMusicLayers();
}

void UAudioSystemManager::SetEnvironmentType(EEnvironmentType NewEnvironment, float TransitionTime)
{
    if (NewEnvironment == CurrentEnvironment)
        return;
    
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Environment transition to %d"), (int32)NewEnvironment);
    
    // Fade out ambiente anterior
    if (EnvironmentLayers.Contains(CurrentEnvironment))
    {
        FAudioLayer& PreviousLayer = EnvironmentLayers[CurrentEnvironment];
        if (PreviousLayer.AudioComponent && IsValid(PreviousLayer.AudioComponent))
        {
            PreviousLayer.AudioComponent->FadeOut(TransitionTime, 0.0f);
            PreviousLayer.bIsActive = false;
        }
    }
    
    CurrentEnvironment = NewEnvironment;
    
    // Fade in novo ambiente
    if (EnvironmentLayers.Contains(NewEnvironment))
    {
        FAudioLayer& NewLayer = EnvironmentLayers[NewEnvironment];
        if (NewLayer.MetaSound)
        {
            if (!NewLayer.AudioComponent || !IsValid(NewLayer.AudioComponent))
            {
                NewLayer.AudioComponent = UGameplayStatics::SpawnSound2D(
                    GetWorld(), NewLayer.MetaSound, 0.0f, 1.0f, 0.0f, nullptr, false, false);
            }
            
            if (NewLayer.AudioComponent)
            {
                NewLayer.AudioComponent->SetSoundClass(EnvironmentSoundClass);
                NewLayer.AudioComponent->FadeIn(TransitionTime, NewLayer.Volume);
                NewLayer.bIsActive = true;
            }
        }
    }
}

void UAudioSystemManager::RegisterDinosaur(const FString& DinosaurID, const FDinosaurAudioProfile& Profile)
{
    DinosaurProfiles.Add(DinosaurID, Profile);
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Registered dinosaur %s"), *DinosaurID);
}

void UAudioSystemManager::UpdateDinosaurBehavior(const FString& DinosaurID, EDinosaurBehaviorState NewBehavior, FVector Location)
{
    if (!DinosaurProfiles.Contains(DinosaurID))
        return;
    
    FDinosaurAudioProfile& Profile = DinosaurProfiles[DinosaurID];
    
    // Parar áudio anterior se existir
    if (ActiveDinosaurAudio.Contains(DinosaurID))
    {
        UAudioComponent* PreviousAudio = ActiveDinosaurAudio[DinosaurID];
        if (PreviousAudio && IsValid(PreviousAudio))
        {
            PreviousAudio->FadeOut(0.5f, 0.0f);
        }
    }
    
    // Tocar novo áudio baseado no comportamento
    if (Profile.BehaviorSounds.Contains(NewBehavior))
    {
        UMetaSoundSource* BehaviorSound = Profile.BehaviorSounds[NewBehavior];
        if (BehaviorSound)
        {
            UAudioComponent* NewAudio = UGameplayStatics::SpawnSoundAtLocation(
                GetWorld(), BehaviorSound, Location, FRotator::ZeroRotator, 1.0f, 1.0f, 0.0f,
                Profile.AttenuationSettings, nullptr, false);
            
            if (NewAudio)
            {
                NewAudio->SetSoundClass(DinosaurSoundClass);
                
                // Aplicar variações únicas baseadas no ID do dinossauro
                float PitchVariation = FMath::RandRange(-Profile.VoicePitchVariation, Profile.VoicePitchVariation);
                float VolumeVariation = FMath::RandRange(-Profile.VolumeVariation, Profile.VolumeVariation);
                
                // Use hash do ID para variações consistentes
                int32 DinosaurHash = GetTypeHash(DinosaurID);
                FMath::RandInit(DinosaurHash);
                PitchVariation = FMath::RandRange(-Profile.VoicePitchVariation, Profile.VoicePitchVariation);
                VolumeVariation = FMath::RandRange(-Profile.VolumeVariation, Profile.VolumeVariation);
                
                NewAudio->SetPitchMultiplier(1.0f + PitchVariation);
                NewAudio->SetVolumeMultiplier(1.0f + VolumeVariation);
                
                // Ajustar baseado no nível de domesticação
                if (Profile.DomesticationProgressAudio > 0.0f)
                {
                    float DomesticationPitchShift = Profile.DomesticationProgressAudio * 0.1f; // Mais agudo quando domesticado
                    NewAudio->SetPitchMultiplier(NewAudio->PitchMultiplier + DomesticationPitchShift);
                }
                
                ActiveDinosaurAudio.Add(DinosaurID, NewAudio);
            }
        }
    }
}

void UAudioSystemManager::UpdateDinosaurDomestication(const FString& DinosaurID, float DomesticationLevel)
{
    if (DinosaurProfiles.Contains(DinosaurID))
    {
        DinosaurProfiles[DinosaurID].DomesticationProgressAudio = FMath::Clamp(DomesticationLevel, 0.0f, 1.0f);
        UpdateDinosaurAudioParameters(DinosaurID);
    }
}

void UAudioSystemManager::SetTimeOfDay(float TimeNormalized)
{
    CurrentTimeOfDay = FMath::Clamp(TimeNormalized, 0.0f, 1.0f);
    UpdateEnvironmentAudio();
}

void UAudioSystemManager::SetWeatherIntensity(float Intensity)
{
    CurrentWeatherIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    
    if (WeatherLayer.MetaSound && WeatherLayer.AudioComponent)
    {
        // Ajustar volume baseado na intensidade
        float TargetVolume = CurrentWeatherIntensity * WeatherLayer.Volume;
        WeatherLayer.AudioComponent->SetVolumeMultiplier(TargetVolume);
        
        // Ajustar parâmetros MetaSound se disponíveis
        WeatherLayer.AudioComponent->SetFloatParameter(FName("Intensity"), CurrentWeatherIntensity);
    }
}

void UAudioSystemManager::AddTensionEvent(FVector Location, float Intensity, float Duration)
{
    // Sistema de tensão baseado em eventos pontuais
    // Implementação futura: criar sistema de "ondas" de tensão que se propagam
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Tension event at intensity %.2f"), Intensity);
    
    // Por agora, ajustar estado emocional baseado na intensidade
    if (Intensity > 0.8f)
    {
        SetEmotionalState(EEmotionalState::Terror, 0.5f);
    }
    else if (Intensity > 0.5f)
    {
        SetEmotionalState(EEmotionalState::Danger, 1.0f);
    }
    else if (Intensity > 0.2f)
    {
        SetEmotionalState(EEmotionalState::Tension, 2.0f);
    }
}

void UAudioSystemManager::SetSilenceMode(bool bEnabled, float FadeTime)
{
    bInSilenceMode = bEnabled;
    
    if (bEnabled)
    {
        // Fade out gradual de todos os layers
        for (auto& Layer : EmotionalLayers)
        {
            if (Layer.Value.AudioComponent && Layer.Value.bIsActive)
            {
                Layer.Value.AudioComponent->FadeOut(FadeTime, 0.0f);
            }
        }
        
        for (auto& Layer : EnvironmentLayers)
        {
            if (Layer.Value.AudioComponent && Layer.Value.bIsActive)
            {
                Layer.Value.AudioComponent->FadeOut(FadeTime, 0.0f);
            }
        }
    }
    else
    {
        // Restaurar áudio baseado no estado atual
        UpdateMusicLayers();
        UpdateEnvironmentAudio();
    }
}

void UAudioSystemManager::OnPlayerHidden(bool bIsHidden)
{
    if (bIsHidden)
    {
        // Reduzir volume geral e aumentar tensão
        if (DynamicSoundMix)
        {
            UGameplayStatics::SetSoundMixClassOverride(GetWorld(), DynamicSoundMix, 
                MasterSoundClass, 0.7f, 1.0f, 2.0f, true);
        }
    }
    else
    {
        // Restaurar volume normal
        if (DynamicSoundMix)
        {
            UGameplayStatics::ClearSoundMixClassOverride(GetWorld(), DynamicSoundMix, 
                MasterSoundClass, 1.0f);
        }
    }
}

void UAudioSystemManager::OnPlayerDetected(float ThreatLevel)
{
    // Escalada imediata de tensão baseada no nível de ameaça
    if (ThreatLevel > 0.9f)
    {
        SetEmotionalState(EEmotionalState::Terror, 0.2f);
    }
    else if (ThreatLevel > 0.6f)
    {
        SetEmotionalState(EEmotionalState::Danger, 0.5f);
    }
    else
    {
        SetEmotionalState(EEmotionalState::Tension, 1.0f);
    }
}

void UAudioSystemManager::OnCraftingActivity(bool bStarted)
{
    if (bStarted)
    {
        // Durante crafting, reduzir música e focar em ambiente
        if (EmotionalLayers.Contains(CurrentEmotionalState))
        {
            FAudioLayer& CurrentLayer = EmotionalLayers[CurrentEmotionalState];
            if (CurrentLayer.AudioComponent)
            {
                CurrentLayer.AudioComponent->SetVolumeMultiplier(0.3f);
            }
        }
    }
    else
    {
        // Restaurar volume da música
        UpdateMusicLayers();
    }
}

void UAudioSystemManager::UpdateMusicLayers()
{
    if (bInSilenceMode)
        return;
    
    // Lógica para ajustar layers musicais baseado no estado atual
    // Implementação futura: crossfading inteligente entre layers
}

void UAudioSystemManager::UpdateEnvironmentAudio()
{
    if (bInSilenceMode)
        return;
    
    // Ajustar ambiente baseado na hora do dia
    float NightIntensity = 0.0f;
    if (CurrentTimeOfDay < 0.25f || CurrentTimeOfDay > 0.75f)
    {
        // Noite: mais sons noturnos, menos atividade diurna
        NightIntensity = 1.0f - FMath::Abs(CurrentTimeOfDay - 0.5f) * 2.0f;
    }
    
    // Aplicar parâmetros aos MetaSounds ambientes
    if (EnvironmentLayers.Contains(CurrentEnvironment))
    {
        FAudioLayer& CurrentLayer = EnvironmentLayers[CurrentEnvironment];
        if (CurrentLayer.AudioComponent)
        {
            CurrentLayer.AudioComponent->SetFloatParameter(FName("TimeOfDay"), CurrentTimeOfDay);
            CurrentLayer.AudioComponent->SetFloatParameter(FName("NightIntensity"), NightIntensity);
            CurrentLayer.AudioComponent->SetFloatParameter(FName("WeatherIntensity"), CurrentWeatherIntensity);
        }
    }
}

void UAudioSystemManager::ProcessTensionEvents()
{
    // Processar eventos de tensão contínuos
    // Implementação futura: sistema de propagação de tensão espacial
}

void UAudioSystemManager::UpdateDinosaurAudioParameters(const FString& DinosaurID)
{
    if (!ActiveDinosaurAudio.Contains(DinosaurID))
        return;
    
    UAudioComponent* DinosaurAudio = ActiveDinosaurAudio[DinosaurID];
    if (!DinosaurAudio || !IsValid(DinosaurAudio))
        return;
    
    const FDinosaurAudioProfile& Profile = DinosaurProfiles[DinosaurID];
    
    // Ajustar parâmetros baseado na domesticação
    float DomesticationLevel = Profile.DomesticationProgressAudio;
    DinosaurAudio->SetFloatParameter(FName("DomesticationLevel"), DomesticationLevel);
    
    // Pitch mais agudo para criaturas domesticadas (sons mais "amigáveis")
    float DomesticationPitchShift = DomesticationLevel * 0.15f;
    DinosaurAudio->SetPitchMultiplier(1.0f + DomesticationPitchShift);
}