#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Engine/World.h"
#include "AudioSystemManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_SoundType : uint8
{
    Dialogue        UMETA(DisplayName = "Dialogue"),
    Ambient         UMETA(DisplayName = "Ambient"),
    Footsteps       UMETA(DisplayName = "Footsteps"),
    DinosaurRoar    UMETA(DisplayName = "Dinosaur Roar"),
    Warning         UMETA(DisplayName = "Warning"),
    Music           UMETA(DisplayName = "Music")
};

UENUM(BlueprintType)
enum class EAudio_Priority : uint8
{
    Low             UMETA(DisplayName = "Low"),
    Normal          UMETA(DisplayName = "Normal"),
    High            UMETA(DisplayName = "High"),
    Critical        UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct FAudio_SoundData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString SoundID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_SoundType SoundType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_Priority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Volume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bIs3D;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MaxDistance;

    FAudio_SoundData()
    {
        SoundID = TEXT("");
        SoundType = EAudio_SoundType::Ambient;
        Priority = EAudio_Priority::Normal;
        AudioURL = TEXT("");
        Volume = 1.0f;
        Duration = 0.0f;
        bIs3D = true;
        MaxDistance = 5000.0f;
    }
};

USTRUCT(BlueprintType)
struct FAudio_SpatialConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    float AttenuationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    float ReverbIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    bool bUseOcclusion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    float DopplerFactor;

    FAudio_SpatialConfig()
    {
        AttenuationRadius = 3000.0f;
        ReverbIntensity = 0.5f;
        bUseOcclusion = true;
        DopplerFactor = 1.0f;
    }
};

/**
 * Sistema de gestão de áudio espacial para o jogo pré-histórico
 * Integra com o sistema de diálogos e efeitos ambientais
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UAudioSystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UAudioSystemManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Sistema de reprodução de áudio
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlaySoundAtLocation(const FAudio_SoundData& SoundData, const FVector& Location, AActor* SourceActor = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayDialogue(const FString& DialogueID, AActor* Speaker, AActor* Listener = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayAmbientSound(const FString& SoundID, const FVector& Location, float FadeInTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StopSound(const FString& SoundID, float FadeOutTime = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StopAllSounds(EAudio_SoundType SoundType = EAudio_SoundType::Ambient);

    // Sistema de configuração espacial
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetSpatialConfig(const FAudio_SpatialConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateListenerPosition(const FVector& Position, const FRotator& Rotation);

    // Sistema de gestão de volume
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetVolumeByType(EAudio_SoundType SoundType, float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    float GetVolumeByType(EAudio_SoundType SoundType) const;

    // Sistema de carregamento de áudio
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void RegisterSoundData(const FAudio_SoundData& SoundData);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    FAudio_SoundData GetSoundData(const FString& SoundID) const;

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    bool HasSoundData(const FString& SoundID) const;

    // Sistema de eventos
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAudioEvent, FString, SoundID, EAudio_SoundType, SoundType);
    
    UPROPERTY(BlueprintAssignable, Category = "Audio Events")
    FOnAudioEvent OnSoundStarted;

    UPROPERTY(BlueprintAssignable, Category = "Audio Events")
    FOnAudioEvent OnSoundFinished;

protected:
    // Base de dados de sons
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio System", meta = (AllowPrivateAccess = "true"))
    TMap<FString, FAudio_SoundData> SoundDatabase;

    // Configuração espacial
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio System", meta = (AllowPrivateAccess = "true"))
    FAudio_SpatialConfig SpatialConfig;

    // Volumes por tipo
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio System", meta = (AllowPrivateAccess = "true"))
    TMap<EAudio_SoundType, float> VolumeSettings;

    // Componentes de áudio activos
    UPROPERTY(BlueprintReadOnly, Category = "Audio System", meta = (AllowPrivateAccess = "true"))
    TMap<FString, UAudioComponent*> ActiveAudioComponents;

    // Volume master
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio System", meta = (AllowPrivateAccess = "true"))
    float MasterVolume;

private:
    void InitializeVolumeSettings();
    UAudioComponent* CreateAudioComponent(AActor* SourceActor, const FVector& Location);
    void CleanupFinishedComponents();
    
    FTimerHandle CleanupTimerHandle;
};