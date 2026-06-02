#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Engine/Engine.h"
#include "Audio_TribalNPCIntegration.generated.h"

UENUM(BlueprintType)
enum class EAudio_TribalRole : uint8
{
    Elder       UMETA(DisplayName = "Elder"),
    Hunter      UMETA(DisplayName = "Hunter"), 
    Scout       UMETA(DisplayName = "Scout"),
    Crafter     UMETA(DisplayName = "Crafter"),
    Gatherer    UMETA(DisplayName = "Gatherer")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_TribalDialogueSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> GreetingSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> WarningSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> FarewellSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> TradingSound;

    FAudio_TribalDialogueSet()
    {
        GreetingSound = nullptr;
        WarningSound = nullptr;
        FarewellSound = nullptr;
        TradingSound = nullptr;
    }
};

/**
 * Audio integration component for Tribal NPCs
 * Manages dialogue audio playback, ambient tribal sounds, and proximity-based audio triggers
 */
UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_TribalNPCIntegration : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_TribalNPCIntegration();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Tribal")
    EAudio_TribalRole TribalRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Tribal")
    FAudio_TribalDialogueSet DialogueSet;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Components")
    TObjectPtr<UAudioComponent> DialogueAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Components")
    TObjectPtr<UAudioComponent> AmbientAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Settings")
    float DialogueVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Settings")
    float AmbientVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Settings")
    float InteractionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Settings")
    float DialogueCooldown;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|State")
    float LastDialogueTime;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|State")
    bool bIsPlayingDialogue;

public:
    UFUNCTION(BlueprintCallable, Category = "Audio|Tribal")
    void PlayGreeting();

    UFUNCTION(BlueprintCallable, Category = "Audio|Tribal")
    void PlayWarning();

    UFUNCTION(BlueprintCallable, Category = "Audio|Tribal")
    void PlayFarewell();

    UFUNCTION(BlueprintCallable, Category = "Audio|Tribal")
    void PlayTradingDialogue();

    UFUNCTION(BlueprintCallable, Category = "Audio|Tribal")
    void StopCurrentDialogue();

    UFUNCTION(BlueprintCallable, Category = "Audio|Tribal")
    bool CanPlayDialogue() const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Tribal")
    void SetTribalRole(EAudio_TribalRole NewRole);

    UFUNCTION(BlueprintCallable, Category = "Audio|Tribal")
    void InitializeAudioComponents();

protected:
    UFUNCTION()
    void OnDialogueFinished();

    void PlayDialogueSound(TSoftObjectPtr<USoundBase> Sound);
    void LoadRoleSpecificAudio();
};