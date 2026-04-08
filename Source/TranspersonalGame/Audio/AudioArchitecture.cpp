#include "AudioArchitecture.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundAttenuation.h"

UAudioManager::UAudioManager()
{
    CurrentEmotionalState = EEmotionalState::Calm_Wonder;
    CurrentAmbientType = EAmbientType::Dense_Forest;
}

void UAudioManager::BeginPlay()
{
    Super::BeginPlay();

    // Criar componentes de áudio
    if (UWorld* World = GetWorld())
    {
        AActor* Owner = GetTypedOuter<AActor>();
        if (!Owner)
        {
            // Criar um actor invisível para hospedar os componentes de áudio
            Owner = World->SpawnActor<AActor>();
        }

        MusicComponent = NewObject<UAudioComponent>(Owner);
        MusicComponent->bAutoActivate = false;
        MusicComponent->AttachToComponent(Owner->GetRootComponent(), 
            FAttachmentTransformRules::KeepRelativeTransform);

        AmbientComponent = NewObject<UAudioComponent>(Owner);
        AmbientComponent->bAutoActivate = false;
        AmbientComponent->AttachToComponent(Owner->GetRootComponent(), 
            FAttachmentTransformRules::KeepRelativeTransform);

        DinosaurComponent = NewObject<UAudioComponent>(Owner);
        DinosaurComponent->bAutoActivate = false;
        DinosaurComponent->AttachToComponent(Owner->GetRootComponent(), 
            FAttachmentTransformRules::KeepRelativeTransform);

        // Iniciar com estado padrão
        if (EmotionalTracks.Contains(CurrentEmotionalState))
        {
            MusicComponent->SetSound(EmotionalTracks[CurrentEmotionalState]);
            MusicComponent->Play();
        }

        if (AmbientSystems.Contains(CurrentAmbientType))
        {
            AmbientComponent->SetSound(AmbientSystems[CurrentAmbientType]);
            AmbientComponent->Play();
        }
    }
}

void UAudioManager::TransitionToEmotionalState(EEmotionalState NewState, float TransitionTime)
{
    if (NewState == CurrentEmotionalState) return;

    UE_LOG(LogTemp, Log, TEXT("Audio: Transitioning to emotional state %d"), (int32)NewState);

    CurrentEmotionalState = NewState;

    if (EmotionalTracks.Contains(NewState))
    {
        if (UWorld* World = GetWorld())
        {
            // Fade out atual
            if (MusicComponent && MusicComponent->IsPlaying())
            {
                MusicComponent->FadeOut(TransitionTime * 0.7f, 0.0f);
            }

            // Agendar fade in da nova música
            World->GetTimerManager().SetTimer(MusicTransitionTimer, 
                FTimerDelegate::CreateUObject(this, &UAudioManager::ExecuteMusicTransition, 
                EmotionalTracks[NewState]), 
                TransitionTime * 0.3f, false);
        }
    }
}

void UAudioManager::SetAmbientEnvironment(EAmbientType NewAmbient, float TransitionTime)
{
    if (NewAmbient == CurrentAmbientType) return;

    UE_LOG(LogTemp, Log, TEXT("Audio: Transitioning to ambient type %d"), (int32)NewAmbient);

    CurrentAmbientType = NewAmbient;

    if (AmbientSystems.Contains(NewAmbient))
    {
        if (UWorld* World = GetWorld())
        {
            // Fade out atual
            if (AmbientComponent && AmbientComponent->IsPlaying())
            {
                AmbientComponent->FadeOut(TransitionTime * 0.8f, 0.0f);
            }

            // Agendar fade in do novo ambiente
            World->GetTimerManager().SetTimer(AmbientTransitionTimer, 
                FTimerDelegate::CreateUObject(this, &UAudioManager::ExecuteAmbientTransition, 
                AmbientSystems[NewAmbient]), 
                TransitionTime * 0.2f, false);
        }
    }
}

void UAudioManager::PlayDinosaurSound(class ADinosaurCharacter* Dinosaur, const FString& SoundType)
{
    // Esta função será expandida quando o sistema de dinossauros estiver implementado
    UE_LOG(LogTemp, Log, TEXT("Audio: Playing dinosaur sound %s"), *SoundType);
}

void UAudioManager::TriggerStingerSound(const FString& StingerType)
{
    UE_LOG(LogTemp, Log, TEXT("Audio: Triggering stinger %s"), *StingerType);
    
    // Implementar sistema de stingers para momentos específicos
    // (descoberta de pistas, morte de dinossauro, etc.)
}

void UAudioManager::ExecuteMusicTransition(UMetaSoundSource* NewTrack)
{
    if (MusicComponent && NewTrack)
    {
        MusicComponent->SetSound(NewTrack);
        MusicComponent->Play();
        MusicComponent->FadeIn(1.5f, 1.0f);
    }
}

void UAudioManager::ExecuteAmbientTransition(UMetaSoundSource* NewAmbient)
{
    if (AmbientComponent && NewAmbient)
    {
        AmbientComponent->SetSound(NewAmbient);
        AmbientComponent->Play();
        AmbientComponent->FadeIn(2.0f, 1.0f);
    }
}

// Implementação do Volume de Detecção de Dinossauros

ADinosaurAudioVolume::ADinosaurAudioVolume()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.5f; // Check every 0.5 seconds
}

void ADinosaurAudioVolume::BeginPlay()
{
    Super::BeginPlay();
    
    // Encontrar o AudioManager no mundo
    if (UWorld* World = GetWorld())
    {
        AudioManager = NewObject<UAudioManager>(this);
        if (AudioManager)
        {
            AudioManager->BeginPlay();
        }
    }
}

void ADinosaurAudioVolume::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    CheckForNearbyDinosaurs();
}

void ADinosaurAudioVolume::CheckForNearbyDinosaurs()
{
    if (!AudioManager) return;

    bool bHasPredators = HasPredatorsInRange();
    
    if (bHasPredators && AudioManager->CurrentEmotionalState == UAudioManager::EEmotionalState::Calm_Wonder)
    {
        AudioManager->TransitionToEmotionalState(TriggerEmotionalState, 1.0f);
    }
    else if (!bHasPredators && AudioManager->CurrentEmotionalState == UAudioManager::EEmotionalState::Tension_Building)
    {
        AudioManager->TransitionToEmotionalState(UAudioManager::EEmotionalState::Calm_Wonder, 3.0f);
    }
}

bool ADinosaurAudioVolume::HasPredatorsInRange()
{
    // Esta função será expandida quando o sistema de dinossauros estiver implementado
    // Por agora, retorna false
    return false;
}