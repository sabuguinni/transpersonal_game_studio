#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "NPCVoiceSystem.generated.h"

UENUM(BlueprintType)
enum class ENPC_VoiceType : uint8
{
    TribalLeader,
    TribalSurvivor,
    TribalElder,
    TribalChild,
    Shaman,
    Hunter,
    Gatherer
};

UENUM(BlueprintType)
enum class ENPC_VoiceContext : uint8
{
    Greeting,
    Warning,
    Combat,
    Fear,
    Calm,
    Trading,
    Storytelling,
    Pain,
    Death
};

USTRUCT(BlueprintType)
struct FNPC_VoiceLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Text;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ENPC_VoiceContext Context;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Priority = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Cooldown = 5.0f;

    FNPC_VoiceLine()
    {
        Text = "";
        AudioURL = "";
        Context = ENPC_VoiceContext::Calm;
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    ENPC_VoiceType VoiceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    TArray<FNPC_VoiceLine> VoiceLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    float VoiceRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    float MinTimeBetweenLines = 3.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Voice")
    float LastVoiceTime;

    UPROPERTY(BlueprintReadOnly, Category = "Voice")
    bool bIsSpeaking;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Voice")
    void PlayVoiceLine(ENPC_VoiceContext Context);

    UFUNCTION(BlueprintCallable, Category = "Voice")
    void PlaySpecificLine(const FString& Text);

    UFUNCTION(BlueprintCallable, Category = "Voice")
    bool CanSpeak() const;

    UFUNCTION(BlueprintCallable, Category = "Voice")
    void AddVoiceLine(const FString& Text, const FString& AudioURL, ENPC_VoiceContext Context, float Priority = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Voice")
    void InitializeDefaultVoiceLines();

private:
    void PlayAudioFromURL(const FString& AudioURL);
    FNPC_VoiceLine* GetBestVoiceLineForContext(ENPC_VoiceContext Context);
    bool IsPlayerInRange() const;
};