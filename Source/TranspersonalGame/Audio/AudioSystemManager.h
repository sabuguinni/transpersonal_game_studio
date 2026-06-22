#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "AudioSystemManager.generated.h"

// ============================================================
// Audio Agent #16 — AudioSystemManager
// Adaptive audio system for prehistoric survival game.
// Controls music tension state, ambient zone transitions,
// and NPC proximity audio triggers.
// ============================================================

UENUM(BlueprintType)
enum class EAudio_TensionState : uint8
{
    Safe        UMETA(DisplayName = "Safe — Camp"),
    Cautious    UMETA(DisplayName = "Cautious — Open World"),
    Danger      UMETA(DisplayName = "Danger — Dino Nearby"),
    Critical    UMETA(DisplayName = "Critical — Under Attack")
};

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    Camp        UMETA(DisplayName = "Camp Zone"),
    Jungle      UMETA(DisplayName = "Jungle Zone"),
    River       UMETA(DisplayName = "River Zone"),
    DinoTerritory UMETA(DisplayName = "Dino Territory"),
    Cave        UMETA(DisplayName = "Cave Zone")
};

USTRUCT(BlueprintType)
struct FAudio_AmbientZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FString ZoneLabel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_ZoneType ZoneType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FVector WorldLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float BlendRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float AmbientVolume;

    FAudio_AmbientZone()
        : ZoneLabel(TEXT("DefaultZone"))
        , ZoneType(EAudio_ZoneType::Jungle)
        , WorldLocation(FVector::ZeroVector)
        , BlendRadius(1500.0f)
        , AmbientVolume(0.8f)
    {}
};

USTRUCT(BlueprintType)
struct FAudio_SoundReference
{
    GENERATED_BODY()

    // Freesound IDs catalogued by Audio Agent #16
    // Campfire: 394952 (crackling campfire, 1805s loop)
    // Dino Roars Pack 1: 586546
    // Dino Roars Pack 2: 586545
    // Dino Growls Pack 2: 586547
    // Croc/Dino Bellow: 811310
    // Berserker Roar (TRex): 837048

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Reference")
    int32 FreesoundID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Reference")
    FString SoundName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Reference")
    FString PreviewURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Reference")
    FString UseCase;

    FAudio_SoundReference()
        : FreesoundID(0)
        , SoundName(TEXT("Unknown"))
        , PreviewURL(TEXT(""))
        , UseCase(TEXT(""))
    {}
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudioSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AAudioSystemManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ---- Tension State ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Tension")
    EAudio_TensionState CurrentTensionState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Tension")
    float TensionBlendSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Tension")
    float DinoProximityRadius;

    // ---- Ambient Zones ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zones")
    TArray<FAudio_AmbientZone> AmbientZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zones")
    EAudio_ZoneType CurrentZone;

    // ---- Sound References (Freesound catalogue) ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Catalogue")
    TArray<FAudio_SoundReference> SoundCatalogue;

    // ---- Audio Components ----

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Components",
        meta = (AllowPrivateAccess = "true"))
    UAudioComponent* CampfireAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Components",
        meta = (AllowPrivateAccess = "true"))
    UAudioComponent* AmbientLoopComponent;

    // ---- Public Functions ----

    UFUNCTION(BlueprintCallable, Category = "Audio|Tension")
    void SetTensionState(EAudio_TensionState NewState);

    UFUNCTION(BlueprintCallable, Category = "Audio|Tension")
    EAudio_TensionState GetTensionState() const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Zones")
    EAudio_ZoneType GetZoneAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Zones")
    void UpdatePlayerZone(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio|Catalogue")
    void InitialiseSoundCatalogue();

    UFUNCTION(BlueprintCallable, Category = "Audio|Tension")
    float GetTensionIntensity() const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Debug")
    FString GetAudioDebugString() const;

private:
    float CurrentTensionIntensity;
    float TargetTensionIntensity;
    float TimeSinceLastDinoDetected;

    void UpdateTensionBlend(float DeltaTime);
    void DetectNearbyDinosaurs();
    void InitialiseDefaultZones();
};
