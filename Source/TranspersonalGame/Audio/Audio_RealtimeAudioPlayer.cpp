#include "Audio_RealtimeAudioPlayer.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "Sound/SoundWave.h"
#include "TimerManager.h"
#include "Engine/World.h"

UAudio_RealtimeAudioPlayer::UAudio_RealtimeAudioPlayer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;

    // Criar componente de áudio
    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    
    // Inicializar estado
    bIsPlaying = false;
    CurrentVoicelineType = EAudio_VoicelineType::ThreatNarration;
    CurrentPlaybackTime = 0.0f;
    CurrentDuration = 0.0f;
}

void UAudio_RealtimeAudioPlayer::BeginPlay()
{
    Super::BeginPlay();
    
    // Configurar componente de áudio
    if (AudioComponent)
    {
        AudioComponent->SetVolumeMultiplier(0.8f);
        AudioComponent->SetPitchMultiplier(1.0f);
        AudioComponent->bAutoDestroy = false;
        AudioComponent->bStopWhenOwnerDestroyed = true;
    }

    // Inicializar base de dados de voicelines
    InitializeDefaultVoicelines();

    UE_LOG(LogTemp, Warning, TEXT("Audio_RealtimeAudioPlayer: BeginPlay completed"));
}

void UAudio_RealtimeAudioPlayer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Actualizar tempo de reprodução
    if (bIsPlaying)
    {
        CurrentPlaybackTime += DeltaTime;
        
        // Verificar se excedeu a duração
        if (CurrentPlaybackTime >= CurrentDuration)
        {
            OnVoicelineFinished();
        }
    }
}

void UAudio_RealtimeAudioPlayer::PlayVoicelineFromURL(const FString& AudioURL, float Duration, EAudio_VoicelineType VoiceType)
{
    if (!AudioComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("Audio_RealtimeAudioPlayer: AudioComponent is null"));
        return;
    }

    // Parar reprodução actual se existir
    StopCurrentVoiceline();

    // Configurar nova reprodução
    CurrentVoicelineType = VoiceType;
    CurrentDuration = Duration;
    CurrentPlaybackTime = 0.0f;
    bIsPlaying = true;

    // Log da reprodução
    UE_LOG(LogTemp, Warning, TEXT("Audio_RealtimeAudioPlayer: Playing voiceline from URL: %s (Duration: %.2f)"), *AudioURL, Duration);

    // Por agora, simular reprodução com timer
    // Em implementação futura, carregar áudio real da URL
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            VoicelineTimerHandle,
            this,
            &UAudio_RealtimeAudioPlayer::OnVoicelineFinished,
            Duration,
            false
        );
    }

    // Notificar início da reprodução
    if (GEngine)
    {
        FString VoiceTypeName = UEnum::GetValueAsString(VoiceType);
        GEngine->AddOnScreenDebugMessage(-1, Duration, FColor::Green, 
            FString::Printf(TEXT("Playing Voiceline: %s"), *VoiceTypeName));
    }
}

void UAudio_RealtimeAudioPlayer::PlayVoicelineByType(EAudio_VoicelineType VoiceType)
{
    FAudio_VoicelineData* VoicelineData = FindVoicelineByType(VoiceType);
    if (VoicelineData)
    {
        PlayVoicelineFromURL(VoicelineData->AudioURL, VoicelineData->Duration, VoiceType);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_RealtimeAudioPlayer: Voiceline type not found in database"));
    }
}

void UAudio_RealtimeAudioPlayer::StopCurrentVoiceline()
{
    if (bIsPlaying)
    {
        bIsPlaying = false;
        CurrentPlaybackTime = 0.0f;
        CurrentDuration = 0.0f;

        // Parar componente de áudio
        if (AudioComponent && AudioComponent->IsPlaying())
        {
            AudioComponent->Stop();
        }

        // Limpar timer
        if (GetWorld())
        {
            GetWorld()->GetTimerManager().ClearTimer(VoicelineTimerHandle);
        }

        UE_LOG(LogTemp, Log, TEXT("Audio_RealtimeAudioPlayer: Stopped current voiceline"));
    }
}

bool UAudio_RealtimeAudioPlayer::IsPlayingVoiceline() const
{
    return bIsPlaying;
}

void UAudio_RealtimeAudioPlayer::RegisterVoiceline(const FAudio_VoicelineData& VoicelineData)
{
    // Verificar se já existe
    for (int32 i = 0; i < VoicelineDatabase.Num(); i++)
    {
        if (VoicelineDatabase[i].VoicelineType == VoicelineData.VoicelineType)
        {
            // Actualizar existente
            VoicelineDatabase[i] = VoicelineData;
            UE_LOG(LogTemp, Log, TEXT("Audio_RealtimeAudioPlayer: Updated existing voiceline"));
            return;
        }
    }

    // Adicionar nova
    VoicelineDatabase.Add(VoicelineData);
    UE_LOG(LogTemp, Log, TEXT("Audio_RealtimeAudioPlayer: Registered new voiceline"));
}

void UAudio_RealtimeAudioPlayer::OnVoicelineFinished()
{
    UE_LOG(LogTemp, Log, TEXT("Audio_RealtimeAudioPlayer: Voiceline finished"));
    
    bIsPlaying = false;
    CurrentPlaybackTime = 0.0f;
    CurrentDuration = 0.0f;

    // Limpar timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(VoicelineTimerHandle);
    }

    // Notificar fim da reprodução
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Voiceline Finished"));
    }
}

FAudio_VoicelineData* UAudio_RealtimeAudioPlayer::FindVoicelineByType(EAudio_VoicelineType VoiceType)
{
    for (FAudio_VoicelineData& VoicelineData : VoicelineDatabase)
    {
        if (VoicelineData.VoicelineType == VoiceType)
        {
            return &VoicelineData;
        }
    }
    return nullptr;
}

USoundWave* UAudio_RealtimeAudioPlayer::LoadAudioFromURL(const FString& URL)
{
    // TODO: Implementar carregamento real de áudio de URL
    // Por agora retornar nullptr e usar simulação
    UE_LOG(LogTemp, Warning, TEXT("Audio_RealtimeAudioPlayer: LoadAudioFromURL not yet implemented for: %s"), *URL);
    return nullptr;
}

void UAudio_RealtimeAudioPlayer::InitializeDefaultVoicelines()
{
    // Registar voicelines padrão geradas pelo TTS
    FAudio_VoicelineData ThreatData;
    ThreatData.VoicelineType = EAudio_VoicelineType::ThreatNarration;
    ThreatData.CharacterName = TEXT("ThreatNarrator");
    ThreatData.Duration = 16.0f;
    ThreatData.AudioURL = TEXT("https://supabase_url/threat_narration.mp3");
    VoicelineDatabase.Add(ThreatData);

    FAudio_VoicelineData SurvivalData;
    SurvivalData.VoicelineType = EAudio_VoicelineType::SurvivalGuide;
    SurvivalData.CharacterName = TEXT("SurvivalGuide");
    SurvivalData.Duration = 14.0f;
    SurvivalData.AudioURL = TEXT("https://supabase_url/survival_guide.mp3");
    VoicelineDatabase.Add(SurvivalData);

    FAudio_VoicelineData ScoutData;
    ScoutData.VoicelineType = EAudio_VoicelineType::ScoutReport;
    ScoutData.CharacterName = TEXT("Scout");
    ScoutData.Duration = 14.0f;
    ScoutData.AudioURL = TEXT("https://supabase_url/scout_report.mp3");
    VoicelineDatabase.Add(ScoutData);

    FAudio_VoicelineData PredatorData;
    PredatorData.VoicelineType = EAudio_VoicelineType::PredatorEncounter;
    PredatorData.CharacterName = TEXT("Survivor");
    PredatorData.Duration = 16.0f;
    PredatorData.AudioURL = TEXT("https://supabase_url/predator_encounter.mp3");
    VoicelineDatabase.Add(PredatorData);

    UE_LOG(LogTemp, Warning, TEXT("Audio_RealtimeAudioPlayer: Initialized %d default voicelines"), VoicelineDatabase.Num());
}