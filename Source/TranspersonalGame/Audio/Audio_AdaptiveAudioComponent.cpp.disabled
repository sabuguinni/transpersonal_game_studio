#include "Audio_AdaptiveAudioComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UAudio_AdaptiveAudioComponent::UAudio_AdaptiveAudioComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    // Configurações padrão
    MasterVolume = 1.0f;
    BiomeDetectionRadius = 5000.0f;
    UpdateInterval = 1.0f;
    CurrentBiome = EBiomeType::Savana;
    CurrentThreatLevel = EThreatLevel::Safe;
    bIsTransitioning = false;
    TransitionTimer = 0.0f;
    TransitionDuration = 2.0f;

    // Criar componentes de áudio
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    MusicAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicAudio"));
    ThreatAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ThreatAudio"));

    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->bAutoActivate = false;
        AmbientAudioComponent->SetVolumeMultiplier(0.7f);
    }

    if (MusicAudioComponent)
    {
        MusicAudioComponent->bAutoActivate = false;
        MusicAudioComponent->SetVolumeMultiplier(0.5f);
    }

    if (ThreatAudioComponent)
    {
        ThreatAudioComponent->bAutoActivate = false;
        ThreatAudioComponent->SetVolumeMultiplier(0.8f);
    }
}

void UAudio_AdaptiveAudioComponent::BeginPlay()
{
    Super::BeginPlay();

    InitializeAudioComponents();
    SetupDefaultAudioData();

    // Iniciar timer de actualização
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(UpdateTimerHandle, this, &UAudio_AdaptiveAudioComponent::UpdateAudioBasedOnLocation, UpdateInterval, true);
    }

    UE_LOG(LogTemp, Warning, TEXT("AdaptiveAudioComponent: BeginPlay - Sistema de áudio inicializado"));
}

void UAudio_AdaptiveAudioComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(UpdateTimerHandle);
    }

    StopAllAudio();
    Super::EndPlay(EndPlayReason);
}

void UAudio_AdaptiveAudioComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Actualizar transições de áudio
    if (bIsTransitioning)
    {
        TransitionTimer += DeltaTime;
        if (TransitionTimer >= TransitionDuration)
        {
            bIsTransitioning = false;
            TransitionTimer = 0.0f;
        }
    }

    // Actualizar threat audio baseado em proximidade
    UpdateThreatAudio();
}

void UAudio_AdaptiveAudioComponent::SetBiome(EBiomeType NewBiome)
{
    if (NewBiome != CurrentBiome)
    {
        UE_LOG(LogTemp, Warning, TEXT("AdaptiveAudioComponent: Mudança de bioma de %d para %d"), (int32)CurrentBiome, (int32)NewBiome);
        TransitionToBiomeAudio(NewBiome);
        CurrentBiome = NewBiome;
    }
}

void UAudio_AdaptiveAudioComponent::SetThreatLevel(EThreatLevel NewThreatLevel)
{
    if (NewThreatLevel != CurrentThreatLevel)
    {
        UE_LOG(LogTemp, Warning, TEXT("AdaptiveAudioComponent: Mudança de threat level de %d para %d"), (int32)CurrentThreatLevel, (int32)NewThreatLevel);
        TransitionToThreatAudio(NewThreatLevel);
        CurrentThreatLevel = NewThreatLevel;
    }
}

void UAudio_AdaptiveAudioComponent::PlayOneShot(USoundBase* Sound, float Volume)
{
    if (Sound && GetWorld())
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, GetOwner()->GetActorLocation(), Volume * MasterVolume);
        UE_LOG(LogTemp, Warning, TEXT("AdaptiveAudioComponent: PlayOneShot - Som reproduzido"));
    }
}

void UAudio_AdaptiveAudioComponent::StopAllAudio()
{
    if (AmbientAudioComponent && AmbientAudioComponent->IsPlaying())
    {
        AmbientAudioComponent->Stop();
    }

    if (MusicAudioComponent && MusicAudioComponent->IsPlaying())
    {
        MusicAudioComponent->Stop();
    }

    if (ThreatAudioComponent && ThreatAudioComponent->IsPlaying())
    {
        ThreatAudioComponent->Stop();
    }

    UE_LOG(LogTemp, Warning, TEXT("AdaptiveAudioComponent: Todos os áudios parados"));
}

void UAudio_AdaptiveAudioComponent::SetMasterVolume(float NewVolume)
{
    MasterVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);

    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->SetVolumeMultiplier(0.7f * MasterVolume);
    }

    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetVolumeMultiplier(0.5f * MasterVolume);
    }

    if (ThreatAudioComponent)
    {
        ThreatAudioComponent->SetVolumeMultiplier(0.8f * MasterVolume);
    }

    UE_LOG(LogTemp, Warning, TEXT("AdaptiveAudioComponent: Master volume definido para %f"), MasterVolume);
}

void UAudio_AdaptiveAudioComponent::UpdateAudioBasedOnLocation()
{
    if (!GetOwner())
        return;

    FVector CurrentLocation = GetOwner()->GetActorLocation();
    EBiomeType DetectedBiome = DetectBiomeAtLocation(CurrentLocation);
    EThreatLevel DetectedThreat = DetectThreatLevel();

    SetBiome(DetectedBiome);
    SetThreatLevel(DetectedThreat);
}

void UAudio_AdaptiveAudioComponent::UpdateThreatAudio()
{
    // Implementar lógica de threat audio baseada em proximidade de dinossauros
    if (CurrentThreatLevel != EThreatLevel::Safe && ThreatAudioComponent)
    {
        if (FAudio_ThreatAudioData* ThreatData = ThreatAudioMap.Find(CurrentThreatLevel))
        {
            if (ThreatData->ThreatSound && !ThreatAudioComponent->IsPlaying())
            {
                ThreatAudioComponent->SetSound(ThreatData->ThreatSound);
                ThreatAudioComponent->SetVolumeMultiplier(ThreatData->ThreatVolume * MasterVolume);
                ThreatAudioComponent->Play();
            }
        }
    }
    else if (CurrentThreatLevel == EThreatLevel::Safe && ThreatAudioComponent && ThreatAudioComponent->IsPlaying())
    {
        ThreatAudioComponent->Stop();
    }
}

EBiomeType UAudio_AdaptiveAudioComponent::DetectBiomeAtLocation(const FVector& Location)
{
    // Coordenadas dos biomas baseadas na memória do Hugo
    // PANTANO (sudoeste): Centro (-50000, -45000, 0)
    if (Location.X >= -77500 && Location.X <= -25000 && Location.Y >= -76500 && Location.Y <= -15000)
    {
        return EBiomeType::Pantano;
    }
    // FLORESTA (noroeste): Centro (-45000, 40000, 0)
    else if (Location.X >= -77500 && Location.X <= -15000 && Location.Y >= 15000 && Location.Y <= 76500)
    {
        return EBiomeType::Floresta;
    }
    // DESERTO (leste): Centro (55000, 0, 0)
    else if (Location.X >= 25000 && Location.X <= 79500 && Location.Y >= -30000 && Location.Y <= 30000)
    {
        return EBiomeType::Deserto;
    }
    // MONTANHA NEVADA (nordeste): Centro (40000, 50000, 500)
    else if (Location.X >= 15000 && Location.X <= 79500 && Location.Y >= 20000 && Location.Y <= 76500)
    {
        return EBiomeType::MontanhaNevada;
    }
    // SAVANA (centro): Centro (0, 0, 0) - padrão
    else
    {
        return EBiomeType::Savana;
    }
}

EThreatLevel UAudio_AdaptiveAudioComponent::DetectThreatLevel()
{
    // Detectar dinossauros próximos para determinar threat level
    if (!GetWorld() || !GetOwner())
        return EThreatLevel::Safe;

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), AllActors);

    float ClosestDangerousDistance = FLT_MAX;
    FVector OwnerLocation = GetOwner()->GetActorLocation();

    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor != GetOwner())
        {
            FString ActorName = Actor->GetName();
            // Detectar dinossauros perigosos
            if (ActorName.Contains(TEXT("TRex")) || ActorName.Contains(TEXT("Allosaurus")) || ActorName.Contains(TEXT("Raptor")))
            {
                float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
                if (Distance < ClosestDangerousDistance)
                {
                    ClosestDangerousDistance = Distance;
                }
            }
        }
    }

    // Determinar threat level baseado na distância
    if (ClosestDangerousDistance < 1000.0f)
    {
        return EThreatLevel::Critical;
    }
    else if (ClosestDangerousDistance < 2500.0f)
    {
        return EThreatLevel::Danger;
    }
    else if (ClosestDangerousDistance < 5000.0f)
    {
        return EThreatLevel::Caution;
    }

    return EThreatLevel::Safe;
}

void UAudio_AdaptiveAudioComponent::TransitionToBiomeAudio(EBiomeType NewBiome)
{
    if (FAudio_BiomeAudioData* BiomeData = BiomeAudioMap.Find(NewBiome))
    {
        bIsTransitioning = true;
        TransitionTimer = 0.0f;
        TransitionDuration = BiomeData->FadeInTime;

        // Parar áudio anterior
        if (AmbientAudioComponent && AmbientAudioComponent->IsPlaying())
        {
            AmbientAudioComponent->FadeOut(BiomeData->FadeOutTime, 0.0f);
        }
        if (MusicAudioComponent && MusicAudioComponent->IsPlaying())
        {
            MusicAudioComponent->FadeOut(BiomeData->FadeOutTime, 0.0f);
        }

        // Iniciar novo áudio
        if (BiomeData->AmbientSound && AmbientAudioComponent)
        {
            AmbientAudioComponent->SetSound(BiomeData->AmbientSound);
            AmbientAudioComponent->FadeIn(BiomeData->FadeInTime, BiomeData->AmbientVolume * MasterVolume);
        }

        if (BiomeData->MusicTrack && MusicAudioComponent)
        {
            MusicAudioComponent->SetSound(BiomeData->MusicTrack);
            MusicAudioComponent->FadeIn(BiomeData->FadeInTime, BiomeData->MusicVolume * MasterVolume);
        }

        UE_LOG(LogTemp, Warning, TEXT("AdaptiveAudioComponent: Transição para bioma %d iniciada"), (int32)NewBiome);
    }
}

void UAudio_AdaptiveAudioComponent::TransitionToThreatAudio(EThreatLevel NewThreatLevel)
{
    if (FAudio_ThreatAudioData* ThreatData = ThreatAudioMap.Find(NewThreatLevel))
    {
        if (ThreatData->ThreatSound && ThreatAudioComponent)
        {
            if (ThreatAudioComponent->IsPlaying())
            {
                ThreatAudioComponent->Stop();
            }

            ThreatAudioComponent->SetSound(ThreatData->ThreatSound);
            ThreatAudioComponent->SetVolumeMultiplier(ThreatData->ThreatVolume * MasterVolume);
            
            if (NewThreatLevel != EThreatLevel::Safe)
            {
                ThreatAudioComponent->Play();
            }
        }
    }
}

void UAudio_AdaptiveAudioComponent::InitializeAudioComponents()
{
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->bAutoActivate = false;
        AmbientAudioComponent->SetUISound(false);
    }

    if (MusicAudioComponent)
    {
        MusicAudioComponent->bAutoActivate = false;
        MusicAudioComponent->SetUISound(false);
    }

    if (ThreatAudioComponent)
    {
        ThreatAudioComponent->bAutoActivate = false;
        ThreatAudioComponent->SetUISound(false);
    }
}

void UAudio_AdaptiveAudioComponent::SetupDefaultAudioData()
{
    // Configurar dados padrão para cada bioma
    FAudio_BiomeAudioData DefaultBiomeData;
    DefaultBiomeData.AmbientVolume = 0.7f;
    DefaultBiomeData.MusicVolume = 0.5f;
    DefaultBiomeData.FadeInTime = 2.0f;
    DefaultBiomeData.FadeOutTime = 2.0f;

    BiomeAudioMap.Add(EBiomeType::Pantano, DefaultBiomeData);
    BiomeAudioMap.Add(EBiomeType::Floresta, DefaultBiomeData);
    BiomeAudioMap.Add(EBiomeType::Savana, DefaultBiomeData);
    BiomeAudioMap.Add(EBiomeType::Deserto, DefaultBiomeData);
    BiomeAudioMap.Add(EBiomeType::MontanhaNevada, DefaultBiomeData);

    // Configurar dados padrão para threat levels
    FAudio_ThreatAudioData DefaultThreatData;
    DefaultThreatData.ThreatVolume = 0.8f;
    DefaultThreatData.TriggerDistance = 1000.0f;
    DefaultThreatData.bLooping = false;

    ThreatAudioMap.Add(EThreatLevel::Safe, DefaultThreatData);
    ThreatAudioMap.Add(EThreatLevel::Caution, DefaultThreatData);
    ThreatAudioMap.Add(EThreatLevel::Danger, DefaultThreatData);
    ThreatAudioMap.Add(EThreatLevel::Critical, DefaultThreatData);

    UE_LOG(LogTemp, Warning, TEXT("AdaptiveAudioComponent: Dados de áudio padrão configurados"));
}