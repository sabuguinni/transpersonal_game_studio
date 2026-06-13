#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "Audio_MetaSoundManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Plains      UMETA(DisplayName = "Plains"),
    River       UMETA(DisplayName = "River"),
    Cave        UMETA(DisplayName = "Cave"),
    Mountain    UMETA(DisplayName = "Mountain")
};

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Caution     UMETA(DisplayName = "Caution"),
    Danger      UMETA(DisplayName = "Danger"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct FAudio_BiomeAudioConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    class UMetaSoundSource* AmbientMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float BaseVolume = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float FadeOutTime = 2.0f;

    FAudio_BiomeAudioConfig()
    {
        AmbientMetaSound = nullptr;
        BaseVolume = 0.5f;
        FadeInTime = 2.0f;
        FadeOutTime = 2.0f;
    }
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_MetaSoundManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_MetaSoundManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Biome audio configurations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio", meta = (AllowPrivateAccess = "true"))
    TMap<EAudio_BiomeType, FAudio_BiomeAudioConfig> BiomeAudioConfigs;

    // Current active biome
    UPROPERTY(BlueprintReadOnly, Category = "Audio State", meta = (AllowPrivateAccess = "true"))
    EAudio_BiomeType CurrentBiome;

    // Current threat level
    UPROPERTY(BlueprintReadOnly, Category = "Audio State", meta = (AllowPrivateAccess = "true"))
    EAudio_ThreatLevel CurrentThreatLevel;

    // Audio components for layered soundscape
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components", meta = (AllowPrivateAccess = "true"))
    class UAudioComponent* BiomeAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components", meta = (AllowPrivateAccess = "true"))
    class UAudioComponent* ThreatAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components", meta = (AllowPrivateAccess = "true"))
    class UAudioComponent* NarrativeAudioComponent;

    // Threat audio assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Audio", meta = (AllowPrivateAccess = "true"))
    TMap<EAudio_ThreatLevel, class UMetaSoundSource*> ThreatAudioSources;

    // Narrative voice lines
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio", meta = (AllowPrivateAccess = "true"))
    TArray<class USoundWave*> SurvivalNarrativeLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio", meta = (AllowPrivateAccess = "true"))
    TArray<class USoundWave*> TribalDialogueLines;

    // Audio transition timers
    UPROPERTY(BlueprintReadOnly, Category = "Audio State", meta = (AllowPrivateAccess = "true"))
    float BiomeTransitionTimer;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State", meta = (AllowPrivateAccess = "true"))
    float ThreatTransitionTimer;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State", meta = (AllowPrivateAccess = "true"))
    bool bIsTransitioning;

public:
    // Biome management functions
    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void SetBiome(EAudio_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void SetThreatLevel(EAudio_ThreatLevel NewThreatLevel);

    // Narrative audio functions
    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void PlaySurvivalNarrative(int32 NarrativeIndex);

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void PlayTribalDialogue(int32 DialogueIndex);

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void StopNarrative();

    // Audio state queries
    UFUNCTION(BlueprintPure, Category = "Audio State")
    EAudio_BiomeType GetCurrentBiome() const { return CurrentBiome; }

    UFUNCTION(BlueprintPure, Category = "Audio State")
    EAudio_ThreatLevel GetCurrentThreatLevel() const { return CurrentThreatLevel; }

    UFUNCTION(BlueprintPure, Category = "Audio State")
    bool IsTransitioning() const { return bIsTransitioning; }

    // MetaSound parameter control
    UFUNCTION(BlueprintCallable, Category = "MetaSound Control")
    void SetMetaSoundParameter(const FName& ParameterName, float Value);

    UFUNCTION(BlueprintCallable, Category = "MetaSound Control")
    void SetBiomeIntensity(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "MetaSound Control")
    void SetThreatIntensity(float Intensity);

private:
    void InitializeAudioComponents();
    void UpdateBiomeTransition(float DeltaTime);
    void UpdateThreatTransition(float DeltaTime);
    void ConfigureBiomeAudio();
    void ConfigureThreatAudio();
};