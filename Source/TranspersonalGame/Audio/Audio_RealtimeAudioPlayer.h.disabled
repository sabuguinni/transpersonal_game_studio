#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Audio_RealtimeAudioPlayer.generated.h"

UENUM(BlueprintType)
enum class EAudio_VoicelineType : uint8
{
    ThreatNarration,
    SurvivalGuide,
    ScoutReport,
    PredatorEncounter,
    HerbalistWisdom,
    NightSurvival
};

USTRUCT(BlueprintType)
struct FAudio_VoicelineData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_VoicelineType VoicelineType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString CharacterName;

    FAudio_VoicelineData()
    {
        AudioURL = TEXT("");
        Duration = 0.0f;
        VoicelineType = EAudio_VoicelineType::ThreatNarration;
        CharacterName = TEXT("");
    }
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_RealtimeAudioPlayer : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_RealtimeAudioPlayer();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Reproduzir voiceline por URL
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayVoicelineFromURL(const FString& AudioURL, float Duration, EAudio_VoicelineType VoiceType);

    // Reproduzir voiceline por tipo
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayVoicelineByType(EAudio_VoicelineType VoiceType);

    // Parar reprodução actual
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopCurrentVoiceline();

    // Verificar se está a reproduzir
    UFUNCTION(BlueprintPure, Category = "Audio")
    bool IsPlayingVoiceline() const;

    // Registar nova voiceline
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void RegisterVoiceline(const FAudio_VoicelineData& VoicelineData);

protected:
    // Componente de áudio para reprodução
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    class UAudioComponent* AudioComponent;

    // Base de dados de voicelines
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TArray<FAudio_VoicelineData> VoicelineDatabase;

    // Estado actual
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    bool bIsPlaying;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    EAudio_VoicelineType CurrentVoicelineType;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    float CurrentPlaybackTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    float CurrentDuration;

    // Timer para controlo de duração
    FTimerHandle VoicelineTimerHandle;

private:
    // Callback quando voiceline termina
    UFUNCTION()
    void OnVoicelineFinished();

    // Encontrar voiceline por tipo
    FAudio_VoicelineData* FindVoicelineByType(EAudio_VoicelineType VoiceType);

    // Carregar áudio de URL (simulado para agora)
    class USoundWave* LoadAudioFromURL(const FString& URL);

    // Inicializar base de dados padrão
    void InitializeDefaultVoicelines();
};