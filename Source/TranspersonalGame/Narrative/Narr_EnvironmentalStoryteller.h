#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "SharedTypes.h"
#include "Narr_EnvironmentalStoryteller.generated.h"

UENUM(BlueprintType)
enum class ENarr_EnvironmentType : uint8
{
    River           UMETA(DisplayName = "River"),
    Cave            UMETA(DisplayName = "Cave"),
    BoneYard        UMETA(DisplayName = "Bone Yard"),
    HunterCamp      UMETA(DisplayName = "Hunter Camp"),
    DangerZone      UMETA(DisplayName = "Danger Zone"),
    SafeHaven       UMETA(DisplayName = "Safe Haven")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_EnvironmentalStory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Story")
    FString StoryID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Story")
    FString NarrationText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Story")
    ENarr_EnvironmentType EnvironmentType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Story")
    bool bIsOneTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Story")
    bool bHasBeenTriggered;

    FNarr_EnvironmentalStory()
    {
        StoryID = TEXT("");
        NarrationText = TEXT("");
        EnvironmentType = ENarr_EnvironmentType::River;
        bIsOneTime = true;
        bHasBeenTriggered = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_EnvironmentalStoryteller : public AActor
{
    GENERATED_BODY()

public:
    ANarr_EnvironmentalStoryteller();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* TriggerBox;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Story")
    FNarr_EnvironmentalStory StoryData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Story")
    float TriggerCooldown;

    UPROPERTY(BlueprintReadOnly, Category = "Environmental Story")
    float LastTriggerTime;

public:
    UFUNCTION(BlueprintCallable, Category = "Environmental Story")
    void TriggerEnvironmentalStory();

    UFUNCTION(BlueprintCallable, Category = "Environmental Story")
    bool CanTriggerStory() const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Environmental Story")
    void OnStoryTriggered();

protected:
    UFUNCTION()
    void OnTriggerBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
    void LoadEnvironmentalStories();
};