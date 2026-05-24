#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/TriggerVolume.h"
#include "Narr_EnvironmentalNarrative.generated.h"

UENUM(BlueprintType)
enum class ENarr_EnvironmentalType : uint8
{
    BoneDiscovery       UMETA(DisplayName = "Bone Discovery"),
    WaterSource         UMETA(DisplayName = "Water Source"),
    DangerZone          UMETA(DisplayName = "Danger Zone"),
    SafeHaven           UMETA(DisplayName = "Safe Haven"),
    TerritoryMarker     UMETA(DisplayName = "Territory Marker"),
    AncientRuins        UMETA(DisplayName = "Ancient Ruins"),
    BloodTrail          UMETA(DisplayName = "Blood Trail"),
    NestSite            UMETA(DisplayName = "Nest Site")
};

USTRUCT(BlueprintType)
struct FNarr_EnvironmentalData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Narrative")
    ENarr_EnvironmentalType EnvironmentType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Narrative")
    FText NarrativeText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Narrative")
    FText PlayerThought;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Narrative")
    USoundCue* AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Narrative")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Narrative")
    bool bOneTimeOnly;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Narrative")
    float FearImpact;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Narrative")
    float KnowledgeGain;

    FNarr_EnvironmentalData()
    {
        EnvironmentType = ENarr_EnvironmentalType::BoneDiscovery;
        NarrativeText = FText::FromString("You discover something significant...");
        PlayerThought = FText::FromString("What could this mean?");
        AmbientSound = nullptr;
        TriggerRadius = 500.0f;
        bOneTimeOnly = true;
        FearImpact = 0.0f;
        KnowledgeGain = 10.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_EnvironmentalNarrative : public AActor
{
    GENERATED_BODY()

public:
    ANarr_EnvironmentalNarrative();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* TriggerSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* VisualMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Narrative")
    FNarr_EnvironmentalData EnvironmentalData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Narrative")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Narrative")
    bool bHasBeenTriggered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Narrative")
    float NarrativeDisplayDuration;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Environmental Narrative")
    void TriggerEnvironmentalNarrative(AActor* TriggeringActor);

    UFUNCTION(BlueprintCallable, Category = "Environmental Narrative")
    void SetEnvironmentalData(const FNarr_EnvironmentalData& NewData);

    UFUNCTION(BlueprintCallable, Category = "Environmental Narrative")
    FNarr_EnvironmentalData GetEnvironmentalData() const;

    UFUNCTION(BlueprintCallable, Category = "Environmental Narrative")
    void ActivateNarrative();

    UFUNCTION(BlueprintCallable, Category = "Environmental Narrative")
    void DeactivateNarrative();

    UFUNCTION(BlueprintCallable, Category = "Environmental Narrative")
    bool IsNarrativeActive() const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Environmental Narrative")
    void OnEnvironmentalNarrativeTriggered(const FNarr_EnvironmentalData& Data);

    UFUNCTION(BlueprintImplementableEvent, Category = "Environmental Narrative")
    void OnPlayerDiscovery(ENarr_EnvironmentalType DiscoveryType, float KnowledgeGained);

protected:
    UFUNCTION()
    void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION(BlueprintCallable, Category = "Environmental Narrative")
    void PlayEnvironmentalSound();

    UFUNCTION(BlueprintCallable, Category = "Environmental Narrative")
    void UpdatePlayerStats(AActor* Player);

private:
    float CurrentDisplayTime;
    bool bIsDisplayingNarrative;
};