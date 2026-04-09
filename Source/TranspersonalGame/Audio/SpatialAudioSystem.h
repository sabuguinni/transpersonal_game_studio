#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundAttenuation.h"
#include "AudioTypes.h"
#include "Engine/World.h"
#include "SpatialAudioSystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialAudio, Log, All);

/**
 * Spatial Audio System
 * 
 * Manages 3D spatial audio positioning, attenuation, and environmental effects.
 * Creates immersive soundscapes where every sound has meaning and spatial context.
 * 
 * Philosophy: Sound exists in space, not in speakers. The world speaks through position.
 */

UENUM(BlueprintType)
enum class ESpatialAudioZone : uint8
{
    Open           UMETA(DisplayName = "Open Space"),
    Dense          UMETA(DisplayName = "Dense Forest"),
    Enclosed       UMETA(DisplayName = "Enclosed Space"),
    Underwater     UMETA(DisplayName = "Underwater"),
    Cave           UMETA(DisplayName = "Cave System"),
    Canyon         UMETA(DisplayName = "Canyon"),
    Clearing       UMETA(DisplayName = "Forest Clearing")
};

UENUM(BlueprintType)
enum class EAudioOcclusionType : uint8
{
    None           UMETA(DisplayName = "No Occlusion"),
    Partial        UMETA(DisplayName = "Partial Occlusion"),
    Heavy          UMETA(DisplayName = "Heavy Occlusion"),
    Complete       UMETA(DisplayName = "Complete Occlusion"),
    Underwater     UMETA(DisplayName = "Underwater Occlusion"),
    Muffled        UMETA(DisplayName = "Muffled")
};

USTRUCT(BlueprintType)
struct FSpatialAudioSource
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    class UAudioComponent* AudioComponent = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    FVector WorldPosition = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    float MaxAudibleDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    float VolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    bool bUseDistanceAttenuation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    bool bUseOcclusion = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    bool bUseReverb = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    EAudioOcclusionType OcclusionType = EAudioOcclusionType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    float OcclusionStrength = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    int32 SourceID = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    bool bIsActive = true;
};

USTRUCT(BlueprintType)
struct FSpatialAudioListener
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    FVector Position = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    FVector Forward = FVector::ForwardVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    FVector Up = FVector::UpVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    ESpatialAudioZone CurrentZone = ESpatialAudioZone::Open;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    float EnvironmentalReverb = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    float AmbientOcclusion = 0.0f;
};

USTRUCT(BlueprintType)
struct FSpatialAudioConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Config")
    float GlobalVolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Config")
    float MaxProcessingDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Config")
    int32 MaxActiveSources = 64;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Config")
    bool bUseAdvancedOcclusion = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Config")
    bool bUseDopplerEffect = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Config")
    bool bUseEnvironmentalReverb = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Config")
    float OcclusionUpdateRate = 10.0f; // Hz

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Config")
    float ReverbUpdateRate = 5.0f; // Hz

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Config")
    class USoundAttenuation* DefaultAttenuation = nullptr;
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API USpatialAudioSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    USpatialAudioSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    // System Initialization
    UFUNCTION(BlueprintCallable, Category = "Spatial Audio")
    void InitializeSpatialAudioSystem(const FSpatialAudioConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Spatial Audio")
    void SetListener(const FSpatialAudioListener& Listener);

    UFUNCTION(BlueprintCallable, Category = "Spatial Audio")
    void UpdateListenerPosition(FVector Position, FVector Forward, FVector Up, FVector Velocity);

    // Source Management
    UFUNCTION(BlueprintCallable, Category = "Spatial Audio Sources")
    int32 RegisterSpatialAudioSource(UAudioComponent* AudioComponent, FVector Position, float MaxDistance = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Spatial Audio Sources")
    bool UnregisterSpatialAudioSource(int32 SourceID);

    UFUNCTION(BlueprintCallable, Category = "Spatial Audio Sources")
    void UpdateSourcePosition(int32 SourceID, FVector NewPosition);

    UFUNCTION(BlueprintCallable, Category = "Spatial Audio Sources")
    void SetSourceVolume(int32 SourceID, float Volume);

    UFUNCTION(BlueprintCallable, Category = "Spatial Audio Sources")
    void SetSourceMaxDistance(int32 SourceID, float MaxDistance);

    UFUNCTION(BlueprintCallable, Category = "Spatial Audio Sources")
    void EnableSourceOcclusion(int32 SourceID, bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Spatial Audio Sources")
    void EnableSourceReverb(int32 SourceID, bool bEnable);

    // Environmental Audio
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void SetSpatialAudioZone(ESpatialAudioZone Zone);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void SetEnvironmentalReverb(float ReverbAmount, float TransitionTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void SetAmbientOcclusion(float OcclusionAmount);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void SetWeatherAudioEffects(EWeatherState WeatherState, float Intensity);

    // Occlusion System
    UFUNCTION(BlueprintCallable, Category = "Audio Occlusion")
    void UpdateOcclusionForSource(int32 SourceID);

    UFUNCTION(BlueprintCallable, Category = "Audio Occlusion")
    void SetSourceOcclusion(int32 SourceID, EAudioOcclusionType OcclusionType, float Strength);

    UFUNCTION(BlueprintCallable, Category = "Audio Occlusion")
    void UpdateAllSourceOcclusion();

    // Distance and Attenuation
    UFUNCTION(BlueprintCallable, Category = "Audio Distance")
    float CalculateDistanceAttenuation(float Distance, float MaxDistance);

    UFUNCTION(BlueprintCallable, Category = "Audio Distance")
    void UpdateSourceAttenuation(int32 SourceID);

    UFUNCTION(BlueprintCallable, Category = "Audio Distance")
    void SetGlobalVolumeMultiplier(float Multiplier);

    // Advanced Features
    UFUNCTION(BlueprintCallable, Category = "Advanced Spatial Audio")
    void EnableDopplerEffect(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Advanced Spatial Audio")
    void SetDopplerScale(float DopplerScale);

    UFUNCTION(BlueprintCallable, Category = "Advanced Spatial Audio")
    void UpdateSourceDoppler(int32 SourceID, FVector SourceVelocity);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Spatial Audio Utility")
    float GetDistanceToListener(FVector Position);

    UFUNCTION(BlueprintCallable, Category = "Spatial Audio Utility")
    bool IsSourceAudible(int32 SourceID);

    UFUNCTION(BlueprintCallable, Category = "Spatial Audio Utility")
    TArray<int32> GetAudibleSources();

    UFUNCTION(BlueprintCallable, Category = "Spatial Audio Utility")
    int32 GetActiveSourceCount();

    // Debug Functions
    UFUNCTION(BlueprintCallable, Category = "Spatial Audio Debug")
    void DebugDrawSpatialAudio(bool bDrawSources = true, bool bDrawAttenuation = true, bool bDrawOcclusion = true);

    UFUNCTION(BlueprintCallable, Category = "Spatial Audio Debug")
    void PrintSpatialAudioStats();

protected:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spatial Audio Config")
    FSpatialAudioConfig AudioConfig;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spatial Audio Config")
    class USoundAttenuation* DefaultAttenuationSettings = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spatial Audio Config")
    TMap<ESpatialAudioZone, float> ZoneReverbMultipliers;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spatial Audio Config")
    TMap<EAudioOcclusionType, float> OcclusionStrengthMap;

    // Runtime State
    UPROPERTY()
    TMap<int32, FSpatialAudioSource> RegisteredSources;

    UPROPERTY()
    FSpatialAudioListener CurrentListener;

    UPROPERTY()
    int32 NextSourceID = 1;

    UPROPERTY()
    float LastOcclusionUpdateTime = 0.0f;

    UPROPERTY()
    float LastReverbUpdateTime = 0.0f;

    UPROPERTY()
    bool bSystemInitialized = false;

    // Doppler Effect
    UPROPERTY()
    bool bDopplerEnabled = true;

    UPROPERTY()
    float DopplerScale = 1.0f;

    // Environmental State
    UPROPERTY()
    ESpatialAudioZone CurrentZone = ESpatialAudioZone::Open;

    UPROPERTY()
    float CurrentEnvironmentalReverb = 0.3f;

    UPROPERTY()
    float CurrentAmbientOcclusion = 0.0f;

private:
    // Internal Methods
    void UpdateSourceSpatialParameters(int32 SourceID, float DeltaTime);
    void ProcessOcclusionForSource(FSpatialAudioSource& Source);
    void ProcessReverbForSource(FSpatialAudioSource& Source);
    void ProcessDistanceAttenuation(FSpatialAudioSource& Source);
    void CleanupInactiveSources();
    bool PerformOcclusionTrace(const FVector& Start, const FVector& End, float& OutOcclusionStrength);
    float CalculateReverbForPosition(const FVector& Position);
    void ApplyEnvironmentalEffects(FSpatialAudioSource& Source);
    void UpdateSourcePriority(FSpatialAudioSource& Source);
};