#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "NPCVoiceSystem.generated.h"

UENUM(BlueprintType)
enum class ENPC_VoiceType : uint8
{
    Greeting,
    Warning,
    Combat,
    Idle,
    Death,
    Discovery
};

USTRUCT(BlueprintType)
struct FNPC_VoiceLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    ENPC_VoiceType VoiceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    FString Text;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    float Cooldown;

    FNPC_VoiceLine()
    {
        VoiceType = ENPC_VoiceType::Idle;
        AudioURL = "";
        Text = "";
        Priority = 1.0f;
        Cooldown = 5.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_VoiceSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_VoiceSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Voice line management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Lines")
    TArray<FNPC_VoiceLine> VoiceLines;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voice System")
    class UAudioComponent* AudioComponent;

    // Voice system settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Settings")
    float VoiceRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Settings")
    float MinTimeBetweenLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Settings")
    bool bCanInterruptLines;

    // Voice triggers
    UFUNCTION(BlueprintCallable, Category = "Voice System")
    void PlayVoiceLine(ENPC_VoiceType VoiceType);

    UFUNCTION(BlueprintCallable, Category = "Voice System")
    void PlayRandomVoiceLine();

    UFUNCTION(BlueprintCallable, Category = "Voice System")
    void StopCurrentVoiceLine();

    UFUNCTION(BlueprintCallable, Category = "Voice System")
    bool IsPlayingVoiceLine() const;

    // Voice line utilities
    UFUNCTION(BlueprintCallable, Category = "Voice System")
    void AddVoiceLine(ENPC_VoiceType VoiceType, const FString& AudioURL, const FString& Text, float Priority = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Voice System")
    TArray<FNPC_VoiceLine> GetVoiceLinesOfType(ENPC_VoiceType VoiceType) const;

    UFUNCTION(BlueprintCallable, Category = "Voice System")
    void SetVoiceRange(float NewRange);

    // Player proximity detection
    UFUNCTION(BlueprintCallable, Category = "Voice System")
    bool IsPlayerInRange() const;

    UFUNCTION(BlueprintCallable, Category = "Voice System")
    float GetDistanceToPlayer() const;

private:
    // Internal state
    float LastVoiceLineTime;
    ENPC_VoiceType CurrentVoiceType;
    bool bIsPlayingLine;

    // Voice line selection
    FNPC_VoiceLine* SelectBestVoiceLine(ENPC_VoiceType VoiceType);
    bool CanPlayVoiceLine(const FNPC_VoiceLine& VoiceLine) const;

    // Audio management
    void LoadAndPlayAudio(const FString& AudioURL);
    void OnAudioFinished();
};