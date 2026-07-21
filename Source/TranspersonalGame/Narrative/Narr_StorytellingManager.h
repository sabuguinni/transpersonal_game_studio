#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/TriggerVolume.h"
#include "Narr_StorytellingManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_StoryType : uint8
{
    CampGathering     UMETA(DisplayName = "Camp Gathering"),
    DangerousCrossing UMETA(DisplayName = "Dangerous Crossing"),
    AncientShelter    UMETA(DisplayName = "Ancient Shelter"),
    PredatorTerritory UMETA(DisplayName = "Predator Territory"),
    ResourceDiscovery UMETA(DisplayName = "Resource Discovery"),
    WeatherWarning    UMETA(DisplayName = "Weather Warning")
};

USTRUCT(BlueprintType)
struct FNarr_StoryData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarr_StoryType StoryType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString NarrativeText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TSoftObjectPtr<USoundCue> AudioCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    float CooldownTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bRequiresLineOfSight;

    FNarr_StoryData()
    {
        StoryType = ENarr_StoryType::CampGathering;
        NarrativeText = TEXT("A story unfolds here...");
        TriggerRadius = 500.0f;
        CooldownTime = 30.0f;
        bRequiresLineOfSight = false;
    }
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API ANarr_StorytellingManager : public AActor
{
    GENERATED_BODY()

public:
    ANarr_StorytellingManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* TriggerSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* AudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    FNarr_StoryData StoryData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    TArray<FNarr_StoryData> AdditionalStories;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float LastTriggerTime;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsPlaying;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bRandomizeStories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MinTimeBetweenStories;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void TriggerStory(APawn* TriggeringPawn);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void StopCurrentStory();

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    bool CanTriggerStory() const;

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    FNarr_StoryData GetRandomStory() const;

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void SetStoryData(const FNarr_StoryData& NewStoryData);

protected:
    UFUNCTION()
    void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UFUNCTION()
    void OnAudioFinished();

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    bool HasLineOfSight(APawn* TargetPawn) const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Storytelling")
    void OnStoryTriggered(const FNarr_StoryData& Story, APawn* TriggeringPawn);

    UFUNCTION(BlueprintImplementableEvent, Category = "Storytelling")
    void OnStoryCompleted(const FNarr_StoryData& Story);
};