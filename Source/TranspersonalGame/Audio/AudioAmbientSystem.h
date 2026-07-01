#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Sound/SoundBase.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AudioAmbientSystem.generated.h"

// ─── Enums ────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EAudio_AmbientZoneType : uint8
{
    ElderCamp       UMETA(DisplayName = "Elder Camp"),
    RaptorRidge     UMETA(DisplayName = "Raptor Ridge"),
    RiverCrossing   UMETA(DisplayName = "River Crossing"),
    OpenPlains      UMETA(DisplayName = "Open Plains"),
    DenseForest     UMETA(DisplayName = "Dense Forest"),
    CaveEntrance    UMETA(DisplayName = "Cave Entrance"),
    Volcanic        UMETA(DisplayName = "Volcanic"),
};

UENUM(BlueprintType)
enum class EAudio_DangerLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Cautious    UMETA(DisplayName = "Cautious"),
    Danger      UMETA(DisplayName = "Danger"),
    Critical    UMETA(DisplayName = "Critical"),
};

// ─── Structs ──────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FAudio_AmbientLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    USoundBase* AmbientSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float FadeInDuration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float FadeOutDuration = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    bool bLooping = true;

    // Freesound.org asset ID for reference
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    int32 FreesoundAssetID = 0;

    // Supabase TTS URL if this is a voice layer
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    FString TTSAudioURL;
};

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_AmbientZoneType ZoneType = EAudio_AmbientZoneType::OpenPlains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    TArray<FAudio_AmbientLayer> AmbientLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float BlendRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float MaxConcurrentLayers = 3;

    // Danger level modifies music intensity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_DangerLevel CurrentDanger = EAudio_DangerLevel::Safe;
};

// ─── Ambient Zone Actor ────────────────────────────────────────────────────────

UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API AAudio_AmbientZoneActor : public AActor
{
    GENERATED_BODY()

public:
    AAudio_AmbientZoneActor();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaTime) override;

    // Zone configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FAudio_ZoneConfig ZoneConfig;

    // Trigger volume — player enters/exits
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Zone")
    USphereComponent* TriggerSphere;

    // Primary ambient audio component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Zone")
    UAudioComponent* PrimaryAmbientAudio;

    // Secondary ambient audio component (layered blend)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Zone")
    UAudioComponent* SecondaryAmbientAudio;

    // Is player currently inside this zone?
    UPROPERTY(BlueprintReadOnly, Category = "Audio|Zone")
    bool bPlayerInZone = false;

    // Current blend weight (0=silent, 1=full volume)
    UPROPERTY(BlueprintReadOnly, Category = "Audio|Zone")
    float CurrentBlendWeight = 0.0f;

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void SetDangerLevel(EAudio_DangerLevel NewDanger);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void FadeIn(float Duration = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void FadeOut(float Duration = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    EAudio_AmbientZoneType GetZoneType() const { return ZoneConfig.ZoneType; }

protected:
    UFUNCTION()
    void OnPlayerEnterZone(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnPlayerExitZone(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    bool bFadingIn = false;
    bool bFadingOut = false;
    float FadeTargetDuration = 2.0f;
    float FadeElapsed = 0.0f;
};

// ─── Audio Manager Subsystem ──────────────────────────────────────────────────

UCLASS()
class TRANSPERSONALGAME_API UAudio_AmbientManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Register an ambient zone with the manager
    UFUNCTION(BlueprintCallable, Category = "Audio|Manager")
    void RegisterAmbientZone(AAudio_AmbientZoneActor* Zone);

    UFUNCTION(BlueprintCallable, Category = "Audio|Manager")
    void UnregisterAmbientZone(AAudio_AmbientZoneActor* Zone);

    // Broadcast danger level change to all zones
    UFUNCTION(BlueprintCallable, Category = "Audio|Manager")
    void BroadcastDangerLevel(EAudio_DangerLevel NewDanger);

    // Get all active zones player is currently inside
    UFUNCTION(BlueprintCallable, Category = "Audio|Manager")
    TArray<AAudio_AmbientZoneActor*> GetActiveZones() const;

    // TTS audio URL registry — maps character name to Supabase URL
    UPROPERTY(BlueprintReadOnly, Category = "Audio|Manager")
    TMap<FString, FString> TTSAudioRegistry;

    UFUNCTION(BlueprintCallable, Category = "Audio|Manager")
    void RegisterTTSAudio(const FString& CharacterName, const FString& AudioURL);

    UFUNCTION(BlueprintCallable, Category = "Audio|Manager")
    FString GetTTSAudioURL(const FString& CharacterName) const;

private:
    UPROPERTY()
    TArray<AAudio_AmbientZoneActor*> RegisteredZones;
};
