#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "Sound/SoundBase.h"
#include "AudioZoneSystem.generated.h"

// ============================================================
// ENUMS — Audio_* prefix to avoid collision with other agents
// ============================================================

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    OpenPlains      UMETA(DisplayName = "Open Plains"),
    DenseForest     UMETA(DisplayName = "Dense Forest"),
    RiverBank       UMETA(DisplayName = "River Bank"),
    RaptorNest      UMETA(DisplayName = "Raptor Nest"),
    CampfireSite    UMETA(DisplayName = "Campfire Site"),
    FlintRidge      UMETA(DisplayName = "Flint Ridge"),
    HerdObserve     UMETA(DisplayName = "Herd Observation"),
    Cave            UMETA(DisplayName = "Cave"),
    Volcano         UMETA(DisplayName = "Volcano")
};

UENUM(BlueprintType)
enum class EAudio_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Day         UMETA(DisplayName = "Day"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

UENUM(BlueprintType)
enum class EAudio_DinoSoundType : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Alert       UMETA(DisplayName = "Alert"),
    Roar        UMETA(DisplayName = "Roar"),
    Footstep    UMETA(DisplayName = "Footstep"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Death       UMETA(DisplayName = "Death")
};

// ============================================================
// STRUCTS
// ============================================================

USTRUCT(BlueprintType)
struct FAudio_SoundEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString SoundName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString FreesoundID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString PreviewURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Duration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bLoop = false;
};

USTRUCT(BlueprintType)
struct FAudio_ZoneAmbience
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::OpenPlains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TArray<FAudio_SoundEntry> DaySounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TArray<FAudio_SoundEntry> NightSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float BlendRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MasterVolume = 1.0f;
};

USTRUCT(BlueprintType)
struct FAudio_DinoSoundSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString SpeciesName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TMap<EAudio_DinoSoundType, FAudio_SoundEntry> SoundMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float RoarTriggerRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FootstepInterval = 0.8f;
};

USTRUCT(BlueprintType)
struct FAudio_NarratorLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString LineID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString TriggerCondition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float CooldownSeconds = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bHasPlayed = false;
};

// ============================================================
// UAudio_ZoneComponent — attach to any actor to define audio zone
// ============================================================

UCLASS(ClassGroup = "Audio", meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_ZoneComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_ZoneComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    FAudio_ZoneAmbience ZoneAmbience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float FadeOutTime = 3.0f;

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void OnPlayerEnterZone(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void OnPlayerExitZone(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void SetTimeOfDay(EAudio_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    EAudio_ZoneType GetZoneType() const { return ZoneAmbience.ZoneType; }

protected:
    virtual void BeginPlay() override;

private:
    EAudio_TimeOfDay CurrentTimeOfDay = EAudio_TimeOfDay::Day;
    bool bPlayerInZone = false;
};

// ============================================================
// AAudio_ZoneActor — world actor with sphere trigger + audio zone
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_ZoneActor : public AActor
{
    GENERATED_BODY()

public:
    AAudio_ZoneActor();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    USphereComponent* TriggerSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UAudio_ZoneComponent* AudioZone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::OpenPlains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float ZoneRadius = 2000.0f;

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void UpdateZoneRadius(float NewRadius);

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};

// ============================================================
// UAudio_DinoSoundManager — manages all dinosaur audio
// ============================================================

UCLASS(ClassGroup = "Audio", meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_DinoSoundManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_DinoSoundManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino Audio")
    TArray<FAudio_DinoSoundSet> DinoSoundSets;

    UFUNCTION(BlueprintCallable, Category = "Dino Audio")
    void PlayDinoSound(const FString& SpeciesName, EAudio_DinoSoundType SoundType, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Dino Audio")
    void TriggerTRexRoar(FVector TRexLocation, bool bHerdStampeding);

    UFUNCTION(BlueprintCallable, Category = "Dino Audio")
    void TriggerRaptorCall(FVector RaptorLocation, bool bNearNest);

    UFUNCTION(BlueprintCallable, Category = "Dino Audio")
    void PlayFootstep(const FString& SpeciesName, FVector Location, float Weight);

    UFUNCTION(BlueprintCallable, Category = "Dino Audio")
    void InitialiseDefaultSoundSets();

protected:
    virtual void BeginPlay() override;

private:
    TMap<FString, FAudio_DinoSoundSet> SoundSetCache;
};

// ============================================================
// UAudio_DayNightMixManager — shifts ambient mix with time of day
// ============================================================

UCLASS(ClassGroup = "Audio", meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_DayNightMixManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_DayNightMixManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Audio")
    float DawnStartHour = 5.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Audio")
    float DayStartHour = 7.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Audio")
    float DuskStartHour = 18.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Audio")
    float NightStartHour = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Audio")
    float MixTransitionTime = 8.0f;

    UFUNCTION(BlueprintCallable, Category = "Day Night Audio")
    void UpdateTimeOfDay(float CurrentHour);

    UFUNCTION(BlueprintCallable, Category = "Day Night Audio")
    EAudio_TimeOfDay GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintCallable, Category = "Day Night Audio")
    void BroadcastTimeChange(EAudio_TimeOfDay NewTime);

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    EAudio_TimeOfDay CurrentTimeOfDay = EAudio_TimeOfDay::Day;
    float LastBroadcastHour = -1.0f;
};

// ============================================================
// UAudio_NarratorSystem — manages narrator voice line playback
// ============================================================

UCLASS(ClassGroup = "Audio", meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_NarratorSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_NarratorSystem();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrator")
    TArray<FAudio_NarratorLine> NarratorLines;

    UFUNCTION(BlueprintCallable, Category = "Narrator")
    void TryPlayLine(const FString& TriggerCondition);

    UFUNCTION(BlueprintCallable, Category = "Narrator")
    void RegisterLine(const FString& LineID, const FString& AudioURL,
        const FString& TriggerCondition, float Cooldown);

    UFUNCTION(BlueprintCallable, Category = "Narrator")
    void InitialiseDefaultLines();

    UFUNCTION(BlueprintCallable, Category = "Narrator")
    bool HasLinePlayed(const FString& LineID) const;

protected:
    virtual void BeginPlay() override;

private:
    TMap<FString, float> LastPlayedTimestamps;
};
