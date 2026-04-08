#include "AudioSystemManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundMix.h"
#include "AudioDevice.h"
#include "Engine/Engine.h"

void UAudioSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Initializing adaptive audio system"));
    
    // Criar componentes de áudio
    if (UWorld* World = GetWorld())
    {
        // Componente principal de música
        MusicAudioComponent = NewObject<UAudioComponent>(this);
        if (MusicAudioComponent)
        {
            MusicAudioComponent->bAutoActivate = false;
            MusicAudioComponent->bIsUISound = false;
            MusicAudioComponent->VolumeMultiplier = 0.7f;
        }

        // Componente de ambiente
        AmbientAudioComponent = NewObject<UAudioComponent>(this);
        if (AmbientAudioComponent)
        {
            AmbientAudioComponent->bAutoActivate = false;
            AmbientAudioComponent->bIsUISound = false;
            AmbientAudioComponent->VolumeMultiplier = 0.5f;
        }

        // Componente de tensão
        TensionAudioComponent = NewObject<UAudioComponent>(this);
        if (TensionAudioComponent)
        {
            TensionAudioComponent->bAutoActivate = false;
            TensionAudioComponent->bIsUISound = false;
            TensionAudioComponent->VolumeMultiplier = 0.3f;
        }
    }

    // Inicializar estado padrão
    CurrentAudioState = EAudioState::Exploration;
    CurrentStateData.TensionLevel = 0.0f;
    CurrentStateData.ProximityThreat = 0.0f;
    CurrentStateData.CurrentEnvironment = EEnvironmentType::DenseForest;

    // Configurar timers para updates contínuos
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            MusicUpdateTimer,
            this,
            &UAudioSystemManager::UpdateMusicParameters,
            0.1f, // Update a cada 100ms para responsividade
            true
        );

        World->GetTimerManager().SetTimer(
            StateUpdateTimer,
            this,
            &UAudioSystemManager::CalculateTensionLevel,
            0.5f, // Cálculo de tensão a cada 500ms
            true
        );
    }
}

void UAudioSystemManager::Deinitialize()
{
    // Limpar timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MusicUpdateTimer);
        World->GetTimerManager().ClearTimer(StateUpdateTimer);
    }

    // Parar todos os áudios
    StopAdaptiveMusic();

    Super::Deinitialize();
}

void UAudioSystemManager::UpdateAudioState(const FAudioStateData& StateData)
{
    CurrentStateData = StateData;
    
    // Determinar novo estado baseado nos dados
    EAudioState NewState = CurrentAudioState;
    
    if (StateData.ProximityThreat > 0.8f)
    {
        NewState = EAudioState::Danger;
    }
    else if (StateData.TensionLevel > 0.6f)
    {
        NewState = EAudioState::Tension;
    }
    else if (StateData.CurrentEnvironment == EEnvironmentType::PlayerBase)
    {
        NewState = EAudioState::Safety;
    }
    else
    {
        NewState = EAudioState::Exploration;
    }

    if (NewState != CurrentAudioState)
    {
        SetAudioState(NewState);
    }

    // Atualizar ambiente baseado no tipo de ambiente
    UpdateAmbientAudio(StateData.CurrentEnvironment);
}

void UAudioSystemManager::SetAudioState(EAudioState NewState)
{
    if (NewState == CurrentAudioState)
        return;

    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Transitioning from %d to %d"), 
           (int32)CurrentAudioState, (int32)NewState);

    TransitionToState(NewState);
    CurrentAudioState = NewState;
}

void UAudioSystemManager::TransitionToState(EAudioState NewState)
{
    // Aplicar Sound Mix apropriado
    if (UWorld* World = GetWorld())
    {
        // Limpar mix anterior
        UGameplayStatics::ClearSoundMixModifiers(World);

        // Aplicar novo mix
        switch (NewState)
        {
            case EAudioState::Exploration:
                if (ExplorationSoundMix)
                {
                    UGameplayStatics::PushSoundMixModifier(World, ExplorationSoundMix);
                }
                TargetMusicIntensity = 0.3f;
                break;

            case EAudioState::Tension:
                TargetMusicIntensity = 0.6f;
                break;

            case EAudioState::Danger:
                if (DangerSoundMix)
                {
                    UGameplayStatics::PushSoundMixModifier(World, DangerSoundMix);
                }
                TargetMusicIntensity = 0.9f;
                break;

            case EAudioState::Combat:
                if (CombatSoundMix)
                {
                    UGameplayStatics::PushSoundMixModifier(World, CombatSoundMix);
                }
                TargetMusicIntensity = 1.0f;
                break;

            case EAudioState::Safety:
                TargetMusicIntensity = 0.1f;
                break;

            case EAudioState::Discovery:
                TargetMusicIntensity = 0.4f;
                break;

            case EAudioState::Domestication:
                TargetMusicIntensity = 0.2f;
                break;
        }
    }
}

void UAudioSystemManager::PlayAdaptiveMusic()
{
    if (MusicAudioComponent && AdaptiveMusicMetaSound)
    {
        MusicAudioComponent->SetSound(AdaptiveMusicMetaSound);
        MusicAudioComponent->Play();
        
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Started adaptive music"));
    }
}

void UAudioSystemManager::StopAdaptiveMusic()
{
    if (MusicAudioComponent && MusicAudioComponent->IsPlaying())
    {
        MusicAudioComponent->FadeOut(2.0f, 0.0f);
    }
}

void UAudioSystemManager::SetMusicIntensity(float Intensity)
{
    TargetMusicIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
}

void UAudioSystemManager::UpdateMusicParameters()
{
    if (!MusicAudioComponent || !MusicAudioComponent->IsPlaying())
        return;

    // Interpolar suavemente para a intensidade alvo
    float InterpSpeed = 2.0f; // Velocidade de transição
    CurrentMusicIntensity = FMath::FInterpTo(
        CurrentMusicIntensity, 
        TargetMusicIntensity, 
        GetWorld()->GetDeltaSeconds(), 
        InterpSpeed
    );

    // Aplicar parâmetros ao MetaSound
    MusicAudioComponent->SetFloatParameter(FName("Intensity"), CurrentMusicIntensity);
    MusicAudioComponent->SetFloatParameter(FName("TensionLevel"), CurrentStateData.TensionLevel);
    MusicAudioComponent->SetFloatParameter(FName("ProximityThreat"), CurrentStateData.ProximityThreat);
    MusicAudioComponent->SetBoolParameter(FName("PlayerHidden"), CurrentStateData.bPlayerHidden);
}

void UAudioSystemManager::UpdateAmbientAudio(EEnvironmentType Environment)
{
    if (!AmbientAudioComponent)
        return;

    // Selecionar MetaSound baseado no ambiente
    UMetaSoundSource* TargetAmbient = nullptr;
    
    switch (Environment)
    {
        case EEnvironmentType::DenseForest:
            TargetAmbient = AmbientForestMetaSound;
            break;
        // Outros ambientes serão adicionados conforme necessário
        default:
            TargetAmbient = AmbientForestMetaSound;
            break;
    }

    if (TargetAmbient && AmbientAudioComponent->GetSound() != TargetAmbient)
    {
        AmbientAudioComponent->SetSound(TargetAmbient);
        if (!AmbientAudioComponent->IsPlaying())
        {
            AmbientAudioComponent->Play();
        }
    }

    // Atualizar parâmetros do ambiente
    if (AmbientAudioComponent->IsPlaying())
    {
        AmbientAudioComponent->SetFloatParameter(FName("DinosaurActivity"), CurrentStateData.DinosaurCount * 0.1f);
        AmbientAudioComponent->SetFloatParameter(FName("TensionLevel"), CurrentStateData.TensionLevel);
    }
}

void UAudioSystemManager::RegisterDinosaurProximity(AActor* Dinosaur, float Distance, bool bIsPredator)
{
    if (!Dinosaur)
        return;

    DinosaurProximityMap.Add(Dinosaur, Distance);
    DinosaurThreatMap.Add(Dinosaur, bIsPredator);

    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Registered dinosaur proximity - Distance: %f, Predator: %s"), 
           Distance, bIsPredator ? TEXT("Yes") : TEXT("No"));
}

void UAudioSystemManager::UnregisterDinosaurProximity(AActor* Dinosaur)
{
    if (!Dinosaur)
        return;

    DinosaurProximityMap.Remove(Dinosaur);
    DinosaurThreatMap.Remove(Dinosaur);
}

void UAudioSystemManager::CalculateTensionLevel()
{
    float NewTensionLevel = 0.0f;
    float NewProximityThreat = 0.0f;
    int32 DinosaurCount = DinosaurProximityMap.Num();

    // Calcular tensão baseada na proximidade de dinossauros
    for (const auto& ProximityPair : DinosaurProximityMap)
    {
        AActor* Dinosaur = ProximityPair.Key;
        float Distance = ProximityPair.Value;
        bool bIsPredator = DinosaurThreatMap.FindRef(Dinosaur);

        if (!Dinosaur || !IsValid(Dinosaur))
            continue;

        // Calcular contribuição para tensão (inverso da distância)
        float TensionContribution = FMath::Clamp(1000.0f / (Distance + 100.0f), 0.0f, 1.0f);
        
        if (bIsPredator)
        {
            TensionContribution *= 2.0f; // Predadores causam mais tensão
            NewProximityThreat = FMath::Max(NewProximityThreat, TensionContribution);
        }

        NewTensionLevel += TensionContribution;
    }

    // Normalizar tensão
    NewTensionLevel = FMath::Clamp(NewTensionLevel, 0.0f, 1.0f);
    NewProximityThreat = FMath::Clamp(NewProximityThreat, 0.0f, 1.0f);

    // Atualizar dados de estado
    CurrentStateData.TensionLevel = NewTensionLevel;
    CurrentStateData.ProximityThreat = NewProximityThreat;
    CurrentStateData.DinosaurCount = DinosaurCount;

    UE_LOG(LogTemp, VeryVerbose, TEXT("AudioSystemManager: Tension: %f, Threat: %f, Count: %d"), 
           NewTensionLevel, NewProximityThreat, DinosaurCount);
}

void UAudioSystemManager::PlayDinosaurAmbient(FVector Location, float Intensity)
{
    if (UWorld* World = GetWorld())
    {
        // Implementar sistema de áudio posicional para dinossauros específicos
        // Este método será expandido para suportar diferentes tipos de dinossauros
        UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Playing dinosaur ambient at location %s with intensity %f"), 
               *Location.ToString(), Intensity);
    }
}

void UAudioSystemManager::UpdateAmbientParameters()
{
    // Método para atualização contínua de parâmetros ambientes
    if (AmbientAudioComponent && AmbientAudioComponent->IsPlaying())
    {
        // Atualizar parâmetros baseados no estado atual
        AmbientAudioComponent->SetFloatParameter(FName("TimeOfDay"), 0.5f); // Placeholder - será conectado ao sistema de dia/noite
        AmbientAudioComponent->SetFloatParameter(FName("WeatherIntensity"), 0.3f); // Placeholder - será conectado ao sistema de clima
    }
}