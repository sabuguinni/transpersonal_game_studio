#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/TriggerVolume.h"
#include "Narr_SurvivalNarrator.generated.h"

UENUM(BlueprintType)
enum class ENarr_NarrationType : uint8
{
    Environmental,
    Danger,
    Discovery,
    Survival,
    Predator
};

USTRUCT(BlueprintType)
struct FNarr_NarrativeEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString EventID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FText NarrativeText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_NarrationType NarrationType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bIsOneShot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float Priority;

    FNarr_NarrativeEvent()
    {
        EventID = TEXT("");
        NarrativeText = FText::GetEmpty();
        NarrationType = ENarr_NarrationType::Environmental;
        TriggerRadius = 500.0f;
        bIsOneShot = true;
        Priority = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_SurvivalNarrator : public AActor
{
    GENERATED_BODY()

public:
    ANarr_SurvivalNarrator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* NarrativeAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* TriggerSphere;

    // Narrative Events
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_NarrativeEvent> NarrativeEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float PlayerDetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float NarrativeCooldown;

    // Audio Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundCue* DefaultNarrativeCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float VolumeMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float PitchMultiplier;

    // State Management
    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsNarrativePlaying;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float LastNarrativeTime;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    TArray<FString> TriggeredEvents;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerNarrative(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayNarrativeEvent(const FNarr_NarrativeEvent& Event);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool CanTriggerNarrative() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StopCurrentNarrative();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_NarrativeEvent GetNarrativeEventByID(const FString& EventID) const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Narrative")
    void OnNarrativeTriggered(const FNarr_NarrativeEvent& Event);

    UFUNCTION(BlueprintImplementableEvent, Category = "Narrative")
    void OnNarrativeCompleted(const FNarr_NarrativeEvent& Event);

protected:
    UFUNCTION()
    void OnTriggerOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnTriggerOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    void CheckPlayerProximity();
    void ProcessNarrativeQueue();
    
    TArray<FNarr_NarrativeEvent> PendingNarratives;
    class APawn* CachedPlayerPawn;
};