#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "Narr_EmergencyDialogueSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_EmergencyDialogue
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergency")
    FString ThreatType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergency")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergency")
    float UrgencyLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergency")
    bool bRequiresImmedateAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergency")
    FVector ThreatLocation;

    FNarr_EmergencyDialogue()
    {
        ThreatType = TEXT("Unknown");
        DialogueText = TEXT("");
        UrgencyLevel = 1.0f;
        bRequiresImmedateAction = false;
        ThreatLocation = FVector::ZeroVector;
    }
};

UENUM(BlueprintType)
enum class ENarr_EmergencyType : uint8
{
    None            UMETA(DisplayName = "None"),
    PredatorSighted UMETA(DisplayName = "Predator Sighted"),
    WeatherWarning  UMETA(DisplayName = "Weather Warning"),
    ResourceCritical UMETA(DisplayName = "Resource Critical"),
    InjuryAlert     UMETA(DisplayName = "Injury Alert"),
    EnvironmentalHazard UMETA(DisplayName = "Environmental Hazard")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_EmergencyDialogueSystem : public AActor
{
    GENERATED_BODY()

public:
    ANarr_EmergencyDialogueSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* AudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergency Dialogue")
    TArray<FNarr_EmergencyDialogue> EmergencyDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergency Dialogue")
    float DetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergency Dialogue")
    float CooldownTime;

    UPROPERTY(BlueprintReadOnly, Category = "Emergency Dialogue")
    float LastTriggerTime;

    UPROPERTY(BlueprintReadOnly, Category = "Emergency Dialogue")
    bool bIsActive;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Emergency Dialogue")
    void TriggerEmergencyDialogue(ENarr_EmergencyType EmergencyType, const FVector& ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "Emergency Dialogue")
    void PlayEmergencyAlert(const FString& AlertText);

    UFUNCTION(BlueprintCallable, Category = "Emergency Dialogue")
    bool CanTriggerAlert() const;

    UFUNCTION(BlueprintCallable, Category = "Emergency Dialogue")
    void SetEmergencyDialogue(const FString& ThreatType, const FString& DialogueText, float UrgencyLevel);

    UFUNCTION(BlueprintImplementableEvent, Category = "Emergency Dialogue")
    void OnEmergencyTriggered(ENarr_EmergencyType EmergencyType, const FString& DialogueText);

protected:
    UFUNCTION()
    void CheckForThreats();

    UFUNCTION()
    FString GetEmergencyDialogueForType(ENarr_EmergencyType EmergencyType);
};