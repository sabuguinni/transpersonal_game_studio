#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundAttenuation.h"
#include "MetasoundSource.h"
#include "DinosaurAudioComponent.generated.h"

UENUM(BlueprintType)
enum class EDinosaurAudioState : uint8
{
    Idle,           // Respiração, movimentos casuais
    Feeding,        // Sons de alimentação
    Calling,        // Chamadas para outros dinossauros
    Alert,          // Detectou algo suspeito
    Aggressive,     // Pronto para atacar
    Hunting,        // Perseguindo presa
    Injured,        // Ferido
    Dying           // Morrendo
};

UENUM(BlueprintType)
enum class EDinosaurSize : uint8
{
    Small,          // Compsognathus, Microraptor
    Medium,         // Velociraptor, Dilophosaurus
    Large,          // Allosaurus, Carnotaurus
    Massive         // T-Rex, Spinosaurus, Brachiosaurus
};

USTRUCT(BlueprintType)
struct FDinosaurAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurSize Size = EDinosaurSize::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsCarnivore = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AggressionLevel = 0.5f; // 0-1

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VocalFrequency = 0.3f; // How often they make sounds

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FootstepVolume = 1.0f; // Multiplier for footstep volume
};

/**
 * Componente de áudio para dinossauros
 * Gere sons procedurais baseados no comportamento e características
 */
UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurAudioComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurAudioComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Dados do dinossauro
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    FDinosaurAudioData AudioData;

    // Estado atual
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EDinosaurAudioState CurrentState = EDinosaurAudioState::Idle;

    // MetaSounds para sons procedurais
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    class UMetaSoundSource* BreathingMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    class UMetaSoundSource* FootstepsMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    class UMetaSoundSource* VocalizationMetaSound;

    // Sound Cues para eventos específicos
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    TMap<EDinosaurAudioState, class USoundCue*> StateSounds;

    // Atenuação de som
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    class USoundAttenuation* AudioAttenuation;

    // Componentes de áudio
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* BreathingComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* FootstepsComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* VocalizationComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* EventSoundComponent;

    // Funções públicas
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void SetAudioState(EDinosaurAudioState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void PlayFootstep(float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void PlayVocalization(float Intensity = 1.0f, float Pitch = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void SetMovementSpeed(float Speed); // 0-1, affects breathing and footsteps

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void SetHealthPercentage(float HealthPercent); // 0-1, affects breathing and vocalizations

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void OnTakeDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void OnSpotPlayer();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void OnLosePlayer();

private:
    // Estado interno
    float CurrentMovementSpeed = 0.0f;
    float CurrentHealthPercent = 1.0f;
    float LastVocalizationTime = 0.0f;
    float TimeSinceLastFootstep = 0.0f;

    // Timers
    FTimerHandle VocalizationTimer;
    FTimerHandle BreathingUpdateTimer;

    // Funções internas
    void UpdateBreathingParameters();
    void UpdateFootstepParameters();
    void UpdateVocalizationParameters();
    void ScheduleNextVocalization();
    void PlayRandomVocalization();
    
    // Helpers
    float GetSizeVolumeMultiplier() const;
    float GetSizePitchMultiplier() const;
    float GetAttenuationDistance() const;
};