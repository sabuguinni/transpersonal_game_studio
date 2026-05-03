#include "AudioSubsystem.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "TimerManager.h"

UAudioSubsystem::UAudioSubsystem()
{
    CurrentBiome = EBiomeType::Savana;
    CurrentThreatLevel = EThreatLevel::Safe;
    CurrentAmbientAudio = nullptr;
    MasterVolume = 1.0f;
    AmbientVolume = 0.7f;
    EffectsVolume = 0.8f;
}

void UAudioSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("AudioSubsystem: Inicializado"));
    
    InitializeAudioData();
    
    // Setup cleanup timer para remover audio components terminados
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            CleanupTimerHandle,
            this,
            &UAudioSubsystem::CleanupFinishedAudio,
            5.0f,  // Cada 5 segundos
            true   // Loop
        );
    }
}

void UAudioSubsystem::Deinitialize()
{
    // Parar todo o áudio activo
    if (CurrentAmbientAudio && IsValid(CurrentAmbientAudio))
    {
        CurrentAmbientAudio->Stop();
        CurrentAmbientAudio = nullptr;
    }
    
    for (UAudioComponent* AudioComp : ActiveEffectAudio)
    {
        if (IsValid(AudioComp))
        {
            AudioComp->Stop();
        }
    }
    ActiveEffectAudio.Empty();
    
    // Limpar timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(CleanupTimerHandle);
    }
    
    Super::Deinitialize();
}

void UAudioSubsystem::InitializeAudioData()
{
    // Configurar dados de áudio por bioma
    FAudio_BiomeAudioData PantanoData;
    PantanoData.Volume = 0.6f;
    PantanoData.FadeInTime = 3.0f;
    BiomeAudioData.Add(EBiomeType::Pantano, PantanoData);
    
    FAudio_BiomeAudioData FlorestaData;
    FlorestaData.Volume = 0.7f;
    FlorestaData.FadeInTime = 2.5f;
    BiomeAudioData.Add(EBiomeType::Floresta, FlorestaData);
    
    FAudio_BiomeAudioData SavanaData;
    SavanaData.Volume = 0.8f;
    SavanaData.FadeInTime = 2.0f;
    BiomeAudioData.Add(EBiomeType::Savana, SavanaData);
    
    FAudio_BiomeAudioData DesertoData;
    DesertoData.Volume = 0.5f;
    DesertoData.FadeInTime = 4.0f;
    BiomeAudioData.Add(EBiomeType::Deserto, DesertoData);
    
    FAudio_BiomeAudioData MontanhaData;
    MontanhaData.Volume = 0.6f;
    MontanhaData.FadeInTime = 3.5f;
    BiomeAudioData.Add(EBiomeType::MontanhaNevada, MontanhaData);
    
    // Configurar dados de áudio por espécie de dinossauro
    FAudio_DinosaurAudioData TRexData;
    TRexData.MaxHearingDistance = 8000.0f;
    TRexData.Volume = 1.0f;
    DinosaurAudioData.Add(EDinosaurSpecies::TyrannosaurusRex, TRexData);
    
    FAudio_DinosaurAudioData RaptorData;
    RaptorData.MaxHearingDistance = 4000.0f;
    RaptorData.Volume = 0.7f;
    DinosaurAudioData.Add(EDinosaurSpecies::Velociraptor, RaptorData);
    
    FAudio_DinosaurAudioData BrachiosaurusData;
    BrachiosaurusData.MaxHearingDistance = 6000.0f;
    BrachiosaurusData.Volume = 0.9f;
    DinosaurAudioData.Add(EDinosaurSpecies::Brachiosaurus, BrachiosaurusData);
    
    FAudio_DinosaurAudioData TriceratopsData;
    TriceratopsData.MaxHearingDistance = 5000.0f;
    TriceratopsData.Volume = 0.8f;
    DinosaurAudioData.Add(EDinosaurSpecies::Triceratops, TriceratopsData);
    
    UE_LOG(LogTemp, Warning, TEXT("AudioSubsystem: Dados de áudio inicializados"));
}

void UAudioSubsystem::SetCurrentBiome(EBiomeType NewBiome)
{
    if (CurrentBiome != NewBiome)
    {
        EBiomeType OldBiome = CurrentBiome;
        CurrentBiome = NewBiome;
        
        UE_LOG(LogTemp, Warning, TEXT("AudioSubsystem: Transição de bioma %d -> %d"), 
               (int32)OldBiome, (int32)NewBiome);
        
        TransitionAmbientAudio(NewBiome);
    }
}

void UAudioSubsystem::SetThreatLevel(EThreatLevel NewThreatLevel)
{
    if (CurrentThreatLevel != NewThreatLevel)
    {
        EThreatLevel OldThreatLevel = CurrentThreatLevel;
        CurrentThreatLevel = NewThreatLevel;
        
        UE_LOG(LogTemp, Warning, TEXT("AudioSubsystem: Nível de ameaça alterado %d -> %d"), 
               (int32)OldThreatLevel, (int32)NewThreatLevel);
        
        // Ajustar volume do áudio ambiente baseado no nível de ameaça
        if (CurrentAmbientAudio && IsValid(CurrentAmbientAudio))
        {
            float ThreatVolumeMultiplier = 1.0f;
            switch (CurrentThreatLevel)
            {
                case EThreatLevel::Safe:
                    ThreatVolumeMultiplier = 1.0f;
                    break;
                case EThreatLevel::Caution:
                    ThreatVolumeMultiplier = 0.8f;
                    break;
                case EThreatLevel::Danger:
                    ThreatVolumeMultiplier = 0.6f;
                    break;
                case EThreatLevel::Critical:
                    ThreatVolumeMultiplier = 0.4f;
                    break;
            }
            
            if (FAudio_BiomeAudioData* BiomeData = BiomeAudioData.Find(CurrentBiome))
            {
                float FinalVolume = BiomeData->Volume * ThreatVolumeMultiplier * AmbientVolume * MasterVolume;
                CurrentAmbientAudio->SetVolumeMultiplier(FinalVolume);
            }
        }
    }
}

void UAudioSubsystem::PlayDinosaurSound(EDinosaurSpecies Species, const FVector& Location, bool bIsThreat)
{
    USoundCue* SoundToPlay = GetRandomDinosaurSound(Species, bIsThreat);
    if (!SoundToPlay)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSubsystem: Sem som disponível para espécie %d"), (int32)Species);
        return;
    }
    
    if (FAudio_DinosaurAudioData* DinoData = DinosaurAudioData.Find(Species))
    {
        UWorld* World = GetWorld();
        if (!World) return;
        
        UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(
            World,
            SoundToPlay,
            Location,
            FRotator::ZeroRotator,
            DinoData->Volume * EffectsVolume * MasterVolume,
            1.0f,  // Pitch
            0.0f,  // Start time
            nullptr,  // Attenuation settings
            nullptr,  // Concurrency settings
            false  // Auto destroy
        );
        
        if (AudioComp)
        {
            ActiveEffectAudio.Add(AudioComp);
            UE_LOG(LogTemp, Log, TEXT("AudioSubsystem: Som de dinossauro reproduzido para espécie %d"), (int32)Species);
        }
    }
}

void UAudioSubsystem::PlayFootstepSound(EDinosaurSpecies Species, const FVector& Location, float VolumeMultiplier)
{
    if (FAudio_DinosaurAudioData* DinoData = DinosaurAudioData.Find(Species))
    {
        // Por agora, usar sons de movimento como footsteps
        if (DinoData->MovementSounds.Num() > 0)
        {
            int32 RandomIndex = FMath::RandRange(0, DinoData->MovementSounds.Num() - 1);
            if (USoundCue* FootstepSound = DinoData->MovementSounds[RandomIndex].LoadSynchronous())
            {
                UWorld* World = GetWorld();
                if (!World) return;
                
                UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(
                    World,
                    FootstepSound,
                    Location,
                    FRotator::ZeroRotator,
                    DinoData->Volume * VolumeMultiplier * EffectsVolume * MasterVolume * 0.6f,  // Footsteps mais baixos
                    1.0f,
                    0.0f,
                    nullptr,
                    nullptr,
                    false
                );
                
                if (AudioComp)
                {
                    ActiveEffectAudio.Add(AudioComp);
                }
            }
        }
    }
}

void UAudioSubsystem::PlayEnvironmentalSound(const FString& SoundName, const FVector& Location, float Volume)
{
    // Implementação para sons ambientais específicos (vento, água, etc.)
    UE_LOG(LogTemp, Log, TEXT("AudioSubsystem: Som ambiental '%s' solicitado em %s"), 
           *SoundName, *Location.ToString());
    
    // Por agora, apenas log - sons específicos serão adicionados conforme necessário
}

void UAudioSubsystem::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    // Actualizar volume do áudio ambiente activo
    if (CurrentAmbientAudio && IsValid(CurrentAmbientAudio))
    {
        if (FAudio_BiomeAudioData* BiomeData = BiomeAudioData.Find(CurrentBiome))
        {
            float FinalVolume = BiomeData->Volume * AmbientVolume * MasterVolume;
            CurrentAmbientAudio->SetVolumeMultiplier(FinalVolume);
        }
    }
}

void UAudioSubsystem::SetAmbientVolume(float Volume)
{
    AmbientVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    SetMasterVolume(MasterVolume);  // Trigger volume update
}

void UAudioSubsystem::SetEffectsVolume(float Volume)
{
    EffectsVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

void UAudioSubsystem::TransitionAmbientAudio(EBiomeType NewBiome)
{
    // Parar áudio ambiente actual
    if (CurrentAmbientAudio && IsValid(CurrentAmbientAudio))
    {
        CurrentAmbientAudio->FadeOut(2.0f, 0.0f);
        CurrentAmbientAudio = nullptr;
    }
    
    // Por agora, apenas log da transição
    // Os SoundCues específicos serão configurados quando os assets estiverem disponíveis
    UE_LOG(LogTemp, Warning, TEXT("AudioSubsystem: Transição para bioma %d"), (int32)NewBiome);
}

void UAudioSubsystem::CleanupFinishedAudio()
{
    // Remover audio components que já terminaram
    ActiveEffectAudio.RemoveAll([](UAudioComponent* AudioComp) {
        return !IsValid(AudioComp) || !AudioComp->IsPlaying();
    });
}

USoundCue* UAudioSubsystem::GetRandomDinosaurSound(EDinosaurSpecies Species, bool bIsThreat)
{
    if (FAudio_DinosaurAudioData* DinoData = DinosaurAudioData.Find(Species))
    {
        TArray<TSoftObjectPtr<USoundCue>>* SoundArray = bIsThreat ? &DinoData->ThreatSounds : &DinoData->IdleSounds;
        
        if (SoundArray->Num() > 0)
        {
            int32 RandomIndex = FMath::RandRange(0, SoundArray->Num() - 1);
            return (*SoundArray)[RandomIndex].LoadSynchronous();
        }
    }
    
    return nullptr;
}