#include "NarrativeManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/AssetManager.h"

ANarrativeManager::ANarrativeManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Criar componente de áudio
    NarrativeAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("NarrativeAudio"));
    RootComponent = NarrativeAudioComponent;
    
    // Configurações padrão
    NarrativeVolume = 0.8f;
    bNarrativeEnabled = true;
    LastProximityCheck = 0.0f;
    
    // Configurar triggers padrão
    FNarr_NarrativeTrigger SpawnTrigger;
    SpawnTrigger.EventType = ENarr_NarrativeEvent::PlayerSpawn;
    SpawnTrigger.NarrativeText = FText::FromString(TEXT("Welcome to the Cretaceous period. Your survival begins now."));
    SpawnTrigger.TriggerRadius = 100.0f;
    SpawnTrigger.bIsOneTimeEvent = true;
    NarrativeTriggers.Add(SpawnTrigger);
    
    FNarr_NarrativeTrigger DinosaurTrigger;
    DinosaurTrigger.EventType = ENarr_NarrativeEvent::FirstDinosaurSighting;
    DinosaurTrigger.NarrativeText = FText::FromString(TEXT("Massive creatures ahead. Move carefully and avoid direct confrontation."));
    DinosaurTrigger.TriggerRadius = 1000.0f;
    DinosaurTrigger.bIsOneTimeEvent = true;
    NarrativeTriggers.Add(DinosaurTrigger);
    
    FNarr_NarrativeTrigger ResourceTrigger;
    ResourceTrigger.EventType = ENarr_NarrativeEvent::ResourceDiscovery;
    ResourceTrigger.NarrativeText = FText::FromString(TEXT("Essential resources detected. Gather what you can, but stay alert."));
    ResourceTrigger.TriggerRadius = 800.0f;
    ResourceTrigger.bIsOneTimeEvent = false;
    NarrativeTriggers.Add(ResourceTrigger);
}

void ANarrativeManager::BeginPlay()
{
    Super::BeginPlay();
    
    if (NarrativeAudioComponent)
    {
        NarrativeAudioComponent->SetVolumeMultiplier(NarrativeVolume);
        NarrativeAudioComponent->bAutoActivate = false;
    }
    
    // Trigger evento inicial
    if (bNarrativeEnabled)
    {
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
        {
            TriggerNarrativeEvent(ENarr_NarrativeEvent::PlayerSpawn, FVector::ZeroVector);
        }, 2.0f, false);
    }
}

void ANarrativeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!bNarrativeEnabled)
        return;
    
    // Check proximity triggers periodicamente
    LastProximityCheck += DeltaTime;
    if (LastProximityCheck >= ProximityCheckInterval)
    {
        LastProximityCheck = 0.0f;
        
        // Obter posição do jogador
        APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
        if (PC && PC->GetPawn())
        {
            FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
            CheckProximityTriggers(PlayerLocation);
        }
    }
}

void ANarrativeManager::TriggerNarrativeEvent(ENarr_NarrativeEvent EventType, const FVector& PlayerLocation)
{
    if (!bNarrativeEnabled)
        return;
    
    // Encontrar trigger correspondente
    for (FNarr_NarrativeTrigger& Trigger : NarrativeTriggers)
    {
        if (Trigger.EventType == EventType)
        {
            // Verificar se já foi disparado (para eventos únicos)
            if (Trigger.bIsOneTimeEvent && Trigger.bHasBeenTriggered)
                continue;
            
            // Marcar como disparado
            Trigger.bHasBeenTriggered = true;
            
            // Log do evento
            UE_LOG(LogTemp, Warning, TEXT("Narrative Event Triggered: %s"), 
                   *Trigger.NarrativeText.ToString());
            
            // Reproduzir áudio se disponível
            if (!Trigger.AudioClipPath.IsEmpty())
            {
                PlayNarrativeAudio(Trigger.AudioClipPath);
            }
            
            // Mostrar texto na tela (placeholder)
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
                    FString::Printf(TEXT("NARRATIVE: %s"), *Trigger.NarrativeText.ToString()));
            }
            
            break;
        }
    }
}

void ANarrativeManager::PlayNarrativeAudio(const FString& AudioPath)
{
    if (!NarrativeAudioComponent || AudioPath.IsEmpty())
        return;
    
    // Tentar carregar o áudio
    USoundBase* AudioClip = LoadObject<USoundBase>(nullptr, *AudioPath);
    if (AudioClip)
    {
        NarrativeAudioComponent->SetSound(AudioClip);
        NarrativeAudioComponent->Play();
        
        UE_LOG(LogTemp, Log, TEXT("Playing narrative audio: %s"), *AudioPath);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load narrative audio: %s"), *AudioPath);
    }
}

void ANarrativeManager::RegisterNarrativeTrigger(const FNarr_NarrativeTrigger& NewTrigger)
{
    NarrativeTriggers.Add(NewTrigger);
    UE_LOG(LogTemp, Log, TEXT("Registered new narrative trigger"));
}

void ANarrativeManager::SetNarrativeEnabled(bool bEnabled)
{
    bNarrativeEnabled = bEnabled;
    
    if (!bEnabled && NarrativeAudioComponent && NarrativeAudioComponent->IsPlaying())
    {
        NarrativeAudioComponent->Stop();
    }
}

void ANarrativeManager::CheckProximityTriggers(const FVector& PlayerLocation)
{
    // Posições conhecidas no mapa para triggers específicos
    TMap<ENarr_NarrativeEvent, FVector> TriggerLocations;
    TriggerLocations.Add(ENarr_NarrativeEvent::FirstDinosaurSighting, FVector(2000, 1000, 100));
    TriggerLocations.Add(ENarr_NarrativeEvent::ResourceDiscovery, FVector(-1500, -1000, 100));
    
    for (const auto& LocationPair : TriggerLocations)
    {
        float Distance = FVector::Dist(PlayerLocation, LocationPair.Value);
        
        // Encontrar trigger correspondente
        for (const FNarr_NarrativeTrigger& Trigger : NarrativeTriggers)
        {
            if (Trigger.EventType == LocationPair.Key && Distance <= Trigger.TriggerRadius)
            {
                if (!Trigger.bIsOneTimeEvent || !Trigger.bHasBeenTriggered)
                {
                    TriggerNarrativeEvent(LocationPair.Key, PlayerLocation);
                    break;
                }
            }
        }
    }
}

void ANarrativeManager::OnNarrativeAudioFinished()
{
    UE_LOG(LogTemp, Log, TEXT("Narrative audio playback finished"));
}