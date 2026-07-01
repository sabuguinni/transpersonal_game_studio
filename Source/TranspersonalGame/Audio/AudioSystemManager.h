#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "AudioSystemManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_EnvironmentZone : uint8
{
    OpenPlains       UMETA(DisplayName = "Open Plains"),
    DenseForest      UMETA(DisplayName = "Dense Forest"),
    RiverBank        UMETA(DisplayName = "River Bank"),
    CaveEntrance     UMETA(DisplayName = "Cave Entrance"),
    VolcanicRegion   UMETA(DisplayName = "Volcanic Region"),
    NightCamp        UMETA(DisplayName = "Night Camp")
};

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Cautious    UMETA(DisplayName = "Cautious"),
    Danger      UMETA(DisplayName = "Danger"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct FAudio_DinosaurSoundProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    FName DinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    USoundCue* RoarSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    USoundCue* FootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    USoundCue* BreathingSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float RoarRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float FootstepGroundShakeRadius;

    FAudio_DinosaurSoundProfile()
        : DinosaurSpecies(NAME_None)
        , RoarSound(nullptr)
        , FootstepSound(nullptr)
        , BreathingSound(nullptr)
        , RoarRadius(3000.f)
        , FootstepGroundShakeRadius(1500.f)
    {}
};

USTRUCT(BlueprintType)
struct FAudio_AmbientLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_EnvironmentZone Zone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    USoundCue* DayAmbience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    USoundCue* NightAmbience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float BaseVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float CrossfadeDuration;

    FAudio_AmbientLayer()
        : Zone(EAudio_EnvironmentZone::OpenPlains)
        , DayAmbience(nullptr)
        , NightAmbience(nullptr)
        , BaseVolume(1.0f)
        , CrossfadeDuration(3.0f)
    {}
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_AdaptiveMusicComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_AdaptiveMusicComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    EAudio_ThreatLevel CurrentThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    float MusicTransitionSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    float TRexProximityRadius;

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void SetThreatLevel(EAudio_ThreatLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void OnDinosaurNearby(FName Species, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void OnPlayerEnteredSafeZone();

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    EAudio_ThreatLevel GetCurrentThreatLevel() const { return CurrentThreatLevel; }

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    float ThreatDecayTimer;
    float CurrentMusicIntensity;
};

UCLASS()
class TRANSPERSONALGAME_API AAudio_SystemManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_SystemManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    TArray<FAudio_AmbientLayer> AmbientLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    TArray<FAudio_DinosaurSoundProfile> DinosaurProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Environment")
    EAudio_EnvironmentZone ActiveZone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Environment")
    bool bIsNightTime;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetEnvironmentZone(EAudio_EnvironmentZone NewZone);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetDayNightState(bool bNight);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void TriggerDinosaurRoar(FName Species, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void TriggerFootstepImpact(FName Species, FVector Location, float Weight);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void TriggerCraftingSound(FName ToolType);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    FAudio_DinosaurSoundProfile GetDinosaurProfile(FName Species) const;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void ApplyScreenShakeFromDinosaur(FName Species, float Distance);

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    UPROPERTY()
    UAudioComponent* ActiveAmbientComponent;

    void CrossfadeToNewAmbient(USoundCue* NewAmbient, float Duration);
    void UpdateDinosaurAwareness(float DeltaTime);
};
