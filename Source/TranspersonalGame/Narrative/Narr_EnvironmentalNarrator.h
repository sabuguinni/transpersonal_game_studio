#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "Narr_EnvironmentalNarrator.generated.h"

UENUM(BlueprintType)
enum class ENarr_EnvironmentType : uint8
{
    Forest        UMETA(DisplayName = "Forest"),
    River         UMETA(DisplayName = "River"),
    Cave          UMETA(DisplayName = "Cave"),
    Plains        UMETA(DisplayName = "Plains"),
    Mountain      UMETA(DisplayName = "Mountain"),
    Swamp         UMETA(DisplayName = "Swamp"),
    Desert        UMETA(DisplayName = "Desert"),
    Coastline     UMETA(DisplayName = "Coastline")
};

USTRUCT(BlueprintType)
struct FNarr_EnvironmentalCue
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental")
    ENarr_EnvironmentType EnvironmentType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental")
    FString NarrativeText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental")
    TSoftObjectPtr<USoundCue> AudioCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental")
    float TriggerChance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental")
    float MinTimeBetweenTriggers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental")
    bool bRequiresPlayerPresence;

    FNarr_EnvironmentalCue()
    {
        EnvironmentType = ENarr_EnvironmentType::Forest;
        NarrativeText = TEXT("The wilderness whispers its ancient secrets...");
        TriggerChance = 0.3f;
        MinTimeBetweenTriggers = 60.0f;
        bRequiresPlayerPresence = true;
    }
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API ANarr_EnvironmentalNarrator : public AActor
{
    GENERATED_BODY()

public:
    ANarr_EnvironmentalNarrator();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* AudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Narration")
    TArray<FNarr_EnvironmentalCue> EnvironmentalCues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float CheckInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float PlayerDetectionRadius;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float LastNarrationTime;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsNarrating;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    ENarr_EnvironmentType CurrentEnvironment;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Environmental Narration")
    void TriggerEnvironmentalNarration(ENarr_EnvironmentType EnvironmentType);

    UFUNCTION(BlueprintCallable, Category = "Environmental Narration")
    void StopCurrentNarration();

    UFUNCTION(BlueprintCallable, Category = "Environmental Narration")
    bool CanTriggerNarration(const FNarr_EnvironmentalCue& Cue) const;

    UFUNCTION(BlueprintCallable, Category = "Environmental Narration")
    ENarr_EnvironmentType DetectCurrentEnvironment() const;

    UFUNCTION(BlueprintCallable, Category = "Environmental Narration")
    bool IsPlayerNearby() const;

    UFUNCTION(BlueprintCallable, Category = "Environmental Narration")
    FNarr_EnvironmentalCue GetCueForEnvironment(ENarr_EnvironmentType EnvironmentType) const;

protected:
    UFUNCTION()
    void OnNarrationFinished();

    UFUNCTION(BlueprintImplementableEvent, Category = "Environmental Narration")
    void OnEnvironmentalNarrationTriggered(const FNarr_EnvironmentalCue& Cue);

    UFUNCTION(BlueprintImplementableEvent, Category = "Environmental Narration")
    void OnEnvironmentalNarrationCompleted(const FNarr_EnvironmentalCue& Cue);

private:
    FTimerHandle CheckTimer;
    void CheckForNarrationTrigger();
};