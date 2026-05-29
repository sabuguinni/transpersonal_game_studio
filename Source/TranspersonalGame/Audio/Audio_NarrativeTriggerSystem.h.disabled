#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/TriggerVolume.h"
#include "Audio_NarrativeTriggerSystem.generated.h"

UENUM(BlueprintType)
enum class EAudio_NarrativeType : uint8
{
    Atmospheric     UMETA(DisplayName = "Atmospheric"),
    Warning         UMETA(DisplayName = "Warning"),
    Discovery       UMETA(DisplayName = "Discovery"),
    Survival        UMETA(DisplayName = "Survival"),
    Danger          UMETA(DisplayName = "Danger")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_NarrativeEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString EventID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    EAudio_NarrativeType NarrativeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TSoftObjectPtr<USoundBase> AudioClip;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString NarrativeText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    int32 Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bOneShot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float CooldownTime;

    FAudio_NarrativeEvent()
    {
        EventID = TEXT("");
        NarrativeType = EAudio_NarrativeType::Atmospheric;
        NarrativeText = TEXT("");
        TriggerRadius = 500.0f;
        Priority = 1;
        bOneShot = false;
        CooldownTime = 10.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_NarrativeTriggerSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_NarrativeTriggerSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Sistema de eventos narrativos
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Events")
    TArray<FAudio_NarrativeEvent> NarrativeEvents;

    // Componente de áudio para reprodução
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    class UAudioComponent* AudioComponent;

    // Trigger sphere para detecção de proximidade
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trigger")
    class USphereComponent* TriggerSphere;

    // Configurações do sistema
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float BaseVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float FadeInTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float FadeOutTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    bool bUseSpatialAudio;

    // Funções públicas
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerNarrativeEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StopCurrentNarrative();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AddNarrativeEvent(const FAudio_NarrativeEvent& NewEvent);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsEventOnCooldown(const FString& EventID) const;

protected:
    // Eventos de trigger
    UFUNCTION()
    void OnTriggerEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnTriggerExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

    // Sistema interno
    void ProcessNarrativeQueue();
    void PlayNarrativeEvent(const FAudio_NarrativeEvent& Event);
    void UpdateCooldowns(float DeltaTime);

private:
    // Estado interno
    TMap<FString, float> EventCooldowns;
    TArray<FAudio_NarrativeEvent> EventQueue;
    FAudio_NarrativeEvent* CurrentEvent;
    bool bPlayerInRange;
    float CurrentFadeTime;
    bool bIsFading;
};