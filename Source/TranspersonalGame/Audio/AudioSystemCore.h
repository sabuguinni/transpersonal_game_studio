#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "AudioGameplayVolume.h"
#include "AudioSystemCore.generated.h"

UENUM(BlueprintType)
enum class EAudioZoneType : uint8
{
    SafeZone,           // Base do jogador - música calma
    ExplorationZone,    // Exploração normal - tensão baixa
    DangerZone,         // Predadores próximos - tensão alta
    HuntingZone,        // Sendo caçado - música de perseguição
    AmbientZone         // Zonas específicas (rios, cavernas, etc)
};

UENUM(BlueprintType)
enum class EThreatLevel : uint8
{
    None,               // Sem ameaças
    Low,                // Herbívoros pacíficos
    Medium,             // Predadores pequenos
    High,               // Predadores grandes
    Extreme             // T-Rex, Spinosaurus, etc
};

USTRUCT(BlueprintType)
struct FAudioStateData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAudioZoneType CurrentZone = EAudioZoneType::SafeZone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EThreatLevel ThreatLevel = EThreatLevel::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StressLevel = 0.0f; // 0-1

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeOfDay = 12.0f; // 0-24 hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsRaining = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsInCover = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<class ADinosaur*> NearbyDinosaurs;
};

/**
 * Sistema central de áudio adaptativo
 * Controla música, ambiente e efeitos baseado no estado do jogo
 */
UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudioSystemCore : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudioSystemCore();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Estado atual do sistema de áudio
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    FAudioStateData CurrentAudioState;

    // MetaSounds para música adaptativa
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    class UMetaSoundSource* AdaptiveMusicSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    class UMetaSoundSource* AmbientSoundscape;

    // Componentes de áudio
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* MusicComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* AmbientComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* StingerComponent;

    // Funções públicas
    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void UpdateAudioZone(EAudioZoneType NewZone);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void UpdateThreatLevel(EThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void UpdateStressLevel(float NewStressLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void PlayStinger(class USoundCue* StingerSound);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void OnDinosaurSpotted(class ADinosaur* Dinosaur);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void OnDinosaurLost(class ADinosaur* Dinosaur);

private:
    // Funções internas
    void UpdateMusicParameters();
    void UpdateAmbientParameters();
    void CheckNearbyThreats();
    
    // Timers e estado
    float LastMusicUpdate = 0.0f;
    float MusicUpdateInterval = 0.5f; // Update music twice per second
    
    FTimerHandle ThreatCheckTimer;
};