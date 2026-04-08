#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "AudioSystemManager.generated.h"

UENUM(BlueprintType)
enum class EAudioEnvironmentType : uint8
{
    DenseForest     UMETA(DisplayName = "Dense Forest"),
    OpenPlains      UMETA(DisplayName = "Open Plains"),
    Wetlands        UMETA(DisplayName = "Wetlands"),
    RockyOutcrops   UMETA(DisplayName = "Rocky Outcrops"),
    Caves           UMETA(DisplayName = "Caves"),
    PlayerBase      UMETA(DisplayName = "Player Base")
};

UENUM(BlueprintType)
enum class EThreatLevel : uint8
{
    Safe            UMETA(DisplayName = "Safe"),
    Cautious        UMETA(DisplayName = "Cautious"),
    Tense           UMETA(DisplayName = "Tense"),
    Danger          UMETA(DisplayName = "Danger"),
    Terror          UMETA(DisplayName = "Terror")
};

UENUM(BlueprintType)
enum class ETimeOfDay : uint8
{
    Dawn            UMETA(DisplayName = "Dawn"),
    Morning         UMETA(DisplayName = "Morning"),
    Midday          UMETA(DisplayName = "Midday"),
    Afternoon       UMETA(DisplayName = "Afternoon"),
    Dusk            UMETA(DisplayName = "Dusk"),
    Night           UMETA(DisplayName = "Night"),
    DeepNight       UMETA(DisplayName = "Deep Night")
};

USTRUCT(BlueprintType)
struct FAudioStateData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAudioEnvironmentType EnvironmentType = EAudioEnvironmentType::DenseForest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EThreatLevel ThreatLevel = EThreatLevel::Safe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETimeOfDay TimeOfDay = ETimeOfDay::Morning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PlayerStealthLevel = 0.0f; // 0.0 = exposed, 1.0 = completely hidden

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WeatherIntensity = 0.0f; // 0.0 = clear, 1.0 = storm

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsInPlayerBase = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 NearbyDinosaurCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DistanceToNearestPredator = 10000.0f; // In Unreal Units
};

/**
 * Central manager for all audio systems in the game
 * Handles adaptive music, environmental audio, and dynamic sound mixing
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudioSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudioSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Current audio state
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    FAudioStateData CurrentAudioState;

    // MetaSound assets for adaptive music
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    class UMetaSoundSource* AdaptiveMusicMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    class UMetaSoundSource* AmbientEnvironmentMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    class UMetaSoundSource* ThreatMusicMetaSound;

    // Audio components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* MusicAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* AmbientAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* ThreatAudioComponent;

    // State transition parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Parameters")
    float MusicTransitionTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Parameters")
    float ThreatDetectionRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Parameters")
    float StealthAudioReduction = 0.7f;

    // Public functions
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateAudioState(const FAudioStateData& NewState);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetThreatLevel(EThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetEnvironmentType(EAudioEnvironmentType NewEnvironment);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void OnDinosaurSpotted(class ADinosaurCharacter* Dinosaur, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void OnPlayerEnterStealth();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void OnPlayerExitStealth();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayDinosaurVocalization(class ADinosaurCharacter* Dinosaur, const FString& VocalizationType);

private:
    // Internal state management
    void UpdateMusicParameters();
    void UpdateAmbientParameters();
    void UpdateThreatParameters();
    void DetectNearbyThreats();
    void CalculateStealthLevel();

    // Audio parameter names (for MetaSound communication)
    static const FName PARAM_ThreatLevel;
    static const FName PARAM_EnvironmentType;
    static const FName PARAM_TimeOfDay;
    static const FName PARAM_StealthLevel;
    static const FName PARAM_WeatherIntensity;
    static const FName PARAM_DinosaurProximity;

    // Internal timers
    float LastThreatUpdate = 0.0f;
    float ThreatUpdateInterval = 0.5f;
    
    // Previous state for smooth transitions
    FAudioStateData PreviousAudioState;
    bool bIsTransitioning = false;
    float TransitionStartTime = 0.0f;
};