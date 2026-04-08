#include "AudioArchitecture.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "AudioMixerBlueprintLibrary.h"

UAudioArchitecture::UAudioArchitecture()
{
    CurrentTensionLevel = 0.0f;
    PreviousEmotionalState = EAudioEmotionalState::Calm;
    
    // Inicializar contexto padrão
    CurrentAudioContext.EmotionalState = EAudioEmotionalState::Calm;
    CurrentAudioContext.Environment = EEnvironmentType::DenseForest;
    CurrentAudioContext.TimeOfDay = ETimeOfDay::Morning;
    CurrentAudioContext.ThreatLevel = 0.0f;
    CurrentAudioContext.PlayerStealthLevel = 0.0f;
    CurrentAudioContext.bIsNearWater = false;
    CurrentAudioContext.bIsInShelter = false;
    CurrentAudioContext.NearbyDinosaurCount = 0;
    CurrentAudioContext.bHasDomesticatedCompanion = false;
}

void UAudioArchitecture::UpdateAudioContext(const FAudioContext& NewContext)
{
    FAudioContext PreviousContext = CurrentAudioContext;
    CurrentAudioContext = NewContext;

    // Calcular novo nível de tensão baseado no contexto
    float NewTensionLevel = CalculateTensionFromContext(NewContext);
    
    // Se mudou o estado emocional, fazer transição
    if (PreviousContext.EmotionalState != NewContext.EmotionalState)
    {
        TransitionToEmotionalState(NewContext.EmotionalState, 2.0f);
    }
    
    // Se mudou o ambiente ou hora do dia, atualizar áudio ambiental
    if (PreviousContext.Environment != NewContext.Environment || 
        PreviousContext.TimeOfDay != NewContext.TimeOfDay)
    {
        UpdateEnvironmentalAudio(NewContext.Environment, NewContext.TimeOfDay);
    }

    // Atualizar parâmetros do MetaSound principal
    if (MasterMusicMetaSound && MusicAudioComponent)
    {
        // Parâmetros dinâmicos para o MetaSound
        MusicAudioComponent->SetFloatParameter(FName("TensionLevel"), NewTensionLevel);
        MusicAudioComponent->SetFloatParameter(FName("ThreatLevel"), NewContext.ThreatLevel);
        MusicAudioComponent->SetFloatParameter(FName("StealthLevel"), NewContext.PlayerStealthLevel);
        MusicAudioComponent->SetBoolParameter(FName("IsNearWater"), NewContext.bIsNearWater);
        MusicAudioComponent->SetBoolParameter(FName("IsInShelter"), NewContext.bIsInShelter);
        MusicAudioComponent->SetIntParameter(FName("DinosaurCount"), NewContext.NearbyDinosaurCount);
    }

    CurrentTensionLevel = NewTensionLevel;
}

void UAudioArchitecture::TransitionToEmotionalState(EAudioEmotionalState NewState, float TransitionTime)
{
    if (NewState == CurrentAudioContext.EmotionalState)
        return;

    EAudioEmotionalState PreviousState = CurrentAudioContext.EmotionalState;
    
    // Executar crossfade entre estados
    PerformAudioCrossfade(PreviousState, NewState, TransitionTime);
    
    // Atualizar estado atual
    PreviousEmotionalState = PreviousState;
    CurrentAudioContext.EmotionalState = NewState;

    UE_LOG(LogTemp, Log, TEXT("Audio: Transição de estado %d para %d em %.1f segundos"), 
           (int32)PreviousState, (int32)NewState, TransitionTime);
}

void UAudioArchitecture::UpdateEnvironmentalAudio(EEnvironmentType Environment, ETimeOfDay TimeOfDay)
{
    // Atualizar MetaSound ambiental com novos parâmetros
    if (MasterAmbientMetaSound && AmbientAudioComponent)
    {
        // Mapear enum para valores numéricos para o MetaSound
        float EnvironmentValue = (float)Environment;
        float TimeValue = (float)TimeOfDay;
        
        AmbientAudioComponent->SetFloatParameter(FName("EnvironmentType"), EnvironmentValue);
        AmbientAudioComponent->SetFloatParameter(FName("TimeOfDay"), TimeValue);
        
        // Calcular volume baseado na hora do dia
        float AmbientVolume = CalculateAmbientVolumeFromTimeOfDay(TimeOfDay);
        AmbientAudioComponent->SetFloatParameter(FName("AmbientVolume"), AmbientVolume);
    }

    UE_LOG(LogTemp, Log, TEXT("Audio: Ambiente atualizado para %d, hora %d"), 
           (int32)Environment, (int32)TimeOfDay);
}

void UAudioArchitecture::RegisterDinosaurAudio(AActor* DinosaurActor, const FString& SpeciesName, bool bIsHostile)
{
    if (!DinosaurActor)
        return;

    // Criar componente de áudio para o dinossauro se não existir
    UAudioComponent* DinosaurAudio = DinosaurActor->FindComponentByClass<UAudioComponent>();
    if (!DinosaurAudio)
    {
        DinosaurAudio = NewObject<UAudioComponent>(DinosaurActor);
        DinosaurActor->AddInstanceComponent(DinosaurAudio);
        DinosaurAudio->RegisterComponent();
    }

    // Configurar áudio baseado na espécie e hostilidade
    if (DinosaurAudio)
    {
        DinosaurAudio->SetBoolParameter(FName("IsHostile"), bIsHostile);
        DinosaurAudio->SetStringParameter(FName("SpeciesName"), SpeciesName);
        
        // Aplicar atenuação 3D apropriada
        if (DinosaurAudio->GetAttenuationSettings())
        {
            // Dinossauros hostis têm maior alcance de áudio para criar tensão
            float MaxDistance = bIsHostile ? 2000.0f : 1000.0f;
            DinosaurAudio->GetAttenuationSettings()->Attenuation.FalloffDistance = MaxDistance;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Audio: Dinossauro %s registrado (Hostil: %s)"), 
           *SpeciesName, bIsHostile ? TEXT("Sim") : TEXT("Não"));
}

void UAudioArchitecture::UnregisterDinosaurAudio(AActor* DinosaurActor)
{
    if (!DinosaurActor)
        return;

    UAudioComponent* DinosaurAudio = DinosaurActor->FindComponentByClass<UAudioComponent>();
    if (DinosaurAudio)
    {
        DinosaurAudio->Stop();
        DinosaurAudio->DestroyComponent();
    }
}

void UAudioArchitecture::PlayPlayerActionFeedback(const FString& ActionType, float Intensity)
{
    // Mapear ações para sons específicos
    FName SoundParameterName = FName(*FString::Printf(TEXT("%s_Intensity"), *ActionType));
    
    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetFloatParameter(SoundParameterName, Intensity);
        
        // Trigger para ação específica
        FName TriggerName = FName(*FString::Printf(TEXT("Trigger_%s"), *ActionType));
        MusicAudioComponent->SetTriggerParameter(TriggerName);
    }

    UE_LOG(LogTemp, Log, TEXT("Audio: Feedback de ação %s com intensidade %.2f"), 
           *ActionType, Intensity);
}

void UAudioArchitecture::UpdateProximityAudio(float DistanceToNearestThreat)
{
    // Calcular nível de proximidade (inverso da distância, normalizado)
    float ProximityLevel = FMath::Clamp(1.0f - (DistanceToNearestThreat / 1000.0f), 0.0f, 1.0f);
    
    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetFloatParameter(FName("ProximityTension"), ProximityLevel);
        
        // Aplicar filtro de frequência baseado na proximidade
        float FilterCutoff = CalculateFilterCutoffFromThreat(ProximityLevel);
        MusicAudioComponent->SetFloatParameter(FName("FilterCutoff"), FilterCutoff);
    }
}

void UAudioArchitecture::PerformAudioCrossfade(EAudioEmotionalState FromState, EAudioEmotionalState ToState, float Duration)
{
    // Implementar crossfade suave entre estados musicais
    if (MusicAudioComponent)
    {
        // Usar parâmetros do MetaSound para controlar o crossfade
        MusicAudioComponent->SetFloatParameter(FName("CrossfadeDuration"), Duration);
        MusicAudioComponent->SetFloatParameter(FName("FromState"), (float)FromState);
        MusicAudioComponent->SetFloatParameter(FName("ToState"), (float)ToState);
        MusicAudioComponent->SetTriggerParameter(FName("TriggerCrossfade"));
    }
}

float UAudioArchitecture::CalculateTensionFromContext(const FAudioContext& Context)
{
    float BaseTension = Context.ThreatLevel;
    
    // Modificadores baseados no contexto
    if (Context.EmotionalState == EAudioEmotionalState::Danger || 
        Context.EmotionalState == EAudioEmotionalState::Chase)
    {
        BaseTension += 0.3f;
    }
    
    if (Context.NearbyDinosaurCount > 0)
    {
        BaseTension += (Context.NearbyDinosaurCount * 0.1f);
    }
    
    if (Context.TimeOfDay == ETimeOfDay::Night || Context.TimeOfDay == ETimeOfDay::DeepNight)
    {
        BaseTension += 0.2f;
    }
    
    if (Context.bIsInShelter)
    {
        BaseTension -= 0.2f;
    }
    
    if (Context.bHasDomesticatedCompanion)
    {
        BaseTension -= 0.1f;
    }
    
    return FMath::Clamp(BaseTension, 0.0f, 1.0f);
}

float UAudioArchitecture::CalculateAmbientVolumeFromTimeOfDay(ETimeOfDay TimeOfDay)
{
    switch (TimeOfDay)
    {
        case ETimeOfDay::Dawn:
        case ETimeOfDay::Dusk:
            return 0.8f; // Momentos de transição são mais ativos
        case ETimeOfDay::Night:
        case ETimeOfDay::DeepNight:
            return 0.6f; // Noite mais silenciosa, mas com sons específicos
        case ETimeOfDay::Morning:
        case ETimeOfDay::Afternoon:
            return 1.0f; // Dia cheio de vida
        case ETimeOfDay::Midday:
            return 0.7f; // Meio-dia mais calmo
        default:
            return 0.8f;
    }
}

float UAudioArchitecture::CalculateFilterCutoffFromThreat(float ThreatLevel)
{
    // Frequências mais altas cortadas quando há ameaça (simula tensão)
    float BaseCutoff = 20000.0f; // 20kHz
    float MinCutoff = 8000.0f;   // 8kHz quando ameaça máxima
    
    return FMath::Lerp(BaseCutoff, MinCutoff, ThreatLevel);
}