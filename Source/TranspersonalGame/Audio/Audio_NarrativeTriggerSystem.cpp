#include "Audio_NarrativeTriggerSystem.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UAudio_NarrativeTriggerSystem::UAudio_NarrativeTriggerSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    // Configurações padrão
    BaseVolume = 1.0f;
    FadeInTime = 2.0f;
    FadeOutTime = 1.5f;
    bUseSpatialAudio = true;

    // Estado inicial
    CurrentEvent = nullptr;
    bPlayerInRange = false;
    CurrentFadeTime = 0.0f;
    bIsFading = false;

    // Criar componente de áudio
    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    if (AudioComponent)
    {
        AudioComponent->bAutoActivate = false;
        AudioComponent->SetVolumeMultiplier(0.0f);
    }

    // Criar trigger sphere
    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    if (TriggerSphere)
    {
        TriggerSphere->SetSphereRadius(500.0f);
        TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        TriggerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
        TriggerSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    }
}

void UAudio_NarrativeTriggerSystem::BeginPlay()
{
    Super::BeginPlay();

    // Configurar eventos de trigger
    if (TriggerSphere)
    {
        TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &UAudio_NarrativeTriggerSystem::OnTriggerEnter);
        TriggerSphere->OnComponentEndOverlap.AddDynamic(this, &UAudio_NarrativeTriggerSystem::OnTriggerExit);
    }

    // Configurar componente de áudio
    if (AudioComponent)
    {
        if (bUseSpatialAudio)
        {
            AudioComponent->SetAttenuationSettings(nullptr); // Usar configurações padrão de atenuação
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Audio_NarrativeTriggerSystem: Iniciado com %d eventos narrativos"), NarrativeEvents.Num());
}

void UAudio_NarrativeTriggerSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Actualizar cooldowns
    UpdateCooldowns(DeltaTime);

    // Processar queue de eventos
    if (bPlayerInRange && EventQueue.Num() > 0)
    {
        ProcessNarrativeQueue();
    }

    // Processar fade in/out
    if (bIsFading && AudioComponent)
    {
        CurrentFadeTime += DeltaTime;
        
        if (CurrentEvent)
        {
            float FadeProgress = FMath::Clamp(CurrentFadeTime / FadeInTime, 0.0f, 1.0f);
            float TargetVolume = BaseVolume * FadeProgress;
            AudioComponent->SetVolumeMultiplier(TargetVolume);

            if (FadeProgress >= 1.0f)
            {
                bIsFading = false;
                CurrentFadeTime = 0.0f;
            }
        }
    }
}

void UAudio_NarrativeTriggerSystem::OnTriggerEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // Verificar se é o jogador
    if (ACharacter* Character = Cast<ACharacter>(OtherActor))
    {
        if (Character->IsPlayerControlled())
        {
            bPlayerInRange = true;
            UE_LOG(LogTemp, Log, TEXT("Audio_NarrativeTriggerSystem: Jogador entrou no trigger"));

            // Adicionar eventos disponíveis à queue
            for (const FAudio_NarrativeEvent& Event : NarrativeEvents)
            {
                if (!IsEventOnCooldown(Event.EventID))
                {
                    EventQueue.Add(Event);
                }
            }

            // Ordenar por prioridade
            EventQueue.Sort([](const FAudio_NarrativeEvent& A, const FAudio_NarrativeEvent& B) {
                return A.Priority > B.Priority;
            });
        }
    }
}

void UAudio_NarrativeTriggerSystem::OnTriggerExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
    if (ACharacter* Character = Cast<ACharacter>(OtherActor))
    {
        if (Character->IsPlayerControlled())
        {
            bPlayerInRange = false;
            UE_LOG(LogTemp, Log, TEXT("Audio_NarrativeTriggerSystem: Jogador saiu do trigger"));

            // Fade out do áudio actual
            if (AudioComponent && AudioComponent->IsPlaying())
            {
                AudioComponent->FadeOut(FadeOutTime, 0.0f);
            }

            // Limpar queue
            EventQueue.Empty();
            CurrentEvent = nullptr;
        }
    }
}

void UAudio_NarrativeTriggerSystem::ProcessNarrativeQueue()
{
    if (EventQueue.Num() == 0 || !AudioComponent)
    {
        return;
    }

    // Se não há áudio a tocar, reproduzir o próximo evento
    if (!AudioComponent->IsPlaying() && !CurrentEvent)
    {
        FAudio_NarrativeEvent NextEvent = EventQueue[0];
        EventQueue.RemoveAt(0);
        
        PlayNarrativeEvent(NextEvent);
    }
}

void UAudio_NarrativeTriggerSystem::PlayNarrativeEvent(const FAudio_NarrativeEvent& Event)
{
    if (!AudioComponent)
    {
        return;
    }

    // Carregar o áudio
    if (Event.AudioClip.IsValid())
    {
        USoundBase* SoundToPlay = Event.AudioClip.LoadSynchronous();
        if (SoundToPlay)
        {
            AudioComponent->SetSound(SoundToPlay);
            AudioComponent->SetVolumeMultiplier(0.0f);
            AudioComponent->Play();

            // Iniciar fade in
            bIsFading = true;
            CurrentFadeTime = 0.0f;
            CurrentEvent = const_cast<FAudio_NarrativeEvent*>(&Event);

            // Adicionar cooldown
            if (Event.bOneShot || Event.CooldownTime > 0.0f)
            {
                EventCooldowns.Add(Event.EventID, Event.CooldownTime);
            }

            UE_LOG(LogTemp, Log, TEXT("Audio_NarrativeTriggerSystem: Reproduzindo evento narrativo: %s"), *Event.EventID);
        }
    }
}

void UAudio_NarrativeTriggerSystem::UpdateCooldowns(float DeltaTime)
{
    TArray<FString> ExpiredCooldowns;

    for (auto& Cooldown : EventCooldowns)
    {
        Cooldown.Value -= DeltaTime;
        if (Cooldown.Value <= 0.0f)
        {
            ExpiredCooldowns.Add(Cooldown.Key);
        }
    }

    // Remover cooldowns expirados
    for (const FString& ExpiredID : ExpiredCooldowns)
    {
        EventCooldowns.Remove(ExpiredID);
    }
}

void UAudio_NarrativeTriggerSystem::TriggerNarrativeEvent(const FString& EventID)
{
    for (const FAudio_NarrativeEvent& Event : NarrativeEvents)
    {
        if (Event.EventID == EventID && !IsEventOnCooldown(EventID))
        {
            EventQueue.Insert(Event, 0); // Adicionar no início da queue (prioridade máxima)
            break;
        }
    }
}

void UAudio_NarrativeTriggerSystem::StopCurrentNarrative()
{
    if (AudioComponent && AudioComponent->IsPlaying())
    {
        AudioComponent->FadeOut(FadeOutTime, 0.0f);
    }

    CurrentEvent = nullptr;
    bIsFading = false;
    CurrentFadeTime = 0.0f;
}

void UAudio_NarrativeTriggerSystem::AddNarrativeEvent(const FAudio_NarrativeEvent& NewEvent)
{
    NarrativeEvents.Add(NewEvent);
    UE_LOG(LogTemp, Log, TEXT("Audio_NarrativeTriggerSystem: Adicionado evento narrativo: %s"), *NewEvent.EventID);
}

bool UAudio_NarrativeTriggerSystem::IsEventOnCooldown(const FString& EventID) const
{
    return EventCooldowns.Contains(EventID);
}