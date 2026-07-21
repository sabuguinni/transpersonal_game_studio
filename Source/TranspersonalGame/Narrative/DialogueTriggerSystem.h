// DialogueTriggerSystem.h
// Agent #15 — Narrative & Dialogue Agent
// PROD_CYCLE_AUTO_20260619_007
// Triggers voice lines based on proximity, survival stats, and dino presence

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include "DialogueTriggerSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_TriggerType : uint8
{
    ProximityEnter      UMETA(DisplayName = "Proximity Enter"),
    SurvivalStatLow     UMETA(DisplayName = "Survival Stat Low"),
    DinoProximity       UMETA(DisplayName = "Dino Proximity"),
    QuestObjective      UMETA(DisplayName = "Quest Objective"),
    BiomeEnter          UMETA(DisplayName = "Biome Enter")
};

UENUM(BlueprintType)
enum class ENarr_SurvivalStat : uint8
{
    Health      UMETA(DisplayName = "Health"),
    Hunger      UMETA(DisplayName = "Hunger"),
    Thirst      UMETA(DisplayName = "Thirst"),
    Stamina     UMETA(DisplayName = "Stamina"),
    Fear        UMETA(DisplayName = "Fear")
};

USTRUCT(BlueprintType)
struct FNarr_VoiceLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    USoundBase* AudioAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float CooldownSeconds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bPlayOnce;

    FNarr_VoiceLine()
        : CharacterName(TEXT("Unknown"))
        , DialogueText(TEXT(""))
        , AudioURL(TEXT(""))
        , AudioAsset(nullptr)
        , CooldownSeconds(120.0f)
        , bPlayOnce(false)
    {}
};

UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ANarr_DialogueTrigger : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueTrigger();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // Trigger configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Trigger")
    ENarr_TriggerType TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Trigger")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Trigger")
    ENarr_SurvivalStat StatToMonitor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Trigger")
    float StatThreshold;

    // Voice line data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FNarr_VoiceLine VoiceLine;

    // State
    UPROPERTY(BlueprintReadOnly, Category = "Narrative|State", meta = (AllowPrivateAccess = "true"))
    bool bHasTriggered;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|State", meta = (AllowPrivateAccess = "true"))
    float LastTriggerTime;

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UBoxComponent* TriggerVolume;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UAudioComponent* AudioComponent;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TryTriggerDialogue(AActor* TriggeringActor);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsCooledDown() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayVoiceLine();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void ResetTrigger();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FString GetCharacterName() const { return VoiceLine.CharacterName; }

private:
    UFUNCTION()
    void OnTriggerVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor, UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    float CheckInterval;
    float TimeSinceLastCheck;
};
